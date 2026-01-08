#pragma once
#include "Platform.h"
#include "Pixel.h"
#include "enum.h"
#include "FileSystem.h"
#include <vk_mem_alloc.h>

struct TextureLoader
{
    Vector<String> TextureFilePath;
    VkGuid TextureId;
    VkFormat TextureByteFormat;
    VkImageAspectFlags ImageType;
    TextureTypeEnum TextureType;
    VkSamplerCreateInfo SamplerCreateInfo;
    bool UsingSRGBFormat;
    bool UseMipMaps;
    bool IsSkyBox;
};


struct Texture
{
    TextureGuid           textureId;
    int                   width = 1;
    int                   height = 1;
    int                   depth = 1;
    uint32                mipMapLevels = 0;
    uint32                textureBufferIndex = 0;

    VkImage               textureImage = VK_NULL_HANDLE;
    VkDeviceMemory        textureMemory = VK_NULL_HANDLE;
    VkImageView           textureView = VK_NULL_HANDLE;
    VkImageView           RenderedCubeMapView = VK_NULL_HANDLE;
    VkImageView           AttachmentArrayView = VK_NULL_HANDLE;
    VkSampler             textureSampler = VK_NULL_HANDLE;
    VkDescriptorSet       ImGuiDescriptorSet = VK_NULL_HANDLE;
    VmaAllocation         TextureAllocation = VK_NULL_HANDLE;

    TextureUsageEnum      textureUsage = TextureUsageEnum::kUse_Undefined;
    TextureTypeEnum       textureType = TextureTypeEnum::TextureType_UNKNOWN;
    VkFormat              textureByteFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout         textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
    ColorChannelUsed      colorChannels = ColorChannelUsed::ChannelRGBA;
};

struct PrefilterSkyboxTexture
{
    uint32                                                         PrefilterMipmapCount;
    Texture                                                        PrefilterCubeMap;
    Vector<VkImageView>                                            PrefilterAttachmentImageViews;
    Vector<VkFramebuffer>                                          PrefilterMipFramebufferList;
};

struct RenderAttachmentLoader;
class TextureSystem
{
public: 
    static TextureSystem& Get();

private:
    TextureSystem() = default;
    ~TextureSystem() = default;
    TextureSystem(const TextureSystem&) = delete;
    TextureSystem& operator=(const TextureSystem&) = delete;
    TextureSystem(TextureSystem&&) = delete;
    TextureSystem& operator=(TextureSystem&&) = delete;

    void UpdateTextureBufferIndex(Texture& texture, uint32 bufferIndex);
    void CreateTextureImage(Texture& texture, VkImageCreateInfo& imageCreateInfo, Vector<byte>& textureData, uint layerCount);
    void CreateTextureView(Texture& texture, VkImageAspectFlags imageAspectFlags);
    void GenerateMipmaps(Texture& texture);

public:
    Texture                                                        CubeMap;
    Texture                                                        IrradianceCubeMap;
    PrefilterSkyboxTexture                                         PrefilterCubeMap;
    UnorderedMap<RenderPassGuid, Texture>                          DepthTextureMap;
    UnorderedMap<RenderPassGuid, Vector<Texture>>                  RenderedTextureListMap;
    UnorderedMap<RenderPassGuid, Texture>                          TextureMap;

    DLL_EXPORT VkGuid                   CreateTexture(const String& texturePath);
    DLL_EXPORT Texture                  CreateRenderPassTexture(const RenderAttachmentLoader& renderAttachmentLoader, ivec2 renderAttachmentResolution);
    DLL_EXPORT void                     CreatePrefilterSkyBoxTexture(const VkRenderPass& renderPass, Texture& texture);
    DLL_EXPORT void                     AddRenderedTexture(RenderPassGuid& renderPassGuid, Vector<Texture>& renderedTextureList);
    DLL_EXPORT void                     AddDepthTexture(RenderPassGuid& renderPassGuid, Texture& depthTexture);
    DLL_EXPORT void                     Update(const float& deltaTime);
    DLL_EXPORT void                     GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList);
    DLL_EXPORT void                     TransitionImageLayout(Texture& texture, VkImageLayout newLayout, uint32 baseMipLevel = 0, uint32 levelCount = VK_REMAINING_MIP_LEVELS);
    DLL_EXPORT void                     TransitionImageLayout(const VkCommandBuffer& commandBuffer, Texture& texture, VkImageLayout newLayout, uint32 baseMipLevel = 0, uint32 levelCount = VK_REMAINING_MIP_LEVELS);
    DLL_EXPORT Texture                  FindTexture(const VkGuid& textureId, int a);
    DLL_EXPORT Texture                  FindTexture(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT Texture&                 FindDepthTexture(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT Texture&                 FindRenderedTexture(const TextureGuid& textureGuid);
    DLL_EXPORT Vector<Texture>&         FindRenderedTextureList(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT const bool               TextureExists(const RenderPassGuid& renderPassGuid) const;
    DLL_EXPORT const bool               DepthTextureExists(const RenderPassGuid& renderPassGuid) const;
    DLL_EXPORT const bool               RenderedTextureExists(const RenderPassGuid& renderPassGuid, const TextureGuid& textureGuid) const;
    DLL_EXPORT const bool               RenderedTextureListExists(const RenderPassGuid& renderPassGuid) const;
    DLL_EXPORT void                     DestroyTexture(Texture& texture);
    DLL_EXPORT void                     DestroyAllTextures();
    DLL_EXPORT const Vector<Texture>    TextureList();
    DLL_EXPORT const Vector<Texture>    DepthTextureList();
};
extern DLL_EXPORT TextureSystem& textureSystem;
inline TextureSystem& TextureSystem::Get()
{
    static TextureSystem instance;
    return instance;
}

namespace nlohmann
{
}