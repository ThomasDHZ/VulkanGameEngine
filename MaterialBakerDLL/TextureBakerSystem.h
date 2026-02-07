#pragma once
#include <DLL.h>
#include <TextureSystem.h>
#include <MaterialSystem.h>

enum class TextureCompressionType {
    None,          // Raw/uncompressed (large files)
    BC7,           // High-quality RGBA, desktop primary
    BC1,           // RGB (DXT1/S3TC), no alpha or punch-through
    BC3,           // RGBA (DXT5), with alpha
    BC5,           // RG (normals, height maps)
    ASTC_4x4,      // Mobile/high-end, variable block size
    ETC2_RGBA,     // Broad mobile support
    // Add more: PVRTC, etc. if needed
};

struct RawMipReadback
{
    void* data = nullptr;
    size_t size = 0;
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    bool needsUnmap = false;
};

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

    RawMipReadback ConvertToRawTextureData(Texture& importTexture, uint32 mipLevel);
    void           DestroyVMATextureBuffer(RawMipReadback& data);
    std::vector<uint8_t> CompressToBC7(
        const void* rgbaData,
        size_t sizeBytes,
        uint32_t width,
        uint32_t height,
        bool isNormalMap = false,
        bool highQuality = false);
public:
    DLL_EXPORT void BakeTexture(const String& baseFilePath, VkGuid renderPassId);
};

extern DLL_EXPORT TextureBakerSystem& textureBakerSystem;
inline TextureBakerSystem& TextureBakerSystem::Get()
{
    static TextureBakerSystem instance;
    return instance;
}
