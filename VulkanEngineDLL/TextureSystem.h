#pragma once
#include "Platform.h"
#include "Pixel.h"
#include "enum.h"
#include "FileSystem.h"
#include <vk_mem_alloc.h>

struct TextureLoader
{
    String TextureFilePath;
    VkGuid TextureId;
    VkFormat TextureByteFormat;
    bool UsingSRGBFormat;
    VkImageAspectFlags ImageType;
    TextureTypeEnum TextureType;
    bool UseMipMaps;
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
    VmaAllocation TextureAllocation = VK_NULL_HANDLE;

    TextureUsageEnum textureUsage = TextureUsageEnum::kUse_Undefined;
    TextureTypeEnum textureType = TextureTypeEnum::kType_UndefinedTexture;
    VkFormat textureByteFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
    ColorChannelUsed colorChannels = ColorChannelUsed::ChannelRGBA;
};

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
    void CreateTextureImage(Texture& texture, VkImageCreateInfo& imageCreateInfo, byte* textureData, VkDeviceSize textureSize);
    void CreateTextureView(Texture& texture, VkImageAspectFlags imageAspectFlags);
    void GenerateMipmaps(Texture& texture);

public:
    UnorderedMap<RenderPassGuid, Texture>                          DepthTextureMap;
    UnorderedMap<RenderPassGuid, Vector<Texture>>                  RenderedTextureListMap;
    UnorderedMap<RenderPassGuid, Texture>                          TextureMap;

    DLL_EXPORT VkGuid                CreateTexture(const String& texturePath);
    DLL_EXPORT Texture               CreateRenderPassTexture(VkGuid& textureId, uint32 width, uint32 height, VkFormat format, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, uint32 mipLevels = 1, bool createSampler = true);
    DLL_EXPORT void                  AddRenderedTexture(RenderPassGuid& renderPassGuid, Vector<Texture>& renderedTextureList);
    DLL_EXPORT void                  AddDepthTexture(RenderPassGuid& renderPassGuid, Texture& depthTexture);
    DLL_EXPORT void                  Update(const float& deltaTime);
    DLL_EXPORT void                  GetTexturePropertiesBuffer(Texture& texture, Vector<VkDescriptorImageInfo>& textureDescriptorList);
    DLL_EXPORT void                  TransitionImageLayout(Texture& texture, VkImageLayout newLayout, uint32 baseMipLevel = 0, uint32 levelCount = VK_REMAINING_MIP_LEVELS);
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
};
extern DLL_EXPORT TextureSystem& textureSystem;
inline TextureSystem& TextureSystem::Get()
{
    static TextureSystem instance;
    return instance;
}