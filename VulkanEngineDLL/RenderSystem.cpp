#include "RenderSystem.h"
#include "MaterialSystem.h"
#include "MeshSystem.h"
#include "BufferSystem.h"
#include <windows.h>
#include <direct.h>
#include <iostream>

RenderSystem renderSystem = RenderSystem();

RenderSystem::RenderSystem() 
{

}

RenderSystem::~RenderSystem() 
{

}

GraphicsRenderer RenderSystem::StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface, VkDebugUtilsMessengerEXT& debugMessenger)
{
    Renderer_RendererSetUp(windowHandle, instance, surface, debugMessenger);
    return renderer;
}

void RenderSystem::Update(void* windowHandle, RenderPassGuid& spriteRenderPass2DGuid, LevelGuid& levelGuid, const float& deltaTime)
{
    if (renderer.RebuildRendererFlag)
    {
        RecreateSwapchain(windowHandle, spriteRenderPass2DGuid, levelGuid, deltaTime);
        renderer.RebuildRendererFlag = false;
    }
}

RenderPassGuid RenderSystem::LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath, ivec2 renderPassResolution)
{
    const char* renderPassJson = File_Read(jsonPath.c_str()).Data;
    RenderPassLoader renderPassLoader = nlohmann::json::parse(renderPassJson).get<RenderPassLoader>();
    if (renderPassLoader.RenderArea.UseDefaultRenderArea)
    {
        renderPassLoader.RenderArea.RenderArea.extent.width = renderPassResolution.x;
        renderPassLoader.RenderArea.RenderArea.extent.height = renderPassResolution.y;
        for (auto& renderTexture : renderPassLoader.RenderedTextureInfoModelList)
        {
            renderTexture.ImageCreateInfo.extent.width = renderPassResolution.x;
            renderTexture.ImageCreateInfo.extent.height = renderPassResolution.y;
            renderTexture.ImageCreateInfo.extent.depth = 1;
        }
    }

    RenderPassAttachementTextures renderPassAttachments;
    VulkanRenderPass vulkanRenderPass = VulkanRenderPass_CreateVulkanRenderPass(renderer, jsonPath.c_str(), renderPassAttachments, renderPassResolution);
    renderSystem.RenderPassMap[vulkanRenderPass.RenderPassId] = vulkanRenderPass;
    renderSystem.RenderPassLoaderJsonMap[vulkanRenderPass.RenderPassId] = jsonPath;

    Vector<Texture> renderTextureList(renderPassAttachments.RenderPassTexture, renderPassAttachments.RenderPassTexture + renderPassAttachments.RenderPassTextureCount);
    textureSystem.AddRenderedTexture(vulkanRenderPass.RenderPassId, renderTextureList);
    if (renderPassAttachments.DepthTexture != VK_NULL_HANDLE)
    {
        textureSystem.AddDepthTexture(vulkanRenderPass.RenderPassId, *renderPassAttachments.DepthTexture);
    }

    for (int x = 0; x < renderPassLoader.RenderPipelineList.size(); x++)
    {
        nlohmann::json pipelineJson = fileSystem.LoadJsonFile(renderPassLoader.RenderPipelineList[x]);
        ShaderPipelineDataDLL shaderPiplineInfo = shaderSystem.LoadPipelineShaderData(Vector<String> { pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] });
        renderSystem.RenderPipelineMap[renderPassLoader.RenderPassId].emplace_back(VulkanPipeline_CreateRenderPipeline(renderSystem.RenderPassMap[vulkanRenderPass.RenderPassId], renderPassLoader.RenderPipelineList[x].c_str(), shaderPiplineInfo));
    }
    memorySystem.DeletePtr(renderPassAttachments.RenderPassTexture);
    memorySystem.DeletePtr(renderPassAttachments.DepthTexture);
    return renderPassLoader.RenderPassId;
}

void RenderSystem::RecreateSwapchain(void* windowHandle, RenderPassGuid& spriteRenderPass2DGuid, LevelGuid& levelGuid, const float& deltaTime) 
{ 
    vkDeviceWaitIdle(renderer.Device);
    Renderer_RebuildSwapChain(windowHandle, renderer);
    for (auto& renderPassPair : renderSystem.RenderPassMap)
    {
        VulkanRenderPass& renderPass = renderPassPair.second;
        ivec2 swapChainResolution = ivec2(renderer.SwapChainResolution.width, renderer.SwapChainResolution.height);
        String renderPassJsonLoader = renderSystem.RenderPassLoaderJsonMap[renderPass.RenderPassId];
        Vector<Texture>& renderedTextureList = textureSystem.FindRenderedTextureList(renderPass.RenderPassId);

        Texture depthTexture = Texture();
        if (textureSystem.DepthTextureExists(renderPass.RenderPassId))
        {
            depthTexture = textureSystem.FindDepthTexture(renderPass.RenderPassId);
        }

        size_t size = renderedTextureList.size();
        renderPass = VulkanRenderPass_RebuildSwapChain(renderer, renderPass, renderSystem.RenderPassLoaderJsonMap[renderPass.RenderPassId].c_str(), swapChainResolution, *renderedTextureList.data(), size, depthTexture);
    }
    // ImGui_RebuildSwapChain(renderer, imGuiRenderer);
}

const VulkanRenderPass RenderSystem::FindRenderPass(const RenderPassGuid& renderPassGuid)
{
    return RenderPassMap.at(renderPassGuid);
}

const Vector<VulkanPipeline> RenderSystem::FindRenderPipelineList(const RenderPassGuid& renderPassGuid)
{
    return RenderPipelineMap.at(renderPassGuid);
}

void RenderSystem::DestroyRenderPasses()
{
    for (auto& renderPass : renderSystem.RenderPassMap)
    {
        VulkanRenderPass_DestroyRenderPass(renderer, renderPass.second);
    }
    renderSystem.RenderPassMap.clear();
}

void RenderSystem::DestroyRenderPipelines()
{
    for (auto& renderPipelineList : renderSystem.RenderPipelineMap)
    {
        for (auto& renderPipeline : renderPipelineList.second)
        {
            VulkanPipeline_Destroy(renderPipeline);
        }
    }
    renderSystem.RenderPipelineMap.clear();
}

void RenderSystem::Destroy()
{
    RenderSystem_DestroyRenderPasses();
    RenderSystem_DestroyRenderPipelines();
    Renderer_DestroyRenderer(renderer);
}

void RenderSystem::DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList)
{
    Renderer_DestroyFrameBuffers(renderer.Device, frameBufferList.data(), frameBufferList.size());
}

void RenderSystem::DestroyCommandBuffers(VkCommandBuffer& commandBuffer)
{
    Renderer_DestroyCommandBuffers(renderer.Device, &renderer.CommandPool, &commandBuffer, 1);
}

void RenderSystem::DestroyBuffer(VkBuffer& buffer)
{
    Renderer_DestroyBuffer(renderer.Device, &buffer);
}

VkCommandBuffer RenderSystem::BeginSingleTimeCommands()
{
    return Renderer_BeginSingleTimeCommands(renderer.Device, renderer.CommandPool);
}

VkCommandBuffer RenderSystem::BeginSingleTimeCommands(VkCommandPool& commandPool)
{
    return Renderer_BeginSingleTimeCommands(renderer.Device, renderer.CommandPool);
}

VkResult RenderSystem::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    return Renderer_EndSingleTimeCommands(renderer.Device, renderer.CommandPool, renderer.GraphicsQueue, commandBuffer);
}

VkResult RenderSystem::EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool& commandPool)
{
    return Renderer_EndSingleTimeCommands(renderer.Device, commandPool, renderer.GraphicsQueue, commandBuffer);
}

VkResult RenderSystem::StartFrame()
{
    renderer.CommandIndex = (renderer.CommandIndex + 1) % MAX_FRAMES_IN_FLIGHT;

    VkResult waitResult = vkWaitForFences(renderer.Device, 1, &renderer.InFlightFences[renderer.CommandIndex], VK_TRUE, UINT64_MAX);
    if (waitResult != VK_SUCCESS)
    {
        fprintf(stderr, "Error: vkWaitForFences failed with error code: %s\n", Renderer_GetError(waitResult));
        return waitResult;
    }

    VkResult resetResult = vkResetFences(renderer.Device, 1, &renderer.InFlightFences[renderer.CommandIndex]);
    if (resetResult != VK_SUCCESS)
    {
        fprintf(stderr, "Error: vkResetFences failed with error code: %s\n", Renderer_GetError(resetResult));
        return resetResult;
    }

    VkResult result = vkAcquireNextImageKHR(renderer.Device, renderer.Swapchain, UINT64_MAX, renderer.AcquireImageSemaphores[renderer.CommandIndex], VK_NULL_HANDLE, &renderer.ImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        renderer.RebuildRendererFlag = true;
        return result;
    }
    else if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Error: vkAcquireNextImageKHR failed with error code: %s\n", Renderer_GetError(result));
        return result;
    }

    return result;
}

VkResult RenderSystem::EndFrame(Vector<VkCommandBuffer> commandBufferSubmitList)
{
    VkPipelineStageFlags waitStages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderer.AcquireImageSemaphores[renderer.CommandIndex],
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = static_cast<uint32>(commandBufferSubmitList.size()),
        .pCommandBuffers = commandBufferSubmitList.data(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &renderer.PresentImageSemaphores[renderer.ImageIndex]
    };

    VkResult submitResult = vkQueueSubmit(renderer.GraphicsQueue, 1, &submitInfo, renderer.InFlightFences[renderer.CommandIndex]);
    if (submitResult == VK_ERROR_OUT_OF_DATE_KHR ||
        submitResult == VK_SUBOPTIMAL_KHR)
    {
        renderer.RebuildRendererFlag = true;
    }
    else if (submitResult != VK_SUCCESS)
    {
        fprintf(stderr, "Error: vkQueueSubmit failed with error code: %s\n", Renderer_GetError(submitResult));
        return submitResult;
    }

    VkPresentInfoKHR presentInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderer.PresentImageSemaphores[renderer.ImageIndex],
        .swapchainCount = 1,
        .pSwapchains = &renderer.Swapchain,
        .pImageIndices = &renderer.ImageIndex
    };

    VkResult result = vkQueuePresentKHR(renderer.PresentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR)
    {
        renderer.RebuildRendererFlag = true;
    }
    else if (result != VK_SUCCESS)
    {
        fprintf(stderr, "Error: vkQueuePresentKHR failed with error code: %s\n", Renderer_GetError(result));
    }

    return result;
}

Vector<VkDescriptorBufferInfo> RenderSystem::GetVertexPropertiesBuffer() 
{ 
    //Vector<MeshStruct> meshList;
        //meshList.reserve(meshSystem.SpriteMeshList.size());
        //std::transform(meshSystem.SpriteMeshList.begin(), meshSystem.SpriteMeshList.end(),
        //    std::back_inserter(meshList),
        //    [](const auto& pair) { return pair.second; });


    Vector<VkDescriptorBufferInfo> vertexPropertiesBuffer;
    //if (meshList.empty())
    //{
    //    vertexPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
    //        {
    //            .buffer = VK_NULL_HANDLE,
    //            .offset = 0,
    //            .range = VK_WHOLE_SIZE
    //        });
    //}
    //else
    //{
    //    for (auto& mesh : meshList)
    //    {
    //        const VulkanBufferStruct& vertexProperties = bufferSystem.VulkanBuffer[mesh.MeshVertexBufferId];
    //        vertexPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
    //            {
    //                .buffer = vertexProperties.Buffer,
    //                .offset = 0,
    //                .range = VK_WHOLE_SIZE
    //            });
    //    }
    //}

    return vertexPropertiesBuffer;
};

Vector<VkDescriptorBufferInfo> RenderSystem::GetIndexPropertiesBuffer() 
{ 
    //Vector<MeshStruct> meshList;
    //meshList.reserve(meshSystem.SpriteMeshList.size());
    //std::transform(meshSystem.SpriteMeshList.begin(), meshSystem.SpriteMeshList.end(),
    //    std::back_inserter(meshList),
    //    [](const auto& pair) { return pair.second; });

    std::vector<VkDescriptorBufferInfo>	indexPropertiesBuffer;
    //if (meshList.empty())
    //{
    //    indexPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
    //        {
    //            .buffer = VK_NULL_HANDLE,
    //            .offset = 0,
    //            .range = VK_WHOLE_SIZE
    //        });
    //}
    //else
    //{
    //    for (auto& mesh : meshList)
    //    {
    //        const VulkanBufferStruct& indexProperties = bufferSystem.VulkanBuffer[mesh.MeshIndexBufferId];
    //        indexPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
    //            {
    //                .buffer = indexProperties.Buffer,
    //                .offset = 0,
    //                .range = VK_WHOLE_SIZE
    //            });
    //    }
    //}
    return indexPropertiesBuffer;
};

Vector<VkDescriptorBufferInfo> RenderSystem::GetGameObjectTransformBuffer() 
{ 
    //Vector<MeshStruct> meshList;
    //meshList.reserve(meshSystem.SpriteMeshList.size());
    //std::transform(meshSystem.SpriteMeshList.begin(), meshSystem.SpriteMeshList.end(),
    //    std::back_inserter(meshList),
    //    [](const auto& pair) { return pair.second; });

    std::vector<VkDescriptorBufferInfo>	transformPropertiesBuffer;
    //if (meshList.empty())
    //{
    //    transformPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
    //        {
    //            .buffer = VK_NULL_HANDLE,
    //            .offset = 0,
    //            .range = VK_WHOLE_SIZE
    //        });
    //}
    //else
    //{
    //    for (auto& mesh : meshList)
    //    {
    //        const VulkanBufferStruct& transformBuffer = bufferSystem.VulkanBuffer[mesh.MeshTransformBufferId];
    //        transformPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
    //            {
    //                .buffer = transformBuffer.Buffer,
    //                .offset = 0,
    //                .range = VK_WHOLE_SIZE
    //            });
    //    }
    //}

    return transformPropertiesBuffer;
};

Vector<VkDescriptorBufferInfo> RenderSystem::GetMeshPropertiesBuffer(const LevelGuid& levelLayerId) 
{ 
    Vector<Mesh> meshList;
    if (levelLayerId == LevelGuid())
    {
        for (auto& sprite : meshSystem.FindMeshByMeshType(MeshTypeEnum::Mesh_SpriteMesh))
        {
            meshList.emplace_back(sprite);

        }
    }
    else
    {
        for (auto& layer : meshSystem.FindMeshByMeshType(MeshTypeEnum::Mesh_LevelMesh))
        {
            meshList.emplace_back(layer);
        }
    }

    Vector<VkDescriptorBufferInfo> meshPropertiesBuffer;
    if (meshList.empty())
    {
        meshPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
            {
                .buffer = VK_NULL_HANDLE,
                .offset = 0,
                .range = VK_WHOLE_SIZE
            });
    }
    else
    {
        for (auto& mesh : meshList)
        {
            const VulkanBuffer& meshProperties = bufferSystem.FindVulkanBuffer(mesh.PropertiesBufferId);
            meshPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
                {
                    .buffer = meshProperties.Buffer,
                    .offset = 0,
                    .range = VK_WHOLE_SIZE
                });
        }
    }

    return meshPropertiesBuffer;
};

Vector<VkDescriptorImageInfo> RenderSystem::GetTexturePropertiesBuffer(const RenderPassGuid& renderPassGuid) 
{ 
    Vector<Texture> textureList;
    const VulkanRenderPass& renderPass = RenderSystem_FindRenderPass(renderPassGuid);
    if (renderPass.InputTextureIdListCount > 0)
    {
        Vector<VkGuid> inputTextureList = Vector<VkGuid>(renderPass.InputTextureIdList, renderPass.InputTextureIdList + renderPass.InputTextureIdListCount);
        for (auto& inputTexture : inputTextureList)
        {
            textureList.emplace_back(textureSystem.FindRenderedTexture(inputTexture));
        }
    }
    else
    {
        textureList = textureSystem.TextureList();
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
        if (vkCreateSampler(renderer.Device, &NullSamplerInfo, nullptr, &nullSampler))
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

 GraphicsRenderer RenderSystem_StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface, VkDebugUtilsMessengerEXT& debugMessenger)
{
     return renderSystem.StartUp(windowHandle, instance, surface, debugMessenger);
}

 void RenderSystem_Update(void* windowHandle, RenderPassGuid& spriteRenderPassGuidId, LevelGuid& levelGuid, const float& deltaTime)
{
     return renderSystem.Update(windowHandle, spriteRenderPassGuidId, levelGuid, deltaTime);
}

 RenderPassGuid RenderSystem_LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath, ivec2 renderPassResolution)
{
     return renderSystem.LoadRenderPass(levelGuid, jsonPath, renderPassResolution);
}

 VkResult RenderSystem_StartFrame()
{
     return renderSystem.StartFrame();
}

 VkResult RenderSystem_EndFrame(VkCommandBuffer* commandBufferListPtr, size_t commandBufferCount)
{
     Vector<VkCommandBuffer> commandBufferList = Vector<VkCommandBuffer>(commandBufferListPtr, commandBufferListPtr + commandBufferCount);
     return renderSystem.EndFrame(commandBufferList);
}

 VulkanRenderPass RenderSystem_FindRenderPass(const RenderPassGuid& renderPassGuid)
 {
     return renderSystem.FindRenderPass(renderPassGuid);
 }

 Vector<VulkanPipeline> RenderSystem_FindRenderPipelineList(const RenderPassGuid& renderPassGuid)
 {
     return renderSystem.FindRenderPipelineList(renderPassGuid);
 }

 void RenderSystem_DestroyRenderPasses()
{
     return renderSystem.DestroyRenderPasses();
}

 void RenderSystem_DestroyRenderPipelines()
{
     return renderSystem.DestroyRenderPipelines();
 }

 void RenderSystem_Destroy()
 {
     return renderSystem.Destroy();
 }