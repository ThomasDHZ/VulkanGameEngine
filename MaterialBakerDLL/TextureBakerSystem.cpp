#include "TextureBakerSystem.h"
#include <TextureSystem.h>
#include <VulkanSystem.h>
#include <BufferSystem.h>
#include <ktx.h>
#include <ktxvulkan.h>
#include <algorithm>
#include "basisu_comp.h"        
#include "basisu_frontend.h"
#include "basisu_gpu_texture.h"   
#include "basisu_enc.h"        
#include "C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\External\bc7enc_rdo1/bc7enc.h"
#include "C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\External\bc7enc_rdo1/rgbcx.h"
#include <thread>

TextureBakerSystem& textureBakerSystem = TextureBakerSystem::Get();

void TextureBakerSystem::BakeTexture(const String& baseFilePath, VkGuid renderPassId)
{
    Vector<Texture> attachmentTextureList = textureSystem.FindRenderedTextureList(renderPassId);
    if (attachmentTextureList.empty())
    {
        fprintf(stderr, "No rendered textures found for render pass %s\n", renderPassId.ToString());
        return;
    }

    for (size_t x = 0; x < attachmentTextureList.size(); ++x)
    {
        Texture importTexture = attachmentTextureList[x];

        // Auto-detect per-attachment settings
        bool isNormalMap = false;
        bool useSRGB = false;
        bool isHDRFloat = false;

        VkFormat srcFormat = importTexture.textureByteFormat;

        // Normal map detection
        if (x == 1 ||  // normalData attachment
            srcFormat == VK_FORMAT_R16G16_UNORM ||
            srcFormat == VK_FORMAT_R16G16_SNORM ||
            srcFormat == VK_FORMAT_R16G16B16A16_SNORM ||
            srcFormat == VK_FORMAT_R8G8_SNORM)
        {
            isNormalMap = true;
        }

        // SRGB for albedo
        if (x == 0 ||
            srcFormat == VK_FORMAT_R8G8B8A8_SRGB ||
            srcFormat == VK_FORMAT_B8G8R8A8_SRGB ||
            srcFormat == VK_FORMAT_A8B8G8R8_SRGB_PACK32)
        {
            useSRGB = true;
        }

        // HDR float detection (emission)
        if (srcFormat == VK_FORMAT_R16G16B16A16_SFLOAT ||
            srcFormat == VK_FORMAT_R32G32B32A32_SFLOAT)
        {
            isHDRFloat = true;
            printf("Warning: HDR float format on attachment %zu — converting to UNORM 0-1 for BC7 (potential precision loss)\n", x);
        }

        VkFormat targetFormat = (useSRGB && !isNormalMap) ? VK_FORMAT_BC7_SRGB_BLOCK : VK_FORMAT_BC7_UNORM_BLOCK;

        ktxTextureCreateInfo createInfo
        {
            .vkFormat = static_cast<ktx_uint32_t>(targetFormat),
            .baseWidth = static_cast<ktx_uint32_t>(importTexture.width),
            .baseHeight = static_cast<ktx_uint32_t>(importTexture.height),
            .baseDepth = static_cast<ktx_uint32_t>(importTexture.depth ? importTexture.depth : 1),
            .numDimensions = 2,
            .numLevels = static_cast<ktx_uint32_t>(importTexture.mipMapLevels),
            .numLayers = 1,
            .numFaces = 1,
            .isArray = KTX_FALSE,
            .generateMipmaps = KTX_FALSE
        };

        ktxTexture2* ktx = nullptr;
        KTX_error_code err = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &ktx);
        if (err != KTX_SUCCESS)
        {
            fprintf(stderr, "ktxTexture2_Create failed for attachment %zu: %s\n", x, ktxErrorString(err));
            continue;
        }

        bool highQuality = true;

        bool success = true;
        ktx_size_t cumulativeOffset = 0;
        for (uint32_t mip = 0; mip < importTexture.mipMapLevels; ++mip)
        {
            RawMipReadback raw = ConvertToRawTextureData(importTexture, mip);
            if (!raw.data || raw.size == 0)
            {
                fprintf(stderr, "Failed to read mip %u data for attachment %zu\n", mip, x);
                DestroyVMATextureBuffer(raw);
                success = false;
                break;
            }

            uint32_t mipWidth = std::max(1u, static_cast<uint32_t>(importTexture.width) >> mip);
            uint32_t mipHeight = std::max(1u, static_cast<uint32_t>(importTexture.height) >> mip);

            // Compute correct bytes per pixel from srcFormat (matches ConvertToRawTextureData logic)
            size_t bytesPerPixel = 4; // default RGBA8
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

            size_t expectedBytes = static_cast<size_t>(mipWidth) * mipHeight * bytesPerPixel;
            if (raw.size != expectedBytes)
            {
                fprintf(stderr, "Mip %u size mismatch for attachment %zu (expected %zu bytes, got %zu)\n",
                    mip, x, expectedBytes, raw.size);
                DestroyVMATextureBuffer(raw);
                success = false;
                break;
            }

            // Convert to temporary RGBA8 buffer for BC7 (which requires uint8 RGBA input)
            std::vector<uint8_t> rgba8Data(mipWidth * mipHeight * 4);
            if (bytesPerPixel == 4)
            {
                // Already RGBA8 — direct copy
                memcpy(rgba8Data.data(), raw.data, raw.size);
            }
            else if (bytesPerPixel == 8)
            {
                // 16-bit formats (UNORM/SNORM/SFLOAT half) ? convert to uint8 0-255
                const uint16_t* src16 = static_cast<const uint16_t*>(raw.data);
                for (size_t p = 0; p < mipWidth * mipHeight; ++p)
                {
                    for (int c = 0; c < 4; ++c)
                    {
                        float norm = 0.0f;
                        if (srcFormat == VK_FORMAT_R16G16B16A16_UNORM ||
                            srcFormat == VK_FORMAT_R16G16B16A16_UINT)
                        {
                            norm = static_cast<float>(src16[p * 4 + c]) / 65535.0f;
                        }
                        else if (srcFormat == VK_FORMAT_R16G16B16A16_SNORM)
                        {
                            norm = std::max(static_cast<float>(static_cast<int16_t>(src16[p * 4 + c])) / 32767.0f, -1.0f);
                            norm = norm * 0.5f + 0.5f; // -1..1 ? 0..1
                        }
                        else if (srcFormat == VK_FORMAT_R16G16B16A16_SFLOAT)
                        {
                            norm = astc_helpers::half_to_float(src16[p * 4 + c]); // need half-float conversion
                            norm = std::clamp(norm, 0.0f, 1.0f);
                        }
                        rgba8Data[p * 4 + c] = static_cast<uint8_t>(norm * 255.0f + 0.5f);
                    }
                }
            }
            else if (bytesPerPixel == 16)
            {
                // 32-bit float ? clamp 0-1
                const float* src32 = static_cast<const float*>(raw.data);
                for (size_t p = 0; p < mipWidth * mipHeight; ++p)
                {
                    for (int c = 0; c < 4; ++c)
                    {
                        float val = src32[p * 4 + c];
                        val = std::clamp(val, 0.0f, 1.0f); // simple clamp (add tonemap later if needed)
                        rgba8Data[p * 4 + c] = static_cast<uint8_t>(val * 255.0f + 0.5f);
                    }
                }
            }

            std::vector<uint8_t> compressed = CompressToBC7(rgba8Data.data(), rgba8Data.size(), mipWidth, mipHeight, isNormalMap, highQuality);
            if (compressed.empty())
            {
                fprintf(stderr, "BC7 compression failed for attachment %zu mip %u\n", x, mip);
                DestroyVMATextureBuffer(raw);
                success = false;
                break;
            }

            ktx_size_t levelSize = ktxTexture_GetImageSize(ktxTexture(ktx), mip);
            if (compressed.size() != levelSize)
            {
                fprintf(stderr, "Warning: BC7 size mismatch for attachment %zu mip %u (%zu vs expected %zu)\n",
                    x, mip, compressed.size(), levelSize);
            }

            memcpy(ktx->pData + cumulativeOffset, compressed.data(), compressed.size());
            cumulativeOffset += levelSize;

            DestroyVMATextureBuffer(raw);
        }

        if (!success)
        {
            ktxTexture_Destroy(ktxTexture(ktx));
            continue;
        }

        String suffix;
        if (x == 0) suffix = "_Albedo";
        else if (x == 1) suffix = "_NormalHeight";
        else if (x == 2) suffix = "_MRO";
        else if (x == 3) suffix = "_SheenSSS";
        else if (x == 4) suffix = "_Unused";
        else if (x == 5) suffix = "_Emission";
        else suffix = "_Attachment" + std::to_string(x);

        String fullPath = baseFilePath + suffix + ".ktx2";

        err = ktxTexture_WriteToNamedFile(ktxTexture(ktx), fullPath.c_str());
        if (err == KTX_SUCCESS)
        {
            printf("Successfully baked BC7 KTX2: %s\n", fullPath.c_str());
        }
        else
        {
            fprintf(stderr, "Failed to write KTX2 %s: %s\n", fullPath.c_str(), ktxErrorString(err));
        }

        ktxTexture_Destroy(ktxTexture(ktx));
    }
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

std::vector<uint8_t> TextureBakerSystem::CompressToBC7(const void* rgbaData, size_t sizeBytes, uint32_t width, uint32_t height, bool isNormalMap, bool highQuality)
{
    // Critical: Initialize the encoder's internal tables (fixes the assertion on g_bc7_mode_1_optimal_endpoints)
      // This must be called once before any compression (static ensures it runs only once)
    static bool tables_initialized = false;
    if (!tables_initialized)
    {
        bc7enc_compress_block_init();  // Exact function name from bc7enc lib — initializes Mode 1/7 tables etc.
        tables_initialized = true;
    }

    if (sizeBytes != static_cast<size_t>(width) * height * 4) {
        fprintf(stderr, "Size mismatch\n");
        return {};
    }
    uint32_t blocksX = (width + 3) / 4;
    uint32_t blocksY = (height + 3) / 4;
    size_t blockCount = blocksX * blocksY;
    size_t outputSize = blockCount * 16; // 16 bytes per BC7 block
    std::vector<uint8_t> compressed(outputSize);
    bc7enc_compress_block_params params;
    memset(&params, 0, sizeof(params));
    bc7enc_compress_block_params_init(&params); // Sets defaults: all modes, perceptual weights, uber 0
    // Quality: Higher uber level = slower + significantly better quality (5 is the practical "ultra" max)
    params.m_uber_level = highQuality ? 5 : 1; // 1-2 for good/fast, 5 for max quality/slow
    // For normal maps: Switch to linear RGB metrics (critical for avoiding artifacts)
    if (isNormalMap) {
        bc7enc_compress_block_params_init_linear_weights(&params);
        // Optional tweak: Boost RG weights if you reconstruct Z (common for normals)
        // params.m_weights[0] = 2; // R/X
        // params.m_weights[1] = 2; // G/Y
        // params.m_weights[2] = 0; // B/Z (ignored)
        // params.m_weights[3] = 0; // A (usually 255, ignored)
    }
    // Optional advanced tweaks (uncomment for even better results)
    // params.m_try_least_squares = true; // Already true in init
    // params.m_mode17_partition_estimation_filterbank = true; // Already true
    // params.m_low_frequency_partition_weight = 2.0f; // Helps smooth areas
    const uint8_t* src = static_cast<const uint8_t*>(rgbaData);
    uint8_t* dst = compressed.data();
    for (uint32_t by = 0; by < blocksY; ++by) {
        for (uint32_t bx = 0; bx < blocksX; ++bx) {
            uint8_t blockPixels[64]; // 4x4x4 RGBA temp buffer
            // Extract/clamp 4x4 block
            for (uint32_t y = 0; y < 4; ++y) {
                for (uint32_t x = 0; x < 4; ++x) {
                    uint32_t px = bx * 4 + x;
                    uint32_t py = by * 4 + y;
                    if (px >= width) px = width - 1;
                    if (py >= height) py = height - 1;
                    uint32_t idx = (py * width + px) * 4;
                    memcpy(blockPixels + (y * 16 + x * 4), src + idx, 4);
                }
            }
            bc7enc_compress_block(dst, blockPixels, &params);
            dst += 16;
        }
    }
    // Optional: Apply Entropy Reduction Transform (ERT) post-process for 5-15% better LZ/Zstd sizes
    // Include "ert.h" and call: ert_process(compressed.data(), compressed.size());
    return compressed;
}