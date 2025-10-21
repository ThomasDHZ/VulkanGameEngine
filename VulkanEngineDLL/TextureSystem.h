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

VkResult Texture_CreateTextureImage(const GraphicsRenderer& renderer, Texture& texture, VkImageCreateInfo& createImageInfo);
VkResult Texture_CreateTextureImage(const GraphicsRenderer& renderer, Texture& texture, VkImageCreateInfo& imageCreateInfo, byte* textureData, VkDeviceSize textureSize);
VkResult Texture_CreateTextureImage(const GraphicsRenderer& renderer, const Pixel& clearColor, ivec2 textureResolution, ColorChannelUsed colorChannels, VkImageAspectFlags imageType);

DLL_EXPORT void Texture_AddRenderedTexture(RenderPassGuid& vkGuid, Vector<Texture>& renderedTextureList);
DLL_EXPORT void Texture_AddDepthTexture(RenderPassGuid& vkGuid, Texture& depthTexture);

DLL_EXPORT Texture& Texture_FindDepthTexture(const RenderPassGuid& guid);
DLL_EXPORT Texture& Texture_FindRenderedTexture(const TextureGuid& textureGuid);
DLL_EXPORT Vector<Texture>& Texture_FindRenderedTextureList(const RenderPassGuid& guid);

DLL_EXPORT bool Texture_DepthTextureExists(const RenderPassGuid& guid);
DLL_EXPORT bool Texture_RenderedTextureExists(const RenderPassGuid& guid, const TextureGuid& textureGuid);
DLL_EXPORT bool Texture_RenderedTextureListExists(const RenderPassGuid& guid);
DLL_EXPORT void Texture_DestroyAllTextures(const GraphicsRenderer& renderer);

#ifdef __cplusplus
extern "C" {
#endif
        DLL_EXPORT bool Texture_TextureExists(const RenderPassGuid& guid);      
        DLL_EXPORT Texture Texture_FindTexture(const RenderPassGuid& guid);
        DLL_EXPORT VkGuid Texture_LoadTexture(const GraphicsRenderer& renderer, const char* texturePath);
        DLL_EXPORT Texture Texture_CreateTexture(const GraphicsRenderer& renderer, VkGuid& textureId, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps);
        DLL_EXPORT void Texture_UpdateTextureSize(const GraphicsRenderer& renderer, Texture& texture, VkImageAspectFlags imageType, vec2& TextureResolution);
        DLL_EXPORT void Texture_UpdateTextureBufferIndex(Texture& texture, uint32 bufferIndex);
        DLL_EXPORT void Texture_GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList);
        DLL_EXPORT void Texture_DestroyTexture(const GraphicsRenderer& renderer, Texture& texture);
        DLL_EXPORT void Texture_UpdateCmdTextureLayout(const GraphicsRenderer& renderer, VkCommandBuffer& commandBuffer, Texture& texture, VkImageLayout& oldImageLayout, VkImageLayout& newImageLayout, uint32 mipmapLevel);
        DLL_EXPORT void Texture_UpdateTextureLayout(const GraphicsRenderer& renderer, Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipmapLevel);

        VkResult Texture_UpdateImage(const GraphicsRenderer& renderer, Texture & texture);
        VkResult Texture_CreateImage(const GraphicsRenderer& renderer, Texture & texture, VkImageCreateInfo& imageCreateInfo);
        VkResult Texture_CreateTextureSampler(const GraphicsRenderer& renderer, Texture& texture, VkSamplerCreateInfo& sampleCreateInfo);
        VkResult Texture_CreateTextureView(const GraphicsRenderer& renderer, Texture & texture, VkImageAspectFlags imageAspectFlags);
        VkResult Texture_TransitionImageLayout(const GraphicsRenderer& renderer, VkCommandBuffer& commandBuffer, Texture& texture, VkImageLayout newLayout);
        VkResult Texture_QuickTransitionImageLayout(const GraphicsRenderer& renderer, Texture& texture, VkImageLayout newLayout);
        VkResult Texture_CommandBufferTransitionImageLayout(const GraphicsRenderer& renderer, VkCommandBuffer commandBuffer, Texture& texture, VkImageLayout newLayout, uint32 mipmapLevel);

        VkResult Texture_CopyBufferToTexture(const GraphicsRenderer& renderer, Texture & texture, VkBuffer buffer);
        VkResult Texture_GenerateMipmaps(const GraphicsRenderer& renderer, Texture & texture);
#ifdef __cplusplus
}
#endif

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

    // Load Texture
    VkGuid LoadTexture(const String& texturePath)
    {
        return Texture_LoadTexture(renderer, texturePath.c_str());
    }

    // Update
    void Update(const float& deltaTime)
    {
        int x = 0;
        for (auto& [id, texture] : TextureMap)
        {
            UpdateTextureBufferIndex(texture, x);
            x++;
        }
    }

    // Create Texture
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

    // Texture Update & Layouts
    void UpdateTextureBufferIndex(Texture& texture, uint32 bufferIndex)
    {
        Texture_UpdateTextureBufferIndex(texture, bufferIndex);
    }

    void UpdateTextureSize(Texture& texture, VkImageAspectFlags imageType, vec2& TextureResolution)
    {
        Texture_UpdateTextureSize(renderer, texture, imageType, TextureResolution);
    }

    void GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList)
    {
        Texture_GetTexturePropertiesBuffer(texture, textureDescriptorList);
    }

    void UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout)
    {
        Texture_UpdateTextureLayout(renderer, texture, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1);
    }

    void UpdateTextureLayout(Texture& texture, VkImageLayout newImageLayout, uint32 mipLevels)
    {
        Texture_UpdateTextureLayout(renderer, texture, texture.textureImageLayout, newImageLayout, mipLevels);
    }

    void UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
    {
        Texture_UpdateTextureLayout(renderer, texture, oldImageLayout, newImageLayout, texture.mipMapLevels - 1);
    }

    void UpdateTextureLayout(Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipLevels)
    {
        Texture_UpdateTextureLayout(renderer, texture, oldImageLayout, newImageLayout, mipLevels);
    }

    void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout)
    {
        Texture_UpdateCmdTextureLayout(renderer, commandBuffer, texture, texture.textureImageLayout, newImageLayout, texture.mipMapLevels - 1);
    }

    void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout newImageLayout, uint32 mipLevels)
    {
        Texture_UpdateCmdTextureLayout(renderer, commandBuffer, texture, texture.textureImageLayout, newImageLayout, mipLevels);
    }

    void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
    {
        Texture_UpdateCmdTextureLayout(renderer, commandBuffer, texture, oldImageLayout, newImageLayout, texture.mipMapLevels - 1);
    }

    void UpdateTextureLayout(Texture& texture, VkCommandBuffer& commandBuffer, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, uint32 mipLevels)
    {
        Texture_UpdateCmdTextureLayout(renderer, commandBuffer, texture, oldImageLayout, newImageLayout, mipLevels);
    }

    void DestroyTexture(Texture& texture)
    {
        Texture_DestroyTexture(renderer, texture);
    }

    void AddRenderedTexture(RenderPassGuid& vkGuid, Vector<Texture>& renderedTextureList)
    {
        Texture_AddRenderedTexture(vkGuid, renderedTextureList);
    }

    void AddDepthTexture(RenderPassGuid& vkGuid, Texture& depthTexture)
    {
        Texture_AddDepthTexture(vkGuid, depthTexture);
    }

    Texture FindTexture(const RenderPassGuid& guid)
    {
        return Texture_FindTexture(guid);
    }

    Texture& FindDepthTexture(const RenderPassGuid& guid)
    {
        return Texture_FindDepthTexture(guid);
    }

    Texture& FindRenderedTexture(const TextureGuid& textureGuid)
    {
        return Texture_FindRenderedTexture(textureGuid);
    }

    Vector<Texture>& FindRenderedTextureList(const RenderPassGuid& guid)
    {
        return Texture_FindRenderedTextureList(guid);
    }

    const bool TextureExists(const RenderPassGuid& guid) const
    {
        return Texture_TextureExists(guid);
    }

    const bool DepthTextureExists(const RenderPassGuid& guid) const
    {
        return Texture_DepthTextureExists(guid);
    }

    const bool RenderedTextureExists(const RenderPassGuid& guid, const TextureGuid& textureGuid) const
    {
        return Texture_RenderedTextureExists(guid, textureGuid);
    }

    const bool RenderedTextureListExists(const RenderPassGuid& guid) const
    {
        return Texture_RenderedTextureListExists(guid);
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
        Texture_DestroyAllTextures(renderer);
    }
};
DLL_EXPORT TextureSystem textureSystem;