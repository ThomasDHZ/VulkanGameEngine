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

void TextureBakerSystem::BakeTexture(const String& textureName, Texture& importTexture)
{
    using namespace basisu;

    static bool basis_init = false;
    if (!basis_init)
    {
        basisu_encoder_init();
        basis_init = true;
    }

    basisu::vector<basisu::image> source_images;
    for (uint32_t mip = 0; mip < importTexture.mipMapLevels; ++mip)
    {
        RawMipReadback raw = ConvertToRawTextureData(importTexture, mip);
        if (!raw.data || raw.size == 0) {
            DestroyVMATextureBuffer(raw);
            continue;
        }

        uint32 mipWidth = std::max(1u, static_cast<uint32>(importTexture.width) >> mip);
        uint32 mipHeight = std::max(1u, static_cast<uint32>(importTexture.height) >> mip);
        if (importTexture.colorChannels != 4)
        {
            fprintf(stderr, "Warning: BasisU expects RGBA (4 channels), but got %u for mip %u. "
                "Conversion needed but not implemented.\n", importTexture.colorChannels, mip);
        }

        size_t expectedBytes = static_cast<size_t>(mipWidth) * mipHeight * 4;
        if (raw.size != expectedBytes) {
            fprintf(stderr, "Mip %u size mismatch (expected %zu bytes, got %zu)\n", mip, expectedBytes, raw.size);
            DestroyVMATextureBuffer(raw);
            continue;
        }

        basisu::image img(mipWidth, mipHeight);
        memcpy(img.get_ptr(), raw.data, expectedBytes);

        source_images.push_back(std::move(img));
        DestroyVMATextureBuffer(raw);
    }

    if (source_images.empty())
    {
        fprintf(stderr, "No valid mip levels for %s\n", textureName.c_str());
        return;
    }

    uint32_t num_threads = std::max(1u, std::thread::hardware_concurrency());
    basisu::job_pool jobPool(num_threads);  // MUST live until after compressor.process()!

    basis_compressor_params params;
    params.m_source_images = std::move(source_images);
    params.m_uastc = true;
    params.m_hdr = false;
    params.m_quality_level = 128;
    params.m_perceptual = true;
    params.m_ktx2_and_basis_srgb_transfer_function = true;

    // These two lines are critical for v2.0+ default behavior
    params.m_multithreading = true;          // Explicitly match default / force it
    params.m_pJob_pool = &jobPool;           // Required when threading active

    basis_compressor compressor;
    if (!compressor.init(params))
    {
        fprintf(stderr, "Basis init failed for %s\n", textureName.c_str());
        return;
    }
    else
    {
        printf("Basis compressor init succeeded (multithreaded, %u threads)\n", num_threads);
    }

    if (!compressor.process())
    {
        fprintf(stderr, "Basis process failed\n");
        return;
    }


    basisu::vector<byte> basis_data = compressor.get_output_basis_file();
    if (basis_data.empty()) {
        fprintf(stderr, "Empty Basis output\n");
        return;
    }

    ktxTextureCreateInfo createInfo
    {
        .vkFormat = VK_FORMAT_UNDEFINED,
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
    if (err != KTX_SUCCESS) {
        fprintf(stderr, "ktxTexture2_Create: %s\n", ktxErrorString(err));
        return;
    }
    ktx->supercompressionScheme = KTX_SS_ZSTD;

    ktx_size_t offset = 0;
    ktx_size_t levelSize = ktxTexture_GetImageSize(ktxTexture(ktx), 0);
    if (basis_data.size() > levelSize)
    {
        fprintf(stderr, "Basis compressed data too large for KTX level 0 (%zu > %zu)\n", basis_data.size(), levelSize);
        ktxTexture_Destroy(ktxTexture(ktx));
        return;
    }
    memcpy(ktx->pData + offset, basis_data.data(), basis_data.size());

    err = ktxTexture2_DeflateZstd(ktx, 22);
    if (err != KTX_SUCCESS)
    {
        fprintf(stderr, "Zstd supercompression failed: %s (file will be saved without supercompression)\n",
            ktxErrorString(err));
        ktx->supercompressionScheme = KTX_SS_NONE;
    }

    err = ktxTexture_WriteToNamedFile(ktxTexture(ktx), textureName.c_str());
    if (err == KTX_SUCCESS)
    {
        printf("Successfully baked Basis Universal + Zstd KTX2: %s\n", textureName.c_str());
    }
    else
    {
        fprintf(stderr, "Failed to write KTX2 file %s: %s\n", textureName.c_str(), ktxErrorString(err));
    }

    ktxTexture_Destroy(ktxTexture(ktx));
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


std::vector<uint8_t> CompressToBC7(
    const void* rgbaData,      // uint8_t* RGBA, tightly packed
    size_t sizeBytes,
    uint32_t width,
    uint32_t height,
    bool isNormalMap = false,
    bool highQuality = false)
{
    if (sizeBytes != static_cast<size_t>(width) * height * 4) {
        fprintf(stderr, "Size mismatch\n");
        return {};
    }

    uint32_t blocksX = (width + 3) / 4;
    uint32_t blocksY = (height + 3) / 4;
    size_t blockCount = blocksX * blocksY;
    size_t outputSize = blockCount * 16;  // 16 bytes per BC7 block
    std::vector<uint8_t> compressed(outputSize);

    bc7enc_compress_block_params params;
    memset(&params, 0, sizeof(params));
    bc7enc_compress_block_params_init(&params);  // Sets defaults: all modes, perceptual weights, uber 0

    // Quality: Higher uber level = slower + significantly better quality (5 is the practical "ultra" max)
    params.m_uber_level = highQuality ? 5 : 1;  // 1-2 for good/fast, 5 for max quality/slow

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
    // params.m_try_least_squares = true;                 // Already true in init
    // params.m_mode17_partition_estimation_filterbank = true;  // Already true
    // params.m_low_frequency_partition_weight = 2.0f;    // Helps smooth areas

    const uint8_t* src = static_cast<const uint8_t*>(rgbaData);
    uint8_t* dst = compressed.data();

    for (uint32_t by = 0; by < blocksY; ++by) {
        for (uint32_t bx = 0; bx < blocksX; ++bx) {
            uint8_t blockPixels[64];  // 4x4x4 RGBA temp buffer
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