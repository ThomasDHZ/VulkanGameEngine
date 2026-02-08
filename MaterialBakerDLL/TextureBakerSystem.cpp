#include "TextureBakerSystem.h"
#include <TextureSystem.h>
#include <VulkanSystem.h>
#include <BufferSystem.h>
#include <algorithm>
#include "basisu_transcoder.h"  // For basist:: namespace (transcoder) - ADD THIS INCLUDE
#include <ktx/include/ktx.h>
#include <ktx/include/ktxvulkan.h>
#include <thread>
#include <basisu_comp.h>
#include <algorithm>

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
        if (x == 1 || // normalData attachment
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

        ktxTextureCreateInfo createInfo{
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

        // Step 1: Read and convert all mips to RGBA8
        std::vector<std::vector<uint8_t>> allMipRGBA8(importTexture.mipMapLevels);
        bool readSuccess = true;

        for (uint32_t mip = 0; mip < importTexture.mipMapLevels; ++mip)
        {
            RawMipReadback raw = ConvertToRawTextureData(importTexture, mip);
            if (!raw.data || raw.size == 0)
            {
                fprintf(stderr, "Failed to read mip %u data for attachment %zu\n", mip, x);
                DestroyVMATextureBuffer(raw);
                readSuccess = false;
                break;
            }

            uint32_t mipWidth = std::max(1u, static_cast<uint32_t>(importTexture.width) >> mip);
            uint32_t mipHeight = std::max(1u, static_cast<uint32_t>(importTexture.height) >> mip);

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

            size_t expectedBytes = static_cast<size_t>(mipWidth) * mipHeight * bytesPerPixel;
            if (raw.size != expectedBytes)
            {
                fprintf(stderr, "Mip %u size mismatch for attachment %zu (expected %zu bytes, got %zu)\n",
                    mip, x, expectedBytes, raw.size);
                DestroyVMATextureBuffer(raw);
                readSuccess = false;
                break;
            }

            std::vector<uint8_t> rgba8Data(mipWidth * mipHeight * 4);

            if (bytesPerPixel == 4)
            {
                memcpy(rgba8Data.data(), raw.data, raw.size);
            }
            else if (bytesPerPixel == 8)
            {
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
                            norm = norm * 0.5f + 0.5f;
                        }
                        else if (srcFormat == VK_FORMAT_R16G16B16A16_SFLOAT)
                        {
                            norm = astc_helpers::half_to_float(src16[p * 4 + c]);
                            norm = std::clamp(norm, 0.0f, 1.0f);
                        }
                        rgba8Data[p * 4 + c] = static_cast<uint8_t>(norm * 255.0f + 0.5f);
                    }
                }
            }
            else if (bytesPerPixel == 16)
            {
                const float* src32 = static_cast<const float*>(raw.data);
                for (size_t p = 0; p < mipWidth * mipHeight; ++p)
                {
                    for (int c = 0; c < 4; ++c)
                    {
                        float val = src32[p * 4 + c];
                        val = std::clamp(val, 0.0f, 1.0f); // simple clamp
                        rgba8Data[p * 4 + c] = static_cast<uint8_t>(val * 255.0f + 0.5f);
                    }
                }
            }

            allMipRGBA8[mip] = std::move(rgba8Data);
            DestroyVMATextureBuffer(raw);
        }

        if (!readSuccess)
        {
            ktxTexture_Destroy(ktxTexture(ktx));
            continue;
        }

        // Step 2: Basis Universal UASTC compression (all mips at once)
        basisu::basis_compressor_params params;
        params.m_uastc = true;
        params.m_perceptual = useSRGB;           // perceptual for sRGB/albedo, linear for normals/data/emission
        params.m_quality_level = 255;
        params.m_multithreading = true;
        params.m_mip_gen = false;                // we supply all mips

        params.m_source_mipmap_images.resize(importTexture.mipMapLevels);
        for (uint32_t mip = 0; mip < importTexture.mipMapLevels; ++mip)
        {
            uint32_t w = std::max(1u, static_cast<uint32>(importTexture.width >> mip));
            uint32_t h = std::max(1u, static_cast<uint32>(importTexture.height >> mip));
            basisu::image img(w, h);
            memcpy(img.get_ptr(), allMipRGBA8[mip].data(), allMipRGBA8[mip].size());
            params.m_source_mipmap_images[mip].push_back(std::move(img));
        }

        basisu::basis_compressor compressor;
        if (!compressor.init(params) || !compressor.process())
        {
            fprintf(stderr, "Basis compression failed for attachment %zu\n", x);
            ktxTexture_Destroy(ktxTexture(ktx));
            continue;
        }

        const basisu::uint8_vec& basisData = compressor.get_output_basis_file();

        // Step 3: Transcode .basis -> raw BC7 blocks
        basist::basisu_transcoder transcoder;
        if (!transcoder.start_transcoding(basisData.data(), static_cast<uint32_t>(basisData.size())))
        {
            fprintf(stderr, "Basis transcoder start failed for attachment %zu\n", x);
            ktxTexture_Destroy(ktxTexture(ktx));
            continue;
        }

        bool transcodeSuccess = true;
        ktx_size_t offset = 0;
        for (uint32_t mip = 0; mip < importTexture.mipMapLevels; ++mip)
        {
            uint32_t w = std::max(1u, static_cast<uint32>(importTexture.width >> mip));
            uint32_t h = std::max(1u, static_cast<uint32>(importTexture.height >> mip));
            uint32_t blocksX = (w + 3) / 4;
            uint32_t blocksY = (h + 3) / 4;
            size_t numBlocks = static_cast<size_t>(blocksX) * blocksY;

            std::vector<uint8_t> bc7Data(numBlocks * 16);

            bool ok = transcoder.transcode_image_level(
                basisData.data(),
                static_cast<uint32_t>(basisData.size()),
                0, mip,
                bc7Data.data(),
                static_cast<uint32_t>(numBlocks),
                basist::transcoder_texture_format::cTFBC7_RGBA,
                0);

            if (!ok)
            {
                fprintf(stderr, "BC7 transcode failed for attachment %zu mip %u\n", x, mip);
                transcodeSuccess = false;
                break;
            }

            ktx_size_t levelSize = ktxTexture_GetImageSize(ktxTexture(ktx), mip);
            if (bc7Data.size() != static_cast<size_t>(levelSize))
            {
                fprintf(stderr, "Warning: BC7 size mismatch for attachment %zu mip %u (%zu vs expected %zu)\n",
                    x, mip, bc7Data.size(), levelSize);
            }

            memcpy(ktx->pData + offset, bc7Data.data(), bc7Data.size());
            offset += levelSize;
        }

        if (!transcodeSuccess)
        {
            ktxTexture_Destroy(ktxTexture(ktx));
            continue;
        }

        // Step 4: Apply ZSTD supercompression
        ktx->supercompressionScheme = KTX_SS_NONE;
        ktx_uint32_t zstdLevel = 12; // 9-12 good balance, up to 22 for max
     //   KTX_error_code compressErr = ktxTexture2_CompressZstd(ktx, zstdLevel);

   /*     if (compressErr != KTX_SUCCESS)
        {
            fprintf(stderr, "ZSTD supercompression failed (%s), writing without supercompression\n", ktxErrorString(compressErr));
            ktx->supercompressionScheme = KTX_SS_NONE;
        }
        else
        {
            printf("ZSTD supercompression applied (level %u)\n", zstdLevel);
        }*/

        // Step 5: Write to file
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
            printf("Successfully baked BC7 KTX2%s: %s\n",
                (ktx->supercompressionScheme == KTX_SS_ZSTD ? " (ZSTD supercompressed)" : ""),
                fullPath.c_str());
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

std::vector<uint8_t> TextureBakerSystem::CompressWithBasisUASTC(const void* rgbaData, size_t sizeBytes, uint32_t width, uint32_t height, bool isNormalMap)
{
    // 1. Create source image
    basisu::image source_image(width, height, 4);  // RGBA
    memcpy(source_image.get_ptr(), rgbaData, sizeBytes); //Failing here

    // 2. Setup params (only safe/confirmed fields for your version)
    basisu::basis_compressor_params params;

    // Add image
    params.m_source_images.push_back(source_image);

    // Enable UASTC mode (this is the key flag that works in v2.0 era)
    params.m_uastc = true;

    // Quality (this field exists in most versions)
    params.m_quality_level = 255;  // Max quality (0-255)

    // Colorspace: perceptual for color textures, linear for normals/data
    params.m_perceptual = !isNormalMap;

    // Optional safe flags
    params.m_mip_gen = true;          // Auto-generate mips if wanted
    params.m_multithreading = true;

    // 3. Compressor
    basisu::basis_compressor compressor;
    if (!compressor.init(params))
    {
        fprintf(stderr, "BasisU init failed\n");
        return {};
    }

    // 4. Compress
    if (!compressor.process())
    {
        fprintf(stderr, "BasisU process failed\n");
        return {};
    }

    // 5. Get .basis output
    const basisu::uint8_vec& basis_output = compressor.get_output_basis_file();
    return std::vector<uint8_t>(basis_output.begin(), basis_output.end());
}