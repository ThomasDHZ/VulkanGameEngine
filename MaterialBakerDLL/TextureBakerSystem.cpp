#define STB_IMAGE_WRITE_IMPLEMENTATION
#define FMT_HEADER_ONLY

#include "TextureBakerSystem.h"
#include <TextureSystem.h>
#include <VulkanSystem.h>
#include <BufferSystem.h>
#include <algorithm>
#include <ktx/include/ktx.h>
#include <ktx/include/ktxvulkan.h>
#include <thread>
#include <fstream>
#include <filesystem>
#include <fmt/format.h>
#include "stb_image_write.h"
#include <lodepng.h>
#include "bc7enc.h"  // <-- your latest bc7enc.h from richgel999/bc7enc_rdo
#include <shellapi.h>   // ShellExecuteA
#include <windows.h>    // Sleep, etc.

namespace fs = std::filesystem;

TextureBakerSystem& textureBakerSystem = TextureBakerSystem::Get();

namespace {
    struct BC7InitGuard {
        BC7InitGuard() {
            bc7enc_compress_block_init();
            printf("bc7enc tables initialized\n");
        }
    } g_bc7InitGuard;
}

// Forward declarations
std::vector<uint8_t> ConvertMipToRGBA8(const void* rawData, size_t rawSize,
    uint32_t width, uint32_t height,
    VkFormat srcFormat);

void TextureBakerSystem::BakeTexture(const String& baseFilePath, VkGuid renderPassId)
{
    Vector<Texture> attachmentTextureList = textureSystem.FindRenderedTextureList(renderPassId);
    if (attachmentTextureList.empty())
    {
        fprintf(stderr, "No rendered textures found for render pass %s\n", renderPassId.ToString().c_str());
        return;
    }

    const char* nvtt_exe = R"(C:\Program Files\NVIDIA Corporation\NVIDIA Texture Tools\nvtt_export.exe)";

    for (size_t x = 0; x < attachmentTextureList.size(); ++x)
    {
        Texture importTexture = attachmentTextureList[x];
        VkFormat srcFormat = importTexture.textureByteFormat;

        bool isNormalMap = (x == 1) ||
            srcFormat == VK_FORMAT_R16G16_UNORM ||
            srcFormat == VK_FORMAT_R16G16_SNORM ||
            srcFormat == VK_FORMAT_R16G16B16A16_SNORM ||
            srcFormat == VK_FORMAT_R8G8_SNORM;

        bool useSRGB = (x == 0) ||
            srcFormat == VK_FORMAT_R8G8B8A8_SRGB ||
            srcFormat == VK_FORMAT_B8G8R8A8_SRGB ||
            srcFormat == VK_FORMAT_A8B8G8R8_SRGB_PACK32;

        bool isHDRFloat = (srcFormat == VK_FORMAT_R16G16B16A16_SFLOAT || srcFormat == VK_FORMAT_R32G32B32A32_SFLOAT);
        if (isHDRFloat)
        {
            printf("Warning: HDR float format on attachment %zu — clamped to 0-1 for BC7\n", x);
        }

        // Step 1: Read and convert all mips to RGBA8 (unchanged)
        std::vector<std::vector<uint8_t>> allMipRGBA8(importTexture.mipMapLevels);
        bool readSuccess = true;

        for (uint32_t mip = 0; mip < importTexture.mipMapLevels && readSuccess; ++mip)
        {
            RawMipReadback raw = ConvertToRawTextureData(importTexture, mip);
            if (!raw.data || raw.size == 0)
            {
                fprintf(stderr, "Failed to read mip %u\n", mip);
                DestroyVMATextureBuffer(raw);
                readSuccess = false;
                break;
            }

            uint32_t mipWidth = std::max(1u, static_cast<uint32_t>(importTexture.width) >> mip);
            uint32_t mipHeight = std::max(1u, static_cast<uint32_t>(importTexture.height) >> mip);

            std::vector<uint8_t> rgba8Data = ConvertMipToRGBA8(raw.data, raw.size, mipWidth, mipHeight, srcFormat);
            if (rgba8Data.empty())
            {
                fprintf(stderr, "ConvertMipToRGBA8 failed for mip %u\n", mip);
                DestroyVMATextureBuffer(raw);
                readSuccess = false;
                break;
            }

            allMipRGBA8[mip] = std::move(rgba8Data);
            DestroyVMATextureBuffer(raw);
        }

        if (!readSuccess)
        {
            continue;
        }

        // Naming
        String suffix;
        if (x == 0)      suffix = "_Albedo";
        else if (x == 1) suffix = "_NormalHeight";
        else if (x == 2) suffix = "_MRO";
        else if (x == 3) suffix = "_SheenSSS";
        else if (x == 4) suffix = "_Unused";
        else if (x == 5) suffix = "_Emission";
        else             suffix = "_Attachment" + std::to_string(x);

        String baseName = suffix.substr(1);  // Albedo, NormalHeight, etc.

        // Step 2: Export PNG preview (mip 0) directly to Assets\Textures
        String search = "Import";
        String textureName = baseFilePath;
        size_t find = baseFilePath.find(search);
        if (find != std::string::npos) 
        {
            textureName.replace(find, search.size(), ""); // Replace with desired string
        }
        fs::path previewPngPath = textureName + suffix + ".png";
        ExportToPng(previewPngPath.string(), importTexture, 0, false);  // mip 0 for preview

        // Step 3: Compress to KTX2 using NVTT (input = preview PNG, output = same folder)
        fs::path ktxPath = textureName + suffix + ".ktx2";
        std::string args = fmt::format( "\"{}\" -o \"{}\" --format bc7 --quality normal --mips --mip-filter box --zcmp 1",  previewPngPath.string(), ktxPath.string());

        if (useSRGB)
        {
            args += " --export-transfer-function srgb";
        }
        else
        {
            args += " --export-transfer-function linear";
        }
        if (isNormalMap) {
            args += " --normal-alpha unchanged";
        }

        // Optional: Maximize CPU threads (good since no CUDA)
        _putenv("OMP_NUM_THREADS=24");  // Adjust to your CPU core count

        printf("Launching NVTT with args:\n%s\n", args.c_str());

        HINSTANCE hInst = ShellExecuteA(
            NULL,
            "open",
            nvtt_exe,
            args.c_str(),
            NULL,
            SW_HIDE  // Change to SW_SHOW temporarily if you want to see NVTT console
        );

        if ((intptr_t)hInst <= 32) {
            fprintf(stderr, "ShellExecuteA failed with code %ld\n", (intptr_t)hInst);
        }
        else {
            printf("NVTT launched successfully\n");

            // Poll for valid file
            bool fileCreated = false;
            for (int i = 0; i < 60; ++i) {
                if (fs::exists(ktxPath) && fs::file_size(ktxPath) > 1024) {  // >1KB
                    fileCreated = true;
                    printf("NVTT succeeded (file detected, size %llu bytes) ? %s\n",
                        fs::file_size(ktxPath), ktxPath.string().c_str());
                    break;
                }
                Sleep(1000);
            }

            if (!fileCreated) {
                fprintf(stderr, "Timeout: KTX2 file not created or empty\n");
            }
        }

        printf("NVTT launched (ShellExecuteA success)\n");

        // Poll for output file (wait up to 60 seconds)
        bool fileCreated = false;
        for (int i = 0; i < 60; ++i)
        {
            if (fs::exists(ktxPath))
            {
                fileCreated = true;
                printf("NVTT succeeded (file detected) ? %s\n", ktxPath.string().c_str());
                break;
            }
            Sleep(1000);
        }

        if (!fileCreated)
        {
            fprintf(stderr, "Timeout: KTX2 file not created after 60 seconds\n");
        }

        // Optional: quick KTX2 verification
        if (fileCreated)
        {
            ktxTexture2* verify = nullptr;
            KTX_error_code err = ktxTexture2_CreateFromNamedFile(ktxPath.string().c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &verify);
            if (err == KTX_SUCCESS && verify)
            {
                printf("KTX2 verified: supercompression = %s, levels = %u, format = %u\n",
                    (verify->supercompressionScheme == KTX_SS_ZSTD ? "ZSTD" : "None"),
                    verify->numLevels,
                    verify->vkFormat);
                ktxTexture2_Destroy(verify);
            }
            else
            {
                fprintf(stderr, "KTX2 verification failed: %s\n", ktxErrorString(err));
            }
        }
    }
}

// ?? ExportToPng (your improved version — full) ???????????????????????????????????????

void TextureBakerSystem::ExportToPng(const String& fileName, Texture& texture, uint32_t mipLevel, bool flipY)
{
    if (mipLevel >= texture.mipMapLevels) {
        std::cerr << "Invalid mip level " << mipLevel << " (max " << texture.mipMapLevels << ")\n";
        return;
    }

    VmaAllocator allocator = bufferSystem.vmaAllocator;

    uint32_t width = std::max(1u, static_cast<uint32_t>(texture.width) >> mipLevel);
    uint32_t height = std::max(1u, static_cast<uint32_t>(texture.height) >> mipLevel);

    size_t bytesPerPixel = 4;
    bool is16Bit = false;
    bool is32BitFloat = false;

    if (texture.textureByteFormat == VK_FORMAT_R32G32B32A32_SFLOAT ||
        texture.textureByteFormat == VK_FORMAT_R32G32B32A32_UINT ||
        texture.textureByteFormat == VK_FORMAT_R32G32B32A32_SINT) {
        bytesPerPixel = 16;
        is32BitFloat = true;
        std::cerr << "32-bit float formats not supported for PNG export yet\n";
        return;
    }
    else if (texture.textureByteFormat >= VK_FORMAT_R16G16B16A16_UNORM &&
        texture.textureByteFormat <= VK_FORMAT_R16G16B16A16_SFLOAT) {
        bytesPerPixel = 8;
        is16Bit = true;
    }

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
        .oldLayout = texture.textureImageLayout,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = texture.textureImage,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = mipLevel,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    if (texture.textureImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    else if (texture.textureImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    else if (texture.textureImageLayout == VK_IMAGE_LAYOUT_GENERAL)
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    else
        barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

    VkCommandBuffer cmd = vulkanSystem.BeginSingleUseCommand();
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier);

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = static_cast<VkDeviceSize>(width) * height * bytesPerPixel,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocInfo = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo allocInfoOut{};
    VULKAN_THROW_IF_FAIL(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAlloc, &allocInfoOut));

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = mipLevel,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {width, height, 1}
    };

    vkCmdCopyImageToBuffer(cmd, texture.textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        stagingBuffer, 1, &region);

    vulkanSystem.EndSingleUseCommand(cmd);

    void* mapped = allocInfoOut.pMappedData;
    bool needsUnmap = false;
    if (!mapped) {
        VULKAN_THROW_IF_FAIL(vmaMapMemory(allocator, stagingAlloc, &mapped));
        needsUnmap = true;
    }

    std::vector<unsigned char> pngData;

    if (!is16Bit) {
        // 8-bit RGBA
        std::vector<uint8_t> pixels(width * height * 4);

        const uint8_t* src = static_cast<const uint8_t*>(mapped);
        uint8_t* dst = pixels.data();

        for (uint32_t y = 0; y < height; ++y) {
            uint32_t srcY = flipY ? (height - 1 - y) : y;
            const uint8_t* srcRow = src + srcY * width * 4;
            uint8_t* dstRow = dst + y * width * 4;
            memcpy(dstRow, srcRow, width * 4);
        }

        unsigned error = lodepng::encode(pngData, pixels.data(), width, height, LCT_RGBA, 8);
        if (error) std::cerr << "lodepng encode error (8-bit): " << lodepng_error_text(error) << "\n";
    }
    else {
        // 16-bit RGBA (PNG big-endian)
        std::vector<uint16_t> pixels(width * height * 4);

        const uint16_t* src = static_cast<const uint16_t*>(mapped);
        uint16_t* dst = pixels.data();

        for (uint32_t y = 0; y < height; ++y) {
            uint32_t srcY = flipY ? (height - 1 - y) : y;
            const uint16_t* srcRow = src + srcY * width * 4;
            uint16_t* dstRow = dst + y * width * 4;

            for (uint32_t x = 0; x < width * 4; ++x) {
                uint16_t val = srcRow[x];
                dstRow[x] = ((val >> 8) & 0xFF) | ((val & 0xFF) << 8);
            }
        }

        unsigned error = lodepng::encode(pngData,
            reinterpret_cast<unsigned char*>(pixels.data()),
            width, height, LCT_RGBA, 16);
        if (error) std::cerr << "lodepng encode error (16-bit): " << lodepng_error_text(error) << "\n";
    }

    if (!pngData.empty()) {
        unsigned saveError = lodepng::save_file(pngData, fileName);
        if (saveError) {
            std::cerr << "Failed to save PNG: " << lodepng_error_text(saveError) << "\n";
        }
        else {
            std::cout << "Exported PNG: " << fileName << "\n";
        }
    }

    if (needsUnmap) vmaUnmapMemory(allocator, stagingAlloc);
    vmaDestroyBuffer(allocator, stagingBuffer, stagingAlloc);
}

// ?? Your existing helper functions (unchanged) ??????????????????????????????????

std::vector<uint8_t> ConvertMipToRGBA8(const void* rawData, size_t rawSize,
    uint32_t width, uint32_t height,
    VkFormat srcFormat)
{
    std::vector<uint8_t> rgba8(width * height * 4);

    size_t bytesPerPixel = 4;
    if (srcFormat == VK_FORMAT_R32G32B32A32_SFLOAT ||
        srcFormat == VK_FORMAT_R32G32B32A32_UINT ||
        srcFormat == VK_FORMAT_R32G32B32A32_SINT)
    {
        bytesPerPixel = 16;
    }
    else if (srcFormat >= VK_FORMAT_R16G16B16A16_UNORM &&
        srcFormat <= VK_FORMAT_R16G16B16A16_SFLOAT)
    {
        bytesPerPixel = 8;
    }

    size_t expectedSize = static_cast<size_t>(width) * height * bytesPerPixel;
    if (rawSize != expectedSize)
    {
        fprintf(stderr, "ConvertMipToRGBA8: size mismatch (expected %zu, got %zu)\n", expectedSize, rawSize);
        return {};
    }

    if (bytesPerPixel == 4)
    {
        memcpy(rgba8.data(), rawData, rawSize);
    }
    else if (bytesPerPixel == 8)
    {
        const uint16_t* src16 = static_cast<const uint16_t*>(rawData);
        for (size_t p = 0; p < static_cast<size_t>(width) * height; ++p)
        {
            for (int c = 0; c < 4; ++c)
            {
                uint16_t val = src16[p * 4 + c];
                float norm = 0.0f;

                if (srcFormat == VK_FORMAT_R16G16B16A16_UNORM ||
                    srcFormat == VK_FORMAT_R16G16B16A16_UINT)
                {
                    norm = static_cast<float>(val) / 65535.0f;
                }
                else if (srcFormat == VK_FORMAT_R16G16B16A16_SNORM)
                {
                    int16_t s = static_cast<int16_t>(val);
                    norm = std::max(static_cast<float>(s) / 32767.0f, -1.0f);
                    norm = norm * 0.5f + 0.5f;
                }
                else if (srcFormat == VK_FORMAT_R16G16B16A16_SFLOAT)
                {
                    uint32_t sign = (val >> 15) & 0x01;
                    uint32_t exp = (val >> 10) & 0x1F;
                    uint32_t mant = val & 0x3FF;
                    float fval;
                    if (exp == 0)
                        fval = (mant / 1024.0f) * std::pow(2.0f, -14);
                    else if (exp == 31)
                        fval = mant ? NAN : (sign ? -INFINITY : INFINITY);
                    else
                        fval = ((1.0f + mant / 1024.0f) * std::pow(2.0f, exp - 15));
                    if (sign) fval = -fval;
                    norm = std::clamp(fval, 0.0f, 1.0f);
                }

                rgba8[p * 4 + c] = static_cast<uint8_t>(norm * 255.0f + 0.5f);
            }
        }
    }
    else if (bytesPerPixel == 16)
    {
        const float* src32 = static_cast<const float*>(rawData);
        for (size_t p = 0; p < static_cast<size_t>(width) * height; ++p)
        {
            for (int c = 0; c < 4; ++c)
            {
                float val = src32[p * 4 + c];
                val = std::clamp(val, 0.0f, 1.0f);
                rgba8[p * 4 + c] = static_cast<uint8_t>(val * 255.0f + 0.5f);
            }
        }
    }

    return rgba8;
}

// Updated CompressToBC7Ultra using latest bc7enc API
std::vector<uint8_t> TextureBakerSystem::CompressToBC7Ultra(
    const uint8_t* rgbaData,
    uint32_t width,
    uint32_t height,
    bool perceptual,
    bool isNormalMap)
{
    if (width == 0 || height == 0 || rgbaData == nullptr) return {};

    uint32_t blocksX = (width + 3) / 4;
    uint32_t blocksY = (height + 3) / 4;
    size_t numBlocks = static_cast<size_t>(blocksX) * blocksY;

    std::vector<uint8_t> bc7Blocks(numBlocks * 16);

    // Create params struct (required by your bc7enc.h)
    bc7enc_compress_block_params params = {};
     
    // Fill in the fields your version actually supports/uses
    params.m_perceptual = perceptual && !isNormalMap;  // perceptual for color/albedo, linear for normals
    params.m_uber_level = 6;               // highest (very slow, closest to NTTE)
    params.m_max_partitions = 64;              // already max
    params.m_mode_mask = 0xFF;            // all modes 0-7
    params.m_try_least_squares = true;
    params.m_perceptual = perceptual && !isNormalMap;
    // If your bc7enc version has these (check header):
    
    // Optional fields (safe to leave default/zero)
    params.m_low_frequency_partition_weight = 1.0f;
    params.m_mode6_error_weight = 1.0f;
    params.m_mode5_error_weight = 1.0f;
    params.m_mode1_error_weight = 1.0f;

    // Optional one-time init (only if your header declares this function)
    // bc7enc_compress_block_init();

    for (uint32_t by = 0; by < blocksY; ++by)
    {
        for (uint32_t bx = 0; bx < blocksX; ++bx)
        {
            uint8_t blockRGBA[64];

            for (int y = 0; y < 4; ++y)
            {
                for (int x = 0; x < 4; ++x)
                {
                    int px = bx * 4 + x;
                    int py = by * 4 + y;
                    int idx = (y * 4 + x) * 4;

                    if (px < static_cast<int>(width) && py < static_cast<int>(height))
                    {
                        size_t srcIdx = (py * width + px) * 4;
                        blockRGBA[idx + 0] = rgbaData[srcIdx + 0];
                        blockRGBA[idx + 1] = rgbaData[srcIdx + 1];
                        blockRGBA[idx + 2] = rgbaData[srcIdx + 2];
                        blockRGBA[idx + 3] = rgbaData[srcIdx + 3];
                    }
                    else
                    {
                        blockRGBA[idx + 0] = 0;
                        blockRGBA[idx + 1] = 0;
                        blockRGBA[idx + 2] = 0;
                        blockRGBA[idx + 3] = 255;  // opaque
                    }
                }
            }

            uint8_t* pDstBlock = &bc7Blocks[(by * blocksX + bx) * 16];

            // Now pass &params (fixes the type mismatch)
            printf("Compressing block %u,%u -> %p\n", bx, by, pDstBlock);
            bc7enc_compress_block(pDstBlock, blockRGBA, &params);
        }
    }

    return bc7Blocks;
}

RawMipReadback TextureBakerSystem::ConvertToRawTextureData(Texture& importTexture, uint32 mipLevel)
{
    VmaAllocator allocator = bufferSystem.vmaAllocator;
    uint32 mipWidth = std::max(1u, static_cast<uint32>(importTexture.width) >> mipLevel);
    uint32 mipHeight = std::max(1u, static_cast<uint32>(importTexture.height) >> mipLevel);

    size_t bytesPerPixel = 4;
    if (importTexture.textureByteFormat == VK_FORMAT_R32G32B32A32_SFLOAT ||
        importTexture.textureByteFormat == VK_FORMAT_R32G32B32A32_UINT ||
        importTexture.textureByteFormat == VK_FORMAT_R32G32B32A32_SINT) {
        bytesPerPixel = 16;
    }
    else if (importTexture.textureByteFormat >= VK_FORMAT_R16G16B16A16_UNORM &&
        importTexture.textureByteFormat <= VK_FORMAT_R16G16B16A16_SFLOAT)
    {
        bytesPerPixel = 8;
    }

    VkBufferCreateInfo bufferInfo =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = static_cast<VkDeviceSize>(mipWidth) * mipHeight * bytesPerPixel,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocInfo =
    {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VmaAllocationInfo allocOut{};
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingAlloc = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAlloc, &allocOut));

    VkBufferImageCopy region =
    {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = mipLevel,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .imageOffset = { 0, 0, 0 },
        .imageExtent =
            {
                static_cast<uint32_t>(mipWidth),
                static_cast<uint32_t>(mipHeight),
                1
            }
    };

    VkImageMemoryBarrier barrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
        .oldLayout = importTexture.textureImageLayout,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = importTexture.textureImage,
        .subresourceRange =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = mipLevel,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    if (importTexture.textureImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    else if (importTexture.textureImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    else if (importTexture.textureImageLayout == VK_IMAGE_LAYOUT_GENERAL) barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    else barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

    VkCommandBuffer command = vulkanSystem.BeginSingleUseCommand();
    vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    vkCmdCopyImageToBuffer(command, importTexture.textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);
    vulkanSystem.EndSingleUseCommand(command);

    bool needsUnmap = false;
    void* mappedData = allocOut.pMappedData;
    if (!mappedData)
    {
        VULKAN_THROW_IF_FAIL(vmaMapMemory(allocator, stagingAlloc, &mappedData));
        needsUnmap = true;
    }

    return RawMipReadback
    {
        .data = mappedData,
        .size = static_cast<VkDeviceSize>(mipWidth) * mipHeight * bytesPerPixel,
        .buffer = stagingBuffer,
        .allocation = stagingAlloc,
        .needsUnmap = needsUnmap,
    };
}

void TextureBakerSystem::DestroyVMATextureBuffer(RawMipReadback& data)
{
    if (data.needsUnmap)
    {
        vmaUnmapMemory(bufferSystem.vmaAllocator, data.allocation);
    }
    vmaDestroyBuffer(bufferSystem.vmaAllocator, data.buffer, data.allocation);
}
