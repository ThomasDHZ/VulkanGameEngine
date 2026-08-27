#pragma once

#include <Platform.h>
#include <VulkanTexture.h>
#include "Pixel.h"
#include "enum.h"
#include "FileSystem.h"
#include <vk_mem_alloc.h>


struct TextureLoader
{
    Vector<String> TextureFilePath;
    VkGuid TextureId;
    VkFormat TextureByteFormat;
    TextureTypeEnum TextureType;
    TextureUsageTypeEnum TextureUsageType;
    VkSamplerCreateInfo SamplerCreateInfo;
    uint32 MipMapCount;
    bool UsingSRGBFormat;
    bool IsSkyBox;
};

struct TextureHandle
{
    uint32 id = 0;
    uint32 generation = 0;
};

struct Texture
{
    TextureGuid           textureGuid = TextureGuid();
    TextureHandle         textureId;
    VulkanTexture         texture;
    TextureTypeEnum       textureType = TextureTypeEnum::kTextureType_Undefined;
    TextureUsageTypeEnum  textureUsageType = TextureUsageTypeEnum::kUsageType_Undefined;
    VkDescriptorSet       imGuiDescriptorSet = VK_NULL_HANDLE;
};

struct TextureReturnFileData
{
    Vector<byte>       TextureData;
    uint32             MipMapCount = 1;
    uint32             ArrayLayers = 1;
    uint32             BytesPerChannel = 0;
    ivec3              TextureDimensions = { 0, 0, 0 };
    VkFormat           TextureByteFormat = VK_FORMAT_UNDEFINED;
    VkImageAspectFlags TextureAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageLayout      TextureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    bool               IsCubeMap = false;
    bool               IsDepthFormat = false;
    bool               IsStencil = false;
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


    TextureReturnFileData LoadGeneralTexture(const TextureLoader& textureLoader);
    TextureReturnFileData LoadPngTexture(const TextureLoader& textureLoader);
    TextureReturnFileData LoadKtxTexture(const TextureLoader& textureLoader);

public:

    UnorderedMap<RenderPassGuid, Texture>                          DepthTextureMap;
    UnorderedMap<RenderPassGuid, Vector<Texture>>                  RenderedTextureListMap;
    Vector<Texture>                                                TextureList;
    Vector<Texture>                                                Texture3DList;
    Vector<Texture>                                                CubeMapTextureList;

    Texture                  LoadTexture(const String& texturePath);
    Texture                  LoadTexture(const TextureLoader& textureLoader);
    void                     GenerateTexture(VkGuid& renderPassId);
    void                     AddRenderedTexture(RenderPassGuid renderPassGuid, Vector<Texture>& renderedTextureList);
    void                     AddDepthTexture(RenderPassGuid renderPassGuid, Texture& depthTexture);

    Texture                  FindTexture(const VkGuid& textureId);
    Texture&                 FindDepthTexture(const RenderPassGuid& renderPassGuid);
    Texture&                 FindRenderedTexture(const TextureGuid& textureGuid);
    Vector<Texture>&         FindRenderedTextureList(const RenderPassGuid& renderPassGuid);

    const bool               TextureExists(const TextureGuid& textureGuid) const;
    const bool               DepthTextureExists(const RenderPassGuid& renderPassGuid) const;
    const bool               RenderedTextureExists(const RenderPassGuid& renderPassGuid, const TextureGuid& textureGuid) const;
    const bool               RenderedTextureListExists(const RenderPassGuid& renderPassGuid) const;
    void                     AddToMemoryPool(Texture& texture);
    void                     Destroy();
};
extern  TextureSystem& textureSystem;
inline TextureSystem& TextureSystem::Get()
{
    static TextureSystem instance;
    return instance;
}

namespace nlohmann
{
}