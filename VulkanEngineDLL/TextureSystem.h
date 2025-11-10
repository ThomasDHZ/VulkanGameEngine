#pragma once
#include <windows.h>
#include "Platform.h"
#include "pixel.h"
#include "enum.h"
#include "FileSystem.h"

struct TextureLoader
{
    String TextureFilePath;
    VkGuid TextureId;
    VkImageAspectFlags ImageType;
    TextureTypeEnum TextureType;
    bool UseMipMaps;
    VkImageCreateInfo ImageCreateInfo;
    VkSamplerCreateInfo SamplerCreateInfo;
};

struct Texture
{
    TextureGuid textureId;
    int width = 1;
    int height = 1;
    int depth = 1;
    uint32 mipMapLevels = 0;
    uint32 textureBufferIndex = 0;

    VkImage textureImage = VK_NULL_HANDLE;
    VkDeviceMemory textureMemory = VK_NULL_HANDLE;
    VkImageView textureView = VK_NULL_HANDLE;
    VkSampler textureSampler = VK_NULL_HANDLE;
    VkDescriptorSet ImGuiDescriptorSet = VK_NULL_HANDLE;

    TextureUsageEnum textureUsage = TextureUsageEnum::kUse_Undefined;
    TextureTypeEnum textureType = TextureTypeEnum::kType_UndefinedTexture;
    VkFormat textureByteFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
    ColorChannelUsed colorChannels = ColorChannelUsed::ChannelRGBA;
};


class TextureSystem
{
private:
    void     UpdateTextureBufferIndex(Texture& texture, uint32 bufferIndex);
    VkResult CreateTextureImage(Texture& texture, VkImageCreateInfo& createImageInfo);
    VkResult CreateTextureImage(Texture& texture, VkImageCreateInfo& imageCreateInfo, byte* textureData, VkDeviceSize textureSize);
    VkResult CreateTextureImage(const Pixel& clearColor, ivec2 textureResolution, ColorChannelUsed colorChannels, VkImageAspectFlags imageType);
    VkResult UpdateImage(Texture& texture);
    VkResult CreateImage(Texture& texture, VkImageCreateInfo& imageCreateInfo);
    VkResult CreateTextureView(Texture& texture, VkImageAspectFlags imageAspectFlags);
    VkResult CreateTextureSampler(Texture& texture, VkSamplerCreateInfo& sampleCreateInfo);
    VkResult TransitionImageLayout(VkCommandBuffer& commandBuffer, Texture& texture, VkImageLayout newLayout);
    VkResult QuickTransitionImageLayout(Texture& texture, VkImageLayout newLayout);
    VkResult CommandBufferTransitionImageLayout(VkCommandBuffer commandBuffer, Texture& texture, VkImageLayout newLayout, uint32 mipmapLevel);
    VkResult CopyBufferToTexture(Texture& texture, VkBuffer buffer);
    VkResult GenerateMipmaps(Texture& texture);

public:
    UnorderedMap<RenderPassGuid, Texture>                          DepthTextureMap;
    UnorderedMap<RenderPassGuid, Vector<Texture>>                  RenderedTextureListMap;
    UnorderedMap<RenderPassGuid, Texture>                          TextureMap;

    TextureSystem();
    ~TextureSystem();

    DLL_EXPORT VkGuid                CreateTexture(const String& texturePath);
    DLL_EXPORT Texture               CreateTexture(VkGuid& textureId, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps);
    DLL_EXPORT void                  AddRenderedTexture(RenderPassGuid& renderPassGuid, Vector<Texture>& renderedTextureList);
    DLL_EXPORT void                  AddDepthTexture(RenderPassGuid& renderPassGuid, Texture& depthTexture);
    DLL_EXPORT void                  Update(const float& deltaTime);
    DLL_EXPORT void                  UpdateTextureSize(Texture& texture, VkImageAspectFlags imageType, vec2& TextureResolution);
    DLL_EXPORT void                  GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList);
    DLL_EXPORT void                  UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout);
    DLL_EXPORT void                  UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout, uint32 mipLevels);
    DLL_EXPORT void                  UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);
    DLL_EXPORT void                  UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipLevels);
    DLL_EXPORT void                  UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout);
    DLL_EXPORT void                  UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout, uint32 mipLevels);
    DLL_EXPORT void                  UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);
    DLL_EXPORT void                  UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipmapLevels);
    DLL_EXPORT Texture               FindTexture(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT Texture&              FindDepthTexture(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT Texture&              FindRenderedTexture(const TextureGuid& textureGuid);
    DLL_EXPORT Vector<Texture>&      FindRenderedTextureList(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT const bool            TextureExists(const RenderPassGuid& renderPassGuid) const;
    DLL_EXPORT const bool            DepthTextureExists(const RenderPassGuid& renderPassGuid) const;
    DLL_EXPORT const bool            RenderedTextureExists(const RenderPassGuid& renderPassGuid, const TextureGuid& textureGuid) const;
    DLL_EXPORT const bool            RenderedTextureListExists(const RenderPassGuid& renderPassGuid) const;
    DLL_EXPORT void                  DestroyTexture(Texture& texture);
    DLL_EXPORT void                  DestroyAllTextures();
    DLL_EXPORT const Vector<Texture> TextureList();
    DLL_EXPORT const Vector<Texture> DepthTextureList();

    //Texture CreateTexture(VkGuid& textureId, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps)
    //{
    //    return Texture_CreateTexture(renderer, textureId, imageType, createImageInfo, samplerCreateInfo, useMipMaps);
    //}
    //
    //Texture CreateTexture(const String& texturePath, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps)
    //{
    //    return Texture_CreateTexture(renderer, texturePath, imageType, createImageInfo, samplerCreateInfo, useMipMaps);
    //}
    //
    //Texture CreateTexture(Pixel& clearColor, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps)
    //{
    //    return Texture_CreateTexture(renderer, clearColor, imageType, createImageInfo, samplerCreateInfo, useMipMaps);
    //}
};
DLL_EXPORT TextureSystem textureSystem;

#ifdef __cplusplus
extern "C"
{
#endif
    DLL_EXPORT VkGuid   TextureSystem_CreateTexture(const char* texturePath);
    DLL_EXPORT void     TextureSystem_AddRenderedTexture(RenderPassGuid& renderPassGuid, Texture* renderedTextureListPtr, size_t renderTextureCount);
    DLL_EXPORT void     TextureSystem_AddDepthTexture(RenderPassGuid& renderPassGuid, Texture& depthTexture);
    DLL_EXPORT void     TextureSystem_Update(const float& deltaTime);
    DLL_EXPORT void     TextureSystem_UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipmapLevel);
    DLL_EXPORT void     TextureSystem_UpdateCmdTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout& oldImageLayout, VkImageLayout& newImageLayout, uint32 mipmapLevel);
    DLL_EXPORT void     TextureSystem_UpdateTextureSize(Texture& texture, VkImageAspectFlags imageType, vec2& textureResolution);
    DLL_EXPORT Texture  TextureSystem_FindTexture(const RenderPassGuid& textureGuid); 
    DLL_EXPORT Texture& TextureSystem_FindDepthTexture(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT Texture& TextureSystem_FindRenderedTexture(const TextureGuid& textureGuid);
    DLL_EXPORT bool     TextureSystem_TextureExists(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT bool     TextureSystem_DepthTextureExists(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT bool     TextureSystem_RenderedTextureExists(const RenderPassGuid& renderPassGuid, const TextureGuid& textureGuid);
    DLL_EXPORT bool     TextureSystem_RenderedTextureListExists(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT void     TextureSystem_GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList);
    DLL_EXPORT void     TextureSystem_DestroyTexture(Texture& texture);
    DLL_EXPORT void     TextureSystem_DestroyAllTextures();
#ifdef __cplusplus
}
#endif

    DLL_EXPORT Vector<Texture>& TextureSystem_FindRenderedTextureList(const RenderPassGuid& guid);