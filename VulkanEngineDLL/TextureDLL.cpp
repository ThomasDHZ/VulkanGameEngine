#include "TextureDLL.h"
#include <Texture.h>
#include <imgui/backends/imgui_impl_vulkan.h>

	void DLL_Texture_UpdateTextureLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkImageLayout* oldImageLayout, VkImageLayout* newImageLayout, uint32 mipLevel)
	{
		return Texture_UpdateTextureLayout(device, commandPool, graphicsQueue, image, oldImageLayout, newImageLayout, mipLevel);
	}

	void DLL_Texture_UpdateCmdTextureLayout(VkCommandBuffer* commandBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout* newImageLayout, uint32 mipLevel)
	{
		return Texture_UpdateCmdTextureLayout(commandBuffer, image, oldImageLayout, newImageLayout, mipLevel);
	}

	VkResult DLL_Texture_TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage* image, uint32 mipmapLevels, VkImageLayout* oldLayout, VkImageLayout newLayout)
	{
		return Texture_TransitionImageLayout(commandBuffer, image, mipmapLevels, oldLayout, newLayout);
	}

	VkResult DLL_Texture_CreateImage(VkDevice device, VkPhysicalDevice physicalDevice, VkImage* image, VkDeviceMemory* memory, VkImageCreateInfo ImageCreateInfo)
	{
		return Texture_CreateImage(device, physicalDevice, image, memory, ImageCreateInfo);
	}

	VkResult DLL_Texture_QuickTransitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, uint32 mipmapLevels, VkImageLayout* oldLayout, VkImageLayout* newLayout)
	{
		return Texture_QuickTransitionImageLayout(device, commandPool, graphicsQueue, image, mipmapLevels, oldLayout, newLayout);
	}

	VkResult DLL_Texture_CommandBufferTransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, uint32 mipmapLevels, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		return Texture_CommandBufferTransitionImageLayout(commandBuffer, image, mipmapLevels, oldLayout, newLayout);
	}

	VkResult DLL_Texture_CopyBufferToTexture(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkBuffer buffer, enum TextureUsageEnum textureType, int width, int height, int depth)
	{
		return Texture_CopyBufferToTexture(device, commandPool, graphicsQueue, image, buffer, textureType, width, height, depth);
	}

	VkResult DLL_Texture_GenerateMipmaps(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat* textureByteFormat, uint32 mipmapLevels, int width, int height, bool usingMipMaps)
	{
		return Texture_GenerateMipmaps(device, physicalDevice, commandPool, graphicsQueue, image, textureByteFormat, mipmapLevels, width, height, usingMipMaps);
	}

	VkResult DLL_Texture_CreateTextureView(VkDevice device, VkImageView* view, VkImage image, VkFormat format, VkImageAspectFlags imageType, uint32 mipmapLevels)
	{
		return Texture_CreateTextureView(device, view, image, format, imageType, mipmapLevels);
	}

	VkResult DLL_Texture_CreateTextureSampler(VkDevice device, VkSamplerCreateInfo* samplerCreateInfo, VkSampler* smapler)
	{
		return Texture_CreateTextureSampler(device, samplerCreateInfo, smapler);
	}

	void DLL_Texture_SaveTexture(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, const char* filename, SharedPtr<Texture> texture, ExportTextureFormat textureFormat, uint32 channels)
	{
		return Texture_SaveTexture(device, commandPool, graphicsQueue, filename, texture, textureFormat, channels);
	}

	 void DLL_Texture_CreateImageTextureFromClearColor(VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkCommandPool commandPool,
		VkQueue graphicsQueue,
		int* width,
		int* height,
		int* depth,
		VkFormat textureByteFormat,
		uint mipmapLevels,
		VkImage* textureImage,
		VkDeviceMemory* textureMemory,
		VkImageLayout* textureImageLayout,
		enum ColorChannelUsed* colorChannelUsed,
		enum TextureUsageEnum textureUsage,
		const Pixel& clearColor,
		bool usingMipMap)
	{
		 return Texture_CreateImageTexture(device,
			 physicalDevice,
			 commandPool,
			 graphicsQueue,
			 width,
			 height,
			 depth,
			 textureByteFormat,
			 mipmapLevels,
			 textureImage,
			 textureMemory,
			 textureImageLayout,
			 colorChannelUsed,
			 textureUsage,
			 clearColor,
			 usingMipMap);
	}

	 void DLL_Texture_CreateImageTextureFromFile(VkDevice device,
		VkPhysicalDevice physicalDevice,
		VkCommandPool commandPool,
		VkQueue graphicsQueue,
		int* width,
		int* height,
		int* depth,
		VkFormat textureByteFormat,
		uint mipmapLevels,
		VkImage* textureImage,
		VkDeviceMemory* textureMemory,
		VkImageLayout* textureImageLayout,
		enum ColorChannelUsed* colorChannelUsed,
		enum TextureUsageEnum textureUsage,
		const char* filePath,
		bool usingMipMap)
	{
		 const std::string path(filePath);
		 return Texture_CreateImageTexture(device,
			 physicalDevice,
			 commandPool,
			 graphicsQueue,
			 width,
			 height,
			 depth,
			 textureByteFormat,
			 mipmapLevels,
			 textureImage,
			 textureMemory,
			 textureImageLayout,
			 colorChannelUsed,
			 textureUsage,
			 path,
			 usingMipMap);
	}