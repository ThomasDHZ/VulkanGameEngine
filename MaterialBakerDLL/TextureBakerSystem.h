#pragma once
#include <nvtt/nvtt.h>
#include <DLL.h>
#include <TextureSystem.h>
#include <nvtt/nvtt_lowlevel.h>
#include <nvtt/nvtt_wrapper.h>

struct ImportTexture
{
    TextureGuid              textureGuid = TextureGuid();
    RenderPassGuid           renderPassGuid = RenderPassGuid();
    size_t                   textureIndex = SIZE_MAX;

    int                      width = 1;
    int                      height = 1;
    int                      depth = 1;
    uint32                   mipMapLevels = 0;

    VkImage                  textureImage = VK_NULL_HANDLE;
    VkDeviceMemory           textureMemory = VK_NULL_HANDLE;
    Vector<VkImageView>      textureViewList;
    VkImageView              RenderedCubeMapView = VK_NULL_HANDLE;
    VkImageView              AttachmentArrayView = VK_NULL_HANDLE;
    VkSampler                textureSampler = VK_NULL_HANDLE;
    VkDescriptorSet          ImGuiDescriptorSet = VK_NULL_HANDLE;
    VmaAllocation            TextureAllocation = VK_NULL_HANDLE;

    TextureUsageEnum         textureUsage = TextureUsageEnum::kUse_Undefined;
    TextureTypeEnum          textureType = TextureTypeEnum::TextureType_UNKNOWN;
    VkFormat                 textureByteFormat = VK_FORMAT_UNDEFINED;
    VkImageLayout            textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSampleCountFlagBits    sampleCount = VK_SAMPLE_COUNT_1_BIT;
    ColorChannelUsed         colorChannels = ColorChannelUsed::ChannelRGBA;

    nvtt::Format             ExportFormat;
    nvtt::Quality            ExportQuality;
};

class TextureBakerSystem
{
public:
    static TextureBakerSystem& Get();

private:
    TextureBakerSystem() = default;
    ~TextureBakerSystem() = default;
    TextureBakerSystem(const TextureBakerSystem&) = delete;
    TextureBakerSystem& operator=(const TextureBakerSystem&) = delete;
    TextureBakerSystem(TextureBakerSystem&&) = delete;
    TextureBakerSystem& operator=(TextureBakerSystem&&) = delete;

    VmaAllocator allocator = VK_NULL_HANDLE;;
    VmaAllocation stagingAlloc = VK_NULL_HANDLE;

    VkBuffer stagingBuffer = VK_NULL_HANDLE;

    void* ConvertToRawTextureData(ImportTexture& inputTexture);
    void  DestroyRawTextureBuffer();
public:
    DLL_EXPORT void BakeTexture(const String& textureName, ImportTexture& texture);
};

extern DLL_EXPORT TextureBakerSystem& textureBakerSystem;
inline TextureBakerSystem& TextureBakerSystem::Get()
{
    static TextureBakerSystem instance;
    return instance;
}
