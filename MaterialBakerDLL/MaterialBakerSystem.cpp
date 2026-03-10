#include "MaterialBakerSystem.h"
#include <EngineConfigSystem.h>
#include <TextureSystem.h>
#include <ShaderSystem.h>
#include <FileSystem.h>
#include "MaterialMemoryPoolSystem.h"
#include <from_json.h>
#include <regex>
#include "TextureSamplers.h"

MaterialBakerSystem& materialBakerSystem = MaterialBakerSystem::Get();

void MaterialBakerSystem::Run()
{
    std::cout << "Run Built" << std::endl;
    const String inDir = configSystem.MaterialSourceDirectory.c_str();
    std::filesystem::path outDir = configSystem.MaterialDstDirectory.c_str();
    std::filesystem::create_directories(outDir);
    std::cout << "create_directories Built" << std::endl;
    Vector<String> ext = { "json" };
    Vector<String> materialFiles = fileSystem.GetFilesFromDirectory(configSystem.MaterialSourceDirectory.c_str(), ext);
    std::cout << "GetFilesFromDirectory Built" << std::endl;
    shaderSystem.LoadShaderPipelineStructPrototypes(Vector<String>{configSystem.TextureAssetRenderer});
    std::cout << "LoadShaderPipelineStructPrototypes Built" << std::endl;
    materialMemoryPoolSystem.StartUp();
    std::cout << "materialMemoryPoolSystem Built" << std::endl;
    renderSystem.UsingMaterialBaker = true;
  /*  for (auto& materialPath : materialFiles)
    {*/
        std::filesystem::path src = materialFiles[0];
        nlohmann::json json = fileSystem.LoadJsonFile(materialFiles[0].c_str());
        std::filesystem::path dst = outDir / (src.filename().stem().string() + ".json");
        std::filesystem::path finalFilePath = outDir / (src.filename().stem().string());
        ivec2 resolution = ivec2(json["TextureSetResolution"][0], json["TextureSetResolution"][1]);
      /*  if (UpdateNeeded(materialPath))
        {*/
        std::cout << "Before LoadMaterial Built" << std::endl;
            LoadMaterial(materialFiles[0]);
            std::cout << "LoadMaterial Built" << std::endl;
            CleanRenderPass();
            std::cout << "CleanRenderPass Built" << std::endl;
            BuildRenderPass(resolution);
            std::cout << "BuildRenderPass Built" << std::endl;
      //       vulkanSystem.StartFrame();
  /*          VkCommandBuffer commandBuffer = vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex];
            Draw(commandBuffer);
            vulkanSystem.EndFrame(commandBuffer);*/
          //  textureBakerSystem.BakeTexture(materialFiles[0], finalFilePath.string(), vulkanRenderPass.RenderPassId);
          //  CleanInputResources();

            std::cout << "Baked: " << src.filename() << std::endl;
 /*       }
        else
        {
            continue;
        }*/
    //}
}

bool MaterialBakerSystem::UpdateNeeded(const String& materialPath)
{

    std::filesystem::path src = materialPath;
    std::filesystem::path dst = std::regex_replace(materialPath, std::regex("Import"), "");
    if (!std::filesystem::exists(dst))
    {
        return true;
    }

    nlohmann::json json = fileSystem.LoadJsonFile(materialPath.c_str());
    auto dstTime = std::filesystem::last_write_time(dst);
    auto is_newer = [&](const std::filesystem::path& texPath) -> bool
        {
            if (texPath.empty() || !std::filesystem::exists(texPath))
            {
                return false;
            }
            auto time = std::filesystem::last_write_time(texPath);
            return std::filesystem::last_write_time(texPath) > dstTime;
        };

    if (is_newer(src))
    {
        return true;
    }

    Vector<String> textureList
    {
        !json["AlbedoMap"].is_null() ? json["AlbedoMap"]["TextureFilePath"][0].get<String>() : String(),
        !json["MetallicMap"].is_null() ? json["MetallicMap"]["TextureFilePath"][0].get<String>() : String(),
        !json["RoughnessMap"].is_null() ? json["RoughnessMap"]["TextureFilePath"][0].get<String>() : String(),
        !json["ThicknessMap"].is_null() ? json["ThicknessMap"]["TextureFilePath"][0].get<String>() : String(),
        !json["SubSurfaceScatteringColorMap"].is_null() ? json["SubSurfaceScatteringColorMap"]["TextureFilePath"][0].get<String>() : String(),
        !json["SheenMap"].is_null() ? json["SheenMap"]["TextureFilePath"][0].get<String>() : String(),
        !json["ClearCoatMap"].is_null() ? json["ClearCoatMap"]["TextureFilePath"][0].get<String>() : String(),
        !json["AmbientOcclusionMap"].is_null() ? json["AmbientOcclusionMap"]["TextureFilePath"][0].get<String>() : String(),
        !json["NormalMap"].is_null() ? json["NormalMap"]["TextureFilePath"][0].get<String>() : String(),
        !json["AlphaMap"].is_null() ? json["AlphaMap"]["TextureFilePath"][0].get<String>() : String(),
        !json["EmissionMap"].is_null() ? json["EmissionMap"]["TextureFilePath"][0].get<String>() : String(),
        !json["HeightMap"].is_null() ? json["HeightMap"]["TextureFilePath"][0].get<String>() : String()
    };

    for (auto& textureMap : textureList)
    {
        String textureMapLocation = json.contains(textureMap) ? json[textureMap]["TextureFilePath"][0].get<String>() : String();
        if (is_newer(textureMap))
        {
            return true;
        }
    }

    printf("No update needed for %s\n", materialPath.c_str());
    return false;
}

void MaterialBakerSystem::Draw(VkCommandBuffer cmd)
{
    // Safety checks first
    if (vulkanRenderPass.RenderPass == VK_NULL_HANDLE || 
        vulkanRenderPass.FrameBufferList.empty() || 
        vulkanRenderPass.FrameBufferList[0] == VK_NULL_HANDLE) {
        std::cerr << "[Baker] Invalid render pass/fb\n";
        return;
    }

    // Transition ALL input textures RIGHT HERE in the same cmd buffer
    for (auto& tex : TextureList) {
        if (tex.textureImageLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            VkImageMemoryBarrier barrier = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .oldLayout = tex.textureImageLayout,
                .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = tex.textureImage,
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = tex.mipMapLevels,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            vkCmdPipelineBarrier(cmd,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,           // assume copies happened earlier
                                 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,    // before fragment shader reads
                                 0, 0, nullptr, 0, nullptr, 1, &barrier);

            tex.textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            std::cout << "[Baker] Transitioned texture " << tex.bindlessTextureIndex 
                      << " to SHADER_READ_ONLY\n";
        }
    }

    // Now safe to begin render pass
    VkViewport viewport = {0,0, (float)vulkanRenderPass.RenderPassResolution.x, (float)vulkanRenderPass.RenderPassResolution.y, 0,1};
    VkRect2D scissor = {{0,0}, {vulkanRenderPass.RenderPassResolution.x, vulkanRenderPass.RenderPassResolution.y}};

    VkRenderPassBeginInfo rpBegin = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = vulkanRenderPass.RenderPass,
        .framebuffer = vulkanRenderPass.FrameBufferList[0],
        .renderArea = scissor,
        .clearValueCount = (uint32_t)vulkanRenderPass.ClearValueList.size(),
        .pClearValues = vulkanRenderPass.ClearValueList.data()
    };

    vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPipeline.Pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            vulkanRenderPipeline.PipelineLayout, 0,
                            (uint32_t)vulkanRenderPipeline.DescriptorSetList.size(),
                            vulkanRenderPipeline.DescriptorSetList.data(), 0, nullptr);

    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRenderPass(cmd);


    std::cout << "Run Built" << std::endl;
    const String inDir = configSystem.MaterialSourceDirectory.c_str();
    std::filesystem::path outDir = configSystem.MaterialDstDirectory.c_str();
    std::filesystem::create_directories(outDir);
    std::cout << "create_directories Built" << std::endl;
    Vector<String> ext = { "json" };
    Vector<String> materialFiles = fileSystem.GetFilesFromDirectory(configSystem.MaterialSourceDirectory.c_str(), ext);
    std::cout << "GetFilesFromDirectory Built" << std::endl;
    shaderSystem.LoadShaderPipelineStructPrototypes(Vector<String>{configSystem.TextureAssetRenderer});
    std::cout << "LoadShaderPipelineStructPrototypes Built" << std::endl;
    materialMemoryPoolSystem.StartUp();
    std::cout << "materialMemoryPoolSystem Built" << std::endl;
    renderSystem.UsingMaterialBaker = true;
    /*  for (auto& materialPath : materialFiles)
      {*/
    std::filesystem::path src = materialFiles[0];
    nlohmann::json json = fileSystem.LoadJsonFile(materialFiles[0].c_str());
    std::filesystem::path dst = outDir / (src.filename().stem().string() + ".json");
    std::filesystem::path finalFilePath = outDir / (src.filename().stem().string());
    ivec2 resolution = ivec2(json["TextureSetResolution"][0], json["TextureSetResolution"][1]);
    textureBakerSystem.BakeTexture(materialFiles[0], finalFilePath.string(), vulkanRenderPass.RenderPassId);
    std::cout << "[Baker] Draw recorded successfully\n";
}

void MaterialBakerSystem::CleanRenderPass()
{
    if (vulkanRenderPass.RenderPass != VK_NULL_HANDLE)
    {
        for (auto& fb : vulkanRenderPass.FrameBufferList)
        {
            vkDestroyFramebuffer(vulkanSystem.Device, fb, nullptr);
        }
        for (auto& texture : textureSystem.FindRenderedTextureList(vulkanRenderPass.RenderPassId))
        {
            materialMemoryPoolSystem.FreeObject(BakerTexture2DMetadataBuffer, texture.bindlessTextureIndex);
            textureSystem.DestroyTexture(texture);
        }

        materialMemoryPoolSystem.FreeObject(BakerMaterialBuffer, 0);
        vkDestroyRenderPass(vulkanSystem.Device, vulkanRenderPass.RenderPass, nullptr);
        vkDestroyPipeline(vulkanSystem.Device, vulkanRenderPipeline.Pipeline, nullptr);
        vkDestroyPipelineLayout(vulkanSystem.Device, vulkanRenderPipeline.PipelineLayout, nullptr);
        for (auto& dsl : vulkanRenderPipeline.DescriptorSetLayoutList) {
            vkDestroyDescriptorSetLayout(vulkanSystem.Device, dsl, nullptr);
        }
    }
}

void MaterialBakerSystem::LoadMaterial(const String& materialPath)
{
    nlohmann::json json = fileSystem.LoadJsonFile(materialPath.c_str());
    ivec2 materialSetResolution = ivec2(json["TextureSetResolution"][0], json["TextureSetResolution"][1]);

    materialMemoryPoolSystem.AllocateObject(BakerMaterialBuffer);
    ImportMaterialShader& material = materialMemoryPoolSystem.UpdateMaterial(0);
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

    if (!json["AlbedoMap"].is_null())
    {
        std::cout << "Before AlbedoMap Built" << std::endl;
        TextureList.emplace_back(LoadTexture(json["AlbedoMap"].get<TextureLoader>()));
        material.AlbedoMap = TextureList.back().bindlessTextureIndex;
        std::cout << "After AlbedoMap Built" << std::endl;
    }
    if (!json["MetallicMap"].is_null())
    {
        std::cout << "Before MetallicMap Built" << std::endl;
        TextureList.emplace_back(LoadTexture(json["MetallicMap"].get<TextureLoader>()));
        material.MetallicMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["RoughnessMap"].is_null())
    {
        std::cout << "Before RoughnessMap Built" << std::endl;
        TextureList.emplace_back(LoadTexture(json["RoughnessMap"].get<TextureLoader>()));
        material.RoughnessMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["ThicknessMap"].is_null())
    {
        std::cout << "Before ThicknessMap Built" << std::endl;
        TextureList.emplace_back(LoadTexture(json["ThicknessMap"].get<TextureLoader>()));
        material.ThicknessMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["SubSurfaceScatteringColorMap"].is_null())
    {
        std::cout << "Before SubSurfaceScatteringColorMap Built" << std::endl;
        TextureList.emplace_back(LoadTexture(json["SubSurfaceScatteringColorMap"].get<TextureLoader>()));
        material.SubSurfaceScatteringColorMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["SheenMap"].is_null())
    {
        std::cout << "Before SheenMap Built" << std::endl;
        TextureList.emplace_back(LoadTexture(json["SheenMap"].get<TextureLoader>()));
        material.SheenMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["ClearCoatMap"].is_null())
    {
        std::cout << "Before ClearCoatMap Built" << std::endl;
        TextureList.emplace_back(LoadTexture(json["ClearCoatMap"].get<TextureLoader>()));
        material.ClearCoatMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["AmbientOcclusionMap"].is_null())
    {
        std::cout << "Before AmbientOcclusionMap Built" << std::endl;
        TextureList.emplace_back(LoadTexture(json["AmbientOcclusionMap"].get<TextureLoader>()));
        material.AmbientOcclusionMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["NormalMap"].is_null())
    {
        std::cout << "Before NormalMap Built" << std::endl;
        TextureList.emplace_back(LoadTexture(json["NormalMap"].get<TextureLoader>()));
        material.NormalMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["AlphaMap"].is_null())
    {
        std::cout << "Before AlphaMap Built" << std::endl;
        TextureList.emplace_back(LoadTexture(json["AlphaMap"].get<TextureLoader>()));
        material.AlphaMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["EmissionMap"].is_null())
    {
        std::cout << "Before EmissionMap Built" << std::endl;
        TextureList.emplace_back(LoadTexture(json["EmissionMap"].get<TextureLoader>()));
        material.EmissionMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["HeightMap"].is_null()) 
    {
        std::cout << "Before HeightMap Built" << std::endl;
        TextureList.emplace_back(LoadTexture(json["HeightMap"].get<TextureLoader>()));
        material.HeightMap = TextureList.back().bindlessTextureIndex;
    }
    materialMemoryPoolSystem.IsHeaderDirty = true;
    materialMemoryPoolSystem.IsDescriptorSetDirty = true;
}

void MaterialBakerSystem::BuildRenderPass(ivec2 renderPassResolution)
{
    RenderPassLoader renderPassLoader = fileSystem.LoadJsonFile(configSystem.TextureAssetRenderer.c_str()).get<RenderPassLoader>();
    renderPassLoader.RenderPassResolution = renderPassResolution;

    vulkanRenderPass = VulkanRenderPass
    {
        .RenderPassId = renderPassLoader.RenderPassId,
        .SubPassCount = renderPassLoader.SubPassCount,
        .SampleCount = renderPassLoader.SampleCount >= vulkanSystem.MaxSampleCount ? vulkanSystem.MaxSampleCount : renderPassLoader.SampleCount,
        .RenderPass = VK_NULL_HANDLE,
        .InputTextureIdList = renderPassLoader.InputTextureList,
        .FrameBufferList = Vector<VkFramebuffer>(),
        .ClearValueList = renderPassLoader.ClearValueList,
        .RenderPassResolution = renderPassLoader.UseDefaultSwapChainResolution ? ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height) : renderPassLoader.RenderPassResolution,
        .MaxPushConstantSize = 0,
        .IsRenderedToSwapchain = renderPassLoader.IsRenderedToSwapchain,
        .UseCubeMapMultiView = renderPassLoader.UseCubeMapMultiView,
        .IsCubeMapRenderPass = renderPassLoader.IsCubeMapRenderPass
    };

    renderSystem.RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId] = renderPassLoader.RenderAttachmentList;
    renderSystem.BuildRenderPass(vulkanRenderPass, renderPassLoader, false);
    renderSystem.BuildFrameBuffer(vulkanRenderPass);
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

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    Vector<VkDescriptorSet> descriptorSetList { materialMemoryPoolSystem.MaterialBakerBindlessDescriptorSet };
    Vector<VkDescriptorSetLayout> descriptorSetLayoutList{ materialMemoryPoolSystem.MaterialBakerBindlessDescriptorSetLayout };
    VkPipelineLayout pipelineLayout = renderSystem.CreatePipelineLayout(renderPipelineLoader, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
    VkPipeline pipeline = renderSystem.CreatePipeline(renderPipelineLoader, pipelineCache, pipelineLayout, descriptorSetList.data(), descriptorSetList.size());

    vulkanRenderPipeline = VulkanPipeline
    {
        .RenderPipelineId = renderPipelineLoader.PipelineId,
        .DescriptorSetLayoutList = descriptorSetLayoutList,
        .DescriptorSetList = descriptorSetList,
        .Pipeline = pipeline,
        .PipelineLayout = pipelineLayout,
        .PipelineCache = pipelineCache
    };
}

void MaterialBakerSystem::CleanInputResources()
{
    auto destroyTexture = [&](Texture& tex) 
        {
            if (tex.textureSampler != VK_NULL_HANDLE) {
                vkDestroySampler(vulkanSystem.Device, tex.textureSampler, nullptr);
                tex.textureSampler = VK_NULL_HANDLE;
            }
            if (!tex.textureViewList.empty()) {
                vkDestroyImageView(vulkanSystem.Device, tex.textureViewList.front(), nullptr);
                tex.textureViewList.front() = VK_NULL_HANDLE;
            }
            if (tex.textureImage != VK_NULL_HANDLE) {
                vmaDestroyImage(bufferSystem.vmaAllocator, tex.textureImage, tex.TextureAllocation);
                tex.textureImage = VK_NULL_HANDLE;
                tex.TextureAllocation = nullptr;
            }
        };

    for (auto& texture : TextureList)
    {
        destroyTexture(texture);
    }
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

        TransitionImageLayout(texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, VK_REMAINING_MIP_LEVELS);
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

    VkImageView imageView = VK_NULL_HANDLE;
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
    VULKAN_THROW_IF_FAIL(vkCreateImageView(vulkanSystem.Device, &viewInfo, nullptr, &imageView));
    texture.textureViewList.emplace_back(imageView);
}

Texture MaterialBakerSystem::LoadTexture(TextureLoader textureLoader)
{
    std::cout << "In LoadTexture Built" << std::endl;
    int width = 0;
    int height = 0;
    int textureChannels = 0;
    Vector<byte> textureData;
    for (size_t x = 0; x < textureLoader.TextureFilePath.size(); x++)
    {
        std::cout << "Befoer LoadFile Built" << std::endl;
        Vector<byte> layerData = fileSystem.LoadImageFile(textureLoader.TextureFilePath[x], width, height, textureChannels);
        std::cout << layerData.size() << std::endl;
        std::cout << "After LoadTexture Built" << std::endl;
        textureData.insert(textureData.end(), layerData.begin(), layerData.end());
        std::cout << "After insert Built" << std::endl;
    }

    std::cout << "Before Texture Allocate Built" << std::endl;
    Texture texture = Texture
    {
        .textureGuid = textureLoader.TextureId,
        .textureIndex = TextureList.size(),
        .bindlessTextureIndex = materialMemoryPoolSystem.AllocateObject(MaterialBakerMemoryPoolTypes::BakerMaterialBuffer),
        .width = width,
        .height = height,
        .depth = 1,
        .mipMapLevels = textureLoader.MipMapCount == UINT32_MAX ? static_cast<uint32>(std::floor(std::log2(std::max(width, height)))) + 1 : 1,
        .textureType = textureLoader.IsSkyBox ? TextureType_SkyboxTexture : TextureType_ColorTexture,
        .textureByteFormat = textureLoader.TextureByteFormat,
        .textureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .sampleCount = VK_SAMPLE_COUNT_1_BIT,
        .colorChannels = static_cast<ColorChannelUsed>(textureChannels)
    };
    std::cout << "Before Texture Built" << std::endl;
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
    std::cout << "Before Image Create Built" << std::endl;
    switch (texture.textureIndex)
    {
        case 0: texture.textureSampler = TextureSamplers::GetImportAlbedoMapSamplerSettings(); break;
        case 1: texture.textureSampler = TextureSamplers::GetImportNormalMapSamplerSettings(); break;
        case 2: texture.textureSampler = TextureSamplers::GetImportPackedORMMapSamplerSettings(); break;
        case 3: texture.textureSampler = TextureSamplers::GetImportParallaxMapSamplerSettings(); break;
        case 4: texture.textureSampler = TextureSamplers::GetImportAlphaMapSamplerSettings(); break;
        case 5: texture.textureSampler = TextureSamplers::GetImportThicknessMapSamplerSettings(); break;
        case 6: texture.textureSampler = TextureSamplers::GetImportSubSurfaceScatteringMapSamplerSettings(); break;
        case 7: texture.textureSampler = TextureSamplers::GetImportSheenMapSamplerSettings(); break;
        case 8: texture.textureSampler = TextureSamplers::GetImportClearCoatMapSamplerSettings(); break;
        case 9: texture.textureSampler = TextureSamplers::GetImportEmissionMapSamplerSettings(); break;
    }
    std::cout << "Before CreateTextureImage Built" << std::endl;
    CreateTextureImage(texture, imageCreateInfo, textureData, textureLoader.TextureFilePath.size());
    std::cout << "After CreateTextureImage Built" << std::endl;
    CreateTextureView(texture, textureLoader.ImageType);
    std::cout << "After CreateTextureView Built" << std::endl;
    materialMemoryPoolSystem.UpdateTextureDescriptorSet(texture, materialMemoryPoolSystem.BakerTexture2DBinding);
    std::cout << "Update DescriptorSet Built" << std::endl;
    return texture;
}