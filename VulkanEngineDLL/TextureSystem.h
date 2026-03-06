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
    uint32 MipMapCount;
    bool UsingSRGBFormat;
    bool IsSkyBox;
};


struct Texture
{
    TextureGuid           textureGuid = TextureGuid();
    size_t                textureIndex = SIZE_MAX;
    size_t                bindlessTextureIndex = SIZE_MAX;
    int                   width = 1;
    int                   height = 1;
    int                   depth = 1;
    uint32                mipMapLevels = 0;

    VkImage               textureImage = VK_NULL_HANDLE;
    Vector<VkImageView>   textureViewList;
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

struct VulkanRenderPass;
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
    void CreateTextureView(Texture& texture, bool usingMultiView, VkImageAspectFlags imageAspectFlags);
    void GenerateMipmaps(Texture& texture);

public:

    UnorderedMap<RenderPassGuid, Texture>                          DepthTextureMap;
    UnorderedMap<RenderPassGuid, Vector<Texture>>                  RenderedTextureListMap;
    Vector<Texture>                                                TextureList;
    Vector<Texture>                                                Texture3DList;
    Vector<Texture>                                                CubeMapTextureList;

    DLL_EXPORT Texture                  CreateTexture(const String& texturePath);
    DLL_EXPORT Texture                  CreateTexture(TextureLoader textureLoader);
    DLL_EXPORT Texture                  LoadKTXTexture(const String& texturePath);
    DLL_EXPORT Texture                  LoadKTXTexture(TextureLoader textureLoader);
    //DLL_EXPORT VkGuid                 CreateTexture(Pixel clearColorPixel, ivec2 textureResolution, VkFormat textureFormat, ColorChannelUsed colorChannels);
    DLL_EXPORT Texture                  CreateRenderPassTexture(VulkanRenderPass& vulkanRenderPass, uint attachmentId);
    DLL_EXPORT void                     AddRenderedTexture(RenderPassGuid& renderPassGuid, Vector<Texture>& renderedTextureList);
    DLL_EXPORT void                     AddDepthTexture(RenderPassGuid& renderPassGuid, Texture& depthTexture);
    DLL_EXPORT void                     GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList);
    DLL_EXPORT void                     GetTexture3DPropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList);
    DLL_EXPORT void                     TransitionImageLayout(Texture& texture, VkImageLayout newLayout, uint32 baseMipLevel = 0, uint32 levelCount = VK_REMAINING_MIP_LEVELS);
    DLL_EXPORT void                     TransitionImageLayout(VkCommandBuffer& commandBuffer, Texture& texture, VkImageLayout newLayout, uint32 baseMipLevel = 0, uint32 levelCount = VK_REMAINING_MIP_LEVELS);
    DLL_EXPORT Texture                  FindTexture(const VkGuid& textureId);
    DLL_EXPORT Texture&                 FindDepthTexture(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT Texture&                 FindRenderedTexture(const TextureGuid& textureGuid);
    DLL_EXPORT Vector<Texture>&         FindRenderedTextureList(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT const bool               TextureExists(const TextureGuid& textureGuid) const;
    DLL_EXPORT const bool               DepthTextureExists(const RenderPassGuid& renderPassGuid) const;
    DLL_EXPORT const bool               RenderedTextureExists(const RenderPassGuid& renderPassGuid, const TextureGuid& textureGuid) const;
    DLL_EXPORT const bool               RenderedTextureListExists(const RenderPassGuid& renderPassGuid) const;
    DLL_EXPORT void                     DestroyTexture(Texture& texture);
    DLL_EXPORT void                     DestroyAllTextures();
    DLL_EXPORT const Vector<Texture>    DepthTextureList();
    DLL_EXPORT const Vector<Texture>    GetTextureList() { return TextureList; }
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