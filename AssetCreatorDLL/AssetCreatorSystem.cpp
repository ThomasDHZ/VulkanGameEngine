#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stb/stb_image_write.h>

#include "AssetCreatorSystem.h"
#include <ShaderSystem.h>
#include <FileSystem.h>
#include <EngineConfigSystem.h>
#include <TextureSystem.h>
#include <from_json.h>
#include <stb_image_write.h>

AssetCreatorSystem& assetCreatorSystem = AssetCreatorSystem::Get();

void AssetCreatorSystem::MaterialUpdate(Material& material)
{
    const uint AlbedoMapId = material.AlbedoMapId != VkGuid() ? textureSystem.FindTexture(material.AlbedoMapId).textureIndex : SIZE_MAX;
    const uint MetallicMapId = material.MetallicMapId != VkGuid() ? textureSystem.FindTexture(material.MetallicMapId).textureIndex : SIZE_MAX;
    const uint RoughnessMapId = material.RoughnessMapId != VkGuid() ? textureSystem.FindTexture(material.RoughnessMapId).textureIndex : SIZE_MAX;
    const uint ThicknessMapId = material.ThicknessMapId != VkGuid() ? textureSystem.FindTexture(material.ThicknessMapId).textureIndex : SIZE_MAX;
    const uint SubSurfaceScatteringMapId = material.SubSurfaceScatteringMapId != VkGuid() ? textureSystem.FindTexture(material.SubSurfaceScatteringMapId).textureIndex : SIZE_MAX;
    const uint SheenMapId = material.SheenMapId != VkGuid() ? textureSystem.FindTexture(material.SheenMapId).textureIndex : SIZE_MAX;
    const uint ClearCoatMapId = material.ClearCoatMapId != VkGuid() ? textureSystem.FindTexture(material.ClearCoatMapId).textureIndex : SIZE_MAX;
    const uint AmbientOcclusionMapId = material.AmbientOcclusionMapId != VkGuid() ? textureSystem.FindTexture(material.AmbientOcclusionMapId).textureIndex : SIZE_MAX;
    const uint NormalMapId = material.NormalMapId != VkGuid() ? textureSystem.FindTexture(material.NormalMapId).textureIndex : SIZE_MAX;
    const uint AlphaMapId = material.AlphaMapId != VkGuid() ? textureSystem.FindTexture(material.AlphaMapId).textureIndex : SIZE_MAX;
    const uint EmissionMapId = material.EmissionMapId != VkGuid() ? textureSystem.FindTexture(material.EmissionMapId).textureIndex : SIZE_MAX;
    const uint HeightMapId = material.HeightMapId != VkGuid() ? textureSystem.FindTexture(material.HeightMapId).textureIndex : SIZE_MAX;

    ShaderStructDLL& shaderStruct = shaderSystem.FindShaderStruct(material.MaterialBufferId);
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
    shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AlbedoMap", AlbedoMapId);
    shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "MetallicMap", MetallicMapId);
    shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "RoughnessMap", RoughnessMapId);
    shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "ThicknessMap", ThicknessMapId);
    shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "SubSurfaceScatteringColorMap", SubSurfaceScatteringMapId);
    shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "SheenMap", SheenMapId);
    shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "ClearCoatMap", ClearCoatMapId);
    shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AmbientOcclusionMap", AmbientOcclusionMapId);
    shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "NormalMap", NormalMapId);
    shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AlphaMap", AlphaMapId);
    shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "EmissionMap", EmissionMapId);
    shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "HeightMap", HeightMapId);
    shaderSystem.UpdateShaderBuffer(shaderStruct, material.MaterialBufferId);
}

VkDescriptorImageInfo AssetCreatorSystem::GetTextureDescriptorbinding(Texture texture, VkSampler sampler)
{
    return VkDescriptorImageInfo
    {
        .sampler = sampler,
        .imageView = texture.textureView,
        .imageLayout = texture.textureImageLayout
    };
}

void AssetCreatorSystem::BuildRenderPass(const String& materialPath)
{
    nlohmann::json json = fileSystem.LoadJsonFile(materialPath.c_str());
    ivec2 materialSetResolution = ivec2(json["TextureSetResolution"][0], json["TextureSetResolution"][1]);

   
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
    material.AlbedoMapId = !json["AlbedoMap"].is_null() ? textureSystem.CreateTexture(json["AlbedoMap"]) : VkGuid();
    material.MetallicMapId = !json["MetallicMap"].is_null() ? textureSystem.CreateTexture(json["MetallicMap"]) :  VkGuid();
    material.RoughnessMapId = !json["RoughnessMap"].is_null() ? textureSystem.CreateTexture(json["RoughnessMap"]) :  VkGuid();
    material.ThicknessMapId = !json["ThicknessMap"].is_null() ? textureSystem.CreateTexture(json["ThicknessMap"]) :  VkGuid();
    material.SubSurfaceScatteringMapId = !json["SubSurfaceScatteringMap"].is_null() ? textureSystem.CreateTexture(json["SubSurfaceScatteringMap"]) :  VkGuid();
    material.SheenMapId = !json["SheenMap"].is_null() ? textureSystem.CreateTexture(json["SheenMap"]) :  VkGuid();
    material.ClearCoatMapId = !json["ClearCoatMap"].is_null() ? textureSystem.CreateTexture(json["ClearCoatMap"]) :  VkGuid();
    material.AmbientOcclusionMapId = !json["AmbientOcclusionMap"].is_null() ? textureSystem.CreateTexture(json["AmbientOcclusionMap"]) :  VkGuid();
    material.NormalMapId = !json["NormalMap"].is_null() ? textureSystem.CreateTexture(json["NormalMap"]) :  VkGuid();
    material.AlphaMapId = !json["AlphaMap"].is_null() ? textureSystem.CreateTexture(json["AlphaMap"]) :  VkGuid();
    material.EmissionMapId = !json["EmissionMap"].is_null() ? textureSystem.CreateTexture(json["EmissionMap"]) :  VkGuid();
    material.HeightMapId = !json["HeightMap"].is_null() ? textureSystem.CreateTexture(json["HeightMap"]) :  VkGuid();

    Vector<VkDescriptorImageInfo>                       textureBindingList = Vector<VkDescriptorImageInfo>();
    if (material.AlbedoMapId != VkGuid())               textureBindingList.emplace_back(GetTextureDescriptorbinding(textureSystem.FindTexture(material.AlbedoMapId), GetAlbedoMapSamplerSettings()));
    if (material.MetallicMapId != VkGuid())             textureBindingList.emplace_back(GetTextureDescriptorbinding(textureSystem.FindTexture(material.MetallicMapId), GetPackedORMMapSamplerSettings()));
    if (material.RoughnessMapId != VkGuid())            textureBindingList.emplace_back(GetTextureDescriptorbinding(textureSystem.FindTexture(material.RoughnessMapId), GetPackedORMMapSamplerSettings()));
    if (material.ThicknessMapId != VkGuid())            textureBindingList.emplace_back(GetTextureDescriptorbinding(textureSystem.FindTexture(material.ThicknessMapId), GetThicknessMapSamplerSettings()));
    if (material.SubSurfaceScatteringMapId != VkGuid()) textureBindingList.emplace_back(GetTextureDescriptorbinding(textureSystem.FindTexture(material.SubSurfaceScatteringMapId), GetSubSurfaceScatteringMapSamplerSettings()));
    if (material.SheenMapId != VkGuid())                textureBindingList.emplace_back(GetTextureDescriptorbinding(textureSystem.FindTexture(material.SheenMapId), GetSheenMapSamplerSettings()));
    if (material.ClearCoatMapId != VkGuid())            textureBindingList.emplace_back(GetTextureDescriptorbinding(textureSystem.FindTexture(material.ClearCoatMapId), GetClearCoatMapSamplerSettings()));
    if (material.AmbientOcclusionMapId != VkGuid())     textureBindingList.emplace_back(GetTextureDescriptorbinding(textureSystem.FindTexture(material.AmbientOcclusionMapId), GetPackedORMMapSamplerSettings()));
    if (material.NormalMapId != VkGuid())               textureBindingList.emplace_back(GetTextureDescriptorbinding(textureSystem.FindTexture(material.NormalMapId), GetNormalMapSamplerSettings()));
    if (material.AlphaMapId != VkGuid())                textureBindingList.emplace_back(GetTextureDescriptorbinding(textureSystem.FindTexture(material.AlphaMapId), GetAlphaMapSamplerSettings()));
    if (material.EmissionMapId != VkGuid())             textureBindingList.emplace_back(GetTextureDescriptorbinding(textureSystem.FindTexture(material.EmissionMapId), GetEmissionMapSamplerSettings()));
    if (material.HeightMapId != VkGuid())               textureBindingList.emplace_back(GetTextureDescriptorbinding(textureSystem.FindTexture(material.HeightMapId), GetParallaxMapSamplerSettings()));

    ShaderStructDLL shaderStruct = shaderSystem.CopyShaderStructProtoType("MaterialProperitiesBuffer");
    material.MaterialBufferId = bufferSystem.VMACreateDynamicBuffer(&shaderStruct, shaderStruct.ShaderBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    shaderSystem.PipelineShaderStructMap[material.MaterialBufferId] = shaderStruct;

    RenderPassLoader renderPassLoader = fileSystem.LoadJsonFile(configSystem.TextureAssetRenderer.c_str()).get<RenderPassLoader>();
    renderPassLoader.RenderPassWidth = materialSetResolution.x;
    renderPassLoader.RenderPassHeight = materialSetResolution.y;

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

    nlohmann::json pipelineJson = fileSystem.LoadJsonFile(renderPassLoader.RenderPipelineList.front().c_str());
    RenderPipelineLoader renderPipelineLoader = pipelineJson.get<RenderPipelineLoader>();
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = vulkanRenderPass.SampleCount;
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = vulkanRenderPass.SampleCount;
    renderPipelineLoader.RenderPassId = vulkanRenderPass.RenderPassId;
    renderPipelineLoader.RenderPass = vulkanRenderPass.RenderPass;
    renderPipelineLoader.RenderPassResolution = vulkanRenderPass.RenderPassResolution;
    renderPipelineLoader.ShaderPiplineInfo = shaderSystem.LoadPipelineShaderData(Vector<String> { pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] });
    renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[0].DescriptorCount = 1;
    renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[0].DescriptorBufferInfo = Vector<VkDescriptorBufferInfo>
    {
        VkDescriptorBufferInfo
        {
            .buffer = bufferSystem.FindVulkanBuffer(material.MaterialBufferId).Buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE
        }
    };
    renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[1].DescriptorCount = textureBindingList.size();
    renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[1].DescriptorImageInfo = textureBindingList;

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = renderSystem.CreatePipelineDescriptorPool(renderPipelineLoader);
    Vector<VkDescriptorSetLayout> descriptorSetLayoutList = renderSystem.CreatePipelineDescriptorSetLayout(renderPipelineLoader);
    Vector<VkDescriptorSet> descriptorSetList = renderSystem.AllocatePipelineDescriptorSets(renderPipelineLoader, descriptorPool, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
    renderSystem.UpdatePipelineDescriptorSets(renderPipelineLoader, descriptorSetList.data(), descriptorSetList.size());
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

void AssetCreatorSystem::Run(String materialPath)
{
    const String inDir = configSystem.MaterialSourceDirectory.c_str();
    std::filesystem::path outDir = configSystem.MaterialDstDirectory.c_str();
    std::filesystem::create_directories(outDir);

    Vector<Material> materialList;
    Vector<String> ext = { "json" };
    VkGuid dummyGuid = VkGuid();
    Vector<String> materialFiles = fileSystem.GetFilesFromDirectory(configSystem.MaterialSourceDirectory.c_str(), ext);
    for (auto& materialPath : materialFiles)
    {
        std::filesystem::path src = materialPath;
        std::filesystem::path dst = outDir / (src.filename().stem().string() + fileSystem.GetFileExtention(materialPath.c_str()) + ".json");
        if (std::filesystem::exists(dst) &&
            std::filesystem::last_write_time(dst) >= std::filesystem::last_write_time(src))
        {
            continue;
        }

        BuildRenderPass(materialPath);
        Draw();
        fileSystem.ExportTexture(vulkanRenderPass.RenderPassId, dst.string());
    }
}

void AssetCreatorSystem::Draw()
{
    MaterialUpdate(material);
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
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    VULKAN_THROW_IF_FAIL(vkEndCommandBuffer(commandBuffer));
    VULKAN_THROW_IF_FAIL(vkCreateFence(vulkanSystem.Device, &fenceCreateInfo, nullptr, &fence));
    VULKAN_THROW_IF_FAIL(vkQueueSubmit(vulkanSystem.GraphicsQueue, 1, &submitInfo, fence));
    VULKAN_THROW_IF_FAIL(vkWaitForFences(vulkanSystem.Device, 1, &fence, VK_TRUE, UINT64_MAX));
    vkDestroyFence(vulkanSystem.Device, fence, nullptr);
}

VkSampler AssetCreatorSystem::GetAlbedoMapSamplerSettings()
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

VkSampler AssetCreatorSystem::GetNormalMapSamplerSettings()
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

VkSampler AssetCreatorSystem::GetPackedORMMapSamplerSettings()
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

VkSampler AssetCreatorSystem::GetParallaxMapSamplerSettings()
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

VkSampler AssetCreatorSystem::GetAlphaMapSamplerSettings()
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

VkSampler AssetCreatorSystem::GetThicknessMapSamplerSettings()
{
    return GetPackedORMMapSamplerSettings();
}

VkSampler AssetCreatorSystem::GetSubSurfaceScatteringMapSamplerSettings()
{
    return GetPackedORMMapSamplerSettings();
}

VkSampler AssetCreatorSystem::GetSheenMapSamplerSettings()
{
    return GetPackedORMMapSamplerSettings();
}

VkSampler AssetCreatorSystem::GetClearCoatMapSamplerSettings()
{
    return GetPackedORMMapSamplerSettings();
}

VkSampler AssetCreatorSystem::GetEmissionMapSamplerSettings()
{
    return GetAlbedoMapSamplerSettings();
}
