#pragma once
extern "C"
{
#include "CVulkanRenderer.h"
}
#include "pixel.h"
#include "Typedef.h"
#include "VkGuid.h"
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "enum.h"
#include "FileSystem.h"

struct TextureLoader;
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

#ifdef __cplusplus
extern "C"
{
#endif
    DLL_EXPORT VkGuid   TextureSystem_LoadTexture(const GraphicsRenderer& renderer, const char* texturePath);
    DLL_EXPORT void     TextureSystem_AddRenderedTexture(RenderPassGuid& vkGuid, Vector<Texture>& renderedTextureList);
    DLL_EXPORT void     TextureSystem_AddDepthTexture(RenderPassGuid& vkGuid, Texture& depthTexture);
    DLL_EXPORT void     TextureSystem_Update(const float& deltaTime);
    DLL_EXPORT void     TextureSystem_UpdateTextureLayout(const GraphicsRenderer& renderer, Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipmapLevel);
    DLL_EXPORT void     TextureSystem_UpdateCmdTextureLayout(const GraphicsRenderer& renderer, VkCommandBuffer& commandBuffer, Texture& texture, VkImageLayout& oldImageLayout, VkImageLayout& newImageLayout, uint32 mipmapLevel);
    DLL_EXPORT void     TextureSystem_UpdateTextureSize(const GraphicsRenderer& renderer, Texture& texture, VkImageAspectFlags imageType, vec2& TextureResolution);
    DLL_EXPORT Texture  TextureSystem_FindTexture(const RenderPassGuid& guid); 
    DLL_EXPORT Texture& TextureSystem_FindDepthTexture(const RenderPassGuid& guid);
    DLL_EXPORT Texture& TextureSystem_FindRenderedTexture(const TextureGuid& textureGuid);
    DLL_EXPORT bool     TextureSystem_TextureExists(const RenderPassGuid& guid);
    DLL_EXPORT bool     TextureSystem_DepthTextureExists(const RenderPassGuid& guid);
    DLL_EXPORT bool     TextureSystem_RenderedTextureExists(const RenderPassGuid& guid, const TextureGuid& textureGuid);
    DLL_EXPORT bool     TextureSystem_RenderedTextureListExists(const RenderPassGuid& guid);
    DLL_EXPORT void     TextureSystem_GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList);
    DLL_EXPORT void     TextureSystem_DestroyTexture(const GraphicsRenderer& renderer, Texture& texture);
    DLL_EXPORT void     TextureSystem_DestroyAllTextures(const GraphicsRenderer& renderer);
#ifdef __cplusplus
}
#endif

    DLL_EXPORT Vector<Texture>& TextureSystem_FindRenderedTextureList(const RenderPassGuid& guid);
    void Texture_UpdateTextureBufferIndex(Texture& texture, uint32 bufferIndex);
    Texture  Texture_CreateTexture(const GraphicsRenderer& renderer, VkGuid& textureId, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps);
    VkResult Texture_CreateTextureImage(const GraphicsRenderer& renderer, Texture& texture, VkImageCreateInfo& createImageInfo);
    VkResult Texture_CreateTextureImage(const GraphicsRenderer& renderer, Texture& texture, VkImageCreateInfo& imageCreateInfo, byte* textureData, VkDeviceSize textureSize);
    VkResult Texture_CreateTextureImage(const GraphicsRenderer& renderer, const Pixel& clearColor, ivec2 textureResolution, ColorChannelUsed colorChannels, VkImageAspectFlags imageType);
    VkResult Texture_UpdateImage(const GraphicsRenderer& renderer, Texture& texture);
    VkResult Texture_CreateImage(const GraphicsRenderer& renderer, Texture& texture, VkImageCreateInfo& imageCreateInfo);
    VkResult Texture_CreateTextureView(const GraphicsRenderer& renderer, Texture& texture, VkImageAspectFlags imageAspectFlags);
    VkResult Texture_CreateTextureSampler(const GraphicsRenderer& renderer, Texture& texture, VkSamplerCreateInfo& sampleCreateInfo);
    VkResult Texture_TransitionImageLayout(const GraphicsRenderer& renderer, VkCommandBuffer& commandBuffer, Texture& texture, VkImageLayout newLayout);
    VkResult Texture_QuickTransitionImageLayout(const GraphicsRenderer& renderer, Texture& texture, VkImageLayout newLayout);
    VkResult Texture_CommandBufferTransitionImageLayout(const GraphicsRenderer& renderer, VkCommandBuffer commandBuffer, Texture& texture, VkImageLayout newLayout, uint32 mipmapLevel);
    VkResult Texture_CopyBufferToTexture(const GraphicsRenderer& renderer, Texture& texture, VkBuffer buffer);
    VkResult Texture_GenerateMipmaps(const GraphicsRenderer& renderer, Texture& texture);

class TextureSystem
{
private:

public:
    UnorderedMap<RenderPassGuid, Texture>                          DepthTextureMap;
    UnorderedMap<RenderPassGuid, Vector<Texture>>                  RenderedTextureListMap;
    UnorderedMap<RenderPassGuid, Texture>                          TextureMap;
    TextureSystem()
    {

    }

    ~TextureSystem()
    {
    }

    VkGuid LoadTexture(const String& texturePath)
    {
        return TextureSystem_LoadTexture(renderer, texturePath.c_str());
    }

    void Update(const float& deltaTime)
    {
        TextureSystem_Update(deltaTime);
    }

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

    void UpdateTextureSize(Texture& texture, VkImageAspectFlags imageType, vec2& TextureResolution)
    {
        TextureSystem_UpdateTextureSize(renderer, texture, imageType, TextureResolution);
    }

    void GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList)
    {
        TextureSystem_GetTexturePropertiesBuffer(texture, textureDescriptorList);
    }

    void UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout)
    {
        TextureSystem_UpdateTextureLayout(renderer, texture, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1);
    }

    void UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout, uint32 mipLevels)
    {
        TextureSystem_UpdateTextureLayout(renderer, texture, texture.textureImageLayout, newImageLayout, mipLevels);
    }

    void UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
    {
        TextureSystem_UpdateTextureLayout(renderer, texture, oldImageLayout, newImageLayout, texture.mipMapLevels - 1);
    }

    void UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipLevels)
    {
        TextureSystem_UpdateTextureLayout(renderer, texture, oldImageLayout, newImageLayout, mipLevels);
    }

    void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout)
    {
        TextureSystem_UpdateCmdTextureLayout(renderer, commandBuffer, texture, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1);
    }

    void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout, uint32 mipLevels)
    {
        TextureSystem_UpdateCmdTextureLayout(renderer, commandBuffer, texture, texture.textureImageLayout, newImageLayout, mipLevels);
    }

    void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
    {
        TextureSystem_UpdateCmdTextureLayout(renderer, commandBuffer, texture, oldImageLayout, newImageLayout, texture.mipMapLevels - 1);
    }

    void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipLevels)
    {
        TextureSystem_UpdateCmdTextureLayout(renderer, commandBuffer, texture, oldImageLayout, newImageLayout, mipLevels);
    }

    void DestroyTexture(Texture& texture)
    {
        TextureSystem_DestroyTexture(renderer, texture);
    }

    void AddRenderedTexture(RenderPassGuid& vkGuid, Vector<Texture>& renderedTextureList)
    {
        TextureSystem_AddRenderedTexture(vkGuid, renderedTextureList);
    }

    void AddDepthTexture(RenderPassGuid& vkGuid, Texture& depthTexture)
    {
        TextureSystem_AddDepthTexture(vkGuid, depthTexture);
    }

    Texture FindTexture(const RenderPassGuid& guid)
    {
        return TextureSystem_FindTexture(guid);
    }

    Texture& FindDepthTexture(const RenderPassGuid& guid)
    {
        return TextureSystem_FindDepthTexture(guid);
    }

    Texture& FindRenderedTexture(const TextureGuid& textureGuid)
    {
        return TextureSystem_FindRenderedTexture(textureGuid);
    }

    Vector<Texture>& FindRenderedTextureList(const RenderPassGuid& guid)
    {
        return TextureSystem_FindRenderedTextureList(guid);
    }

    const bool TextureExists(const RenderPassGuid& guid) const
    {
        return TextureSystem_TextureExists(guid);
    }

    const bool DepthTextureExists(const RenderPassGuid& guid) const
    {
        return TextureSystem_DepthTextureExists(guid);
    }

    const bool RenderedTextureExists(const RenderPassGuid& guid, const TextureGuid& textureGuid) const
    {
        return TextureSystem_RenderedTextureExists(guid, textureGuid);
    }

    const bool RenderedTextureListExists(const RenderPassGuid& guid) const
    {
        return TextureSystem_RenderedTextureListExists(guid);
    }

    const Vector<Texture> TextureList()
    {
        Vector<Texture> list;
        list.reserve(TextureMap.size());
        for (const auto& pair : TextureMap)
        {
            list.emplace_back(pair.second);
        }
        return list;
    }

    const Vector<Texture> DepthTextureList()
    {
        Vector<Texture> list;
        list.reserve(DepthTextureMap.size());
        for (const auto& pair : DepthTextureMap)
        {
            list.emplace_back(pair.second);
        }
        return list;
    }

    void DestroyAllTextures()
    {
        TextureSystem_DestroyAllTextures(renderer);
    }
};
DLL_EXPORT TextureSystem textureSystem;