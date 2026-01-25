#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stb/stb_image_write.h>

#include "MaterialBakerSystem.h"
#include <ShaderSystem.h>
#include <FileSystem.h>
#include <EngineConfigSystem.h>
#include <TextureSystem.h>
#include <from_json.h>
#include <stb_image_write.h>

MaterialBakerSystem& materialBakerSystem = MaterialBakerSystem::Get();

void MaterialBakerSystem::Run()
{
    InitDummyAndSamplers(); 

    const String inDir = configSystem.MaterialSourceDirectory.c_str();
    std::filesystem::path outDir = configSystem.MaterialDstDirectory.c_str();
    std::filesystem::create_directories(outDir);

    shaderSystem.LoadShaderPipelineStructPrototypes(Vector<String>{configSystem.TextureAssetRenderer});

    Vector<String> ext = { "json" };
    Vector<String> materialFiles = fileSystem.GetFilesFromDirectory(configSystem.MaterialSourceDirectory.c_str(), ext);

    for (auto& materialPath : materialFiles)
    {
        std::filesystem::path src = materialPath;
        std::filesystem::path dst = outDir / (src.filename().stem().string() + ".json");
        std::filesystem::path finalFilePath = outDir / (src.filename().stem().string());
        if (std::filesystem::exists(dst) &&
            std::filesystem::last_write_time(dst) >= std::filesystem::last_write_time(src))
        {
            continue;
        }

        LoadMaterial(materialPath);
        nlohmann::json json = fileSystem.LoadJsonFile(materialPath.c_str());
        ivec2 resolution = ivec2(json["TextureSetResolution"][0], json["TextureSetResolution"][1]);

        CleanRenderPass();
        BuildRenderPass(resolution);
        UpdateDescriptorSets();
        Draw();
        fileSystem.ExportTexture(vulkanRenderPass.RenderPassId, finalFilePath.string());

        CleanInputResources();         
        textureBindingList.clear();

        std::cout << "Baked: " << src.filename() << std::endl;
    }
}

void MaterialBakerSystem::Draw()
{
    VkCommandBufferBeginInfo beginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(vulkanRenderPass.RenderPassResolution.x),
        .height = static_cast<float>(vulkanRenderPass.RenderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = vulkanRenderPass.RenderPass,
        .framebuffer = vulkanRenderPass.FrameBufferList[vulkanSystem.ImageIndex],
        .renderArea = VkRect2D
        {
           .offset = VkOffset2D {.x = 0, .y = 0 },
           .extent = VkExtent2D {.width = static_cast<uint>(vulkanRenderPass.RenderPassResolution.x), .height = static_cast<uint>(vulkanRenderPass.RenderPassResolution.y) }
        },
        .clearValueCount = static_cast<uint32>(vulkanRenderPass.ClearValueList.size()),
        .pClearValues = vulkanRenderPass.ClearValueList.data()
    };

    VkFence fence = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = vulkanSystem.BeginSingleUseCommand();
    VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    VkFenceCreateInfo fenceCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = 0
    };


    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &renderPassBeginInfo.renderArea);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPipeline.PipelineLayout, 0, vulkanRenderPipeline.DescriptorSetList.size(), vulkanRenderPipeline.DescriptorSetList.data(), 0, nullptr);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    VULKAN_THROW_IF_FAIL(vkEndCommandBuffer(commandBuffer));
    VULKAN_THROW_IF_FAIL(vkCreateFence(vulkanSystem.Device, &fenceCreateInfo, nullptr, &fence));
    VULKAN_THROW_IF_FAIL(vkQueueSubmit(vulkanSystem.GraphicsQueue, 1, &submitInfo, fence));
    VULKAN_THROW_IF_FAIL(vkWaitForFences(vulkanSystem.Device, 1, &fence, VK_TRUE, UINT64_MAX));
    vkDestroyFence(vulkanSystem.Device, fence, nullptr);
}

void MaterialBakerSystem::CleanRenderPass()
{
    if (vulkanRenderPass.RenderPass != VK_NULL_HANDLE)
    {
        for (auto& fb : vulkanRenderPass.FrameBufferList)
        {
            vkDestroyFramebuffer(vulkanSystem.Device, fb, nullptr);
        }
        for (auto& tex : textureSystem.FindRenderedTextureList(vulkanRenderPass.RenderPassId))
        {
            textureSystem.DestroyTexture(tex);
        }

        vkDestroyRenderPass(vulkanSystem.Device, vulkanRenderPass.RenderPass, nullptr);
        vkDestroyPipeline(vulkanSystem.Device, vulkanRenderPipeline.Pipeline, nullptr);
        vkDestroyPipelineLayout(vulkanSystem.Device, vulkanRenderPipeline.PipelineLayout, nullptr);
        for (auto& dsl : vulkanRenderPipeline.DescriptorSetLayoutList) {
            vkDestroyDescriptorSetLayout(vulkanSystem.Device, dsl, nullptr);
        }
        vkDestroyDescriptorPool(vulkanSystem.Device, vulkanRenderPipeline.DescriptorPool, nullptr);
    }
}

Texture MaterialBakerSystem::LoadTexture(const String& texturePath, size_t bindingNumber)
{
    TextureLoader textureLoader = fileSystem.LoadJsonFile(texturePath.c_str());

    int width = 0;
    int height = 0;
    int textureChannels = 0;
    Vector<byte> textureData;
    for (size_t x = 0; x < textureLoader.TextureFilePath.size(); x++)
    {
        Vector<byte> layerData = fileSystem.LoadImageFile(textureLoader.TextureFilePath[x], width, height, textureChannels);
        textureData.insert(textureData.end(), layerData.begin(), layerData.end());
    }

    VkFormat detectedFormat = VK_FORMAT_UNDEFINED;
    switch (textureChannels)
    {
    case 1: detectedFormat = VK_FORMAT_R8_UNORM; break;
    case 2: detectedFormat = VK_FORMAT_R8G8_UNORM; break;
    case 3: detectedFormat = textureLoader.UsingSRGBFormat ? VK_FORMAT_R8G8B8_SRGB : VK_FORMAT_R8G8B8_UNORM; break;
    case 4: detectedFormat = textureLoader.UsingSRGBFormat ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM; break;
    default:
    {
        std::cout << "[TextureSystem WARNING] Unsupported channel count: " << textureChannels << " for " << textureLoader.TextureFilePath[0] << std::endl;
        detectedFormat = textureLoader.UsingSRGBFormat ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
        break;
    }
    }

    VkFormat finalFormat = detectedFormat;
    if (textureLoader.TextureByteFormat != VK_FORMAT_UNDEFINED)
    {
        finalFormat = textureLoader.TextureByteFormat;
    }

    Texture texture = Texture
    {
        .textureGuid = textureLoader.TextureId,
        .textureIndex = bindingNumber,
        .width = width,
        .height = height,
        .depth = 1,
        .mipMapLevels = textureLoader.UseMipMaps ? static_cast<uint32>(std::floor(std::log2(std::max(width, height)))) + 1 : 1,
        .textureType = textureLoader.IsSkyBox ? TextureType_SkyboxTexture : TextureType_ColorTexture,
        .textureByteFormat = finalFormat,
        .textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .sampleCount = VK_SAMPLE_COUNT_1_BIT,
        .colorChannels = static_cast<ColorChannelUsed>(textureChannels)
    };

    VkImageCreateInfo imageCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = textureLoader.IsSkyBox ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : static_cast<VkImageViewCreateFlags>(0),
        .imageType = VK_IMAGE_TYPE_2D,
        .format = texture.textureByteFormat,
        .extent =
        {
            .width = static_cast<uint32>(texture.width),
            .height = static_cast<uint32>(texture.height),
            .depth = 1,
        },
        .mipLevels = texture.mipMapLevels,
        .arrayLayers = textureLoader.IsSkyBox ? static_cast<uint>(6) : static_cast<uint>(1),
        .samples = texture.sampleCount,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = texture.textureImageLayout
    };
    if (texture.mipMapLevels > 1) imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    CreateTextureImage(texture, imageCreateInfo, textureData, textureLoader.TextureFilePath.size());
    CreateTextureView(texture, textureLoader.ImageType);
    return texture;
}
void MaterialBakerSystem::LoadMaterial(const String& materialPath)
{
    shaderStruct = shaderSystem.CopyShaderStructProtoType("MaterialProperitiesBuffer");
    nlohmann::json json = fileSystem.LoadJsonFile(materialPath.c_str());
    ivec2 materialSetResolution = ivec2(json["TextureSetResolution"][0], json["TextureSetResolution"][1]);

    material.materialGuid = json["MaterialGuid"];
    material.MaterialBufferId = bufferSystem.VMACreateDynamicBuffer(&shaderStruct, shaderStruct.ShaderBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    material.Albedo = vec3(json["Albedo"][0], json["Albedo"][1], json["Albedo"][2]);
    material.SheenColor = vec3(json["SheenColor"][0], json["SheenColor"][1], json["SheenColor"][2]);
    material.SubSurfaceScatteringColor = vec3(json["SubSurfaceScatteringColor"][0], json["SubSurfaceScatteringColor"][1], json["SubSurfaceScatteringColor"][2]);
    material.Emission = vec3(json["Emission"][0], json["Emission"][1], json["Emission"][2]);

    material.ClearcoatTint = json["ClearcoatTint"];
    material.Metallic = json["Metallic"];
    material.Roughness = json["Roughness"];
    material.AmbientOcclusion = json["AmbientOcclusion"];
    material.ClearcoatStrength = json["ClearcoatStrength"];
    material.ClearcoatRoughness = json["ClearcoatRoughness"];
    material.Thickness = json["Thickness"];
    material.SheenIntensity = json["SheenIntensity"];
    material.NormalStrength = json["NormalStrength"];
    material.HeightScale = json["HeightScale"];
    material.Height = json["Height"];
    material.Alpha = json["Alpha"];

    shaderSystem.UpdateShaderStructValue<vec3>(shaderStruct, "Albedo", material.Albedo);
    shaderSystem.UpdateShaderStructValue<vec3>(shaderStruct, "SheenColor", material.SheenColor);
    shaderSystem.UpdateShaderStructValue<vec3>(shaderStruct, "SubSurfaceScatteringColor", material.SubSurfaceScatteringColor);
    shaderSystem.UpdateShaderStructValue<vec3>(shaderStruct, "Emission", material.Emission);
    shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "ClearcoatTint", material.ClearcoatTint);
    shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "Metallic", material.Metallic);
    shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "Roughness", material.Roughness);
    shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "AmbientOcclusion", material.AmbientOcclusion);
    shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "ClearcoatStrength", material.ClearcoatStrength);
    shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "ClearcoatRoughness", material.ClearcoatRoughness);
    shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "SheenIntensity", material.SheenIntensity);
    shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "Thickness", material.Thickness);
    shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "NormalStrength", material.NormalStrength);
    shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "HeightScale", material.HeightScale);
    shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "Height", material.Height);
    shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "Alpha", material.Alpha);

    if (!json["AlbedoMap"].is_null())
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AlbedoMap", 1);
        material.AlbedoMap = LoadTexture(json["AlbedoMap"], 1);
        textureBindingList.emplace_back(GetTextureDescriptorbinding(material.AlbedoMap, cachedAlbedoSampler));
    }
    else
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AlbedoMap", 0xFFFFFFFFu);
        textureBindingList.emplace_back(VkDescriptorImageInfo{
            .sampler = cachedAlbedoSampler,
            .imageView = dummyTextureColor.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
    }

    if (!json["MetallicMap"].is_null())
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "MetallicMap", 1);
        material.MetallicMap = LoadTexture(json["MetallicMap"], 2);
        textureBindingList.emplace_back(GetTextureDescriptorbinding(material.MetallicMap, cachedPackedORMSampler));
    }
    else
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "MetallicMap", 0xFFFFFFFFu);
        textureBindingList.emplace_back(VkDescriptorImageInfo{
            .sampler = cachedPackedORMSampler,
            .imageView = dummyTextureScalar.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
    }

    if (!json["RoughnessMap"].is_null())
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "RoughnessMap", 1);
        material.RoughnessMap = LoadTexture(json["RoughnessMap"], 3);
        textureBindingList.emplace_back(GetTextureDescriptorbinding(material.RoughnessMap, cachedPackedORMSampler));
    }
    else
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "RoughnessMap", 0xFFFFFFFFu);
        textureBindingList.emplace_back(VkDescriptorImageInfo{
            .sampler = cachedPackedORMSampler,
            .imageView = dummyTextureScalar.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
    }

    if (!json["ThicknessMap"].is_null())
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "ThicknessMap", 1);
        material.ThicknessMap = LoadTexture(json["ThicknessMap"], 4);
        textureBindingList.emplace_back(GetTextureDescriptorbinding(material.ThicknessMap, cachedPackedORMSampler));
    }
    else
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "ThicknessMap", 0xFFFFFFFFu);
        textureBindingList.emplace_back(VkDescriptorImageInfo{
            .sampler = cachedPackedORMSampler,
            .imageView = dummyTextureScalar.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
    }

    if (!json["SubSurfaceScatteringColorMap"].is_null())
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "SubSurfaceScatteringColorMap", 1);
        material.SubSurfaceScatteringMap = LoadTexture(json["SubSurfaceScatteringColorMap"], 5);
        textureBindingList.emplace_back(GetTextureDescriptorbinding(material.SubSurfaceScatteringMap, cachedPackedORMSampler));
    }
    else
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "SubSurfaceScatteringColorMap", 0xFFFFFFFFu);
        textureBindingList.emplace_back(VkDescriptorImageInfo{
            .sampler = cachedPackedORMSampler,
            .imageView = dummyTextureScalar.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
    }

    if (!json["SheenMap"].is_null())
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "SheenMap", 1);
        material.SheenMap = LoadTexture(json["SheenMap"], 6);
        textureBindingList.emplace_back(GetTextureDescriptorbinding(material.SheenMap, cachedPackedORMSampler));
    }
    else
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "SheenMap", 0xFFFFFFFFu);
        textureBindingList.emplace_back(VkDescriptorImageInfo{
            .sampler = cachedPackedORMSampler,
            .imageView = dummyTextureScalar.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
    }

    if (!json["ClearCoatMap"].is_null())
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "ClearCoatMap", 1);
        material.ClearCoatMap = LoadTexture(json["ClearCoatMap"], 7);
        textureBindingList.emplace_back(GetTextureDescriptorbinding(material.ClearCoatMap, cachedPackedORMSampler));
    }
    else
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "ClearCoatMap", 0xFFFFFFFFu);
        textureBindingList.emplace_back(VkDescriptorImageInfo{
            .sampler = cachedPackedORMSampler,
            .imageView = dummyTextureScalar.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
    }

    if (!json["AmbientOcclusionMap"].is_null())
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AmbientOcclusionMap", 1);
        material.AmbientOcclusionMap = LoadTexture(json["AmbientOcclusionMap"], 8);
        textureBindingList.emplace_back(GetTextureDescriptorbinding(material.AmbientOcclusionMap, cachedPackedORMSampler));
    }
    else
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AmbientOcclusionMap", 0xFFFFFFFFu);
        textureBindingList.emplace_back(VkDescriptorImageInfo{
            .sampler = cachedPackedORMSampler,
            .imageView = dummyTextureScalar.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
    }

    if (!json["NormalMap"].is_null())
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "NormalMap", 1);
        material.NormalMap = LoadTexture(json["NormalMap"], 9);
        textureBindingList.emplace_back(GetTextureDescriptorbinding(material.NormalMap, cachedNormalSampler));
    }
    else
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "NormalMap", 0xFFFFFFFFu);
        textureBindingList.emplace_back(VkDescriptorImageInfo{
            .sampler = cachedNormalSampler,
            .imageView = dummyTextureNormal.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
    }

    if (!json["AlphaMap"].is_null())
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AlphaMap", 1);
        material.AlphaMap = LoadTexture(json["AlphaMap"], 10);
        textureBindingList.emplace_back(GetTextureDescriptorbinding(material.AlphaMap, cachedAlphaSampler));
    }
    else
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AlphaMap", 0xFFFFFFFFu);
        textureBindingList.emplace_back(VkDescriptorImageInfo{
            .sampler = cachedAlphaSampler,
            .imageView = dummyTextureScalar.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
    }

    if (!json["EmissionMap"].is_null())
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "EmissionMap", 1);
        material.EmissionMap = LoadTexture(json["EmissionMap"], 11);
        textureBindingList.emplace_back(GetTextureDescriptorbinding(material.EmissionMap, cachedAlbedoSampler));
    }
    else
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "EmissionMap", 0xFFFFFFFFu);
        textureBindingList.emplace_back(VkDescriptorImageInfo{
            .sampler = cachedAlbedoSampler,
            .imageView = dummyTextureColor.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
    }

    if (!json["HeightMap"].is_null())
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "HeightMap", 1);
        material.HeightMap = LoadTexture(json["HeightMap"], 12);
        textureBindingList.emplace_back(GetTextureDescriptorbinding(material.HeightMap, cachedParallaxSampler));
    }
    else
    {
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "HeightMap", 0xFFFFFFFFu);
        textureBindingList.emplace_back(VkDescriptorImageInfo{
            .sampler = cachedParallaxSampler,
            .imageView = dummyTextureScalar.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
    }
    shaderSystem.UpdateShaderBuffer(shaderStruct, material.MaterialBufferId);
}

void MaterialBakerSystem::UpdateDescriptorSets()
{
    vkDeviceWaitIdle(vulkanSystem.Device);

    VkDescriptorBufferInfo materialDescriptorSet
    {
        .buffer = bufferSystem.FindVulkanBuffer(material.MaterialBufferId).Buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };

    Vector<VkWriteDescriptorSet> descriptorSetList;
    VkDescriptorSet targetSet = vulkanRenderPipeline.DescriptorSetList[0];
    descriptorSetList.emplace_back(VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, targetSet, 0, 0, 1,  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         nullptr, &materialDescriptorSet,  nullptr });
    descriptorSetList.emplace_back(VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, targetSet, 1, 0, 1,  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureBindingList[0],  nullptr, nullptr });
    descriptorSetList.emplace_back(VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, targetSet, 2, 0, 1,  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureBindingList[1],  nullptr, nullptr });
    descriptorSetList.emplace_back(VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, targetSet, 3, 0, 1,  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureBindingList[2],  nullptr, nullptr });
    descriptorSetList.emplace_back(VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, targetSet, 4, 0, 1,  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureBindingList[3],  nullptr, nullptr });
    descriptorSetList.emplace_back(VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, targetSet, 5, 0, 1,  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureBindingList[4],  nullptr, nullptr });
    descriptorSetList.emplace_back(VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, targetSet, 6, 0, 1,  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureBindingList[5],  nullptr, nullptr });
    descriptorSetList.emplace_back(VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, targetSet, 7, 0, 1,  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureBindingList[6],  nullptr, nullptr });
    descriptorSetList.emplace_back(VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, targetSet, 8, 0, 1,  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureBindingList[7],  nullptr, nullptr });
    descriptorSetList.emplace_back(VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, targetSet, 9, 0, 1,  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureBindingList[8],  nullptr, nullptr });
    descriptorSetList.emplace_back(VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, targetSet, 10, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureBindingList[9],  nullptr, nullptr });
    descriptorSetList.emplace_back(VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, targetSet, 11, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureBindingList[10], nullptr, nullptr });
    descriptorSetList.emplace_back(VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, targetSet, 12, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textureBindingList[11], nullptr, nullptr });
    vkUpdateDescriptorSets(vulkanSystem.Device, static_cast<uint32_t>(descriptorSetList.size()), descriptorSetList.data(), 0, nullptr);
}

void MaterialBakerSystem::BuildRenderPass(ivec2 renderPassResolution)
{
    RenderPassLoader renderPassLoader = fileSystem.LoadJsonFile(configSystem.TextureAssetRenderer.c_str()).get<RenderPassLoader>();
    renderPassLoader.RenderPassWidth = renderPassResolution.x;
    renderPassLoader.RenderPassHeight = renderPassResolution.y;

    vulkanRenderPass = VulkanRenderPass
    {
        .RenderPassId = renderPassLoader.RenderPassId,
        .SampleCount = renderPassLoader.RenderAttachmentList[0].SampleCount >= vulkanSystem.MaxSampleCount ? vulkanSystem.MaxSampleCount : renderPassLoader.RenderAttachmentList[0].SampleCount,
        .InputTextureIdList = renderPassLoader.InputTextureList,
        .ClearValueList = renderPassLoader.ClearValueList,
        .RenderPassResolution = renderPassLoader.UseDefaultSwapChainResolution ? ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height) : ivec2(renderPassLoader.RenderPassWidth, renderPassLoader.RenderPassHeight),
        .IsRenderedToSwapchain = renderPassLoader.IsRenderedToSwapchain
    };
    renderSystem.BuildRenderPass(vulkanRenderPass, renderPassLoader);
    shaderSystem.LoadShaderPipelineStructPrototypes(Vector<String> { configSystem.TextureAssetRenderer });

    nlohmann::json pipelineJson = fileSystem.LoadJsonFile(renderPassLoader.RenderPipelineList.front().c_str());
    RenderPipelineLoader renderPipelineLoader = pipelineJson.get<RenderPipelineLoader>();
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = vulkanRenderPass.SampleCount;
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = vulkanRenderPass.SampleCount;
    renderPipelineLoader.RenderPassId = vulkanRenderPass.RenderPassId;
    renderPipelineLoader.RenderPass = vulkanRenderPass.RenderPass;
    renderPipelineLoader.RenderPassResolution = vulkanRenderPass.RenderPassResolution;
    renderPipelineLoader.ShaderPiplineInfo = shaderSystem.LoadPipelineShaderData(Vector<String> { pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] });

    renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[0].DescriptorCount = 1;
    renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[0].DescriptorBufferInfo = {};
    for(int x = 1; x < renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.size(); x++)
    { 
        renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = 1;
        renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = {};
    }

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    Vector<VkDescriptorSet> descriptorSetList(1, VK_NULL_HANDLE);
    VkDescriptorPool descriptorPool = renderSystem.CreatePipelineDescriptorPool(renderPipelineLoader);
    Vector<VkDescriptorSetLayout> descriptorSetLayoutList = renderSystem.CreatePipelineDescriptorSetLayout(renderPipelineLoader);

    Vector<uint32> variableCounts(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.size(), 1);
    VkDescriptorSetVariableDescriptorCountAllocateInfo varAllocInfo = {};
    varAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    varAllocInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSetLayoutList.size());
    varAllocInfo.pDescriptorCounts = variableCounts.data();

    for (int x = 0; x < descriptorSetLayoutList.size(); x++)
    {
        VkDescriptorSetAllocateInfo allocInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = &varAllocInfo,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &descriptorSetLayoutList[x],
        };
        vkAllocateDescriptorSets(vulkanSystem.Device, &allocInfo, &descriptorSetList[x]);
    }

    VkPipelineLayout pipelineLayout = renderSystem.CreatePipelineLayout(renderPipelineLoader, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
    VkPipeline pipeline = renderSystem.CreatePipeline(renderPipelineLoader, pipelineCache, pipelineLayout, descriptorSetList.data(), descriptorSetList.size());

    vulkanRenderPipeline = VulkanPipeline
    {
        .RenderPipelineId = renderPipelineLoader.PipelineId,
        .DescriptorPool = descriptorPool,
        .DescriptorSetLayoutList = descriptorSetLayoutList,
        .DescriptorSetList = descriptorSetList,
        .Pipeline = pipeline,
        .PipelineLayout = pipelineLayout,
        .PipelineCache = pipelineCache
    };
}

void MaterialBakerSystem::CreateTextureImage(Texture& texture, VkImageCreateInfo& imageCreateInfo, Vector<byte>& textureData, uint layerCount)
{
    VmaAllocationCreateInfo allocInfo =
    {
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    };
    if (imageCreateInfo.extent.width * imageCreateInfo.extent.height > 1024 * 1024)
    {
        allocInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    }

    VmaAllocationInfo allocOut = {};
    VmaAllocation allocation = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vmaCreateImage(bufferSystem.vmaAllocator, &imageCreateInfo, &allocInfo, &texture.textureImage, &allocation, &allocOut));

    texture.TextureAllocation = allocation;
    if (textureData.size() > 0)
    {
        VkBufferCreateInfo stagingBufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = textureData.size(),
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VmaAllocationCreateInfo stagingAllocInfo =
        {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        VmaAllocationInfo stagingAllocOut;
        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingAllocation = VK_NULL_HANDLE;
        VULKAN_THROW_IF_FAIL(vmaCreateBuffer(bufferSystem.vmaAllocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, &stagingAllocOut));

        void* mapped = stagingAllocOut.pMappedData;
        if (!mapped)
        {
            VULKAN_THROW_IF_FAIL(vmaMapMemory(bufferSystem.vmaAllocator, stagingAllocation, &mapped));
        }

        memcpy(mapped, textureData.data(), textureData.size());
        vmaFlushAllocation(bufferSystem.vmaAllocator, stagingAllocation, 0, textureData.size());
        if (!stagingAllocOut.pMappedData)
        {
            vmaUnmapMemory(bufferSystem.vmaAllocator, stagingAllocation);
        }

        std::vector<VkBufferImageCopy> copyRegions;
        copyRegions.reserve(imageCreateInfo.arrayLayers);
        TransitionImageLayout(texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        for (uint32 layer = 0; layer < imageCreateInfo.arrayLayers; ++layer)
        {
            VkBufferImageCopy copyRegion
            {
                .bufferOffset = layer * (textureData.size() / layerCount),
                .bufferRowLength = 0,
                .bufferImageHeight = 0,
                .imageSubresource
                    {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .mipLevel = 0,
                        .baseArrayLayer = layer,
                        .layerCount = 1
                    },
                .imageOffset = { 0, 0, 0 },
                .imageExtent = {
                    static_cast<uint32_t>(texture.width),
                    static_cast<uint32_t>(texture.height),
                    1
                }
            };
            copyRegions.push_back(copyRegion);
        }
        VkCommandBuffer commandBuffer = vulkanSystem.BeginSingleUseCommand();
        vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, texture.textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
        vulkanSystem.EndSingleUseCommand(commandBuffer);

        if (texture.mipMapLevels > 1)
        {
            TransitionImageLayout(texture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 0, 1);
        }
        else
        {
            TransitionImageLayout(texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        vmaDestroyBuffer(bufferSystem.vmaAllocator, stagingBuffer, stagingAllocation);
        texture.textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
}

void MaterialBakerSystem::CreateTextureView(Texture& texture, VkImageAspectFlags imageAspectFlags)
{
    VkImageAspectFlags aspect = imageAspectFlags;

    if (aspect == 0)
    {
        std::cout << "CreateTextureView: imageAspectFlags not set ? using auto-detect." << std::endl;

        bool isDepthFormat = (texture.textureByteFormat >= VK_FORMAT_D16_UNORM &&
            texture.textureByteFormat <= VK_FORMAT_D32_SFLOAT_S8_UINT) ||
            (texture.textureByteFormat == VK_FORMAT_X8_D24_UNORM_PACK32);

        if (isDepthFormat)
        {
            aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (texture.textureByteFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
                texture.textureByteFormat == VK_FORMAT_D24_UNORM_S8_UINT)
            {
                aspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else
        {
            aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    VkImageViewCreateInfo viewInfo =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = texture.textureImage,
        .viewType = texture.textureType == TextureTypeEnum::TextureType_SkyboxTexture ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
        .format = texture.textureByteFormat,
        .subresourceRange =
        {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = texture.mipMapLevels,
            .baseArrayLayer = 0,
            .layerCount = texture.textureType == TextureTypeEnum::TextureType_SkyboxTexture ? static_cast<uint>(6) : static_cast<uint>(1),
        }
    };
    VULKAN_THROW_IF_FAIL(vkCreateImageView(vulkanSystem.Device, &viewInfo, nullptr, &texture.textureView));
}

VkDescriptorImageInfo MaterialBakerSystem::GetTextureDescriptorbinding(Texture texture, VkSampler sampler)
{
    return VkDescriptorImageInfo
    {
        .sampler = sampler,
        .imageView = texture.textureView,
        .imageLayout = texture.textureImageLayout
    };
}

VkSampler MaterialBakerSystem::GetAlbedoMapSamplerSettings()
{
    VkSamplerCreateInfo samplerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = -0.5f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 16.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &samplerCreateInfo, nullptr, &sampler));
    return sampler;
}

VkSampler MaterialBakerSystem::GetNormalMapSamplerSettings()
{
    VkSamplerCreateInfo samplerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = -0.75f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 16.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &samplerCreateInfo, nullptr, &sampler));
    return sampler;
}

VkSampler MaterialBakerSystem::GetPackedORMMapSamplerSettings()
{
    VkSamplerCreateInfo samplerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 8.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &samplerCreateInfo, nullptr, &sampler));
    return sampler;
}

VkSampler MaterialBakerSystem::GetParallaxMapSamplerSettings()
{
    VkSamplerCreateInfo samplerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = -0.5f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 12.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &samplerCreateInfo, nullptr, &sampler));
    return sampler;
}

VkSampler MaterialBakerSystem::GetAlphaMapSamplerSettings()
{
    VkSamplerCreateInfo samplerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 8.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &samplerCreateInfo, nullptr, &sampler));
    return sampler;
}

void MaterialBakerSystem::TransitionImageLayout(Texture& texture, VkImageLayout newLayout, uint32 baseMipLevel, uint32 levelCount)
{
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    if (texture.textureImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL || texture.textureImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
    {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    VkImageMemoryBarrier barrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = texture.textureImageLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = texture.textureImage,
        .subresourceRange =
        {
            .aspectMask = aspectMask,
            .baseMipLevel = baseMipLevel,
            .levelCount = levelCount,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS
        }
    };

    VkAccessFlags srcAccess = 0;
    VkAccessFlags dstAccess = 0;
    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    if (barrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        srcAccess = 0;
        dstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstAccess = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstAccess = VK_ACCESS_TRANSFER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        srcAccess = VK_ACCESS_TRANSFER_READ_BIT;
        dstAccess = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;

    VkCommandBuffer commandBuffer = vulkanSystem.BeginSingleUseCommand();
    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    vulkanSystem.EndSingleUseCommand(commandBuffer);

    texture.textureImageLayout = newLayout;
}

void MaterialBakerSystem::CleanInputResources()
{
    auto destroyTexture = [&](Texture& tex) 
        {
        if (tex.textureSampler != VK_NULL_HANDLE) {
            vkDestroySampler(vulkanSystem.Device, tex.textureSampler, nullptr);
            tex.textureSampler = VK_NULL_HANDLE;
        }
        if (tex.textureView != VK_NULL_HANDLE) {
            vkDestroyImageView(vulkanSystem.Device, tex.textureView, nullptr);
            tex.textureView = VK_NULL_HANDLE;
        }
        if (tex.textureImage != VK_NULL_HANDLE) {
            vmaDestroyImage(bufferSystem.vmaAllocator, tex.textureImage, tex.TextureAllocation);
            tex.textureImage = VK_NULL_HANDLE;
            tex.TextureAllocation = nullptr;
        }
        };

    destroyTexture(material.AlbedoMap);
    destroyTexture(material.MetallicMap);
    destroyTexture(material.RoughnessMap);
    destroyTexture(material.ThicknessMap);
    destroyTexture(material.SubSurfaceScatteringMap);
    destroyTexture(material.SheenMap);
    destroyTexture(material.ClearCoatMap);
    destroyTexture(material.AmbientOcclusionMap);
    destroyTexture(material.NormalMap);
    destroyTexture(material.AlphaMap);
    destroyTexture(material.EmissionMap);
    destroyTexture(material.HeightMap);

    if (material.MaterialBufferId != SIZE_MAX && material.MaterialBufferId != UINT32_MAX) {
        bufferSystem.VulkanBufferMap.erase(material.MaterialBufferId);
        material.MaterialBufferId = SIZE_MAX;
    }
}

void MaterialBakerSystem::InitDummyAndSamplers()
{
    if (dummyTextureScalar.textureImage != VK_NULL_HANDLE) return;

    cachedAlbedoSampler = GetAlbedoMapSamplerSettings();
    cachedNormalSampler = GetNormalMapSamplerSettings();
    cachedPackedORMSampler = GetPackedORMMapSamplerSettings();
    cachedParallaxSampler = GetParallaxMapSamplerSettings();
    cachedAlphaSampler = GetAlphaMapSamplerSettings();

    auto createDummy = [this](Texture& tex, VkFormat format, const Vector<byte>& data) {
        tex = Texture{};
        tex.width = 1;
        tex.height = 1;
        tex.depth = 1;
        tex.mipMapLevels = 1;
        tex.textureType = TextureType_ColorTexture;
        tex.textureByteFormat = format;
        tex.textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        tex.sampleCount = VK_SAMPLE_COUNT_1_BIT;
        tex.colorChannels = ColorChannelUsed::ChannelRGBA;

        VkImageCreateInfo imgCI{};
        imgCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imgCI.imageType = VK_IMAGE_TYPE_2D;
        imgCI.format = format;
        imgCI.extent = { 1, 1, 1 };
        imgCI.mipLevels = 1;
        imgCI.arrayLayers = 1;
        imgCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imgCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imgCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imgCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        CreateTextureImage(tex, imgCI, const_cast<Vector<byte>&>(data), 1);
        CreateTextureView(tex, VK_IMAGE_ASPECT_COLOR_BIT);

        };

    Vector<byte> white = { 255, 255, 255, 255 };
    createDummy(dummyTextureScalar, VK_FORMAT_R8G8B8A8_UNORM, white);
    createDummy(dummyTextureColor, VK_FORMAT_R8G8B8A8_UNORM, white);

    Vector<byte> normalData = { 128, 128, 255, 255 };
    createDummy(dummyTextureNormal, VK_FORMAT_R8G8B8A8_UNORM, normalData);
}