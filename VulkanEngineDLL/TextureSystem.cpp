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
#include <imgui/backends/imgui_impl_vulkan.h>

TextureSystem& textureSystem = TextureSystem::Get();

Texture TextureSystem::CreateTexture(const String& texturePath)
{
	return CreateTexture(fileSystem.LoadJsonFile(texturePath.c_str()).get<TextureLoader>());
}

Texture TextureSystem::CreateTexture(TextureLoader textureLoader)
{
	if (TextureExists(textureLoader.TextureId))
	{
		return FindTexture(textureLoader.TextureId);
	}

	int width = 0;
	int height = 0;
	uint bitsPerChannel = 0;
	int textureChannels = 0;
	Vector<byte> textureData;
	for (size_t x = 0; x < textureLoader.TextureFilePath.size(); x++)
	{
		Vector<byte> layerData;
		String ext = fileSystem.GetFileExtention(textureLoader.TextureFilePath[x].c_str());
		if (ext == "png" && textureLoader.TextureFilePath.size() == 1)
		{
			uint uWidth = 0;
			uint uHeight = 0;
			layerData = fileSystem.LoadPNG(textureLoader.TextureFilePath[x], uWidth, uHeight, bitsPerChannel, textureChannels);
			width = static_cast<int>(uWidth);
			height = static_cast<int>(uHeight);
		}
		else
		{
			bitsPerChannel = 8;
			layerData = fileSystem.LoadImageFile(textureLoader.TextureFilePath[x], width, height, textureChannels);
		}
		textureData.insert(textureData.end(), layerData.begin(), layerData.end());
	}

	Texture texture = Texture
	{
		.textureGuid = textureLoader.TextureId,
		.textureIndex = TextureList.size(),
		.width = width,
		.height = height,
		.depth = 1,
		.mipMapLevels = textureLoader.UseMipMaps ? static_cast<uint32>(std::floor(std::log2(std::max(width, height)))) + 1 : 1,
		.textureType = textureLoader.IsSkyBox ? TextureType_SkyboxTexture : TextureType_ColorTexture,
		.textureByteFormat = textureLoader.TextureByteFormat,
		.textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.sampleCount = VK_SAMPLE_COUNT_1_BIT,
		.colorChannels = static_cast<ColorChannelUsed>(textureChannels)
	};

	VkImageCreateInfo imageCreateInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.flags = textureLoader.IsSkyBox ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : static_cast<VkImageViewCreateFlags>(0),
		.imageType = VK_IMAGE_TYPE_2D,
		.format = texture.textureByteFormat,
		.extent =
		{
			.width = static_cast<uint32>(texture.width),
			.height = static_cast<uint32>(texture.height),
			.depth = 1,
		},
		.mipLevels = texture.mipMapLevels,
		.arrayLayers = textureLoader.IsSkyBox ? static_cast<uint>(6) : static_cast<uint>(1),
		.samples = texture.sampleCount,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = texture.textureImageLayout
	};
	if (texture.mipMapLevels > 1) imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	CreateTextureImage(texture, imageCreateInfo, textureData, textureLoader.TextureFilePath.size());
	CreateTextureView(texture, false, textureLoader.ImageType);
	VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &textureLoader.SamplerCreateInfo, NULL, &texture.textureSampler));

	if (textureLoader.IsSkyBox) CubeMap = texture;
	else TextureList.emplace_back(texture);

	//#ifndef NDEBUG
	//	std::cout << "[TextureDebug] Created Texture:" << texturePath
	//		<< " Texture ID: " << texture.textureId.ToString()
	//		<< " Image: " << texture.textureImage
	//		<< " Format: " << texture.textureByteFormat
	//		<< " InitialLayout: " << texture.textureImageLayout << std::endl;
	//#endif

	return texture;
}

//VkGuid TextureSystem::CreateTexture(Pixel clearColorPixel, ivec2 textureResolution, VkFormat textureFormat, ColorChannelUsed colorChannels)
//{
//	Texture texture = Texture
//	{
//		.textureGuid = VkGuid(),
//		.textureIndex = TextureList.size(),
//		.width = textureResolution.x,
//		.height = textureResolution.y,
//		.depth = 1,
//		.mipMapLevels = 1,
//		.textureType = TextureType_ColorTexture,
//		.textureByteFormat = textureFormat,
//		.textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
//		.sampleCount = VK_SAMPLE_COUNT_1_BIT,
//		.colorChannels = colorChannels
//	};
//
//	VkImageCreateInfo imageCreateInfo =
//	{
//		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
//		.flags = 0,
//		.imageType = VK_IMAGE_TYPE_2D,
//		.format = texture.textureByteFormat,
//		.extent =
//		{
//			.width = static_cast<uint32>(textureResolution.x),
//			.height = static_cast<uint32>(textureResolution.y),
//			.depth = 1,
//		},
//		.mipLevels = texture.mipMapLevels,
//		.arrayLayers = 1,
//		.samples = texture.sampleCount,
//		.tiling = VK_IMAGE_TILING_LINEAR,
//		.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
//		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
//		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
//	};
//
//	Vector<byte> byteData;
//	for (size_t x = 0; x < textureResolution.x * textureResolution.y; x++)
//	{
//		Vector<byte> pixelData = { clearColorPixel.Red, clearColorPixel.Green, clearColorPixel.Blue, clearColorPixel.Alpha };
//		byteData.insert(byteData.end(), pixelData.begin(), pixelData.end());
//	}
//	CreateTextureImage(texture, imageCreateInfo, byteData, byteData.size());
//	CreateTextureView(texture, VK_IMAGE_ASPECT_COLOR_BIT);
//
//	return textureLoader.TextureId;
//}

Texture TextureSystem::CreateRenderPassTexture(VulkanRenderPass& vulkanRenderPass, uint attachmentId)
{
	const RenderPassAttachmentTexture renderPassAttachmentTexture = renderSystem.RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId][attachmentId];

	bool isDepthFormat = (renderPassAttachmentTexture.Format >= VK_FORMAT_D16_UNORM && renderPassAttachmentTexture.Format <= VK_FORMAT_D32_SFLOAT_S8_UINT) || (renderPassAttachmentTexture.Format == VK_FORMAT_X8_D24_UNORM_PACK32);
	bool hasStencil = (renderPassAttachmentTexture.Format == VK_FORMAT_D32_SFLOAT_S8_UINT || renderPassAttachmentTexture.Format == VK_FORMAT_D24_UNORM_S8_UINT);

	// Base usage flags required for all render pass textures
	VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT |
		VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	Texture texture = {
		.textureGuid = vulkanRenderPass.RenderPassId,
		.width = vulkanRenderPass.RenderPassResolution.x,
		.height = vulkanRenderPass.RenderPassResolution.y,
		.depth = 1,
		.mipMapLevels = renderPassAttachmentTexture.UseMipMaps ? renderPassAttachmentTexture.MipMapCount : 1,
		.textureByteFormat = renderPassAttachmentTexture.Format,
		.sampleCount = vulkanRenderPass.SampleCount,
	};

	switch (renderPassAttachmentTexture.RenderTextureType)
	{
		case RenderType_DepthBufferTexture:     texture.textureType = TextureType_DepthTexture; break;
		case RenderType_GBufferTexture:         texture.textureType = TextureType_ColorTexture; break;
		case RenderType_IrradianceTexture:      texture.textureType = TextureType_IrradianceMapTexture; break;
		case RenderType_PrefilterTexture:       texture.textureType = TextureType_PrefilterMapTexture; break;
		case RenderType_OffscreenColorTexture:  texture.textureType = TextureType_ColorTexture; break;
		case RenderType_SwapChainTexture:       texture.textureType = TextureType_ColorTexture; break;
	}

	if (isDepthFormat)
	{
		usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		texture.textureImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		texture.colorChannels = ColorChannelUsed::ChannelR;
	}
	else
	{
		usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		texture.textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		texture.colorChannels = ColorChannelUsed::ChannelRGBA;
	}

	if (texture.mipMapLevels > 1)
	{
		usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	VkImageCreateInfo imageInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.flags = renderPassAttachmentTexture.IsCubeMapAttachment ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = texture.textureByteFormat,
		.extent = { static_cast<uint32_t>(texture.width), static_cast<uint32_t>(texture.height), 1 },
		.mipLevels = texture.mipMapLevels,
		.arrayLayers = renderPassAttachmentTexture.IsCubeMapAttachment ? 6u : 1u,
		.samples = texture.sampleCount,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VmaAllocationCreateInfo allocInfo = {
		.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
	};

	VmaAllocation allocation = VK_NULL_HANDLE;
	VULKAN_THROW_IF_FAIL(vmaCreateImage(bufferSystem.vmaAllocator, &imageInfo, &allocInfo, &texture.textureImage, &allocation, nullptr));
	texture.TextureAllocation = allocation;

	VkImageAspectFlags aspectMask;
	if (isDepthFormat)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (hasStencil)
		{
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	CreateTextureView(texture, vulkanRenderPass.UseCubeMapMultiView, aspectMask);
	if (renderPassAttachmentTexture.UseSampler)
	{
		VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &renderPassAttachmentTexture.SamplerCreateInfo, nullptr, &texture.textureSampler));
	}
	else
	{
		VkSamplerCreateInfo samplerInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.minLod = 0.0f,
			.maxLod = static_cast<float>(texture.mipMapLevels)
		};
		VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &samplerInfo, nullptr, &texture.textureSampler));
	}
	return texture;
}

void TextureSystem::GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList)
{
	textureDescriptorList.emplace_back(VkDescriptorImageInfo
		{
			.sampler = texture.textureSampler,
			.imageView = texture.textureViewList.front(),
			.imageLayout = texture.textureImageLayout
		});
}

Texture TextureSystem::FindTexture(const VkGuid& textureId)
{
	for (auto& pair : RenderedTextureListMap)
	{
		for (auto& texture : pair.second)
		{
			if (texture.textureGuid == textureId)
			{
				return texture;
			}
		}
	}

	for (auto& pair : DepthTextureMap)
	{
		if (pair.second.textureGuid == textureId)
		{
			return pair.second;
		}
	}

	for (auto& texture : TextureList)
	{
		if (texture.textureGuid == textureId)
		{
			return texture;
		}
	}
	throw std::out_of_range("Texture with given ID not found");
}

void TextureSystem::AddRenderedTexture(RenderPassGuid& renderPassGuid, Vector<Texture>& renderedTextureList)
{
	RenderedTextureListMap[renderPassGuid] = renderedTextureList;
}

void TextureSystem::AddDepthTexture(RenderPassGuid& renderPassGuid, Texture& depthTexture)
{
	DepthTextureMap[renderPassGuid] = depthTexture;
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

void TextureSystem::CreateTextureImage(Texture& texture, VkImageCreateInfo& imageCreateInfo, Vector<byte>& textureData, uint layerCount)
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
	if (textureData.size() > 0)
	{
		VkBufferCreateInfo stagingBufferInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = textureData.size(),
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

		memcpy(mapped, textureData.data(), textureData.size());
		vmaFlushAllocation(bufferSystem.vmaAllocator, stagingAllocation, 0, textureData.size());
		if (!stagingAllocOut.pMappedData)
		{
			vmaUnmapMemory(bufferSystem.vmaAllocator, stagingAllocation);
		}

		std::vector<VkBufferImageCopy> copyRegions;
		copyRegions.reserve(imageCreateInfo.arrayLayers);
		TransitionImageLayout(texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		for (uint32 layer = 0; layer < imageCreateInfo.arrayLayers; ++layer)
		{
			VkBufferImageCopy copyRegion
			{
				.bufferOffset = layer * (textureData.size() / layerCount),
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource
					{
						.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
						.mipLevel = 0,
						.baseArrayLayer = layer,
						.layerCount = 1
					},
				.imageOffset = { 0, 0, 0 },
				.imageExtent = {
					static_cast<uint32_t>(texture.width),
					static_cast<uint32_t>(texture.height),
					1
				}
			};
			copyRegions.push_back(copyRegion);
		}
		VkCommandBuffer commandBuffer = vulkanSystem.BeginSingleUseCommand();
		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, texture.textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
		vulkanSystem.EndSingleUseCommand(commandBuffer);

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

void TextureSystem::CreateTextureView(Texture& texture, bool usingMultiView, VkImageAspectFlags imageAspectFlags)
{
	VkImageAspectFlags aspect = imageAspectFlags;
	if (aspect == 0)
	{
		std::cout << "CreateTextureView: imageAspectFlags not set ? using auto-detect." << std::endl;

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

	for (uint32_t mip = 0; mip < texture.mipMapLevels; ++mip)
	{
		VkImageView imageView = VK_NULL_HANDLE;
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = texture.textureImage;

		if (texture.textureType == TextureTypeEnum::TextureType_SkyboxTexture ||
			texture.textureType == TextureTypeEnum::TextureType_IrradianceMapTexture ||
			texture.textureType == TextureTypeEnum::TextureType_PrefilterMapTexture)
		{
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		}
		else
		{
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		}

		viewInfo.format = texture.textureByteFormat;
		viewInfo.subresourceRange.aspectMask = aspect;
		viewInfo.subresourceRange.baseMipLevel = mip;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = usingMultiView ? 6u : 1u;

		VULKAN_THROW_IF_FAIL(vkCreateImageView(vulkanSystem.Device, &viewInfo, nullptr, &imageView));
		texture.textureViewList.emplace_back(imageView);
	}
}

//Texture TextureSystem::FindTexture(const TextureGuid& textureGuid)
//{
//	auto it = std::find_if(TextureList.begin(), TextureList.end(),
//		[&textureGuid](const Texture& texture)
//		{
//			return texture.textureGuid == textureGuid;
//		});
//	return *it;
//}

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
				return texture.textureGuid == textureGuid;
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

const bool TextureSystem::DepthTextureExists(const RenderPassGuid& renderPassGuid) const
{
	return DepthTextureMap.contains(renderPassGuid);
}

const bool TextureSystem::TextureExists(const TextureGuid& textureGuid) const
{
	auto it = std::find_if(TextureList.begin(), TextureList.end(),
		[&textureGuid](const Texture& texture)
		{
			return texture.textureGuid == textureGuid;
		});
	return it != TextureList.end();
}

const bool TextureSystem::RenderedTextureExists(const RenderPassGuid& renderPassGuid, const TextureGuid& textureGuid) const
{
	auto it = RenderedTextureListMap.find(renderPassGuid);
	if (it != RenderedTextureListMap.end())
	{
		return std::any_of(it->second.begin(), it->second.end(),
			[&textureGuid](const Texture& texture) { return texture.textureGuid == textureGuid; });
	}
	return RenderedTextureListMap.contains(textureGuid);
}

const bool TextureSystem::RenderedTextureListExists(const RenderPassGuid& renderPassGuid) const
{
	return RenderedTextureListMap.find(renderPassGuid) != RenderedTextureListMap.end();
}

void TextureSystem::DestroyTexture(Texture& texture)
{
	if (texture.textureSampler) vkDestroySampler(vulkanSystem.Device, texture.textureSampler, nullptr);
	if (texture.textureViewList.front()) vkDestroyImageView(vulkanSystem.Device, texture.textureViewList.front(), nullptr);
	if (texture.textureImage && texture.TextureAllocation)
	{
		vmaDestroyImage(bufferSystem.vmaAllocator, texture.textureImage, texture.TextureAllocation);
	}
}

void TextureSystem::DestroyAllTextures()
{
	for (auto& texture : TextureList)
	{
		DestroyTexture(texture);
	}
	TextureList.clear();

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
		std::cout << "[TextureSystem WARNING] Format " << texture.textureByteFormat << " does not support linear blitting ? mipmaps will be poor quality" << std::endl;
	}

	VkCommandBuffer commandBuffer = vulkanSystem.BeginSingleUseCommand();
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
	vulkanSystem.EndSingleUseCommand(commandBuffer);
}

void TextureSystem::TransitionImageLayout(Texture& texture, VkImageLayout newLayout, uint32 baseMipLevel, uint32 levelCount)
{
	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if (texture.textureImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL || texture.textureImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

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
			.aspectMask = aspectMask,
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

	VkCommandBuffer commandBuffer = vulkanSystem.BeginSingleUseCommand();
	vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	vulkanSystem.EndSingleUseCommand(commandBuffer);

	texture.textureImageLayout = newLayout;
}

void TextureSystem::TransitionImageLayout(const VkCommandBuffer& commandBuffer, Texture& texture, VkImageLayout newLayout, uint32 baseMipLevel, uint32 levelCount)
{
	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if (texture.textureImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL || texture.textureImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

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
			.aspectMask = aspectMask,
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

