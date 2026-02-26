#include "RenderSystem.h"
#include <vulkan/vulkan.h>
#include <iostream>
#include "MaterialSystem.h"
#include "MeshSystem.h"
#include "BufferSystem.h"
#include "LightSystem.h"
#include "RenderSystem.h"
#include "from_json.h"
#include <unordered_set>

RenderSystem& renderSystem = RenderSystem::Get();

void RenderSystem::StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface)
{
    vulkanSystem.RendererSetUp(windowHandle, instance, surface);
}

void RenderSystem::Update(void* windowHandle, LevelGuid& levelGuid, const float& deltaTime)
{
    if (vulkanSystem.RebuildRendererFlag)
    {
        RecreateSwapchain(windowHandle, deltaTime);
        vulkanSystem.RebuildRendererFlag = false;
    }
}

void RenderSystem::UpdateDescriptorSet(VulkanPipeline& pipeline, Vector<VkDescriptorBufferInfo>& descriptorInfo, uint32 descriptorBindingSet, uint32 descriptorBindingSlot)
{
    VkWriteDescriptorSet writeDescriptorSet = VkWriteDescriptorSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = pipeline.DescriptorSetList[descriptorBindingSet],
        .dstBinding = descriptorBindingSlot,
        .descriptorCount = static_cast<uint32>(descriptorInfo.size()),
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = descriptorInfo.data()
    };
    vkUpdateDescriptorSets(vulkanSystem.Device, 1, &writeDescriptorSet, 0, nullptr);
}

void RenderSystem::GenerateTexture(VkGuid& renderPassId)
{
    const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
    VulkanPipeline pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
    Vector<Texture> renderPassTexture = textureSystem.FindRenderedTextureList(renderPassId);

    if (renderPassTexture.empty()) 
    {
        std::cerr << "[GenerateTexture] No render texture found for pass " << renderPassId.ToString() << std::endl;
        return;
    }

    VkImage targetImage = renderPassTexture[0].textureImage;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;

    auto cleanup = [&]() {
        if (commandBuffer != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(vulkanSystem.Device, vulkanSystem.CommandPool, 1, &commandBuffer);
            commandBuffer = VK_NULL_HANDLE;
        }
        if (fence != VK_NULL_HANDLE) {
            vkDestroyFence(vulkanSystem.Device, fence, nullptr);
            fence = VK_NULL_HANDLE;
        }
        };

    VkCommandBufferAllocateInfo allocInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vulkanSystem.CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    if (vkAllocateCommandBuffers(vulkanSystem.Device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        std::cerr << "[GenerateTexture] Failed to allocate command buffer" << std::endl;
        cleanup();
        return;
    }

    VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };


    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderPass.RenderPassResolution.x),
        .height = static_cast<float>(renderPass.RenderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor
    {
        .offset = {0, 0},
        .extent = 
        {
            static_cast<uint32_t>(renderPass.RenderPassResolution.x),
            static_cast<uint32_t>(renderPass.RenderPassResolution.y)
        }
    };

    VkRenderPassBeginInfo renderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList[0],
        .renderArea = {
            .offset = {0, 0},
            .extent = {
                static_cast<uint32_t>(renderPass.RenderPassResolution.x),
                static_cast<uint32_t>(renderPass.RenderPassResolution.y)
            }
        },
        .clearValueCount = static_cast<uint32_t>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        std::cerr << "[GenerateTexture] Failed to begin command buffer" << std::endl;
        cleanup();
        return;
    }

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, static_cast<uint32_t>(pipeline.DescriptorSetList.size()), pipeline.DescriptorSetList.data(), 0, nullptr);
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        std::cerr << "[GenerateTexture] Failed to end command buffer" << std::endl;
        cleanup();
        return;
    }

    VkFenceCreateInfo fenceCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = 0
    };

    if (vkCreateFence(vulkanSystem.Device, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS) 
    {
        std::cerr << "[GenerateTexture] Failed to create fence" << std::endl;
        cleanup();
        return;
    }

    VkSubmitInfo submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    if (vkQueueSubmit(vulkanSystem.GraphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS) 
    {
        std::cerr << "[GenerateTexture] Failed to submit queue" << std::endl;
        cleanup();
        return;
    }

    if (vkWaitForFences(vulkanSystem.Device, 1, &fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) 
    {
        std::cerr << "[GenerateTexture] Failed to wait for fence" << std::endl;
        cleanup();
        return;
    }

    vkFreeCommandBuffers(vulkanSystem.Device, vulkanSystem.CommandPool, 1, &commandBuffer);
    vkDestroyFence(vulkanSystem.Device, fence, nullptr);
    commandBuffer = VK_NULL_HANDLE;
    fence = VK_NULL_HANDLE;
}

void RenderSystem::GenerateCubeMapTexture(VkGuid& renderPassId)
{
    const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
    VulkanPipeline skyboxPipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
    Vector<Texture> renderPassTexture = textureSystem.FindRenderedTextureList(renderPassId);
    const Vector<Mesh>& skyBoxList = meshSystem.FindMeshByMeshType(MeshTypeEnum::kMesh_SkyBoxMesh);

    if (renderPassTexture.empty() || renderPassTexture[0].textureImage == VK_NULL_HANDLE) {
        std::cerr << "[GenerateCubeMapTexture] No valid cubemap texture found for pass " << renderPassId.ToString() << std::endl;
        return;
    }

    VkImage targetCubemap = renderPassTexture[0].textureImage;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;

    auto cleanup = [&]() {
        if (commandBuffer != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(vulkanSystem.Device, vulkanSystem.CommandPool, 1, &commandBuffer);
            commandBuffer = VK_NULL_HANDLE;
        }
        if (fence != VK_NULL_HANDLE) {
            vkDestroyFence(vulkanSystem.Device, fence, nullptr);
            fence = VK_NULL_HANDLE;
        }
        };

    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vulkanSystem.CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    if (vkAllocateCommandBuffers(vulkanSystem.Device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        std::cerr << "[GenerateCubeMapTexture] Failed to allocate command buffer" << std::endl;
        cleanup();
        return;
    }

    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderPass.RenderPassResolution.x),
        .height = static_cast<float>(renderPass.RenderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor{
        .offset = {0, 0},
        .extent = {
            static_cast<uint32_t>(renderPass.RenderPassResolution.x),
            static_cast<uint32_t>(renderPass.RenderPassResolution.y)
        }
    };

    VkRenderPassBeginInfo renderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList[0],
        .renderArea = {
            .offset = {0, 0},
            .extent = {
                static_cast<uint32_t>(renderPass.RenderPassResolution.x),
                static_cast<uint32_t>(renderPass.RenderPassResolution.y)
            }
        },
        .clearValueCount = static_cast<uint32_t>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        std::cerr << "[GenerateCubeMapTexture] Failed to begin command buffer" << std::endl;
        cleanup();
        return;
    }

    VkDeviceSize offsets[] = { 0 };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline.PipelineLayout, 0, static_cast<uint32_t>(skyboxPipeline.DescriptorSetList.size()), skyboxPipeline.DescriptorSetList.data(), 0, nullptr);
    for (const auto& skyboxMesh : skyBoxList)
    {
        const MeshAssetData& meshAsset = meshSystem.FindMeshAssetData(skyboxMesh.SharedAssetId);
        const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.VertexBufferId).Buffer;
        const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(meshAsset.IndexBufferId).Buffer;

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, meshAsset.IndexCount, 1, 0, 0, 0);
    }
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        std::cerr << "[GenerateCubeMapTexture] Failed to end command buffer" << std::endl;
        cleanup();
        return;
    }

    VkFenceCreateInfo fenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = 0
    };

    if (vkCreateFence(vulkanSystem.Device, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS) {
        std::cerr << "[GenerateCubeMapTexture] Failed to create fence" << std::endl;
        cleanup();
        return;
    }

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    if (vkQueueSubmit(vulkanSystem.GraphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS) {
        std::cerr << "[GenerateCubeMapTexture] Failed to submit queue" << std::endl;
        cleanup();
        return;
    }

    if (vkWaitForFences(vulkanSystem.Device, 1, &fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
        std::cerr << "[GenerateCubeMapTexture] Failed to wait for fence" << std::endl;
        cleanup();
        return;
    }

    vkFreeCommandBuffers(vulkanSystem.Device, vulkanSystem.CommandPool, 1, &commandBuffer);
    vkDestroyFence(vulkanSystem.Device, fence, nullptr);
    commandBuffer = VK_NULL_HANDLE;
    fence = VK_NULL_HANDLE;
}

RenderPassGuid RenderSystem::LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath)
{
    RenderPassLoader renderPassLoader = fileSystem.LoadJsonFile(jsonPath).get<RenderPassLoader>();
    renderSystem.RenderPassLoaderJsonMap[renderPassLoader.RenderPassId] = jsonPath;

    VulkanRenderPass vulkanRenderPass = VulkanRenderPass
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
        .UseDefaultSwapChainResolution = renderPassLoader.UseDefaultSwapChainResolution,
        .IsRenderedToSwapchain = renderPassLoader.IsRenderedToSwapchain,
        .UseCubeMapMultiView = renderPassLoader.UseCubeMapMultiView,
        .IsCubeMapRenderPass = renderPassLoader.IsCubeMapRenderPass
    };
    RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId] = renderPassLoader.RenderAttachmentList;
    BuildRenderPass(vulkanRenderPass, renderPassLoader);
    BuildFrameBuffer(vulkanRenderPass);
    RenderPassMap[vulkanRenderPass.RenderPassId] = vulkanRenderPass;

    for(auto& renderPipeline : renderPassLoader.RenderPipelineList)
    {
        nlohmann::json pipelineJson = fileSystem.LoadJsonFile(renderPipeline.c_str());
        RenderPipelineLoader renderPipelineLoader = pipelineJson.get<RenderPipelineLoader>();
        renderPipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = vulkanRenderPass.SampleCount;
        renderPipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = vulkanRenderPass.SampleCount;
        renderPipelineLoader.RenderPassId = vulkanRenderPass.RenderPassId;
        renderPipelineLoader.RenderPass = vulkanRenderPass.RenderPass;
        renderPipelineLoader.RenderPassResolution = vulkanRenderPass.RenderPassResolution;
        renderPipelineLoader.ShaderPiplineInfo = shaderSystem.LoadPipelineShaderData(Vector<String> { pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] });
        if (!renderPipelineLoader.ShaderPiplineInfo.PushConstantList.empty())
        {
            RenderPassMap[vulkanRenderPass.RenderPassId].MaxPushConstantSize = renderPipelineLoader.ShaderPiplineInfo.PushConstantList[0].PushConstantSize > RenderPassMap[vulkanRenderPass.RenderPassId].MaxPushConstantSize ? renderPipelineLoader.ShaderPiplineInfo.PushConstantList[0].PushConstantSize : RenderPassMap[vulkanRenderPass.RenderPassId].MaxPushConstantSize;
        }

        VkPipelineCache pipelineCache = VK_NULL_HANDLE;
        PipelineBindingData(renderPipelineLoader);
        VkDescriptorPool descriptorPool = CreatePipelineDescriptorPool(renderPipelineLoader);
        Vector<VkDescriptorSetLayout> descriptorSetLayoutList = CreatePipelineDescriptorSetLayout(renderPipelineLoader);
        Vector<VkDescriptorSet> descriptorSetList = AllocatePipelineDescriptorSets(renderPipelineLoader, descriptorPool, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
        UpdatePipelineDescriptorSets(renderPipelineLoader, descriptorSetList.data(), descriptorSetList.size());
        VkPipelineLayout pipelineLayout = CreatePipelineLayout(renderPipelineLoader, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
        VkPipeline pipeline = CreatePipeline(renderPipelineLoader, pipelineCache, pipelineLayout, descriptorSetList.data(), descriptorSetList.size());

        renderSystem.RenderPipelineMap[renderPassLoader.RenderPassId].emplace_back(VulkanPipeline
        {
            .RenderPipelineId = renderPipelineLoader.PipelineId,
            .DescriptorPool = descriptorPool,
            .DescriptorSetLayoutList = descriptorSetLayoutList,
            .DescriptorSetList = descriptorSetList,
            .Pipeline = pipeline,
            .PipelineLayout = pipelineLayout,
            .PipelineCache = pipelineCache
        });
    }
    return renderPassLoader.RenderPassId;
}

RenderPassGuid RenderSystem::LoadRenderPass(LevelGuid& levelGuid, RenderPassLoader& renderPassLoader)
{
    VulkanRenderPass vulkanRenderPass = VulkanRenderPass
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
        .UseDefaultSwapChainResolution = renderPassLoader.UseDefaultSwapChainResolution,
        .IsRenderedToSwapchain = renderPassLoader.IsRenderedToSwapchain,
        .UseCubeMapMultiView = renderPassLoader.UseCubeMapMultiView,
        .IsCubeMapRenderPass = renderPassLoader.IsCubeMapRenderPass
    };
    RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId] = renderPassLoader.RenderAttachmentList;
    BuildRenderPass(vulkanRenderPass, renderPassLoader);
    BuildFrameBuffer(vulkanRenderPass);
    RenderPassMap[vulkanRenderPass.RenderPassId] = vulkanRenderPass;

    for (auto& renderPipeline : renderPassLoader.RenderPipelineList)
    {
        nlohmann::json pipelineJson = fileSystem.LoadJsonFile(renderPipeline.c_str());
        RenderPipelineLoader renderPipelineLoader = pipelineJson.get<RenderPipelineLoader>();
        renderPipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = vulkanRenderPass.SampleCount;
        renderPipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = vulkanRenderPass.SampleCount;
        renderPipelineLoader.RenderPassId = vulkanRenderPass.RenderPassId;
        renderPipelineLoader.RenderPass = vulkanRenderPass.RenderPass;
        renderPipelineLoader.RenderPassResolution = vulkanRenderPass.RenderPassResolution;
        renderPipelineLoader.ShaderPiplineInfo = shaderSystem.LoadPipelineShaderData(Vector<String> { pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] });

        VkPipelineCache pipelineCache = VK_NULL_HANDLE;
        PipelineBindingData(renderPipelineLoader);
        VkDescriptorPool descriptorPool = CreatePipelineDescriptorPool(renderPipelineLoader);
        Vector<VkDescriptorSetLayout> descriptorSetLayoutList = CreatePipelineDescriptorSetLayout(renderPipelineLoader);
        Vector<VkDescriptorSet> descriptorSetList = AllocatePipelineDescriptorSets(renderPipelineLoader, descriptorPool, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
        UpdatePipelineDescriptorSets(renderPipelineLoader, descriptorSetList.data(), descriptorSetList.size());
        VkPipelineLayout pipelineLayout = CreatePipelineLayout(renderPipelineLoader, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
        VkPipeline pipeline = CreatePipeline(renderPipelineLoader, pipelineCache, pipelineLayout, descriptorSetList.data(), descriptorSetList.size());

        renderSystem.RenderPipelineMap[renderPassLoader.RenderPassId].emplace_back(VulkanPipeline
            {
                .RenderPipelineId = renderPipelineLoader.PipelineId,
                .DescriptorPool = descriptorPool,
                .DescriptorSetLayoutList = descriptorSetLayoutList,
                .DescriptorSetList = descriptorSetList,
                .Pipeline = pipeline,
                .PipelineLayout = pipelineLayout,
                .PipelineCache = pipelineCache
            });
    }
    return renderPassLoader.RenderPassId;
}

void RenderSystem::RecreateSwapchain(void* windowHandle, const float& deltaTime)
{
    vkDeviceWaitIdle(vulkanSystem.Device);
    vulkanSystem.RebuildSwapChain(windowHandle);
    for (auto& renderPassPair : renderSystem.RenderPassMap)
    {
        VulkanRenderPass& renderPass = renderPassPair.second;
        RebuildSwapChain(renderPass);
    }
    // ImGui_RebuildSwapChain(renderer, imGuiRenderer);
}

void RenderSystem::RebuildSwapChain(VulkanRenderPass& vulkanRenderPass)
{
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, vulkanRenderPass.FrameBufferList.data(), vulkanSystem.SwapChainImageCount);
    if (vulkanRenderPass.IsRenderedToSwapchain)
    {
        vulkanRenderPass.RenderPassResolution = ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height);
        //DestoryRenderPassSwapChainTextures(renderedTextureList.data(), renderedTextureList.size(), depthTexture);
        BuildRenderPassAttachmentTextures(vulkanRenderPass);
        BuildFrameBuffer(vulkanRenderPass);
    }
    else
    {
        vulkanRenderPass.RenderPassResolution = vulkanRenderPass.UseDefaultSwapChainResolution ? ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height) : vulkanRenderPass.RenderPassResolution;
        BuildRenderPassAttachmentTextures(vulkanRenderPass);
        BuildFrameBuffer(vulkanRenderPass);
    }
}

void RenderSystem::BuildRenderPass(VulkanRenderPass& vulkanRenderPass, const RenderPassLoader& renderPassJsonLoader)
{
    VkAttachmentReference unusedRef = {};
    VkAttachmentReference depthReference = VkAttachmentReference();
    Vector<bool> useDepthReferences(vulkanRenderPass.SubPassCount, false);
    Vector<VkAttachmentReference> depthReferences(vulkanRenderPass.SubPassCount);
    Vector<VkSubpassDescription> subPassDescriptionList;
    Vector<Vector<VkAttachmentReference>> inputAttachmentReferenceList(vulkanRenderPass.SubPassCount);
    Vector<Vector<VkAttachmentReference>> colorAttachmentReferenceList(vulkanRenderPass.SubPassCount);
    Vector<Vector<VkAttachmentReference>> resolveAttachmentReferenceList(vulkanRenderPass.SubPassCount);
    Vector<Vector<VkSubpassDescription>> preserveAttachmentReferenceList(vulkanRenderPass.SubPassCount);
    Vector<RenderPassAttachmentTexture> renderPassAttachmentTextureInfoMap = RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId];
    for (int x = 0; x < vulkanRenderPass.SubPassCount; x++)
    {
        bool useDepthForThisSubpass = false;
        VkAttachmentReference depthRefForThisSubpass = {};
        for (int y = 0; y < renderPassAttachmentTextureInfoMap.size(); y++)
        {
            RenderPassAttachmentTexture renderAttachment = renderPassAttachmentTextureInfoMap[y];
            switch (renderAttachment.RenderAttachmentTypes[x])
            {
            case RenderAttachmentTypeEnum::ColorRenderedTexture: colorAttachmentReferenceList[x].emplace_back(VkAttachmentReference{ .attachment = static_cast<uint32>(y), .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }); break;
            case RenderAttachmentTypeEnum::InputAttachmentTexture: {
                bool is_depth = (renderAttachment.Format >= VK_FORMAT_D16_UNORM && renderAttachment.Format <= VK_FORMAT_D32_SFLOAT_S8_UINT); // Adjust for your formats
                VkImageLayout input_layout = is_depth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                inputAttachmentReferenceList[x].emplace_back(VkAttachmentReference{ .attachment = static_cast<uint32>(y), .layout = input_layout });
                break;
            }
            case RenderAttachmentTypeEnum::ResolveAttachmentTexture: resolveAttachmentReferenceList[x].emplace_back(VkAttachmentReference{ .attachment = static_cast<uint32>(y), .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }); break;
            case RenderAttachmentTypeEnum::DepthRenderedTexture:  depthRefForThisSubpass = VkAttachmentReference{ .attachment = (uint)(y), .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }; useDepthForThisSubpass = true; break;
            case RenderAttachmentTypeEnum::SkipSubPass: break;
            default: throw std::runtime_error("Case doesn't exist: RenderedTextureType");
            }
        }

        depthReferences[x] = depthRefForThisSubpass;
        useDepthReferences[x] = useDepthForThisSubpass;

        subPassDescriptionList.emplace_back(VkSubpassDescription{
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = static_cast<uint32_t>(inputAttachmentReferenceList[x].size()),
            .pInputAttachments = inputAttachmentReferenceList[x].empty() ? nullptr : inputAttachmentReferenceList[x].data(),
            .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferenceList[x].size()),
            .pColorAttachments = colorAttachmentReferenceList[x].empty() ? nullptr : colorAttachmentReferenceList[x].data(),
            .pResolveAttachments = resolveAttachmentReferenceList[x].empty() ? nullptr : resolveAttachmentReferenceList[x].data(),
            .pDepthStencilAttachment = useDepthReferences[x] ? &depthReferences[x] : nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
            });
    }

    Vector<VkAttachmentDescription> attachmentDescriptionList = BuildRenderPassAttachments(vulkanRenderPass);
    Vector<Texture> frameBufferTextureList = BuildRenderPassAttachmentTextures(vulkanRenderPass);

    VkRenderPassMultiviewCreateInfo multiviewCreateInfo{};
    if (renderPassJsonLoader.UseCubeMapMultiView)
    {
        const uint32_t viewMask = 0b00111111;
        const uint32_t correlationMask = 0b00111111;
        multiviewCreateInfo = VkRenderPassMultiviewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
            .subpassCount = 1,
            .pViewMasks = &viewMask,
            .correlationMaskCount = 1,
            .pCorrelationMasks = &correlationMask,
        };
    }

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = renderPassJsonLoader.UseCubeMapMultiView ? &multiviewCreateInfo : nullptr,
        .attachmentCount = static_cast<uint32_t>(attachmentDescriptionList.size()),
        .pAttachments = attachmentDescriptionList.data(),
        .subpassCount = static_cast<uint32_t>(subPassDescriptionList.size()),
        .pSubpasses = subPassDescriptionList.data(),
        .dependencyCount = static_cast<uint32_t>(renderPassJsonLoader.SubpassDependencyModelList.size()),
        .pDependencies = renderPassJsonLoader.SubpassDependencyModelList.data(),
    };
    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkanSystem.Device, &renderPassInfo, nullptr, &vulkanRenderPass.RenderPass));
}

Vector<VkAttachmentDescription> RenderSystem::BuildRenderPassAttachments(VulkanRenderPass& vulkanRenderPass)
{
    Vector<VkAttachmentDescription> attachmentDescriptionList;
    Vector<RenderPassAttachmentTexture> renderPassAttachmentTextureInfoList = RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId];
    for (int x = 0; x < renderPassAttachmentTextureInfoList.size(); x++)
    {
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        const RenderPassAttachmentTexture& renderAttachment = renderPassAttachmentTextureInfoList[x];
        switch (renderAttachment.RenderTextureType)
        {
            case RenderType_SwapChainTexture:      initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;         finalLayout = VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR;               break;
            case RenderType_OffscreenColorTexture: initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case RenderType_DepthBufferTexture:    initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;  break;
            case RenderType_GBufferTexture:        initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case RenderType_IrradianceTexture:     initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case RenderType_PrefilterTexture:      initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;         finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case RenderType_CubeMapTexture:        initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            default: throw std::runtime_error("Unknown RenderTextureType");
        }

        attachmentDescriptionList.emplace_back(VkAttachmentDescription
            {
            .format = renderAttachment.Format,
            .samples = vulkanRenderPass.SampleCount >= vulkanSystem.MaxSampleCount ? vulkanSystem.MaxSampleCount : vulkanRenderPass.SampleCount,
            .loadOp = renderAttachment.LoadOp,
            .storeOp = renderAttachment.StoreOp,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = initialLayout,
            .finalLayout = finalLayout
            });
    }
    return attachmentDescriptionList;
}

Vector<Texture> RenderSystem::BuildRenderPassAttachmentTextures(VulkanRenderPass& vulkanRenderPass)
{
    Texture depthTexture;
    Vector<Texture> renderedTextureList;
    Vector<Texture> frameBufferTextureList;
    Vector<RenderPassAttachmentTexture> renderPassAttachmentTextureInfoList = RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId];
    for (int x = 0; x < renderPassAttachmentTextureInfoList.size(); x++)
    {
        Texture texture = textureSystem.CreateRenderPassTexture(vulkanRenderPass, x);
        if (texture.textureType == TextureType_IrradianceMapTexture)
        {
            textureSystem.IrradianceMapId = textureSystem.CubeMapTextureList.size();
            textureSystem.CubeMapTextureList.emplace_back(texture);
            renderedTextureList.emplace_back(texture);
            frameBufferTextureList.emplace_back(texture);
        }
        else if (texture.textureType == TextureType_PrefilterMapTexture)
        {
            textureSystem.PrefilterMapId = textureSystem.CubeMapTextureList.size();
            textureSystem.CubeMapTextureList.emplace_back(texture);
            renderedTextureList.emplace_back(texture);
            frameBufferTextureList.emplace_back(texture);
        }
        else if (texture.textureType == TextureType_DepthTexture)
        {
            depthTexture = texture;
            renderedTextureList.emplace_back(texture);
            frameBufferTextureList.emplace_back(texture);
        }
        else
        {
            renderedTextureList.emplace_back(texture);
            frameBufferTextureList.emplace_back(texture);
        }
    }
    if (!renderedTextureList.empty()) textureSystem.AddRenderedTexture(vulkanRenderPass.RenderPassId, renderedTextureList);
    if (depthTexture.textureImage != VK_NULL_HANDLE) textureSystem.AddDepthTexture(vulkanRenderPass.RenderPassId, depthTexture);
    return frameBufferTextureList;
}

void RenderSystem::BuildFrameBuffer(VulkanRenderPass& vulkanRenderPass)
{
    Vector<Texture>  frameBufferAttachment = textureSystem.FindRenderedTextureList(vulkanRenderPass.RenderPassId);
    if (vulkanRenderPass.IsRenderedToSwapchain)
    {
        vulkanRenderPass.FrameBufferList.resize(vulkanSystem.SwapChainImageCount);
        for (size_t i = 0; i < vulkanSystem.SwapChainImageCount; ++i)
        {
            std::vector<VkImageView> attachments{ vulkanSystem.SwapChainImageViews[i] };
            VkFramebufferCreateInfo info = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = vulkanRenderPass.RenderPass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = static_cast<uint32_t>(vulkanRenderPass.RenderPassResolution.x),
                .height = static_cast<uint32_t>(vulkanRenderPass.RenderPassResolution.y),
                .layers = 1
            };
            VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkanSystem.Device, &info, nullptr, &vulkanRenderPass.FrameBufferList[i]));
        }
    }
    else if (!frameBufferAttachment.empty() &&
            (frameBufferAttachment[0].textureType == TextureType_SkyboxTexture ||
             frameBufferAttachment[0].textureType == TextureType_IrradianceMapTexture ||
             frameBufferAttachment[0].textureType == TextureType_PrefilterMapTexture))
    {
        uint32_t mipLevels = frameBufferAttachment[0].mipMapLevels;
        uint32_t baseSize = frameBufferAttachment[0].width;
        vulkanRenderPass.FrameBufferList.resize(mipLevels);
        for (uint32_t mip = 0; mip < mipLevels; ++mip)
        {
            uint32_t mipWidth = std::max(1u, baseSize >> mip);
            uint32_t mipHeight = std::max(1u, baseSize >> mip);
            Vector<VkImageView> attachments{ frameBufferAttachment[0].textureViewList[mip] };
            VkFramebufferCreateInfo info = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = vulkanRenderPass.RenderPass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = mipWidth,
                .height = mipHeight,
                .layers = vulkanRenderPass.UseCubeMapMultiView ? 1u : 6u
            };
            VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkanSystem.Device, &info, nullptr, &vulkanRenderPass.FrameBufferList[mip]));
        }
    }
    else
    {
        vulkanRenderPass.FrameBufferList.resize(1);
        std::vector<VkImageView> attachments;
        for (const auto& texture : frameBufferAttachment)
        {
            attachments.emplace_back(texture.textureViewList.front());
        }
        VkFramebufferCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = vulkanRenderPass.RenderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = static_cast<uint32_t>(vulkanRenderPass.RenderPassResolution.x),
            .height = static_cast<uint32_t>(vulkanRenderPass.RenderPassResolution.y),
            .layers = 1
        };
        VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkanSystem.Device, &info, nullptr, &vulkanRenderPass.FrameBufferList[0]));
    }
}

void RenderSystem::CreateGlobalBindlessDescriptorSets()
{
    Vector<VkDescriptorPoolSize> poolSizes =
    {
        VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1 },
        VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1 },
        VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 65536 }, // 2D textures
        VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 65536 }, // 3D textures
        VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 }   // cubemaps
    };
    VkDescriptorPoolCreateInfo poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 1,
        .poolSizeCount = static_cast<uint32>(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorPool(vulkanSystem.Device, &poolInfo, nullptr, &memoryPoolSystem.GlobalBindlessPool));

    Vector<VkDescriptorSetLayoutBinding> bindings = Vector<VkDescriptorSetLayoutBinding>
    {
        VkDescriptorSetLayoutBinding
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,  //Scenedata
            .stageFlags = VK_SHADER_STAGE_ALL
        },
         VkDescriptorSetLayoutBinding
        {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,  // array sized via variable count
            .stageFlags = VK_SHADER_STAGE_ALL
        },
        VkDescriptorSetLayoutBinding
        {
            .binding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,  // 2D textures array
            .stageFlags = VK_SHADER_STAGE_ALL
        },
        VkDescriptorSetLayoutBinding
        {
            .binding = 3,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,  // 3D textures array
            .stageFlags = VK_SHADER_STAGE_ALL
        },
        VkDescriptorSetLayoutBinding
        {
            .binding = 4,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,  // cubemaps array
            .stageFlags = VK_SHADER_STAGE_ALL
        }
    };
    Vector<VkDescriptorBindingFlags> flags =
    {
        VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT },
        VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT },
        VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT },  // 2D
        VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT },  // 3D
        VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT }   // cubemap
    };
    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = static_cast<uint32>(flags.size()),
        .pBindingFlags = flags.data()
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo = VkDescriptorSetLayoutCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &flagsInfo,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = static_cast<uint32>(bindings.size()),
        .pBindings = bindings.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorSetLayout(vulkanSystem.Device, &layoutInfo, nullptr, &memoryPoolSystem.GlobalBindlessDescriptorSetLayout));

    Vector<uint32> variableCounts = { 1, 1, 65536, 32, 1024 };
    VkDescriptorSetVariableDescriptorCountAllocateInfo varInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        .descriptorSetCount = static_cast<uint32>(variableCounts.size()),
        .pDescriptorCounts = variableCounts.data()
    };
    VkDescriptorSetAllocateInfo allocInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = &varInfo,
        .descriptorPool = memoryPoolSystem.GlobalBindlessPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &memoryPoolSystem.GlobalBindlessDescriptorSetLayout
    };
    VULKAN_THROW_IF_FAIL(vkAllocateDescriptorSets(vulkanSystem.Device, &allocInfo, &memoryPoolSystem.GlobalBindlessDescriptorSet));
}

VkDescriptorPool RenderSystem::CreatePipelineDescriptorPool(RenderPipelineLoader& renderPipelineLoader)
{
    Vector<VkDescriptorPoolSize> descriptorPoolSizeList = Vector<VkDescriptorPoolSize>();
    for (const auto& binding : renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList)
    {
        descriptorPoolSizeList.emplace_back(VkDescriptorPoolSize{
            .type = binding.DescripterType,
            .descriptorCount = static_cast<uint32>(binding.DescriptorCount)
            });
    }

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorPoolCreateInfo poolCreateInfo = VkDescriptorPoolCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.empty() ? 1000 : static_cast<uint32>(descriptorPoolSizeList.size()) * 1000,
        .poolSizeCount = static_cast<uint32>(descriptorPoolSizeList.size()),
        .pPoolSizes = descriptorPoolSizeList.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorPool(vulkanSystem.Device, &poolCreateInfo, nullptr, &descriptorPool));
    return descriptorPool;
}

Vector<VkDescriptorSetLayout> RenderSystem::CreatePipelineDescriptorSetLayout(RenderPipelineLoader& renderPipelineLoader)
{
    std::unordered_set<int> uniqueValues;
    std::for_each(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.begin(), renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.end(), [&](const ShaderDescriptorBindingDLL& binding) {
        uniqueValues.insert(binding.DescriptorSet);
        });
    size_t countDistinct = uniqueValues.size();

    Vector<Vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutBindingList = Vector<Vector<VkDescriptorSetLayoutBinding>>(countDistinct);
    for (auto& descriptorBinding : renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList)
    {
        descriptorSetLayoutBindingList[descriptorBinding.DescriptorSet].emplace_back(VkDescriptorSetLayoutBinding
            {
                .binding = descriptorBinding.Binding,
                .descriptorType = descriptorBinding.DescripterType,
                .descriptorCount = static_cast<uint32>(descriptorBinding.DescriptorCount),
                .stageFlags = descriptorBinding.ShaderStageFlags,
                .pImmutableSamplers = nullptr
            });
    }

    Vector<VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutCreateInfoList = Vector<VkDescriptorSetLayoutCreateInfo>(countDistinct);
    for (int x = 0; x < descriptorSetLayoutCreateInfoList.size(); x++)
    {
        descriptorSetLayoutCreateInfoList[x] = VkDescriptorSetLayoutCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV,
            .bindingCount = static_cast<uint32>(descriptorSetLayoutBindingList[x].size()),
            .pBindings = descriptorSetLayoutBindingList[x].data()
        };
    }

    Vector<VkDescriptorSetLayout> descriptorSetLayoutList = Vector<VkDescriptorSetLayout>(descriptorSetLayoutCreateInfoList.size());
    for (int x = 0; x < descriptorSetLayoutList.size(); x++)
    {
        vkCreateDescriptorSetLayout(vulkanSystem.Device, &descriptorSetLayoutCreateInfoList[x], nullptr, &descriptorSetLayoutList[x]);
    }
    return descriptorSetLayoutList;
}

Vector<VkDescriptorSet> RenderSystem::AllocatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader, const VkDescriptorPool& descriptorPool, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount)
{
    Vector<VkDescriptorSet> descriptorSetList = Vector<VkDescriptorSet>(descriptorSetLayoutCount, VK_NULL_HANDLE);
    for (int x = 0; x < descriptorSetLayoutCount; x++)
    {
        VkDescriptorSetAllocateInfo allocInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &descriptorSetLayoutList[x]
        };
        vkAllocateDescriptorSets(vulkanSystem.Device, &allocInfo, &descriptorSetList[x]);
    }
    return descriptorSetList;
}

void RenderSystem::UpdatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount)
{

    Span<VkDescriptorSet> descriptorSetLayouts(descriptorSetList, descriptorSetCount);
    for (int x = 0; x < descriptorSetLayouts.size(); x++)
    {
        Vector<VkWriteDescriptorSet> writeDescriptorSet = Vector<VkWriteDescriptorSet>();
        for (auto& descriptorSetBinding : renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList)
        {
            if (descriptorSetBinding.DescriptorSet != x) continue;
            writeDescriptorSet.emplace_back(VkWriteDescriptorSet
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = descriptorSetLayouts[x],
                    .dstBinding = descriptorSetBinding.Binding,
                    .dstArrayElement = 0,
                    .descriptorCount = static_cast<uint32>(descriptorSetBinding.DescriptorCount),
                    .descriptorType = descriptorSetBinding.DescripterType,
                    .pImageInfo = descriptorSetBinding.DescriptorImageInfo.data(),
                    .pBufferInfo = descriptorSetBinding.DescriptorBufferInfo.data(),
                    .pTexelBufferView = nullptr
                });
        }
        vkUpdateDescriptorSets(vulkanSystem.Device, static_cast<uint32>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
    }
}

VkPipelineLayout RenderSystem::CreatePipelineLayout(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount)
{
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    Vector<VkPushConstantRange> pushConstantRangeList = Vector<VkPushConstantRange>();
    if (!renderPipelineLoader.ShaderPiplineInfo.PushConstantList.empty())
    {
        pushConstantRangeList.emplace_back(VkPushConstantRange
            {
                .stageFlags = renderPipelineLoader.ShaderPiplineInfo.PushConstantList[0].ShaderStageFlags,
                .offset = 0,
                .size = static_cast<uint>(renderPipelineLoader.ShaderPiplineInfo.PushConstantList[0].PushConstantSize)
            });
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = VkPipelineLayoutCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = static_cast<uint32>(descriptorSetLayoutCount),
        .pSetLayouts = descriptorSetLayoutList,
        .pushConstantRangeCount = static_cast<uint32>(pushConstantRangeList.size()),
        .pPushConstantRanges = pushConstantRangeList.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreatePipelineLayout(vulkanSystem.Device, &pipelineLayoutInfo, nullptr, &pipelineLayout));
    return pipelineLayout;
}

VkPipeline RenderSystem::CreatePipeline(RenderPipelineLoader& renderPipelineLoader, VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount)
{
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = VkPipelineVertexInputStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = static_cast<uint>(renderPipelineLoader.ShaderPiplineInfo.VertexInputBindingList.size()),
        .pVertexBindingDescriptions = renderPipelineLoader.ShaderPiplineInfo.VertexInputBindingList.data(),
        .vertexAttributeDescriptionCount = static_cast<uint>(renderPipelineLoader.ShaderPiplineInfo.VertexInputAttributeList.size()),
        .pVertexAttributeDescriptions = renderPipelineLoader.ShaderPiplineInfo.VertexInputAttributeList.data()
    };

    for (auto& viewPort : renderPipelineLoader.ViewportList)
    {
        viewPort.width = static_cast<float>(renderPipelineLoader.RenderPassResolution.x);
        viewPort.height = static_cast<float>(renderPipelineLoader.RenderPassResolution.y);
    }

    for (auto& scissor : renderPipelineLoader.ScissorList)
    {
        scissor.extent.width = static_cast<float>(renderPipelineLoader.RenderPassResolution.x);
        scissor.extent.height = static_cast<float>(renderPipelineLoader.RenderPassResolution.y);
    }

    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = static_cast<uint32>(renderPipelineLoader.ViewportList.size() ? renderPipelineLoader.ViewportList.size() : 1),
        .pViewports = renderPipelineLoader.ViewportList.data(),
        .scissorCount = static_cast<uint32>(renderPipelineLoader.ScissorList.size() ? renderPipelineLoader.ScissorList.size() : 1),
        .pScissors = renderPipelineLoader.ScissorList.data()
    };

    Vector<VkDynamicState> dynamicStateList;
    if (renderPipelineLoader.ViewportList.empty() || renderPipelineLoader.ScissorList.empty())
    {
        dynamicStateList.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        dynamicStateList.push_back(VK_DYNAMIC_STATE_SCISSOR);
    }

    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = static_cast<uint32>(dynamicStateList.size()),
        .pDynamicStates = dynamicStateList.data()
    };

    Vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfoList = Vector<VkPipelineShaderStageCreateInfo>
    {
        shaderSystem.LoadShader(renderPipelineLoader.ShaderPiplineInfo.ShaderList[0].c_str(), VK_SHADER_STAGE_VERTEX_BIT),
        shaderSystem.LoadShader(renderPipelineLoader.ShaderPiplineInfo.ShaderList[1].c_str(), VK_SHADER_STAGE_FRAGMENT_BIT)
    };

    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfoModel = renderPipelineLoader.PipelineColorBlendStateCreateInfoModel;
    pipelineColorBlendStateCreateInfoModel.attachmentCount = renderPipelineLoader.PipelineColorBlendAttachmentStateList.size();
    pipelineColorBlendStateCreateInfoModel.pAttachments = renderPipelineLoader.PipelineColorBlendAttachmentStateList.data();

    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = renderPipelineLoader.PipelineMultisampleStateCreateInfo;
    pipelineMultisampleStateCreateInfo.rasterizationSamples = pipelineMultisampleStateCreateInfo.rasterizationSamples >= vulkanSystem.MaxSampleCount ? vulkanSystem.MaxSampleCount : pipelineMultisampleStateCreateInfo.rasterizationSamples;
    pipelineMultisampleStateCreateInfo.sampleShadingEnable = pipelineMultisampleStateCreateInfo.rasterizationSamples > VK_SAMPLE_COUNT_1_BIT ? VK_TRUE : VK_FALSE;
    pipelineMultisampleStateCreateInfo.pSampleMask = nullptr;

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = VkGraphicsPipelineCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = static_cast<uint32>(pipelineShaderStageCreateInfoList.size()),
        .pStages = pipelineShaderStageCreateInfoList.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &renderPipelineLoader.PipelineInputAssemblyStateCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &pipelineViewportStateCreateInfo,
        .pRasterizationState = &renderPipelineLoader.PipelineRasterizationStateCreateInfo,
        .pMultisampleState = &pipelineMultisampleStateCreateInfo,
        .pDepthStencilState = &renderPipelineLoader.PipelineDepthStencilStateCreateInfo,
        .pColorBlendState = &pipelineColorBlendStateCreateInfoModel,
        .pDynamicState = &pipelineDynamicStateCreateInfo,
        .layout = pipelineLayout,
        .renderPass = renderPipelineLoader.RenderPass,
        .subpass = renderPipelineLoader.SubPassId,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0
    };

    VULKAN_THROW_IF_FAIL(vkCreateGraphicsPipelines(vulkanSystem.Device, pipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline));
    for (auto& shader : pipelineShaderStageCreateInfoList)
    {
        vkDestroyShaderModule(vulkanSystem.Device, shader.module, nullptr);
    }

    return pipeline;
}

void RenderSystem::PipelineBindingData(RenderPipelineLoader& renderPipelineLoader)
{

        std::sort(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.begin(), renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.end(), [](const ShaderDescriptorBindingDLL& a, const ShaderDescriptorBindingDLL& b) {
            return a.DescriptorSet > b.DescriptorSet;
            });
 

    Vector<ShaderDescriptorBinding> bindingList;
    for (int x = 0; x < renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.size(); x++)
    {
        switch (renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBindingType)
        {
        case kSceneDataDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = 1;
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = memoryPoolSystem.GetSceneDataBufferDescriptor();
            break;
        }
        case kBindlessDataDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = 1;
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = memoryPoolSystem.GetBindlessDataBufferDescriptor();
            break;
        }
           /* case kMeshPropertiesDescriptor:
            {
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = meshSystem.GetMeshBufferInfo().size();
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = meshSystem.GetMeshBufferInfo();
                break;
            }*/
            case kTextureDescriptor:
            {
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetTexturePropertiesBuffer(renderPipelineLoader.RenderPassId).size();
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = renderSystem.GetTexturePropertiesBuffer(renderPipelineLoader.RenderPassId);
                break;
            }
       /*     case kMaterialDescriptor:
            {
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = materialSystem.GetMaterialBufferInfo().size();
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = materialSystem.GetMaterialBufferInfo();
                break;
            }
            case kDirectionalLightDescriptor:
            {
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = lightSystem.GetDirectionalLightPropertiesBuffer().size();
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = lightSystem.GetDirectionalLightPropertiesBuffer();
                break;
            }
            case kPointLightDescriptor:
            {
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = lightSystem.GetPointLightPropertiesBuffer().size();
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = lightSystem.GetPointLightPropertiesBuffer();
                break;
            }*/
            case kSkyBoxDescriptor:
            {
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetCubeMapTextureBuffer().size();
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = renderSystem.GetCubeMapTextureBuffer();
                break;
            }
            //case kIrradianceMapDescriptor:
            //{
            //    renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetIrradianceMapTextureBuffer().size();
            //    renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = renderSystem.GetIrradianceMapTextureBuffer();
            //    break;
            //}
            //case kPrefilterMapDescriptor:
            //{
            //    renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetPrefilterMapTextureBuffer().size();
            //    renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = renderSystem.GetPrefilterMapTextureBuffer();
            //    break;
            //}
            case kTexture3DDescriptor:
            {
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetTexture3DPropertiesBuffer(renderPipelineLoader.RenderPassId).size();
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = renderSystem.GetTexture3DPropertiesBuffer(renderPipelineLoader.RenderPassId);
                break;
            }
            case kSubpassInputDescriptor:
            {
                Texture inputTexture = textureSystem.FindRenderedTextureList(renderPipelineLoader.RenderPassId)[x];
                VkDescriptorImageInfo descriptorImage = VkDescriptorImageInfo
                {
                    .sampler = inputTexture.textureSampler,
                    .imageView = inputTexture.textureViewList.front(),
                    .imageLayout = inputTexture.textureImageLayout,
                };
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = 1;
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = Vector<VkDescriptorImageInfo>{ descriptorImage };
                break;
            }
    /*        case kBRDFMapDescriptor:
            {
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetBRDFMapTextureBuffer().size();
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = renderSystem.GetBRDFMapTextureBuffer();
                break;
            } */
       /*     case kEnvironmentMapDescriptor:
            {
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetBRDFMapTextureBuffer().size();
                renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = renderSystem.GetBRDFMapTextureBuffer();
                break;
            }*/
            default:
            {
                throw std::runtime_error("Binding case hasn't been handled yet");
            }
        }
    }
}

void RenderSystem::DestoryRenderPassSwapChainTextures(Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture)
{
    Vector<Texture> renderedTextureList = Vector<Texture>(&renderedTextureListPtr, &renderedTextureListPtr + renderedTextureCount);
    for (auto& renderedTexture : renderedTextureList)
    {
        textureSystem.DestroyTexture(renderedTexture);
    }
    std::memset(static_cast<void*>(&renderedTextureListPtr), 0x00, sizeof(Texture) * renderedTextureCount);
    renderedTextureCount = 0;
    renderedTextureList.clear();
}

void RenderSystem::DestroyRenderPass(VulkanRenderPass& renderPass)
{
    vulkanSystem.DestroyRenderPass(vulkanSystem.Device, &renderPass.RenderPass);
//    vulkanSystem.DestroyCommandBuffers(vulkanSystem.Device, &vulkanSystem.CommandPool, &renderPass.com, 1);
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, &renderPass.FrameBufferList[0], vulkanSystem.SwapChainImageCount);

    renderPass.RenderPassId = VkGuid();
    renderPass.SubPassCount = UINT32_MAX;
    renderPass.SampleCount = VK_SAMPLE_COUNT_1_BIT;
    renderPass.RenderPass = VK_NULL_HANDLE;
    renderPass.InputTextureIdList.clear();
    renderPass.FrameBufferList.clear();
    renderPass.ClearValueList.clear();
    renderPass.RenderPassResolution = ivec2();
    renderPass.MaxPushConstantSize = 0;
    renderPass.UseDefaultSwapChainResolution = false;
    renderPass.IsRenderedToSwapchain = false;
    renderPass.UseCubeMapMultiView = false;
    renderPass.IsCubeMapRenderPass = false;
}

void RenderSystem::Destroy()
{
    DestroyRenderPipelines();
    DestroyRenderPasses();
}

void RenderSystem::DestroyRenderPasses()
{
    for (auto& renderPass : renderSystem.RenderPassMap)
    {
        DestroyRenderPass(renderPass.second);
    }
    //renderSystem.RenderPassMap.clear();
}

void RenderSystem::DestroyRenderPipelines()
{
    for (auto& renderPipelineList : renderSystem.RenderPipelineMap)
    {
        for (auto& renderPipeline : renderPipelineList.second)
        {
            DestroyPipeline(renderPipeline);
        }
    }
    renderSystem.RenderPipelineMap.clear();
}

void RenderSystem::DestroyPipeline(VulkanPipeline& vulkanPipeline)
{
    vulkanPipeline.RenderPipelineId = VkGuid();
    vulkanSystem.DestroyPipeline(vulkanSystem.Device, &vulkanPipeline.Pipeline);
    vulkanSystem.DestroyPipelineLayout(vulkanSystem.Device, &vulkanPipeline.PipelineLayout);
    vulkanSystem.DestroyPipelineCache(vulkanSystem.Device, &vulkanPipeline.PipelineCache);
    vulkanSystem.DestroyDescriptorPool(vulkanSystem.Device, &vulkanPipeline.DescriptorPool);
}

void RenderSystem::DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList)
{
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, frameBufferList.data(), frameBufferList.size());
}

void RenderSystem::DestroyCommandBuffers(VkCommandBuffer& commandBuffer)
{
    vulkanSystem.DestroyCommandBuffers(vulkanSystem.Device, &vulkanSystem.CommandPool, &commandBuffer, 1);
}

void RenderSystem::DestroyBuffer(VkBuffer& buffer)
{
    vulkanSystem.DestroyBuffer(vulkanSystem.Device, &buffer);
}

Vector<VkDescriptorImageInfo> RenderSystem::GetTexturePropertiesBuffer(const RenderPassGuid& renderPassGuid)
{
    Vector<Texture> textureList;
    const VulkanRenderPass& renderPass = FindRenderPass(renderPassGuid);
    if (renderPass.InputTextureIdList.size() > 0)
    {
        for (auto& inputTexture : renderPass.InputTextureIdList)
        {
            Texture texture = textureSystem.FindTexture(inputTexture);
            if (!texture.RenderedCubeMapView)
            {
                textureList.emplace_back(texture);
            }
        }
    }
    else
    {
        textureList = textureSystem.TextureList;
    }

    Vector<VkDescriptorImageInfo>	texturePropertiesBuffer;
    if (textureList.empty())
    {
        VkSamplerCreateInfo NullSamplerInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = 16.0f,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0,
            .maxLod = 0,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };

        VkSampler nullSampler = VK_NULL_HANDLE;
        if (vkCreateSampler(vulkanSystem.Device, &NullSamplerInfo, nullptr, &nullSampler))
        {
            throw std::runtime_error("Failed to create Sampler.");
        }

        VkDescriptorImageInfo nullBuffer =
        {
            .sampler = nullSampler,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        texturePropertiesBuffer.emplace_back(nullBuffer);
    }
    else
    {
        for (auto& texture : textureList)
        {
            textureSystem.GetTexturePropertiesBuffer(texture, texturePropertiesBuffer);
        }
    }
    
    return texturePropertiesBuffer;
}

Vector<VkDescriptorImageInfo> RenderSystem::GetTexture3DPropertiesBuffer(const RenderPassGuid& renderPassGuid)
{
    Vector<VkDescriptorImageInfo>	texturePropertiesBuffer;
    if (textureSystem.Texture3DList.empty())
    {
        VkSamplerCreateInfo NullSamplerInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = 16.0f,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0,
            .maxLod = 0,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };

        VkSampler nullSampler = VK_NULL_HANDLE;
        if (vkCreateSampler(vulkanSystem.Device, &NullSamplerInfo, nullptr, &nullSampler))
        {
            throw std::runtime_error("Failed to create Sampler.");
        }

        VkDescriptorImageInfo nullBuffer =
        {
            .sampler = nullSampler,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        texturePropertiesBuffer.emplace_back(nullBuffer);
    }
    else
    {
        for (auto& texture : textureSystem.Texture3DList)
        {
            textureSystem.GetTexture3DPropertiesBuffer(texture, texturePropertiesBuffer);
        }
    }

    return texturePropertiesBuffer;
}

Vector<VkDescriptorImageInfo> RenderSystem::GetCubeMapTextureBuffer()
{
    Vector<VkDescriptorImageInfo>	texturePropertiesBuffer;
    for (auto& cubeMap : textureSystem.CubeMapTextureList)
    {
        texturePropertiesBuffer.emplace_back(VkDescriptorImageInfo
        {
            .sampler = cubeMap.textureSampler,
            .imageView = cubeMap.textureViewList.front(),
            .imageLayout = cubeMap.textureImageLayout
        });
    }
    return texturePropertiesBuffer;
}

VulkanRenderPass RenderSystem::FindRenderPass(const RenderPassGuid& renderPassGuid)
{
    return RenderPassMap.at(renderPassGuid);
}

const Vector<VulkanPipeline> RenderSystem::FindRenderPipelineList(const RenderPassGuid& renderPassGuid)
{
    return RenderPipelineMap.at(renderPassGuid);
}