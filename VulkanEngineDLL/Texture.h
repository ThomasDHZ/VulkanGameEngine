#pragma once
extern "C"
{
#include "CVulkanRenderer.h"
}
#include "pixel.h"
#include "Typedef.h"
#include "VkGuid.h"
#include "json.h"
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"

typedef enum ColorChannelUsed
{
    ChannelR = 1,
    ChannelRG,
    ChannelRGB,
    ChannelRGBA
};

typedef enum TextureUsageEnum
{
    kUse_Undefined,
    kUse_2DImageTexture,
    kUse_2DDataTexture,
    kUse_2DRenderedTexture,
    kUse_2DDepthTexture,
    kUse_3DImageTexture,
    kUse_3DDataTexture,
    kUse_3DRenderedTexture,
    kUse_3DDepthTexture,
    kUse_CubeMapTexture,
    kUse_CubeMapDepthTexture
};

typedef enum TextureTypeEnum
{
    kType_UndefinedTexture,
    kType_TextureAtlas,
    kType_RenderedColorTexture,
    kType_RenderedDepthTexture,
    kType_ReadableTexture,
    kType_DiffuseTextureMap,
    kType_SpecularTextureMap,
    kType_AlbedoTextureMap,
    kType_MetallicTextureMap,
    kType_RoughnessTextureMap,
    kType_AmbientOcclusionTextureMap,
    kType_NormalTextureMap,
    kType_DepthTextureMap,
    kType_AlphaTextureMap,
    kType_EmissionTextureMap,
    kType_PaletteRotationMap,
    kType_CubeMapTexture,
    kType_CubeMapDepthTexture,
    kType_EnvironmentTexture,
    kType_RenderedCubeMap,
    kType_BakedTexture
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

DLL_EXPORT Texture Texture_CreateTexture(const GraphicsRenderer& renderer, const String& texturePath, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps);
DLL_EXPORT Texture Texture_CreateTexture(const GraphicsRenderer& renderer, Pixel& clearColor, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps);

void Texture_CreateTextureImage(const GraphicsRenderer& renderer, Texture & texture, const Pixel& clearColor);
void Texture_CreateTextureImage(const GraphicsRenderer& renderer, Texture & texture, const String& filePath);
void Texture_CreateTextureImage(const GraphicsRenderer& renderer, Texture & texture, VkImageCreateInfo& createImageInfo);

#ifdef __cplusplus
extern "C" {
#endif
        DLL_EXPORT Texture Texture_LoadTexture(const GraphicsRenderer& renderer, const char* jsonText);
        DLL_EXPORT Texture Texture_CreateTexture(const GraphicsRenderer& renderer, VkGuid& textureId, VkImageAspectFlags imageType, VkImageCreateInfo& createImageInfo, VkSamplerCreateInfo& samplerCreateInfo, bool useMipMaps);
        DLL_EXPORT void Texture_UpdateTextureSize(const GraphicsRenderer& renderer, Texture& texture, VkImageAspectFlags imageType, vec2& TextureResolution);
        DLL_EXPORT void Texture_UpdateTextureBufferIndex(Texture& texture, uint32 bufferIndex);
        DLL_EXPORT void Texture_GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList);
        DLL_EXPORT void Texture_DestroyTexture(const GraphicsRenderer& renderer, Texture& texture);
        DLL_EXPORT void Texture_UpdateCmdTextureLayout(const GraphicsRenderer& renderer, VkCommandBuffer& commandBuffer, Texture& texture, VkImageLayout& oldImageLayout, VkImageLayout& newImageLayout);
        DLL_EXPORT void Texture_UpdateTextureLayout(const GraphicsRenderer& renderer, Texture& texture, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);

        VkResult Texture_UpdateImage(const GraphicsRenderer& renderer, Texture & texture);
        VkResult Texture_CreateImage(const GraphicsRenderer& renderer, Texture & texture, VkImageCreateInfo& imageCreateInfo);
        VkResult Texture_CreateTextureView(const GraphicsRenderer& renderer, Texture & texture, VkImageAspectFlags imageAspectFlags);
        VkResult Texture_CreateSpriteTextureSampler(const GraphicsRenderer& renderer, VkSampler& smapler);
        VkResult Texture_CreateRenderedTextureSampler(const GraphicsRenderer& renderer, VkSampler& smapler);
        VkResult Texture_CreateDepthTextureSampler(const GraphicsRenderer& renderer, VkSampler& smapler);

        VkResult Texture_TransitionImageLayout(const GraphicsRenderer& renderer, VkCommandBuffer& commandBuffer, Texture& texture, VkImageLayout newLayout);
        VkResult Texture_QuickTransitionImageLayout(const GraphicsRenderer& renderer, Texture& texture, VkImageLayout newLayout);
        VkResult Texture_CommandBufferTransitionImageLayout(const GraphicsRenderer& renderer, VkCommandBuffer commandBuffer, Texture& texture, VkImageLayout newLayout);

        VkResult Texture_CopyBufferToTexture(const GraphicsRenderer& renderer, Texture & texture, VkBuffer buffer);
        VkResult Texture_GenerateMipmaps(const GraphicsRenderer& renderer, Texture & texture);
#ifdef __cplusplus
}
#endif