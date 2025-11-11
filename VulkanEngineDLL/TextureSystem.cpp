#include "TextureSystem.h"
#include "RenderSystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include "GPUSystem.h"
#include "JsonStruct.h"
#include "from_json.h"
#include <algorithm>
#include <cmath>

TextureSystem textureSystem = TextureSystem();

TextureSystem::TextureSystem()
{

}

TextureSystem::~TextureSystem()
{

}

VkGuid TextureSystem::CreateTexture(const String& texturePath)
{
	int width = 0;
	int height = 0;
	ColorChannelUsed colorChannels;

	TextureLoader textureLoader = fileSystem.LoadJsonFile(texturePath);
	if (TextureExists(textureLoader.TextureId))
	{
		return textureLoader.TextureId;
	}

	byte* data = stbi_load(textureLoader.TextureFilePath.c_str(), &width, &height, (int*)&colorChannels, 0);
	VkDeviceSize bufferSize = width * height * colorChannels;

	Texture texture = Texture
	{
		.textureId = textureLoader.TextureId,
		.width = width,
		.height = height,
		.depth = 1,
		.mipMapLevels = textureLoader.UseMipMaps ? static_cast<uint32>(std::floor(std::log2(std::max(texture.width, texture.height)))) + 1 : 1,
		.textureBufferIndex = 0,
		.textureImage = VK_NULL_HANDLE,
		.textureMemory = VK_NULL_HANDLE,
		.textureView = VK_NULL_HANDLE,
		.textureSampler = VK_NULL_HANDLE,
		.ImGuiDescriptorSet = VK_NULL_HANDLE,
		.textureUsage = kUse_2DImageTexture,
		.textureType = kType_UndefinedTexture,
		.textureByteFormat = textureLoader.ImageCreateInfo.format,
		.textureImageLayout = textureLoader.ImageCreateInfo.initialLayout,
		.sampleCount = textureLoader.ImageCreateInfo.samples >= gpuSystem.MaxSampleCount ? gpuSystem.MaxSampleCount : textureLoader.ImageCreateInfo.samples,
		.colorChannels = colorChannels,
	};

	textureLoader.ImageCreateInfo.extent.width = width;
	textureLoader.ImageCreateInfo.extent.height = height;
	textureLoader.ImageCreateInfo.extent.depth = 1;
	VULKAN_RESULT(CreateTextureImage(texture, textureLoader.ImageCreateInfo, data, bufferSize));
	VULKAN_RESULT(CreateTextureView(texture, textureLoader.ImageType));
	VULKAN_RESULT(vkCreateSampler(renderer.Device, &textureLoader.SamplerCreateInfo, NULL, &texture.textureSampler));
	VULKAN_RESULT(GenerateMipmaps(texture));
	stbi_image_free(data);

	TextureMap[textureLoader.TextureId] = texture;
	return textureLoader.TextureId;
}

Texture  TextureSystem::CreateTexture(VkGuid& textureId, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps)
{
	Texture texture = Texture
	{
		.textureId = textureId,
		.width = static_cast<int>(createImageInfo.extent.width),
		.height = static_cast<int>(createImageInfo.extent.height),
		.depth = (static_cast<int>(createImageInfo.extent.depth) < 1) ? 1 : static_cast<int>(createImageInfo.extent.depth),
		.mipMapLevels = useMipMaps ? static_cast<uint32>(std::floor(std::log2(std::max(texture.width, texture.height)))) + 1 : 1,
		.textureBufferIndex = 0,
		.textureImage = texture.textureImage,
		.textureMemory = texture.textureMemory,
		.textureView = texture.textureView,
		.textureSampler = texture.textureSampler,
		.ImGuiDescriptorSet = texture.ImGuiDescriptorSet,
		.textureUsage = kUse_2DImageTexture,
		.textureType = kType_UndefinedTexture,
		.textureByteFormat = createImageInfo.format,
		.textureImageLayout = createImageInfo.initialLayout,
		.sampleCount = createImageInfo.samples >= gpuSystem.MaxSampleCount ? gpuSystem.MaxSampleCount : createImageInfo.samples,
		.colorChannels = ChannelRGBA,
	};
	createImageInfo.mipLevels = texture.mipMapLevels;
	VULKAN_RESULT(CreateTextureImage(texture, createImageInfo));
	VULKAN_RESULT(CreateTextureView(texture, imageType));
	VULKAN_RESULT(vkCreateSampler(renderer.Device, &samplerCreateInfo, NULL, &texture.textureSampler));
	VULKAN_RESULT(GenerateMipmaps(texture));
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

void TextureSystem::UpdateTextureSize(Texture& texture, VkImageAspectFlags imageType, vec2& TextureResolution)
{
	texture.width = TextureResolution.x;
	texture.height = TextureResolution.y;

	DestroyTexture(texture);
	UpdateImage(texture);
	CreateTextureView(texture, imageType);

	//ImGuiDescriptorSet = ImGui_ImplVulkan_AddTexture(Sampler, View, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void TextureSystem::GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList)
{
	textureDescriptorList.emplace_back(VkDescriptorImageInfo
		{
			.sampler = texture.textureSampler,
			.imageView = texture.textureView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		});
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout)
{
	UpdateTextureLayout(texture, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout, uint32 mipLevels)
{
	UpdateTextureLayout(texture, texture.textureImageLayout, newImageLayout, mipLevels);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
{
	UpdateTextureLayout(texture, oldImageLayout, newImageLayout, texture.mipMapLevels - 1);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipLevels)
{
	VkImageMemoryBarrier imageMemoryBarrier =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.oldLayout = oldImageLayout,
		.newLayout = newImageLayout,
		.image = texture.textureImage,
		.subresourceRange =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = mipLevels,
			.levelCount = VK_REMAINING_MIP_LEVELS,
			.layerCount = 1
		}
	};

	auto singleCommand = Renderer_BeginSingleUseCommandBuffer(renderer.Device, renderer.CommandPool);
	vkCmdPipelineBarrier(singleCommand, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
	VkResult result = Renderer_EndSingleUseCommandBuffer(renderer.Device, renderer.CommandPool, renderer.GraphicsQueue, singleCommand);
	if (result == VK_SUCCESS)
	{
		texture.textureImageLayout = newImageLayout;
	}
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout)
{
	UpdateTextureLayout(texture, commandBuffer, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout, uint32 mipLevels)
{
	UpdateTextureLayout(texture, commandBuffer, texture.textureImageLayout, newImageLayout, mipLevels);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
{
	UpdateTextureLayout(texture, commandBuffer, oldImageLayout, newImageLayout, texture.mipMapLevels - 1);
}

void TextureSystem::UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipmapLevels)
{
	VkImageMemoryBarrier imageMemoryBarrier =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.oldLayout = texture.textureImageLayout,
		.newLayout = newImageLayout,
		.image = texture.textureImage,
		.subresourceRange =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = mipmapLevels,
			.levelCount = VK_REMAINING_MIP_LEVELS,
			.layerCount = 1
		}
	};

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
	texture.textureImageLayout = newImageLayout;
}

void TextureSystem::DestroyTexture(Texture& texture)
{
	Renderer_DestroyImageView(renderer.Device, &texture.textureView);
	Renderer_DestroySampler(renderer.Device, &texture.textureSampler);
	Renderer_DestroyImage(renderer.Device, &texture.textureImage);
	Renderer_FreeDeviceMemory(renderer.Device, &texture.textureMemory);
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
		TextureSystem_DestroyTexture(pair.second);
	}
	TextureMap.clear();

	for (auto& pair : DepthTextureMap)
	{
		TextureSystem_DestroyTexture(pair.second);
	}
	DepthTextureMap.clear();

	for (auto& list : RenderedTextureListMap)
	{
		for (auto& texture : list.second)
		{
			TextureSystem_DestroyTexture(texture);
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

VkResult TextureSystem::CreateTextureImage(Texture& texture, VkImageCreateInfo& createImageInfo)
{
	VULKAN_RESULT(vkCreateImage(renderer.Device, &createImageInfo, nullptr, &texture.textureImage));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(renderer.Device, texture.textureImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = Renderer_GetMemoryType(renderer.PhysicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	VULKAN_RESULT(vkAllocateMemory(renderer.Device, &allocInfo, nullptr, &texture.textureMemory));
	return vkBindImageMemory(renderer.Device, texture.textureImage, texture.textureMemory, 0);
}

VkResult TextureSystem::CreateTextureImage(Texture& texture, VkImageCreateInfo& imageCreateInfo, byte* textureData, VkDeviceSize textureSize)
{
	VkBuffer buffer = VK_NULL_HANDLE;
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
	VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
	VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VkMemoryPropertyFlags bufferProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	VULKAN_RESULT(bufferSystem.Buffer_CreateStagingBuffer(renderer, &stagingBuffer, &buffer, &stagingBufferMemory, &bufferMemory, textureData, textureSize, bufferUsage, bufferProperties));
	VULKAN_RESULT(CreateImage(texture, imageCreateInfo));
	VULKAN_RESULT(QuickTransitionImageLayout(texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));
	VULKAN_RESULT(CopyBufferToTexture(texture, buffer));
	VULKAN_RESULT(QuickTransitionImageLayout(texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
	VULKAN_RESULT(GenerateMipmaps(texture));

	Renderer_DestroyBuffer(renderer.Device, &buffer);
	Renderer_FreeDeviceMemory(renderer.Device, &bufferMemory);
	Renderer_DestroyBuffer(renderer.Device, &stagingBuffer);
	Renderer_FreeDeviceMemory(renderer.Device, &stagingBufferMemory);

	return VK_SUCCESS;
}

VkResult TextureSystem::CreateTextureImage(const Pixel& clearColor, ivec2 textureResolution, ColorChannelUsed colorChannels, VkImageAspectFlags imageType)
{
	//Vector<Pixel> pixels(textureResolution.x * textureResolution.y, clearColor);
	//VkDeviceSize bufferSize = (textureResolution.x * textureResolution.y * colorChannels);

	//Texture texture = Texture
	//{
	//	.width = textureResolution.x,
	//	.height = textureResolution.y,
	//	.depth = 1,
	//	.mipMapLevels = 1,
	//	.textureBufferIndex = 0,
	//	.textureImage = VK_NULL_HANDLE,
	//	.textureMemory = VK_NULL_HANDLE,
	//	.textureView = VK_NULL_HANDLE,
	//	.textureSampler = VK_NULL_HANDLE,
	//	.ImGuiDescriptorSet = VK_NULL_HANDLE,
	//	.textureUsage = kUse_2DImageTexture,
	//	.textureType = kType_UndefinedTexture,
	//	.textureByteFormat = VK_FORMAT_R8G8B8A8_SRGB,
	//	.textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	//	.sampleCount = VK_SAMPLE_COUNT_1_BIT,
	//	.colorChannels = colorChannels,
	//};

	//VkImageCreateInfo imageCreateInfo =
	//{
	//	.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	//	.imageType = VK_IMAGE_TYPE_2D,
	//	.format = texture.textureByteFormat,
	//	.extent =
	//	{
	//		.width = static_cast<uint32_t>(texture.width),
	//		.height = static_cast<uint32_t>(texture.height),
	//		.depth = 1
	//	},
	//	.mipLevels = texture.mipMapLevels,
	//	.arrayLayers = 1,
	//	.samples = texture.sampleCount,
	//	.tiling = VK_IMAGE_TILING_OPTIMAL,
	//	.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	//	.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	//	.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	//};

	//byte* pixelData = static_cast<byte*>(pixels.data());
	//VULKAN_RESULT(Texture_CreateTextureImage(renderer, texture, ImageCreateInfo, static_cast<byte*>(pixels.data()), bufferSize));
	//VULKAN_RESULT(Texture_CreateTextureView(renderer, texture, imageType));
	//VULKAN_RESULT(Texture_CreateTextureSampler(renderer, texture, textureLoader.SamplerCreateInfo));
	//VULKAN_RESULT(Texture_GenerateMipmaps(renderer, texture));
	//return texture;
	return VK_SUCCESS;
}

VkResult TextureSystem::UpdateImage(Texture& texture)
{
	VkImageCreateInfo imageCreateInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = texture.textureByteFormat,
		.extent =
		{
			.width = static_cast<uint32_t>(texture.width),
			.height = static_cast<uint32_t>(texture.height),
			.depth = 1
		},
		.mipLevels = texture.mipMapLevels,
		.arrayLayers = 1,
		.samples = texture.sampleCount,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};
	VULKAN_RESULT(vkCreateImage(renderer.Device, &imageCreateInfo, NULL, &texture.textureImage));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(renderer.Device, texture.textureImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = Renderer_GetMemoryType(renderer.PhysicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	VULKAN_RESULT(vkAllocateMemory(renderer.Device, &allocInfo, NULL, &texture.textureMemory));
	return vkBindImageMemory(renderer.Device, texture.textureImage, texture.textureMemory, 0);
}

VkResult TextureSystem::CreateImage(Texture& texture, VkImageCreateInfo& imageCreateInfo)
{
	VULKAN_RESULT(vkCreateImage(renderer.Device, &imageCreateInfo, NULL, &texture.textureImage));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(renderer.Device, texture.textureImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = Renderer_GetMemoryType(renderer.PhysicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	VULKAN_RESULT(vkAllocateMemory(renderer.Device, &allocInfo, NULL, &texture.textureMemory));
	return vkBindImageMemory(renderer.Device, texture.textureImage, texture.textureMemory, 0);
}

VkResult TextureSystem::CreateTextureView(Texture& texture, VkImageAspectFlags imageAspectFlags)
{
	VkImageViewCreateInfo TextureImageViewInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = texture.textureImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = texture.textureByteFormat,
		.subresourceRange =
		{
			.aspectMask = imageAspectFlags,
			.baseMipLevel = 0,
			.levelCount = texture.mipMapLevels,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}
	};
	return vkCreateImageView(renderer.Device, &TextureImageViewInfo, NULL, &texture.textureView);
}

VkResult TextureSystem::TransitionImageLayout(VkCommandBuffer& commandBuffer, Texture& texture, VkImageLayout newLayout)
{
	VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;
	VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;
	VkImageMemoryBarrier barrier =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = texture.textureImageLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = texture.textureImage,
		.subresourceRange = VkImageSubresourceRange
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = texture.mipMapLevels,
			.baseArrayLayer = 0,
			.layerCount = VK_REMAINING_ARRAY_LAYERS,
		}
	};
	if (texture.textureImageLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (texture.textureImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, NULL, 0, NULL, 1, &barrier);
	texture.textureImageLayout = newLayout;
	return VK_SUCCESS;
}

VkResult TextureSystem::QuickTransitionImageLayout(Texture& texture, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = renderSystem.BeginSingleTimeCommands();
	TransitionImageLayout(commandBuffer, texture, newLayout);
	VkResult result = renderSystem.EndSingleTimeCommands(commandBuffer);
	if (result == VK_SUCCESS)
	{
		texture.textureImageLayout = newLayout;
	}
	return result;
}

VkResult TextureSystem::CommandBufferTransitionImageLayout(VkCommandBuffer commandBuffer, Texture& texture, VkImageLayout newLayout, uint32 mipmapLevel)
{
	return TransitionImageLayout(commandBuffer, texture, newLayout);
}

VkResult TextureSystem::CopyBufferToTexture(Texture& texture, VkBuffer buffer)
{
	VkBufferImageCopy BufferImage =
	{
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = VkImageSubresourceLayers
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = static_cast<uint>((texture.textureType == kUse_CubeMapTexture ? 6 : 1))
		},
		.imageOffset = VkOffset3D
		{
			.x = 0,
			.y = 0,
			.z = 0,
		},
		.imageExtent = VkExtent3D
		{
			.width = static_cast<uint>(texture.width),
			.height = static_cast<uint>(texture.height),
			.depth = static_cast<uint>(texture.depth),
		}
	};
	VkCommandBuffer commandBuffer = renderSystem.BeginSingleTimeCommands();
	vkCmdCopyBufferToImage(commandBuffer, buffer, texture.textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferImage);
	return renderSystem.EndSingleTimeCommands(commandBuffer);
}

VkResult TextureSystem::GenerateMipmaps(Texture& texture)
{
	if (texture.mipMapLevels == 1)
	{
		return VK_SUCCESS;
	}

	int32 mipWidth = texture.width;
	int32 mipHeight = texture.height;

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(renderer.PhysicalDevice, texture.textureByteFormat, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		RENDERER_ERROR("Texture image format does not support linear blitting.");
	}

	VkCommandBuffer commandBuffer = renderSystem.BeginSingleTimeCommands();
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
	return renderSystem.EndSingleTimeCommands(commandBuffer);
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

/*oid TextureSystem_AddRenderedTexture(RenderPassGuid& renderPassGuid, Texture* renderedTextureListPtr, size_t renderedTextureCount)
{
	Vector<Texture> renderedTextureList(renderedTextureListPtr, renderedTextureListPtr + renderedTextureCount);
	AddRenderedTexture(renderPassGuid, renderedTextureList);
}

 Vector<Texture> Texture_TextureList()
{
	Vector<Texture> list;
	list.reserve(TextureMap.size());
	for (const auto& pair : TextureMap)
	{
		list.emplace_back(pair.second);
	}
	return list;
}

const Vector<Texture> Texture_DepthTextureList()
{
	Vector<Texture> list;
	list.reserve(DepthTextureMap.size());
	for (const auto& pair : DepthTextureMap)
	{
		list.emplace_back(pair.second);
	}
	return list;
}

void TextureSystem_UpdateCmdTextureLayout(VkCommandBuffer& commandBuffer, Texture& texture, VkImageLayout& oldImageLayout, VkImageLayout& newImageLayout, uint32 mipmapLevel)
{
	VkImageMemoryBarrier imageMemoryBarrier =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.oldLayout = texture.textureImageLayout,
		.newLayout = newImageLayout,
		.image = texture.textureImage,
		.subresourceRange =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = mipmapLevel,
			.levelCount = VK_REMAINING_MIP_LEVELS,
			.layerCount = 1
		}
	};

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
	texture.textureImageLayout = newImageLayout;
}*/

VkGuid TextureSystem_CreateTexture(const char* texturePath)
{
	return textureSystem.CreateTexture(texturePath);
}

void TextureSystem_AddRenderedTexture(RenderPassGuid& renderPassGuid, Texture* renderedTextureListPtr, size_t renderTextureCount)
{
	Vector<Texture> renderedTextureList = Vector<Texture>(renderedTextureListPtr, renderedTextureListPtr + renderTextureCount);
	return textureSystem.AddRenderedTexture(renderPassGuid, renderedTextureList);
}

 void TextureSystem_AddDepthTexture(RenderPassGuid& renderPassGuid, Texture& depthTexture)
 {
	return textureSystem.AddDepthTexture(renderPassGuid, depthTexture);
 }

 void TextureSystem_Update(const float& deltaTime)
 {
	return textureSystem.Update(deltaTime);
 }

 void TextureSystem_UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipmapLevel)
 {
	return textureSystem.UpdateTextureLayout(texture, oldImageLayout, newImageLayout, mipmapLevel);
 }

 void TextureSystem_UpdateCmdTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout& oldImageLayout, VkImageLayout& newImageLayout, uint32 mipmapLevel)
 {
	 return textureSystem.UpdateTextureLayout(texture, commandBuffer, oldImageLayout, newImageLayout, mipmapLevel);
 }

 void TextureSystem_UpdateTextureSize(Texture& texture, VkImageAspectFlags imageType, vec2& textureResolution)
 {
	return textureSystem.UpdateTextureSize(texture, imageType, textureResolution);
 }

 Texture TextureSystem_FindTexture(const RenderPassGuid& textureGuid)
 {
	return textureSystem.FindTexture(textureGuid);
 }

 Texture& TextureSystem_FindDepthTexture(const RenderPassGuid& renderPassGuid)
 {
	 return textureSystem.FindDepthTexture(renderPassGuid);
 }

 Texture& TextureSystem_FindRenderedTexture(const TextureGuid& textureGuid)
 {
	 return textureSystem.FindRenderedTexture(textureGuid);
 }

 bool TextureSystem_TextureExists(const RenderPassGuid& renderPassGuid)
 {
	 return textureSystem.TextureExists(renderPassGuid);
 }

 bool TextureSystem_DepthTextureExists(const RenderPassGuid& renderPassGuid)
 {
	 return textureSystem.DepthTextureExists(renderPassGuid);
 }

 bool TextureSystem_RenderedTextureExists(const RenderPassGuid& renderPassGuid, const TextureGuid& textureGuid)
 {
	 return textureSystem.RenderedTextureExists(renderPassGuid, textureGuid);
 }

 bool TextureSystem_RenderedTextureListExists(const RenderPassGuid& renderPassGuid)
 {
	 return textureSystem.RenderedTextureListExists(renderPassGuid);
 }

 void TextureSystem_GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList)
 {
	 textureSystem.GetTexturePropertiesBuffer(texture, textureDescriptorList);
 }

 void TextureSystem_DestroyTexture(Texture& texture)
 {
	 textureSystem.DestroyTexture(texture);
 }

 void TextureSystem_DestroyAllTextures()
 {
	 textureSystem.DestroyAllTextures();
 }
