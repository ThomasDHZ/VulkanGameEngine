#include <TypeDef.h>
#include "DLL.h"
#include <CTexture.h>
#include <Texture.h>
#include <imgui/imgui.h>

extern "C"
{
	DLL_EXPORT void DLL_Texture_UpdateTextureLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkImageLayout* oldImageLayout, VkImageLayout* newImageLayout, uint32 mipLevel);
	DLL_EXPORT void DLL_Texture_UpdateCmdTextureLayout(VkCommandBuffer* commandBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout* newImageLayout, uint32 mipLevel);
	DLL_EXPORT VkResult DLL_Texture_TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage* image, uint32 mipmapLevels, VkImageLayout* oldLayout, VkImageLayout newLayout);
	DLL_EXPORT VkResult DLL_Texture_CreateImage(VkDevice device, VkPhysicalDevice physicalDevice, VkImage* image, VkDeviceMemory* memory, VkImageCreateInfo ImageCreateInfo);
	DLL_EXPORT VkResult DLL_Texture_QuickTransitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, uint32 mipmapLevels, VkImageLayout* oldLayout, VkImageLayout* newLayout);
	DLL_EXPORT VkResult DLL_Texture_CommandBufferTransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, uint32 mipmapLevels, VkImageLayout oldLayout, VkImageLayout newLayout);
	DLL_EXPORT VkResult DLL_Texture_CopyBufferToTexture(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkBuffer buffer, enum TextureUsageEnum textureType, int width, int height, int depth);
	DLL_EXPORT VkResult DLL_Texture_GenerateMipmaps(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat* textureByteFormat, uint32 mipmapLevels, int width, int height, bool usingMipMaps);
	DLL_EXPORT VkResult DLL_Texture_CreateTextureView(VkDevice device, VkImageView* view, VkImage image, VkFormat format, VkImageAspectFlags imageType, uint32 mipmapLevels);
	DLL_EXPORT VkResult DLL_Texture_CreateTextureSampler(VkDevice device, VkSamplerCreateInfo* samplerCreateInfo, VkSampler* smapler);
	DLL_EXPORT void DLL_Texture_SaveTexture(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, const char* filename, SharedPtr<Texture> texture, ExportTextureFormat textureFormat, uint32 channels);
	DLL_EXPORT void DLL_Texture_CreateImageTextureFromClearColor(VkDevice device,
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
		bool usingMipMap);

	DLL_EXPORT void DLL_Texture_CreateImageTextureFromFile(VkDevice device,
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
		bool usingMipMap);
}