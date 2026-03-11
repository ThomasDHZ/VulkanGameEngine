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
    const String inDir = configSystem.MaterialSourceDirectory.c_str();
    std::filesystem::path outDir = configSystem.MaterialDstDirectory.c_str();
    std::filesystem::create_directories(outDir);
    Vector<String> ext = { "json" };
    Vector<String> materialFiles = fileSystem.GetFilesFromDirectory(configSystem.MaterialSourceDirectory.c_str(), ext);
    shaderSystem.LoadShaderPipelineStructPrototypes(Vector<String>{configSystem.TextureAssetRenderer});
    materialMemoryPoolSystem.StartUp();
    renderSystem.UsingMaterialBaker = true;
    for (auto& materialPath : materialFiles)
    {
        std::filesystem::path src = materialPath;
        nlohmann::json json = fileSystem.LoadJsonFile(materialPath.c_str());
        std::filesystem::path dst = outDir / (src.filename().stem().string() + ".json");
        std::filesystem::path finalFilePath = outDir / (src.filename().stem().string());
        ivec2 resolution = ivec2(json["TextureSetResolution"][0], json["TextureSetResolution"][1]);
        /*  if (UpdateNeeded(materialPath))
          {*/
        LoadMaterial(materialPath);
        BuildRenderPass(resolution);
       // vulkanSystem.StartFrame();
        VkCommandBuffer commandBuffer = vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex];
        Draw(commandBuffer);
       //vulkanSystem.EndFrame(commandBuffer);
        textureBakerSystem.BakeTexture(materialPath, finalFilePath.string(), vulkanRenderPass.RenderPassId);
        vkQueueWaitIdle(vulkanSystem.GraphicsQueue);
        CleanRenderPass();
      //  CleanInputResources();

        std::cout << "Baked: " << src.filename() << std::endl;
        //}
       /* else
        {
            continue;
        }*/
    }

    for (auto dsl : vulkanRenderPipeline.DescriptorSetLayoutList) 
    {
        vkDestroyDescriptorSetLayout(vulkanSystem.Device, dsl, nullptr);
    }
    vulkanRenderPipeline.DescriptorSetLayoutList.clear();
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
    return false;
}

void MaterialBakerSystem::Draw(VkCommandBuffer& commandBuffer2)
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
        .framebuffer = vulkanRenderPass.FrameBufferList[0],
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
    
    for (const auto& tex : TextureList) {
        if (tex.textureImageLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            char layoutName[64];
            switch (tex.textureImageLayout) {
            case VK_IMAGE_LAYOUT_UNDEFINED: strcpy(layoutName, "UNDEFINED"); break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: strcpy(layoutName, "TRANSFER_SRC_OPTIMAL"); break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: strcpy(layoutName, "TRANSFER_DST_OPTIMAL"); break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: strcpy(layoutName, "SHADER_READ_ONLY_OPTIMAL"); break;
            default: sprintf(layoutName, "0x%08x", tex.textureImageLayout); break;
            }
            printf("Texture bindless idx %u (%dx%d) is in wrong layout: %s\n",
                tex.bindlessTextureIndex, tex.width, tex.height, layoutName);
        }
    }

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPipeline.PipelineLayout, 0, vulkanRenderPipeline.DescriptorSetList.size(), vulkanRenderPipeline.DescriptorSetList.data(), 0, nullptr);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    VULKAN_CHECK(vkEndCommandBuffer(commandBuffer));
    VULKAN_CHECK(vkCreateFence(vulkanSystem.Device, &fenceCreateInfo, nullptr, &fence));
    VULKAN_CHECK(vkQueueSubmit(vulkanSystem.GraphicsQueue, 1, &submitInfo, fence));
    VULKAN_CHECK(vkWaitForFences(vulkanSystem.Device, 1, &fence, VK_TRUE, UINT64_MAX * 1000ull));
    vkDestroyFence(vulkanSystem.Device, fence, nullptr);
    vulkanSystem.EndSingleUseCommand(commandBuffer);
}

void MaterialBakerSystem::CleanRenderPass()
{
    if (vulkanRenderPass.RenderPass == VK_NULL_HANDLE) {
        return;
    }

    vkQueueWaitIdle(vulkanSystem.GraphicsQueue);
    for (auto fb : vulkanRenderPass.FrameBufferList) 
    {
        vkDestroyFramebuffer(vulkanSystem.Device, fb, nullptr);
    }
    vulkanRenderPass.FrameBufferList.clear();

    for (auto& tex : TextureList) 
    {
        if(tex.bindlessTextureIndex != UINT32_MAX) materialMemoryPoolSystem.FreeObject(BakerTexture2DMetadataBuffer, tex.bindlessTextureIndex);
        textureSystem.DestroyTexture(tex);
    }
    TextureList.clear();

    vkDestroyRenderPass(vulkanSystem.Device, vulkanRenderPass.RenderPass, nullptr);
    vulkanRenderPass.RenderPass = VK_NULL_HANDLE;

    vkDestroyPipeline(vulkanSystem.Device, vulkanRenderPipeline.Pipeline, nullptr);
    vulkanRenderPipeline.Pipeline = VK_NULL_HANDLE;

    vkDestroyPipelineLayout(vulkanSystem.Device, vulkanRenderPipeline.PipelineLayout, nullptr);
    vulkanRenderPipeline.PipelineLayout = VK_NULL_HANDLE;
    vulkanRenderPass = VulkanRenderPass{};
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
        TextureList.emplace_back(LoadTexture(json["AlbedoMap"].get<TextureLoader>()));
        material.AlbedoMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["MetallicMap"].is_null())
    {
        TextureList.emplace_back(LoadTexture(json["MetallicMap"].get<TextureLoader>()));
        material.MetallicMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["RoughnessMap"].is_null())
    {
        TextureList.emplace_back(LoadTexture(json["RoughnessMap"].get<TextureLoader>()));
        material.RoughnessMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["ThicknessMap"].is_null())
    {
        TextureList.emplace_back(LoadTexture(json["ThicknessMap"].get<TextureLoader>()));
        material.ThicknessMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["SubSurfaceScatteringColorMap"].is_null())
    {
        TextureList.emplace_back(LoadTexture(json["SubSurfaceScatteringColorMap"].get<TextureLoader>()));
        material.SubSurfaceScatteringColorMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["SheenMap"].is_null())
    {
        TextureList.emplace_back(LoadTexture(json["SheenMap"].get<TextureLoader>()));
        material.SheenMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["ClearCoatMap"].is_null())
    {
        TextureList.emplace_back(LoadTexture(json["ClearCoatMap"].get<TextureLoader>()));
        material.ClearCoatMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["AmbientOcclusionMap"].is_null())
    {
        TextureList.emplace_back(LoadTexture(json["AmbientOcclusionMap"].get<TextureLoader>()));
        material.AmbientOcclusionMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["NormalMap"].is_null())
    {
        TextureList.emplace_back(LoadTexture(json["NormalMap"].get<TextureLoader>()));
        material.NormalMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["AlphaMap"].is_null())
    {
        TextureList.emplace_back(LoadTexture(json["AlphaMap"].get<TextureLoader>()));
        material.AlphaMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["EmissionMap"].is_null())
    {
        TextureList.emplace_back(LoadTexture(json["EmissionMap"].get<TextureLoader>()));
        material.EmissionMap = TextureList.back().bindlessTextureIndex;
    }
    if (!json["HeightMap"].is_null()) 
    {
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
        materialMemoryPoolSystem.FreeObject(BakerTexture2DMetadataBuffer, texture.bindlessTextureIndex);
    }
    materialMemoryPoolSystem.FreeObject(BakerMaterialBuffer, 0);
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
    int width = 0;
    int height = 0;
    int textureChannels = 0;
    Vector<byte> textureData;
    for (size_t x = 0; x < textureLoader.TextureFilePath.size(); x++)
    {
        Vector<byte> layerData = fileSystem.LoadImageFile(textureLoader.TextureFilePath[x], width, height, textureChannels);
        textureData.insert(textureData.end(), layerData.begin(), layerData.end());
    }

    Texture texture = Texture
    {
        .textureGuid = textureLoader.TextureId,
        .textureIndex = TextureList.size(),
        .bindlessTextureIndex = materialMemoryPoolSystem.AllocateObject(MaterialBakerMemoryPoolTypes::BakerTexture2DMetadataBuffer),
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
    CreateTextureImage(texture, imageCreateInfo, textureData, textureLoader.TextureFilePath.size());
    CreateTextureView(texture, textureLoader.ImageType);
    materialMemoryPoolSystem.UpdateTextureDescriptorSet(texture, materialMemoryPoolSystem.BakerTexture2DBinding);

    TransitionImageLayout(texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, VK_REMAINING_MIP_LEVELS);
    texture.textureImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    return texture;
}