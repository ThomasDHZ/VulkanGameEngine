//#include "TextureBakerSystem.h"
//#include <TextureSystem.h>
//#include <ktx.h>
//#include <ktxvulkan.h>
//#include <lodepng.h>
//#include <VulkanSystem.h>
//#include <basisu_comp.h>          
//#include <basisu_frontend.h>      
//#include <basisu_backend.h>    
//#include <basisu_gpu_texture.h>  
//#include <basisu_enc.h>  
//
//TextureBakerSystem& textureBakerSystem = TextureBakerSystem::Get();
//
//using namespace basisu;
//
//#include <ktx.h>  // from libktx
//
//bool WrapBasisToKTX2Zstd(
//    const std::vector<uint8_t>& basisData,
//    uint32_t width, uint32_t height,
//    uint32_t levels,             // mip count from compressor.m_params.m_mip_levels or calculate
//    std::vector<uint8_t>& ktx2Output,
//    int zstdLevel = 9)
//{
//    ktxTexture2* tex;
//    ktxTextureCreateInfo createInfo = {};
//    createInfo.glInternalformat = KTX_GL_COMPRESSED_RGBA_BASISU_EXT;  // or appropriate for Basis
//    createInfo.baseWidth = width;
//    createInfo.baseHeight = height;
//    createInfo.baseDepth = 1;
//    createInfo.numDimensions = 2;
//    createInfo.numLevels = levels;
//    createInfo.numLayers = 1;
//    createInfo.numFaces = 1;
//    createInfo.isArray = KTX_FALSE;
//    createInfo.generateMipmaps = KTX_FALSE;
//
//    KTX_error_code ret = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &tex);
//    if (ret != KTX_SUCCESS) return false;
//
//    // Set the single level (Basis is usually level 0 only for the container)
//    ret = ktxTexture_SetImageFromMemory(ktxTexture(tex), 0, 0, 0, basisData.data(), basisData.size());
//    if (ret != KTX_SUCCESS) { ktxTexture_Destroy(ktxTexture(tex)); return false; }
//
//    // Apply Zstd supercompression
//    ret = ktxTexture2_CompressZstd(tex, zstdLevel);
//    if (ret != KTX_SUCCESS) { ktxTexture_Destroy(ktxTexture(tex)); return false; }
//
//    ktx_uint8_t* ktxBytes = nullptr;
//    ktx_size_t ktxSize = 0;
//    ret = ktxTexture_WriteToMemory(ktxTexture(tex), &ktxBytes, &ktxSize);
//    ktxTexture_Destroy(ktxTexture(tex));
//
//    if (ret != KTX_SUCCESS) { free(ktxBytes); return false; }
//
//    ktx2Output.assign(ktxBytes, ktxBytes + ktxSize);
//    free(ktxBytes);
//
//    printf("KTX2+Zstd size: %zu bytes\n", ktx2Output.size());
//    return true;
//}
//
//bool CompressToBasisRaw(
//    const uint8_t* rgba8_pixels,
//    uint32_t width,
//    uint32_t height,
//    std::vector<uint8_t>& basisDataOut,
//    bool useUASTC = true,               // true = high quality (UASTC/BC7-like)
//    int etc1s_quality = 128,            // 1–255 for ETC1S mode
//    bool generate_mipmaps = true,
//    int effort_level = 2)               // 0–5; higher = slower/better compression
//{
//    basisu_encoder_init();  // Call once per process (safe to call multiple times)
//
//    basis_compressor_params p;
//    p.m_source_images.emplace_back(rgba8_pixels, width, height, 4);
//
//    // Quality / mode selection
//    p.m_uastc = useUASTC;
//    if (!useUASTC) {
//        p.m_quality_level = etc1s_quality;  // Valid range
//    }
//
//    // Mipmaps
//    p.m_mip_gen = generate_mipmaps;
//    p.m_mip_smallest = 1;               // Valid: down to 1x1
//
//    // Effort / tuning
//    p.m_compression_level = effort_level;  // Valid: 0–5 (not BASISU_MAX_...)
//    p.m_tex_type = basist::cBASISTexType2DArray;  // Change to cBASISTexTypeNormalMap if normal map
//
//    // Optional: perceptual/sRGB, normal map flags, etc.
//    // p.m_perceptual = true;
//    // p.m_normal_map = true;
//
//    basis_compressor compressor;
//    if (!compressor.init(p)) {
//        fprintf(stderr, "compressor.init failed\n");
//        return false;
//    }
//
//    auto status = compressor.process();
//    if (status != basis_compressor::cECSuccess) {
//        fprintf(stderr, "process failed: %d\n", status);
//        return false;
//    }
//
//    // Get raw .basis data (this is the compressed output)
//    basisDataOut = compressor.get_output_basis_file();
//
//    printf("Raw Basis data size: %zu bytes\n", basisDataOut.size());
//    return true;
//}
//
//// 2. Wrap raw .basis ? KTX2 container + Zstd supercompression (using libktx)
//bool WrapBasisToKTX2Zstd(
//    const std::vector<uint8_t>& basisData,
//    uint32_t baseWidth,
//    uint32_t baseHeight,
//    uint32_t mipLevels,                 // usually 1 if no mips, or log2(max(w,h))+1
//    std::vector<uint8_t>& ktx2Output,
//    int zstdLevel = 9)
//{
//    ktxTexture2* tex = nullptr;
//    ktxTextureCreateInfo createInfo{};
//    createInfo.glInternalformat = ktxTextureCreateinfo::KTX_GL_BASISU_EXT;  // Correct constant for Basis in KTX2
//    createInfo.vkFormat = VK_FORMAT_UNDEFINED;        // Basis is opaque to Vulkan
//    createInfo.baseWidth = baseWidth;
//    createInfo.baseHeight = baseHeight;
//    createInfo.baseDepth = 1;
//    createInfo.numDimensions = 2;
//    createInfo.numLevels = mipLevels;
//    createInfo.numLayers = 1;
//    createInfo.numFaces = 1;
//    createInfo.isArray = KTX_FALSE;
//    createInfo.generateMipmaps = KTX_FALSE;
//
//    KTX_error_code ret = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &tex);
//    if (ret != KTX_SUCCESS) {
//        fprintf(stderr, "ktxTexture2_Create failed: %d\n", ret);
//        return false;
//    }
//
//    // Basis data goes into level 0 (KTX2 treats Basis as single compressed blob)
//    ret = ktxTexture_SetImageFromMemory(ktxTexture(tex), 0, 0, 0,
//        basisData.data(), basisData.size());
//    if (ret != KTX_SUCCESS) {
//        ktxTexture_Destroy(ktxTexture(tex));
//        fprintf(stderr, "SetImage failed: %d\n", ret);
//        return false;
//    }
//
//    // Apply Zstd supercompression
//    ret = ktxTexture2_CompressZstd(tex, zstdLevel);
//    if (ret != KTX_SUCCESS) {
//        ktxTexture_Destroy(ktxTexture(tex));
//        fprintf(stderr, "CompressZstd failed: %d\n", ret);
//        return false;
//    }
//
//    ktx_uint8_t* ktxBytes = nullptr;
//    ktx_size_t ktxSize = 0;
//    ret = ktxTexture_WriteToMemory(ktxTexture(tex), &ktxBytes, &ktxSize);
//    ktxTexture_Destroy(ktxTexture(tex));
//
//    if (ret != KTX_SUCCESS || ktxBytes == nullptr) {
//        free(ktxBytes);
//        fprintf(stderr, "WriteToMemory failed: %d\n", ret);
//        return false;
//    }
//
//    ktx2Output.assign(ktxBytes, ktxBytes + ktxSize);
//    free(ktxBytes);
//
//    printf("KTX2+Zstd size: %zu bytes\n", ktx2Output.size());
//    return true;
//}
//
//bool CompressToSuperKTX2(
//    const uint8_t* rgba8_pixels,        // input: raw RGBA8, row-major, no padding
//    uint32_t width,
//    uint32_t height,
//    std::vector<uint8_t>& ktx2Data,
//    bool useUASTC = true,               // true = high quality (UASTC), false = smaller/faster (ETC1S)
//    int etc1s_quality = 128,            // 1–255, only used when useUASTC = false
//    bool generate_mipmaps = true,
//    int zstd_level = 9)                 // 0–22, higher = better compression but slower
//{
//    basisu_encoder_init();  // global init (call once per process)
//
//    basis_compressor_params p;
//    p.m_source_images.emplace_back(rgba8_pixels, width, height, 4);  // RGBA8
//
//    // Quality mode
//    p.m_uastc = useUASTC;
//    if (!useUASTC) {
//        p.m_quality_level = etc1s_quality;  // typical range 40–220
//    }
//
//    // Mipmaps
//    p.m_mip_gen = generate_mipmaps;
//    p.m_mip_smallest_dimension = 1;     // generate down to 1×1
//
//    // Supercompression (Zstd)
//    p.m_etc1s_compression_level = BASISU_MAX_COMPRESSION_LEVEL;  // max effort
//    p.m_ktx2_uastc_supercompression = true;
//    p.m_create_ktx2_file = true;
//    p.m_ktx2_zstd_supercompression_level = zstd_level;
//
//    basis_compressor compressor;
//    if (!compressor.init(p)) {
//        fprintf(stderr, "compressor.init failed\n");
//        return false;
//    }
//
//    auto status = compressor.process();
//    if (status != basis_compressor::cECSuccess) {
//        fprintf(stderr, "compression failed: %d\n", status);
//        return false;
//    }
//
//    // Get final KTX2 data
//    ktx2Data = compressor.get_output_ktx2_file();
//
//    printf("Compressed to supercompressed KTX2: %zu bytes\n", ktx2Data.size());
//    return true;
//}
//
//void TextureBakerSystem::BakeTexture(const String& textureName, ImportTexture& texture)
//{
//    //using namespace nvtt;
//
//    //Surface surf;
//
//    //// Load raw pixels into the surface (depth=1 for 2D textures)
//    //if (!surf.setImage(InputFormat, texture.width, texture.height, texture.depth, inputPixels)) {
//    //    printf("Failed to set image data\n");
//    //    return;
//    //}
//
//    //// Optional: mark as normal map if compressing normals (affects BC5/BC6H etc.)
//    //// surf.setNormalMap(true);
//
//    //// Generate mipmaps if requested (all levels down to 1x1)
//    ////if (generateMipmaps) {
//    ////    // Kaiser is the default high-quality filter; can change to Box/Triangle for faster
//    ////    if (!surf.generateMipmaps()) {
//    ////        printf("Failed to generate mipmaps\n");
//    ////        return false;
//    ////    }
//    ////}
//
//    //CompressionOptions compressionOptions;
//    //compressionOptions.setFormat(texture.ExportFormat);
//    //compressionOptions.setQuality(texture.ExportQuality);
//
//    //OutputOptions outputOptions;
//    //MemoryOutputHandler memHandler;
//    //outputOptions.setOutputHandler(&memHandler);
//    //outputOptions.setContainer(Container_None);
//
//    //SimpleErrorHandler errorHandler;
//    //outputOptions.setErrorHandler(&errorHandler);
//
//    //Context context;
//    //context.enableCudaAcceleration(true);  // Uncomment if you have CUDA and want GPU acceleration
//
//    //// Compress all faces (usually 1) and all mip levels
//    //int mipCount = generateMipmaps ? surf.mipmapCount() : 1;
//    //int faceCount = surf.faceCount();  // 1 for 2D, 6 for cubemaps
//
//    //for (int face = 0; face < faceCount; ++face) {
//    //    for (int mip = 0; mip < mipCount; ++mip) {
//    //        if (!context.compress(surf, face, mip, compressionOptions, outputOptions)) {
//    //            printf("Compression failed at face %d mip %d\n", face, mip);
//    //            return false;
//    //        }
//    //    }
//    //}
//
//    //printf("Compression succeeded. Raw size: %zu bytes\n", memHandler.buffer.size());
//
//    //// memHandler.buffer now holds the raw compressed blocks (all mips concatenated if generated)
//    //// Block size rules same as before: 8 bytes per 4x4 for BC1/BC4, 16 bytes for BC3/BC7 etc.
//
//
//    //// ... load image, set format/quality/mipmaps as before ...
//
//    //nvtt::OutputOptions outOpts;
//    //outOpts.setFileName("output.ktx2");  // Or explicitly:
//    //outOpts.setContainer(nvtt::Container);
//    //outOpts.setSrgbFlag(texture.ExportSRGB);
//    //// Supercompression (Zstd) - exact method may vary by version:
//    //// In recent NVTT 3, something like:
//    //outOpts.setSupercompressionScheme(nvtt::Supercompression_ZSTD);  // Or similar enum
//    //outOpts.setCompressionLevel(5);  // 0-22
//    //ktxTextuer2_def
//    //    // Then output header + compress as usual
//    //    ctx.outputHeader(img, mipLevels, compOpts, outOpts);
//    //// ... compress loops ...
//}
//
//void* TextureBakerSystem::ConvertToRawTextureData(ImportTexture& inputTexture)
//{
//   /* Texture texture = attachmentTextureList[x];
//    VmaAllocator allocator = bufferSystem.vmaAllocator;
//    VkImageMemoryBarrier barrier =
//    {
//        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
//        .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
//        .oldLayout = texture.textureImageLayout,
//        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//        .image = texture.textureImage,
//        .subresourceRange =
//        {
//            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
//            .baseMipLevel = 0,
//            .levelCount = 1,
//            .baseArrayLayer = 0,
//            .layerCount = 1,
//        }
//    };
//
//    size_t bytesPerPixel = 4;
//    if (texture.textureByteFormat == VK_FORMAT_R32G32B32A32_SFLOAT ||
//        texture.textureByteFormat == VK_FORMAT_R32G32B32A32_UINT ||
//        texture.textureByteFormat == VK_FORMAT_R32G32B32A32_SINT) {
//        bytesPerPixel = 16;
//    }
//    else if (texture.textureByteFormat >= VK_FORMAT_R16G16B16A16_UNORM &&
//        texture.textureByteFormat <= VK_FORMAT_R16G16B16A16_SFLOAT)
//    {
//        bytesPerPixel = 8;
//    }
//
//    if (texture.textureImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//    else if (texture.textureImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
//    else if (texture.textureImageLayout == VK_IMAGE_LAYOUT_GENERAL) barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
//    else barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
//
//    VkBufferCreateInfo bufferInfo =
//    {
//        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
//        .size = static_cast<VkDeviceSize>(texture.width) * texture.height * bytesPerPixel,
//        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
//        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
//    };
//
//    VmaAllocationCreateInfo allocInfo =
//    {
//        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
//        .usage = VMA_MEMORY_USAGE_AUTO
//    };
//
//    VkBufferImageCopy region =
//    {
//        .bufferOffset = 0,
//        .bufferRowLength = 0,
//        .bufferImageHeight = 0,
//        .imageSubresource
//            {
//                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
//                .mipLevel = 0,
//                .baseArrayLayer = 0,
//                .layerCount = 1,
//            },
//        .imageOffset = { 0, 0, 0 },
//        .imageExtent =
//            {
//                static_cast<uint32_t>(texture.width),
//                static_cast<uint32_t>(texture.height),
//                1
//            }
//    };
//
//    VmaAllocationInfo allocOut{};
//    VkBuffer stagingBuffer = VK_NULL_HANDLE;
//    VmaAllocation stagingAlloc = VK_NULL_HANDLE;
//    VkCommandBuffer command = vulkanSystem.BeginSingleUseCommand();
//    vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
//    VULKAN_THROW_IF_FAIL(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAlloc, &allocOut));
//
//    bool needsUnmap = false;
//    void* mappedData = allocOut.pMappedData;
//    if (!mappedData)
//    {
//        VULKAN_THROW_IF_FAIL(vmaMapMemory(allocator, stagingAlloc, &mappedData));
//        needsUnmap = true;
//    }
//    vkCmdCopyImageToBuffer(command, texture.textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);
//    vulkanSystem.EndSingleUseCommand(command);*/
//}
//
//void TextureBakerSystem::DestroyRawTextureBuffer()
//{
//    //if (needsUnmap)
//    //{
//    //    vmaUnmapMemory(allocator, stagingAlloc);
//    //}
//    //vmaDestroyBuffer(allocator, stagingBuffer, stagingAlloc);
//}
