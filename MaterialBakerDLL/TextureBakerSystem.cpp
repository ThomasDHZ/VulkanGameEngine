#define FMT_HEADER_ONLY

#include "TextureBakerSystem.h"
#include <EngineConfigSystem.h>
#include <TextureSystem.h>
#include <VulkanSystem.h>
#include <BufferSystem.h>
#include <ktx/include/ktx.h>
#include <ktx/include/ktxvulkan.h>
#include <algorithm>
#include <thread>
#include <fstream>
#include <filesystem>
#include <fmt/format.h>
#include <shellapi.h>
#include <windows.h> 

TextureBakerSystem& textureBakerSystem = TextureBakerSystem::Get();
void TextureBakerSystem::BakeTexture(const String& materialLoader, const String& baseFilePath, VkGuid renderPassId)
{
    Vector<Texture> attachmentTextureList = textureSystem.FindRenderedTextureList(renderPassId);
    if (attachmentTextureList.empty()) 
    {
        fprintf(stderr, "No rendered textures found for render pass %s\n", renderPassId.ToString().c_str());
        return;
    }

    nlohmann::json exportJson;
    nlohmann::json importJson = fileSystem.LoadJsonFile(materialLoader.c_str());

    exportJson["MaterialId"] = importJson.value("MaterialGuid", "00000000-0000-0000-0000-000000000000");
    exportJson["Name"] = importJson.value("Name", "UnnamedMaterial");
    exportJson["ShaderMaterialBufferIndex"] = importJson.value("ShaderMaterialBufferIndex", 0);
    exportJson["MaterialBufferId"] = importJson.value("MaterialBufferId", 0);

    String textureName = baseFilePath;
    size_t pos = textureName.find("Import");
    if (pos != String::npos) {
        textureName.erase(pos, 6);
    }

    std::vector<std::filesystem::path> tempFilesToClean;
    for (size_t x = 0; x < attachmentTextureList.size(); ++x) 
    {
        Texture& importTexture = attachmentTextureList[x];
        VkFormat srcFormat = importTexture.textureByteFormat;

        String textureAttachment;
        switch (x)
        {
            case AlbedoAttachment: textureAttachment = "AlbedoData"; break;
            case NormalDataAttachment: textureAttachment = "NormalData"; break;
            case PackedMROAttachment:  textureAttachment = "PackedMROData"; break;
            case PackedSheenSSSAttachment: textureAttachment = "PackedSheenSSSData"; break;
            case UnusedAttachment: textureAttachment = "UnusedData"; break;
            case EmissionAttachment: textureAttachment = "EmissionData"; break;
        }

        bool isNormalMap = (x == 1) ||
            srcFormat == VK_FORMAT_R16G16_UNORM ||
            srcFormat == VK_FORMAT_R16G16_SNORM ||
            srcFormat == VK_FORMAT_R16G16B16A16_SNORM ||
            srcFormat == VK_FORMAT_R8G8_SNORM;

        bool isSRGB = (x == 0 || x == 5) ||
            srcFormat == VK_FORMAT_R8G8B8A8_SRGB ||
            srcFormat == VK_FORMAT_B8G8R8A8_SRGB ||
            srcFormat == VK_FORMAT_A8B8G8R8_SRGB_PACK32 ||
            srcFormat == VK_FORMAT_R16G16B16A16_SFLOAT ||
            srcFormat == VK_FORMAT_R32G32B32A32_SFLOAT;

        bool isHDRFloat = (srcFormat == VK_FORMAT_R16G16B16A16_SFLOAT || srcFormat == VK_FORMAT_R32G32B32A32_SFLOAT);
        if (isHDRFloat) {
            printf("Warning: HDR float format on attachment %zu — clamped to 0-1 for BC7\n", x);
        }

        bool readSuccess = true;
        Vector<Vector<byte>> allMipRGBA8(importTexture.mipMapLevels);
        for (uint32 mip = 0; mip < importTexture.mipMapLevels && readSuccess; ++mip)
        {
            RawMipReadback raw = ConvertToRawTextureData(importTexture, mip);
            if (!raw.data || raw.size == 0) {
                fprintf(stderr, "Failed to read mip %u for %s\n", mip, textureAttachment.c_str());
                DestroyVMATextureBuffer(raw);
                readSuccess = false;
                break;
            }

            uint32 mipWidth = std::max(1u, static_cast<uint>(importTexture.width >> mip));
            uint32 mipHeight = std::max(1u, static_cast<uint>(importTexture.height >> mip));
            Vector<byte> rgba8Data = ConvertMipToRGBA8(raw.data, raw.size, mipWidth, mipHeight, srcFormat);
            if (rgba8Data.empty())
            {
                fprintf(stderr, "ConvertMipToRGBA8 failed for mip %u on %s\n", mip, textureAttachment.c_str());
                DestroyVMATextureBuffer(raw);
                readSuccess = false;
                break;
            }

            allMipRGBA8[mip] = std::move(rgba8Data);
            DestroyVMATextureBuffer(raw);
        }
        if (!readSuccess) continue;

        String suffix = GetAttachmentSuffix(x);
        std::filesystem::path previewPngPath = textureName + suffix + ".png";
        std::filesystem::path ktxPath = textureName + suffix + ".ktx2";
        ExportToPng(previewPngPath.string(), importTexture, 0, false);
        tempFilesToClean.push_back(previewPngPath);

        uint32 actualMips;
        uint32 requestedMips;
        importJson.at("ExportMipMapCount").get_to(requestedMips);

        uint32 maxDim = std::max(importTexture.width, importTexture.height);
        uint32 maxMips = static_cast<uint32>(std::floor(std::log2(maxDim))) + 1;

        if (requestedMips == UINT32_MAX) actualMips = maxMips;
        else actualMips = std::clamp(requestedMips, 1u, maxMips);

        bool generateMips = (actualMips > 1);
        std::string args = fmt::format("\"{}\" -o \"{}\" " "--format bc7 " "--quality production " "--zcmp 22 " "--export-transfer-function {}", previewPngPath.string(), ktxPath.string(), isSRGB ? "srgb" : "linear");

        if (isNormalMap)
        {
            args += " --normal-alpha unchanged";
        }

        if (generateMips) 
        {
            args += " --mips "
                "--mip-filter kaiser "
                "--mip-gamma-correct "
                "--mip-pre-alpha "
                "--max-mip-levels " + std::to_string(actualMips);
        }
        else
        {
            args += " --no-mips";
        }
        printf("Launching NVTT for %s with args:\n%s\n", textureAttachment.c_str(), args.c_str());

        HINSTANCE hInst = ShellExecuteA(NULL, "open", configSystem.NvidiaTextureTool.c_str(), args.c_str(), NULL, SW_HIDE);
        if ((intptr_t)hInst <= 32)
        {
            fprintf(stderr, "ShellExecuteA failed with code %ld for %s\n", (intptr_t)hInst, textureAttachment.c_str());
            continue;
        }

        bool fileCreated = false;
        for (int x = 0; x < 60; ++x) {
            if (std::filesystem::exists(ktxPath) && std::filesystem::file_size(ktxPath) > 1024)
            {
                fileCreated = true;
                printf("NVTT succeeded for %s (size %llu bytes)\n", textureAttachment.c_str(), std::filesystem::file_size(ktxPath));
                break;
            }
            Sleep(1000);
        }

        if (!fileCreated) {
            fprintf(stderr, "Timeout: KTX2 not created for %s\n", textureAttachment.c_str());
            continue;
        }


        nlohmann::json& exportEntry = exportJson[textureAttachment];
        exportEntry["TextureFilePath"] = nlohmann::json::array({ ktxPath.string() });
        exportEntry["TextureByteFormat"] = isSRGB ? static_cast<int>(VK_FORMAT_BC7_SRGB_BLOCK) : static_cast<int>(VK_FORMAT_BC7_UNORM_BLOCK);
        exportEntry["MipMapCount"] = actualMips;
        exportEntry["SampleCount"] = 1;
        exportEntry["ImageType"] = 1;
        exportEntry["TextureType"] = 1;
        exportEntry["IsSkyBox"] = importJson["IsSkyBox"];

        switch(x)
        {
            case AlbedoAttachment: exportEntry["TextureId"] = importJson["AlbedoAttachmentId"]; importJson["IsTiled"] ? TextureSamplers::GetTiledAlbedoMaterialSamplerSettings(exportEntry) : TextureSamplers::GetAlbedoMaterialSamplerSettings(exportEntry); break;
            case NormalDataAttachment: exportEntry["TextureId"] = importJson["NormalDataAttachmentId"]; importJson["IsTiled"] ? TextureSamplers::GetTiledNormalMaterialSamplerSettings(exportEntry) : TextureSamplers::GetNormalMaterialSamplerSettings(exportEntry); break;
            case PackedMROAttachment: exportEntry["TextureId"] = importJson["PackedMROAttachmentId"]; importJson["IsTiled"] ? TextureSamplers::GetTiledMROMaterialSamplerSettings(exportEntry) : TextureSamplers::GetMROMaterialSamplerSettings(exportEntry); break;
            case PackedSheenSSSAttachment: exportEntry["TextureId"] = importJson["PackedSheenSSSAttachmentId"]; importJson["IsTiled"] ? TextureSamplers::GetTiledSheenSSSSamplerSettings(exportEntry) : TextureSamplers::GetSheenSSSSamplerSettings(exportEntry); break;
            case UnusedAttachment: exportEntry["TextureId"] = importJson["UnusedAttachmentId"]; importJson["IsTiled"] ? TextureSamplers::GetTiledUnusedSamplerSettings(exportEntry) : TextureSamplers::GetUnusedSamplerSettings(exportEntry); break;
            case EmissionAttachment: exportEntry["TextureId"] = importJson["EmissionAttachmentId"]; importJson["IsTiled"] ? TextureSamplers::GetTiledEmissionSamplerSettings(exportEntry) : TextureSamplers::GetEmissionSamplerSettings(exportEntry); break;
        }
    }

    pos = textureName.find("Texture");
    if (pos != String::npos) {
        textureName.replace(pos, 7, "Material");
    }

    String materialName = textureName + ".json";
    try
    {
        std::ofstream file(materialName.c_str());
        if (!file.is_open())
        {
            fprintf(stderr, "Failed to open material JSON file: %s\n", materialName.c_str());
            return;
        }
        file << std::setw(4) << exportJson << std::endl;
        file.close();
        printf("Material JSON saved to: %s\n", materialName.c_str());
    }
    catch (const std::exception& e) 
    {
        fprintf(stderr, "JSON write error: %s\n", e.what());
    }

    for (const auto& png : tempFilesToClean) 
    {
        if (std::filesystem::exists(png)) 
        {
            std::filesystem::remove(png);
            printf("Cleaned temp preview: %s\n", png.string().c_str());
        }
    }
}

void TextureBakerSystem::ExportToPng(const String& fileName, Texture& texture, uint32 mipLevel, bool flipY)
{
    if (mipLevel >= texture.mipMapLevels) {
        std::cerr << "Invalid mip level " << mipLevel << " (max " << texture.mipMapLevels << ")\n";
        return;
    }

    VmaAllocator allocator = bufferSystem.vmaAllocator;

    uint32 width = std::max(1u, static_cast<uint32>(texture.width) >> mipLevel);
    uint32 height = std::max(1u, static_cast<uint32>(texture.height) >> mipLevel);

    bool is16Bit = false;
    bool is32BitFloat = false;
    size_t bytesPerPixel = 4;
    if (texture.textureByteFormat == VK_FORMAT_R32G32B32A32_SFLOAT ||
        texture.textureByteFormat == VK_FORMAT_R32G32B32A32_UINT ||
        texture.textureByteFormat == VK_FORMAT_R32G32B32A32_SINT) 
    {
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

    if (texture.textureImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    else if (texture.textureImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    else if (texture.textureImageLayout == VK_IMAGE_LAYOUT_GENERAL) barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    else barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

    VkCommandBuffer cmd = vulkanSystem.BeginSingleUseCommand();
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

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

    VmaAllocationInfo allocInfoOut{};
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingAlloc = VK_NULL_HANDLE;
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

    Vector<byte> pngData;
    if (!is16Bit) 
    {
        // 8-bit RGBA
       Vector<byte> pixels(width * height * 4);

        const byte* src = static_cast<const byte*>(mapped);
        byte* dst = pixels.data();

        for (uint32 y = 0; y < height; ++y) 
        {
            uint32 srcY = flipY ? (height - 1 - y) : y;
            const byte* srcRow = src + srcY * width * 4;
            byte* dstRow = dst + y * width * 4;
            memcpy(dstRow, srcRow, width * 4);
        }

        unsigned error = lodepng::encode(pngData, pixels.data(), width, height, LCT_RGBA, 8);
        if (error) std::cerr << "lodepng encode error (8-bit): " << lodepng_error_text(error) << "\n";
    }
    else 
    {
        // 16-bit RGBA (PNG big-endian)
        Vector<uint16> pixels(width * height * 4);

        const uint16* src = static_cast<const uint16*>(mapped);
        uint16* dst = pixels.data();

        for (uint32 y = 0; y < height; ++y) {
            uint32 srcY = flipY ? (height - 1 - y) : y;
            const uint16* srcRow = src + srcY * width * 4;
            uint16* dstRow = dst + y * width * 4;

            for (uint32 x = 0; x < width * 4; ++x) 
            {
                uint16 val = srcRow[x];
                dstRow[x] = ((val >> 8) & 0xFF) | ((val & 0xFF) << 8);
            }
        }

        unsigned error = lodepng::encode(pngData, reinterpret_cast<unsigned char*>(pixels.data()), width, height, LCT_RGBA, 16);
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

Vector<byte> TextureBakerSystem::ConvertMipToRGBA8(const void* rawData, size_t rawSize, uint32 width, uint32 height, VkFormat srcFormat)
{
    Vector<byte> rgba8(width * height * 4);

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
                    int16 s = static_cast<int16_t>(val);
                    norm = std::max(static_cast<float>(s) / 32767.0f, -1.0f);
                    norm = norm * 0.5f + 0.5f;
                }
                else if (srcFormat == VK_FORMAT_R16G16B16A16_SFLOAT)
                {
                    uint32 sign = (val >> 15) & 0x01;
                    uint32 exp = (val >> 10) & 0x1F;
                    uint32 mant = val & 0x3FF;
                    float fval;
                    if (exp == 0) fval = (mant / 1024.0f) * std::pow(2.0f, -14);
                    else if (exp == 31) fval = mant ? NAN : (sign ? -INFINITY : INFINITY);
                    else fval = ((1.0f + mant / 1024.0f) * std::pow(2.0f, exp - 15));

                    if (sign) fval = -fval;
                    norm = std::clamp(fval, 0.0f, 1.0f);
                }

                rgba8[p * 4 + c] = static_cast<byte>(norm * 255.0f + 0.5f);
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
                rgba8[p * 4 + c] = static_cast<byte>(val * 255.0f + 0.5f);
            }
        }
    }

    return rgba8;
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
                static_cast<uint32>(mipWidth),
                static_cast<uint32>(mipHeight),
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

String TextureBakerSystem::GetAttachmentSuffix(uint x)
{
    String suffix;
    if (x == 0)      suffix = "_Albedo";
    else if (x == 1) suffix = "_NormalHeight";
    else if (x == 2) suffix = "_MRO";
    else if (x == 3) suffix = "_SheenSSS";
    else if (x == 4) suffix = "_Unused";
    else if (x == 5) suffix = "_Emission";
    else             suffix = "_Attachment" + std::to_string(x);
    return suffix;
}