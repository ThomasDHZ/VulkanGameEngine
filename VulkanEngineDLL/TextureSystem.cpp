#include "TextureSystem.h"
#include "RenderSystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include "JsonStruct.h"
#include "from_json.h"
#include <algorithm>
#include <cmath>
#include <stb/stb_image.h> 
#include <stb/stb_image_write.h>
#include "JsonStruct.h"

TextureSystem& textureSystem = TextureSystem::Get();

VkGuid TextureSystem::CreateTexture(const String& texturePath)
{
	TextureLoader textureLoader = fileSystem.LoadJsonFile(texturePath.c_str());
	if (TextureExists(textureLoader.TextureId))
	{
		return textureLoader.TextureId;
	}

	int width = 0;
	int height = 0;
	int colorChannels = 0;
    Vector<byte> textureData = fileSystem.LoadImageFile(textureLoader.TextureFilePath.c_str(), width, height, colorChannels);

	VkFormat detectedFormat = VK_FORMAT_UNDEFINED;
	switch (colorChannels)
	{
		case 1: detectedFormat = VK_FORMAT_R8_UNORM; break;
		case 2: detectedFormat = VK_FORMAT_R8G8_UNORM; break;
		case 3: detectedFormat = textureLoader.UsingSRGBFormat ? VK_FORMAT_R8G8B8_SRGB : VK_FORMAT_R8G8B8_UNORM; break;
		case 4: detectedFormat = textureLoader.UsingSRGBFormat ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM; break;
		default:
		{
			std::cout << "[TextureSystem WARNING] Unsupported channel count: " << colorChannels << " for " << textureLoader.TextureFilePath << std::endl;
			detectedFormat = textureLoader.UsingSRGBFormat ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
			break;
		}
	}

	VkFormat finalFormat = detectedFormat;
	if (textureLoader.TextureByteFormat != VK_FORMAT_UNDEFINED)
	{
		finalFormat = textureLoader.TextureByteFormat;
	}

	Texture texture = Texture
	{
		.textureId = textureLoader.TextureId,
		.width = width,
		.height = height,
		.depth = 1,
		.mipMapLevels = textureLoader.UseMipMaps ? static_cast<uint32>(std::floor(std::log2(std::max(width, height)))) + 1 : 1,
		.textureByteFormat = finalFormat,
		.textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.sampleCount = VK_SAMPLE_COUNT_1_BIT,
		.colorChannels = static_cast<ColorChannelUsed>(colorChannels)
	};

	VkImageCreateInfo imageCreateInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = texture.textureByteFormat,
		.extent =
		{
			.width = static_cast<uint32>(texture.width),
			.height = static_cast<uint32>(texture.height),
			.depth = 1,
		},
		.mipLevels = texture.mipMapLevels,
		.arrayLayers = 1,
		.samples = texture.sampleCount,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = texture.textureImageLayout
	};
	if (texture.mipMapLevels > 1) imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	CreateTextureImage(texture, imageCreateInfo, textureData.data(), textureData.size());
	CreateTextureView(texture, textureLoader.ImageType);
	VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &textureLoader.SamplerCreateInfo, NULL, &texture.textureSampler));

	TextureMap[textureLoader.TextureId] = texture;

#ifndef NDEBUG
	std::cout << "[TextureDebug] Created Texture:" << texturePath
		<< " Texture ID: " << texture.textureId.ToString()
		<< " Image: " << texture.textureImage
		<< " Format: " << texture.textureByteFormat
		<< " InitialLayout: " << texture.textureImageLayout << std::endl;
#endif

	return textureLoader.TextureId;
}


Texture TextureSystem::CreateRenderPassTexture(const RenderAttachmentLoader& renderAttachmentLoader, ivec2 renderAttachmentResolution)
{

	VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	bool hasStencil = (renderAttachmentLoader.Format == VK_FORMAT_D32_SFLOAT_S8_UINT || renderAttachmentLoader.Format == VK_FORMAT_D24_UNORM_S8_UINT);
	bool isDepthFormat = (renderAttachmentLoader.Format >= VK_FORMAT_D16_UNORM && renderAttachmentLoader.Format <= VK_FORMAT_D32_SFLOAT_S8_UINT) || (renderAttachmentLoader.Format == VK_FORMAT_X8_D24_UNORM_PACK32);
	Texture texture =
	{
		.textureId = renderAttachmentLoader.RenderedTextureId,
		.width = renderAttachmentResolution.x,
		.height = renderAttachmentResolution.y,
		.depth = 1,
		.mipMapLevels = renderAttachmentLoader.UseMipMaps ? renderAttachmentLoader.MipMapCount : 1,
		.textureByteFormat = renderAttachmentLoader.Format,
		.textureImageLayout = renderAttachmentLoader.FinalLayout,
		.sampleCount = renderAttachmentLoader.SampleCount 
	};

	if (isDepthFormat)
	{
		usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		texture.textureImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		texture.colorChannels = ColorChannelUsed::ChannelR;
	}
	else
	{
		usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		texture.colorChannels = ColorChannelUsed::ChannelRGBA;
	}
	if (texture.mipMapLevels > 1)
	{
		usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	VkImageCreateInfo imageInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = texture.textureByteFormat,
		.extent = { static_cast<uint32>(texture.width), static_cast<uint32>(texture.height), 1 },
		.mipLevels = texture.mipMapLevels,
		.arrayLayers = 1,
		.samples = texture.sampleCount,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = texture.textureImageLayout,
	};

	VmaAllocationCreateInfo allocInfo =
	{
		.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
	};

	VmaAllocation allocation = VK_NULL_HANDLE;
	VULKAN_THROW_IF_FAIL(vmaCreateImage(bufferSystem.vmaAllocator, &imageInfo, &allocInfo, &texture.textureImage, &allocation, nullptr));
	texture.TextureAllocation = allocation;

	VkImageViewCreateInfo viewInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = texture.textureImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = texture.textureByteFormat,
		.subresourceRange =
		{
			.baseMipLevel = 0,
			.levelCount = texture.mipMapLevels,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	if (isDepthFormat)
	{
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (hasStencil)
		{
			viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VULKAN_THROW_IF_FAIL(vkCreateImageView(vulkanSystem.Device, &viewInfo, nullptr, &texture.textureView));
	if (renderAttachmentLoader.UseSampler && (usage & VK_IMAGE_USAGE_SAMPLED_BIT) && !isDepthFormat)
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(texture.mipMapLevels);

		VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &samplerInfo, nullptr, &texture.textureSampler));
	}

#ifndef NDEBUG
	std::cout << "[TextureDebug] Created texture ID: " << texture.textureId.ToString()
		<< " Image: " << texture.textureImage
		<< " Format: " << texture.textureByteFormat
		<< " InitialLayout: " << texture.textureImageLayout << std::endl;
#endif
	return texture;
}

void TextureSystem::Update(const float& deltaTime)
{
	int x = 0;
	for (auto& [id, texture] : TextureMap)
	{
		UpdateTextureBufferIndex(texture, x);
		x++;
	}
}

void TextureSystem::GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList)
{
	textureDescriptorList.emplace_back(VkDescriptorImageInfo
		{
			.sampler = texture.textureSampler,
			.imageView = texture.textureView,
			.imageLayout = texture.textureImageLayout
		});
}

void TextureSystem::TransitionImageLayout(const VkCommandBuffer& commandBuffer, Texture& texture, VkImageLayout newLayout, uint32 baseMipLevel, uint32 levelCount)
{
	VkImageMemoryBarrier barrier =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = texture.textureImageLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = texture.textureImage,
		.subresourceRange =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = baseMipLevel,
			.levelCount = levelCount,
			.baseArrayLayer = 0,
			.layerCount = VK_REMAINING_ARRAY_LAYERS
		}
	};

	VkAccessFlags srcAccess = 0;
	VkAccessFlags dstAccess = 0;
	VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		srcAccess = 0;
		dstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
		dstAccess = VK_ACCESS_SHADER_READ_BIT;
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
		dstAccess = VK_ACCESS_TRANSFER_READ_BIT;
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		srcAccess = VK_ACCESS_TRANSFER_READ_BIT;
		dstAccess = VK_ACCESS_SHADER_READ_BIT;
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;

	vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	texture.textureImageLayout = newLayout;
}

void TextureSystem::TransitionImageLayout(Texture& texture, VkImageLayout newLayout, uint32 baseMipLevel, uint32 levelCount)
{
	VkImageMemoryBarrier barrier =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = texture.textureImageLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = texture.textureImage,
		.subresourceRange =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = baseMipLevel,
			.levelCount = levelCount,
			.baseArrayLayer = 0, 
			.layerCount = VK_REMAINING_ARRAY_LAYERS
		}
	};

	VkAccessFlags srcAccess = 0;
	VkAccessFlags dstAccess = 0;
	VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		srcAccess = 0;
		dstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
		dstAccess = VK_ACCESS_SHADER_READ_BIT;
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
		dstAccess = VK_ACCESS_TRANSFER_READ_BIT;
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
			newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		srcAccess = VK_ACCESS_TRANSFER_READ_BIT;
		dstAccess = VK_ACCESS_SHADER_READ_BIT;
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;

	VkCommandBuffer commandBuffer = renderSystem.BeginSingleUseCommand();
	vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	renderSystem.EndSingleUseCommand(commandBuffer);

	texture.textureImageLayout = newLayout;
}

void TextureSystem::DestroyTexture(Texture& texture)
{
	if (texture.textureSampler) vkDestroySampler(vulkanSystem.Device, texture.textureSampler, nullptr);
	if (texture.textureView) vkDestroyImageView(vulkanSystem.Device, texture.textureView, nullptr);
	if (texture.textureImage && texture.TextureAllocation)
	{
		vmaDestroyImage(bufferSystem.vmaAllocator, texture.textureImage, texture.TextureAllocation);
	}
}

void TextureSystem::AddRenderedTexture(RenderPassGuid& renderPassGuid, Vector<Texture>& renderedTextureList)
{
	RenderedTextureListMap[renderPassGuid] = renderedTextureList;
}

void TextureSystem::AddDepthTexture(RenderPassGuid& renderPassGuid, Texture& depthTexture)
{
	DepthTextureMap[renderPassGuid] = depthTexture;
}

void TextureSystem::DestroyAllTextures()
{
	for (auto& pair : TextureMap)
	{
		DestroyTexture(pair.second);
	}
	TextureMap.clear();

	for (auto& pair : DepthTextureMap)
	{
		DestroyTexture(pair.second);
	}
	DepthTextureMap.clear();

	for (auto& list : RenderedTextureListMap)
	{
		for (auto& texture : list.second)
		{
			DestroyTexture(texture);
		}
	}
	RenderedTextureListMap.clear();
}

const Vector<Texture> TextureSystem::TextureList()
{
	Vector<Texture> list;
	list.reserve(TextureMap.size());
	for (const auto& pair : TextureMap)
	{
		list.emplace_back(pair.second);
	}
	return list;
}

const Vector<Texture> TextureSystem::DepthTextureList()
{
	Vector<Texture> list;
	list.reserve(DepthTextureMap.size());
	for (const auto& pair : DepthTextureMap)
	{
		list.emplace_back(pair.second);
	}
	return list;
}

void TextureSystem::UpdateTextureBufferIndex(Texture& texture, uint32 bufferIndex)
{
	texture.textureBufferIndex = bufferIndex;
}

void TextureSystem::CreateTextureImage(Texture& texture, VkImageCreateInfo& imageCreateInfo, byte* textureData, VkDeviceSize textureSize)
{
	VmaAllocationCreateInfo allocInfo =
	{
		.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
	};
	if (imageCreateInfo.extent.width * imageCreateInfo.extent.height > 1024 * 1024)
	{
		allocInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	}

	VmaAllocationInfo allocOut = {};
	VmaAllocation allocation = VK_NULL_HANDLE;
	VULKAN_THROW_IF_FAIL(vmaCreateImage(bufferSystem.vmaAllocator, &imageCreateInfo, &allocInfo, &texture.textureImage, &allocation, &allocOut));
	
	texture.TextureAllocation = allocation;
	if (textureData && textureSize > 0)
	{
		VkBufferCreateInfo stagingBufferInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = textureSize,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

		VmaAllocationCreateInfo stagingAllocInfo =
		{
			.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
			.usage = VMA_MEMORY_USAGE_AUTO
		};

		VmaAllocationInfo stagingAllocOut;
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingAllocation = VK_NULL_HANDLE;
		VULKAN_THROW_IF_FAIL(vmaCreateBuffer(bufferSystem.vmaAllocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, &stagingAllocOut));

		void* mapped = stagingAllocOut.pMappedData;
		if (!mapped)
		{
			VULKAN_THROW_IF_FAIL(vmaMapMemory(bufferSystem.vmaAllocator, stagingAllocation, &mapped));
		}

		memcpy(mapped, textureData, textureSize);
		vmaFlushAllocation(bufferSystem.vmaAllocator, stagingAllocation, 0, textureSize);
		if (!stagingAllocOut.pMappedData)
		{
			vmaUnmapMemory(bufferSystem.vmaAllocator, stagingAllocation);
		}

		TransitionImageLayout(texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		VkBufferImageCopy copyRegion =
		{
			.imageSubresource = 
				{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
			.imageExtent = imageCreateInfo.extent,
		};
		VkCommandBuffer commandBuffer = renderSystem.BeginSingleUseCommand();
		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, texture.textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
		renderSystem.EndSingleUseCommand(commandBuffer);

		if (texture.mipMapLevels > 1)
		{
			TransitionImageLayout(texture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 0, 1);
		}
		else
		{
			TransitionImageLayout(texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		vmaDestroyBuffer(bufferSystem.vmaAllocator, stagingBuffer, stagingAllocation);
		if (texture.mipMapLevels > 1)
		{
			GenerateMipmaps(texture);
		}
		texture.textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
}

void TextureSystem::CreateTextureView(Texture& texture, VkImageAspectFlags imageAspectFlags)
{
	VkImageAspectFlags aspect = imageAspectFlags;

	if (aspect == 0)
	{
		std::cout << "CreateTextureView: imageAspectFlags not set — using auto-detect." << std::endl;

		bool isDepthFormat = (texture.textureByteFormat >= VK_FORMAT_D16_UNORM &&
			texture.textureByteFormat <= VK_FORMAT_D32_SFLOAT_S8_UINT) ||
			(texture.textureByteFormat == VK_FORMAT_X8_D24_UNORM_PACK32);

		if (isDepthFormat)
		{
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (texture.textureByteFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
				texture.textureByteFormat == VK_FORMAT_D24_UNORM_S8_UINT)
			{
				aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}

	VkImageViewCreateInfo viewInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = texture.textureImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = texture.textureByteFormat,
		.subresourceRange =
		{
			.aspectMask = aspect,
			.baseMipLevel = 0,
			.levelCount = texture.mipMapLevels,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}
	};

	VULKAN_THROW_IF_FAIL(vkCreateImageView(vulkanSystem.Device, &viewInfo, nullptr, &texture.textureView));
}

void TextureSystem::GenerateMipmaps(Texture& texture)
{
	if (texture.mipMapLevels == 1)
	{
		return;
	}

	int32 mipWidth = texture.width;
	int32 mipHeight = texture.height;

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(vulkanSystem.PhysicalDevice, texture.textureByteFormat, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		std::cout << "[TextureSystem WARNING] Format " << texture.textureByteFormat << " does not support linear blitting — mipmaps will be poor quality" << std::endl;
	}

	VkCommandBuffer commandBuffer = renderSystem.BeginSingleUseCommand();
	VkImageMemoryBarrier ImageMemoryBarrier =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = texture.textureImage,
		.subresourceRange = VkImageSubresourceRange
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}
	};
	for (uint32 x = 1; x < texture.mipMapLevels; x++)
	{
		ImageMemoryBarrier.subresourceRange.baseMipLevel = x - 1;
		ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &ImageMemoryBarrier);

		VkImageBlit ImageBlit =
		{
			.srcSubresource = VkImageSubresourceLayers
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = x - 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.srcOffsets =
			{
				VkOffset3D
				{
					.x = 0,
					.y = 0,
					.z = 0,
				},
				VkOffset3D
				{
					.x = mipWidth,
					.y = mipHeight,
					.z = 1,
				}
			},
			.dstSubresource = VkImageSubresourceLayers
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = x,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.dstOffsets =
			{
				VkOffset3D
				{
					.x = 0,
					.y = 0,
					.z = 0,
				},
				VkOffset3D
				{
					.x = static_cast<int32_t>(mipWidth > 1 ? mipWidth / 2 : 1),
					.y = static_cast<int32_t>(mipHeight > 1 ? mipHeight / 2 : 1),
					.z = static_cast<int32_t>(1),
				}
			}
		};
		vkCmdBlitImage(commandBuffer, texture.textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture.textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &ImageBlit, VK_FILTER_LINEAR);

		ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &ImageMemoryBarrier);

		if (mipWidth > 1)
		{
			mipWidth /= 2;
		}
		if (mipHeight > 1)
		{
			mipHeight /= 2;
		};
	}

	ImageMemoryBarrier.subresourceRange.baseMipLevel = texture.mipMapLevels - 1;
	ImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	ImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &ImageMemoryBarrier);
	renderSystem.EndSingleUseCommand(commandBuffer);
}

Texture TextureSystem::FindTexture(const RenderPassGuid& renderPassGuid)
{
	return TextureMap.at(renderPassGuid);
}

Texture& TextureSystem::FindDepthTexture(const RenderPassGuid& renderPassGuid)
{
	return DepthTextureMap.at(renderPassGuid);
}

Texture& TextureSystem::FindRenderedTexture(const TextureGuid& textureGuid)
{
	for (auto& pair : RenderedTextureListMap)
	{
		auto& textureList = pair.second;
		auto it = std::find_if(textureList.begin(), textureList.end(),
			[&textureGuid](const Texture& texture)
			{
				return texture.textureId == textureGuid;
			});
		if (it != textureList.end())
			return *it;
	}
	throw std::out_of_range("Texture with given ID not found");
}

Vector<Texture>& TextureSystem::FindRenderedTextureList(const RenderPassGuid& renderPassGuid)
{
	return RenderedTextureListMap.at(renderPassGuid);
}

const bool TextureSystem::TextureExists(const RenderPassGuid& renderPassGuid) const
{
	return TextureMap.contains(renderPassGuid);
}

const bool TextureSystem::DepthTextureExists(const RenderPassGuid& renderPassGuid) const
{
	return DepthTextureMap.contains(renderPassGuid);
}

const bool TextureSystem::RenderedTextureExists(const RenderPassGuid& renderPassGuid, const TextureGuid& textureGuid) const
{
	auto it = RenderedTextureListMap.find(renderPassGuid);
	if (it != RenderedTextureListMap.end())
	{
		return std::any_of(it->second.begin(), it->second.end(),
			[&textureGuid](const Texture& texture) { return texture.textureId == textureGuid; });
	}
	return RenderedTextureListMap.contains(textureGuid);
}

const bool TextureSystem::RenderedTextureListExists(const RenderPassGuid& renderPassGuid) const
{
	return RenderedTextureListMap.find(renderPassGuid) != RenderedTextureListMap.end();
}