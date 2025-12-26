#include "RenderSystem.h"
#include <vulkan/vulkan.h>
#include <iostream>
#include "MaterialSystem.h"
#include "MeshSystem.h"
#include "BufferSystem.h"
#include "from_json.h"


RenderSystem& renderSystem = RenderSystem::Get();

void RenderSystem::StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface)
{
    vulkanSystem.RendererSetUp(windowHandle, instance, surface);
}

void RenderSystem::Update(void* windowHandle, RenderPassGuid& spriteRenderPass2DGuid, LevelGuid& levelGuid, const float& deltaTime)
{
    if (vulkanSystem.RebuildRendererFlag)
    {
        RecreateSwapchain(windowHandle, spriteRenderPass2DGuid, levelGuid, deltaTime);
        vulkanSystem.RebuildRendererFlag = false;
    }
}

RenderPassGuid RenderSystem::LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath, ivec2 renderPassResolution)
{
    RenderPassLoader renderPassLoader = fileSystem.LoadJsonFile(jsonPath).get<RenderPassLoader>();
    RenderPassGuid vulkanRenderPassId = CreateVulkanRenderPass(jsonPath.c_str(), renderPassResolution);
    renderSystem.RenderPassLoaderJsonMap[vulkanRenderPassId] = jsonPath;
    for (int x = 0; x < renderPassLoader.RenderPipelineList.size(); x++)
    {
        nlohmann::json pipelineJson = fileSystem.LoadJsonFile(renderPassLoader.RenderPipelineList[x].c_str());
        ShaderPipelineDataDLL shaderPiplineInfo = shaderSystem.LoadPipelineShaderData(Vector<String> { pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] });
        renderSystem.RenderPipelineMap[renderPassLoader.RenderPassId].emplace_back(CreateRenderPipeline(renderSystem.RenderPassMap[vulkanRenderPassId], renderPassLoader.RenderPipelineList[x].c_str(), shaderPiplineInfo));
    }
    return renderPassLoader.RenderPassId;
}

void RenderSystem::RecreateSwapchain(void* windowHandle, RenderPassGuid& spriteRenderPass2DGuid, LevelGuid& levelGuid, const float& deltaTime) 
{ 
    vkDeviceWaitIdle(vulkanSystem.Device);
    vulkanSystem.RebuildSwapChain(windowHandle);
    for (auto& renderPassPair : renderSystem.RenderPassMap)
    {
        VulkanRenderPass& renderPass = renderPassPair.second;
        ivec2 swapChainResolution = ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height);
        String renderPassJsonLoader = renderSystem.RenderPassLoaderJsonMap[renderPass.RenderPassId];
        Vector<Texture>& renderedTextureList = textureSystem.FindRenderedTextureList(renderPass.RenderPassId);

        Texture depthTexture = Texture();
        if (textureSystem.DepthTextureExists(renderPass.RenderPassId))
        {
            depthTexture = textureSystem.FindDepthTexture(renderPass.RenderPassId);
        }

        size_t size = renderedTextureList.size();
        renderPass = RebuildSwapChain(renderPass, renderSystem.RenderPassLoaderJsonMap[renderPass.RenderPassId].c_str(), swapChainResolution, *renderedTextureList.data(), size, depthTexture);
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
        DestroyRenderPass(renderPass.second);
    }
    renderSystem.RenderPassMap.clear();
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

void RenderSystem::Destroy()
{
    DestroyRenderPasses();
    DestroyRenderPipelines();
    vulkanSystem.DestroyRenderer();
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

VkCommandBuffer RenderSystem::BeginSingleUseCommand()
{
    return vulkanSystem.BeginSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool);
}

VkCommandBuffer RenderSystem::BeginSingleUseCommand(VkCommandPool& commandPool)
{
    return vulkanSystem.BeginSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool);
}

void RenderSystem::EndSingleUseCommand(VkCommandBuffer commandBuffer)
{
    vulkanSystem.EndSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool, vulkanSystem.GraphicsQueue, commandBuffer);
}

void RenderSystem::EndSingleUseCommand(VkCommandBuffer commandBuffer, VkCommandPool& commandPool)
{
    vulkanSystem.EndSingleUseCommand(vulkanSystem.Device, commandPool, vulkanSystem.GraphicsQueue, commandBuffer);
}

void RenderSystem::StartFrame()
{
    VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vulkanSystem.CommandIndex = (vulkanSystem.CommandIndex + 1) % vulkanSystem.SwapChainImageCount;

    VULKAN_THROW_IF_FAIL(vkWaitForFences(vulkanSystem.Device, 1, &vulkanSystem.InFlightFences[vulkanSystem.CommandIndex], VK_TRUE, UINT64_MAX));
    VULKAN_THROW_IF_FAIL(vkResetFences(vulkanSystem.Device, 1, &vulkanSystem.InFlightFences[vulkanSystem.CommandIndex]));
    VULKAN_THROW_IF_FAIL(vkResetCommandBuffer(vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex], 0));
    VULKAN_THROW_IF_FAIL(vkBeginCommandBuffer(vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex], &commandBufferBeginInfo));
    VkResult result = vkAcquireNextImageKHR(vulkanSystem.Device, vulkanSystem.Swapchain, UINT64_MAX, vulkanSystem.AcquireImageSemaphores[vulkanSystem.CommandIndex], VK_NULL_HANDLE, &vulkanSystem.ImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        vulkanSystem.RebuildRendererFlag = true;
    }
    else if (result != VK_SUCCESS)
    {
        VULKAN_THROW_IF_FAIL(result);
    }
}

void RenderSystem::EndFrame(VkCommandBuffer& commandBufferSubmit)
{
    VkPipelineStageFlags waitStages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vulkanSystem.AcquireImageSemaphores[vulkanSystem.CommandIndex],
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBufferSubmit,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &vulkanSystem.PresentImageSemaphores[vulkanSystem.ImageIndex]
    };

    VULKAN_THROW_IF_FAIL(vkEndCommandBuffer(vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex]));
    VkResult submitResult = vkQueueSubmit(vulkanSystem.GraphicsQueue, 1, &submitInfo, vulkanSystem.InFlightFences[vulkanSystem.CommandIndex]);
    if (submitResult == VK_ERROR_OUT_OF_DATE_KHR ||
        submitResult == VK_SUBOPTIMAL_KHR)
    {
        vulkanSystem.RebuildRendererFlag = true;
    }
    else if (submitResult != VK_SUCCESS)
    {
        VULKAN_THROW_IF_FAIL(submitResult);
    }

    VkPresentInfoKHR presentInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vulkanSystem.PresentImageSemaphores[vulkanSystem.ImageIndex],
        .swapchainCount = 1,
        .pSwapchains = &vulkanSystem.Swapchain,
        .pImageIndices = &vulkanSystem.ImageIndex
    };

    VkResult result = vkQueuePresentKHR(vulkanSystem.PresentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR)
    {
        vulkanSystem.RebuildRendererFlag = true;
    }
    else if (result != VK_SUCCESS)
    {
        VULKAN_THROW_IF_FAIL(result);
    }
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
    const VulkanRenderPass& renderPass = FindRenderPass(renderPassGuid);
    if (renderPass.InputTextureIdList.size() > 0)
    {
        for (auto& inputTexture : renderPass.InputTextureIdList)
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

VulkanPipeline RenderSystem::CreateRenderPipeline(VulkanRenderPass& vulkanRenderPass, const char* pipelineJsonFilePath, ShaderPipelineDataDLL& shaderPipelineData)
{
    nlohmann::json pipelineJson = fileSystem.LoadJsonFile(pipelineJsonFilePath);
    RenderPipelineLoader renderPipelineLoader = pipelineJson.get<RenderPipelineLoader>();
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = vulkanRenderPass.SampleCount;
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = vulkanRenderPass.SampleCount;
    renderPipelineLoader.RenderPassId = vulkanRenderPass.RenderPassId;
    renderPipelineLoader.RenderPass = vulkanRenderPass.RenderPass;
    renderPipelineLoader.RenderPassResolution = vulkanRenderPass.RenderPassResolution;
    renderPipelineLoader.ShaderPiplineInfo = shaderPipelineData;

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    PipelineBindingData(renderPipelineLoader);
    VkDescriptorPool descriptorPool = CreatePipelineDescriptorPool(renderPipelineLoader);
    Vector<VkDescriptorSetLayout> descriptorSetLayoutList = CreatePipelineDescriptorSetLayout(renderPipelineLoader);
    Vector<VkDescriptorSet> descriptorSetList = AllocatePipelineDescriptorSets(renderPipelineLoader, descriptorPool, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
    UpdatePipelineDescriptorSets(renderPipelineLoader, descriptorSetList.data(), descriptorSetList.size());
    VkPipelineLayout pipelineLayout = CreatePipelineLayout(renderPipelineLoader, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
    VkPipeline pipeline = CreatePipeline(renderPipelineLoader, pipelineCache, pipelineLayout, descriptorSetList.data(), descriptorSetList.size());

    return VulkanPipeline
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

VulkanPipeline RenderSystem::RebuildSwapChain(VulkanPipeline& oldPipeline, VulkanRenderPass& vulkanRenderPass, const char* pipelineJsonFilePath, ShaderPipelineDataDLL& shaderPipelineData)
{
    DestroyPipeline(oldPipeline);
    return CreateRenderPipeline(vulkanRenderPass, pipelineJsonFilePath, shaderPipelineData);
}

void RenderSystem::DestroyPipeline(VulkanPipeline& vulkanPipeline)
{
    vulkanPipeline.RenderPipelineId = VkGuid();
    vulkanSystem.DestroyPipeline(vulkanSystem.Device, &vulkanPipeline.Pipeline);
    vulkanSystem.DestroyPipelineLayout(vulkanSystem.Device, &vulkanPipeline.PipelineLayout);
    vulkanSystem.DestroyPipelineCache(vulkanSystem.Device, &vulkanPipeline.PipelineCache);
    vulkanSystem.DestroyDescriptorPool(vulkanSystem.Device, &vulkanPipeline.DescriptorPool);
}

VkDescriptorPool RenderSystem::CreatePipelineDescriptorPool(RenderPipelineLoader& renderPipelineLoader)
{
    Vector<VkDescriptorPoolSize> descriptorPoolSizeList = Vector<VkDescriptorPoolSize>();
    for (int x = 0; x < renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.size(); x++)
    {
        descriptorPoolSizeList.emplace_back(VkDescriptorPoolSize
            {
                .type = renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescripterType,
                .descriptorCount = static_cast<uint32>(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.size())
            });
    }

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorPoolCreateInfo poolCreateInfo = VkDescriptorPoolCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = static_cast<uint32>(descriptorPoolSizeList.size()) * 100,
        .poolSizeCount = static_cast<uint32>(descriptorPoolSizeList.size()),
        .pPoolSizes = descriptorPoolSizeList.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorPool(vulkanSystem.Device, &poolCreateInfo, nullptr, &descriptorPool));
    return descriptorPool;
}

Vector<VkDescriptorSetLayout> RenderSystem::CreatePipelineDescriptorSetLayout(RenderPipelineLoader& renderPipelineLoader)
{
    Vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList = Vector<VkDescriptorSetLayoutBinding>();
    for (auto& descriptorBinding : renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList)
    {
        descriptorSetLayoutBindingList.emplace_back(VkDescriptorSetLayoutBinding
            {
                .binding = descriptorBinding.Binding,
                .descriptorType = descriptorBinding.DescripterType,
                .descriptorCount = static_cast<uint32>(descriptorBinding.DescriptorCount),
                .stageFlags = descriptorBinding.ShaderStageFlags,
                .pImmutableSamplers = nullptr
            });
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = VkDescriptorSetLayoutCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = static_cast<uint32>(descriptorSetLayoutBindingList.size()),
        .pBindings = descriptorSetLayoutBindingList.data()
    };

    Vector<VkDescriptorSetLayout> descriptorSetLayoutList = Vector<VkDescriptorSetLayout>(1);
    for (auto& descriptorSetLayout : descriptorSetLayoutList)
    {
        VULKAN_THROW_IF_FAIL(vkCreateDescriptorSetLayout(vulkanSystem.Device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));
    }

    return descriptorSetLayoutList;
}

Vector<VkDescriptorSet> RenderSystem::AllocatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader, const VkDescriptorPool& descriptorPool, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount)
{
    VkDescriptorSetAllocateInfo allocInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32>(descriptorSetLayoutCount),
        .pSetLayouts = descriptorSetLayoutList
    };

    Vector<VkDescriptorSet> descriptorSetList = Vector<VkDescriptorSet>(1, VK_NULL_HANDLE);
    for (auto& descriptorSet : descriptorSetList)
    {
        VULKAN_THROW_IF_FAIL(vkAllocateDescriptorSets(vulkanSystem.Device, &allocInfo, &descriptorSet));
    }
    return descriptorSetList;
}

void RenderSystem::UpdatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount)
{
    Span<VkDescriptorSet> descriptorSetLayouts(descriptorSetList, descriptorSetCount);
    for (auto& descriptorSet : descriptorSetLayouts)
    {
        Vector<VkWriteDescriptorSet> writeDescriptorSet = Vector<VkWriteDescriptorSet>();
        for (auto& descriptorSetBinding : renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList)
        {
            writeDescriptorSet.emplace_back(VkWriteDescriptorSet
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = descriptorSet,
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
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
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
    Vector<ShaderDescriptorBinding> bindingList;
    for (int x = 0; x < renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.size(); x++)
    {
        switch (renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBindingType)
        {
        case kVertexDescsriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetVertexPropertiesBuffer().size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = renderSystem.GetVertexPropertiesBuffer();
            break;
        }
        case kIndexDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetIndexPropertiesBuffer().size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = renderSystem.GetIndexPropertiesBuffer();
            break;
        }
        case kTransformDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetGameObjectTransformBuffer().size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = renderSystem.GetGameObjectTransformBuffer();
            break;
        }
        case kMeshPropertiesDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetMeshPropertiesBuffer(renderPipelineLoader.LevelId).size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = renderSystem.GetMeshPropertiesBuffer(renderPipelineLoader.LevelId);
            break;
        }
        case kTextureDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetTexturePropertiesBuffer(renderPipelineLoader.RenderPassId).size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = renderSystem.GetTexturePropertiesBuffer(renderPipelineLoader.RenderPassId);
            break;
        }
        case kMaterialDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = materialSystem.GetMaterialPropertiesBuffer().size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = materialSystem.GetMaterialPropertiesBuffer();
            break;
        }
        default:
        {
            throw std::runtime_error("Binding case hasn't been handled yet");
        }
        }
    }
}

RenderPassGuid RenderSystem::CreateVulkanRenderPass(const char* renderPassJsonFilePath, ivec2& renderPassResolution)
{
    RenderPassLoader renderPassLoader = fileSystem.LoadJsonFile(renderPassJsonFilePath).get<RenderPassLoader>();

    Texture depthTexture;
    Vector<Texture> renderedTextureList;
    VulkanRenderPass vulkanRenderPass = VulkanRenderPass
    {
        .RenderPassId = renderPassLoader.RenderPassId,
        .SampleCount = renderPassLoader.RenderAttachmentList[0].SampleCount >= vulkanSystem.MaxSampleCount ? vulkanSystem.MaxSampleCount : renderPassLoader.RenderAttachmentList[0].SampleCount,
        .InputTextureIdList = renderPassLoader.InputTextureList,
        .ClearValueList = renderPassLoader.ClearValueList,
        .CommandBuffer = VK_NULL_HANDLE,
        .RenderPassResolution = renderPassResolution,
        .IsRenderedToSwapchain = renderPassLoader.IsRenderedToSwapchain
    };

    vulkanRenderPass.RenderPass = BuildRenderPass(renderPassLoader, renderedTextureList, depthTexture);
    BuildRenderPassAttachments(renderPassLoader, renderedTextureList, depthTexture);
    Vector<VkFramebuffer> frameBufferList = BuildFrameBuffer(vulkanRenderPass, renderedTextureList, depthTexture);
    vulkanRenderPass.FrameBufferList = frameBufferList;

    RenderPassAttachementTextures renderPassAttachments = RenderPassAttachementTextures
    {
        .RenderPassTextureCount = renderPassLoader.RenderAttachmentList.size(),
        .RenderPassTexture = memorySystem.AddPtrBuffer<Texture>(renderedTextureList.data(), renderedTextureList.size(), __FILE__, __LINE__, __func__),
        .DepthTexture = memorySystem.AddPtrBuffer<Texture>(&depthTexture, 1, __FILE__, __LINE__, __func__)
    };

    Vector<Texture> renderTextureList(renderPassAttachments.RenderPassTexture, renderPassAttachments.RenderPassTexture + renderPassAttachments.RenderPassTextureCount);
    textureSystem.AddRenderedTexture(renderPassLoader.RenderPassId, renderTextureList);
    if (renderPassAttachments.DepthTexture != VK_NULL_HANDLE)
    {
        textureSystem.AddDepthTexture(renderPassLoader.RenderPassId, *renderPassAttachments.DepthTexture);
    }

    RenderPassMap[vulkanRenderPass.RenderPassId] = vulkanRenderPass;
    memorySystem.DeletePtr(renderPassAttachments.RenderPassTexture);
    memorySystem.DeletePtr(renderPassAttachments.DepthTexture);

    return vulkanRenderPass.RenderPassId;
}

VulkanRenderPass RenderSystem::RebuildSwapChain(VulkanRenderPass& vulkanRenderPass, const char* renderPassJsonFilePath, ivec2& renderPassResolution, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture)
{
    RenderPassLoader renderPassLoader = fileSystem.LoadJsonFile(renderPassJsonFilePath).get<RenderPassLoader>();

    Vector<Texture> renderedTextureList;
    Vector<VkFramebuffer> frameBufferList;
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, vulkanRenderPass.FrameBufferList.data(), vulkanSystem.SwapChainImageCount);
    if (vulkanRenderPass.IsRenderedToSwapchain)
    {
        vulkanRenderPass.RenderPassResolution = ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height);
        renderedTextureList = Vector<Texture>(&renderedTextureListPtr, &renderedTextureListPtr + renderedTextureCount);

        DestoryRenderPassSwapChainTextures(renderedTextureListPtr, renderedTextureCount, depthTexture);
        vulkanSystem.DestroyRenderPass(vulkanSystem.Device, &vulkanRenderPass.RenderPass);
        renderedTextureList.clear();

        vulkanRenderPass.RenderPass = BuildRenderPass(renderPassLoader, renderedTextureList, depthTexture);
        BuildRenderPassAttachments(renderPassLoader, renderedTextureList, depthTexture);
        frameBufferList = BuildFrameBuffer(vulkanRenderPass, renderedTextureList, depthTexture);
    }
    else
    {
        renderedTextureList = Vector<Texture>(&renderedTextureListPtr, &renderedTextureListPtr + renderedTextureCount);
        frameBufferList = BuildFrameBuffer(vulkanRenderPass, renderedTextureList, depthTexture);
    }

    vulkanRenderPass.InputTextureIdList = renderPassLoader.InputTextureList;
    vulkanRenderPass.ClearValueList = renderPassLoader.ClearValueList;
    vulkanRenderPass.FrameBufferList = frameBufferList;

    renderedTextureCount = renderedTextureList.size();
    renderedTextureListPtr = *renderedTextureList.data();
    return vulkanRenderPass;
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
    vulkanSystem.DestroyCommandBuffers(vulkanSystem.Device, &vulkanSystem.CommandPool, &renderPass.CommandBuffer, 1);
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, &renderPass.FrameBufferList[0], vulkanSystem.SwapChainImageCount);

    renderPass.RenderPassId = VkGuid();
    renderPass.SampleCount = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
    renderPass.RenderPass = VK_NULL_HANDLE;
    renderPass.CommandBuffer = VK_NULL_HANDLE;
    renderPass.IsRenderedToSwapchain = false;
}

VkRenderPass RenderSystem::BuildRenderPass(const RenderPassLoader& renderPassJsonLoader, Vector<Texture>& renderedTextureList, Texture& depthTexture)
{
    Vector<VkAttachmentDescription> attachmentDescriptionList = Vector<VkAttachmentDescription>();
    Vector<VkAttachmentReference> inputAttachmentReferenceList = Vector<VkAttachmentReference>();
    Vector<VkAttachmentReference> colorAttachmentReferenceList = Vector<VkAttachmentReference>();
    Vector<VkAttachmentReference> resolveAttachmentReferenceList = Vector<VkAttachmentReference>();
    Vector<VkSubpassDescription> preserveAttachmentReferenceList = Vector<VkSubpassDescription>();
    Vector<VkAttachmentReference> depthReference = Vector<VkAttachmentReference>();
    for (int x = 0; x < renderPassJsonLoader.RenderAttachmentList.size(); x++)
    {
        VkImageLayout initialLayout;
        switch (renderPassJsonLoader.RenderAttachmentList[x].RenderTextureType)
        {
        case RenderType_SwapChainTexture:
        case RenderType_OffscreenColorTexture:
            initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
        case RenderType_DepthBufferTexture:
            initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        default:
            throw std::runtime_error("Unknown RenderTextureType");
        }

        attachmentDescriptionList.emplace_back(VkAttachmentDescription
            {
                .format = renderPassJsonLoader.RenderAttachmentList[x].Format,
                .samples = renderPassJsonLoader.RenderAttachmentList[x].SampleCount >= vulkanSystem.MaxSampleCount ? vulkanSystem.MaxSampleCount : renderPassJsonLoader.RenderAttachmentList[x].SampleCount,
                .loadOp = renderPassJsonLoader.RenderAttachmentList[x].LoadOp,
                .storeOp = renderPassJsonLoader.RenderAttachmentList[x].StoreOp,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = renderPassJsonLoader.RenderAttachmentList[x].FinalLayout
            });

        switch (renderPassJsonLoader.RenderAttachmentList[x].RenderAttachmentType)
        {
            case RenderAttachmentTypeEnum::ColorRenderedTexture:
            {
                colorAttachmentReferenceList.emplace_back(VkAttachmentReference
                    {
                        .attachment = static_cast<uint32>(x),
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    });
                break;
            }
            case RenderAttachmentTypeEnum::InputAttachmentTexture:
            {
                inputAttachmentReferenceList.emplace_back(VkAttachmentReference
                    {
                        .attachment = static_cast<uint32>(x),
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    });
                break;
            }
            case RenderAttachmentTypeEnum::ResolveAttachmentTexture:
            {
                resolveAttachmentReferenceList.emplace_back(VkAttachmentReference
                    {
                        .attachment = static_cast<uint32>(x),
                        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                    });
                break;
            }
            case RenderAttachmentTypeEnum::DepthRenderedTexture:
            {
                depthReference.emplace_back(VkAttachmentReference
                    {
                        .attachment = (uint)(x),
                        .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
                    });
                break;
            }
            default:
            {
                throw std::runtime_error("Case doesn't exist: RenderedTextureType");
            }
        }
    }

    Vector<VkSubpassDescription> subpassDescriptionList =
    {
        VkSubpassDescription
        {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = static_cast<uint32>(inputAttachmentReferenceList.size()),
            .pInputAttachments = inputAttachmentReferenceList.data(),
            .colorAttachmentCount = depthReference.empty() ? static_cast<uint32>(colorAttachmentReferenceList.size()) : static_cast<uint32>(colorAttachmentReferenceList.size() - 1) ,
            .pColorAttachments = colorAttachmentReferenceList.data(),
            .pResolveAttachments = resolveAttachmentReferenceList.data(),
            .pDepthStencilAttachment = depthReference.empty() ? nullptr : depthReference.data(),
            .preserveAttachmentCount = static_cast<uint32>(inputAttachmentReferenceList.size()),
            .pPreserveAttachments = nullptr,
        }
    };

    Vector<VkSubpassDependency> subPassList = Vector<VkSubpassDependency>();
    for (VkSubpassDependency subpass : renderPassJsonLoader.SubpassDependencyModelList)
    {
        subPassList.emplace_back(subpass);
    }

    VkRenderPassCreateInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount =static_cast<uint32>(attachmentDescriptionList.size()),
        .pAttachments = attachmentDescriptionList.data(),
        .subpassCount = static_cast<uint32>(subpassDescriptionList.size()),
        .pSubpasses = subpassDescriptionList.data(),
        .dependencyCount = static_cast<uint32>(subPassList.size()),
        .pDependencies = subPassList.data(),
    };

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkanSystem.Device, &renderPassInfo, nullptr, &renderPass));
    return renderPass;
}

void RenderSystem::BuildRenderPassAttachments(const RenderPassLoader& renderPassLoader, Vector<Texture>& renderedTextureList, Texture& depthTexture)
{
    for (auto& renderPassAttachmentInfo : renderPassLoader.RenderAttachmentList)
    {
        switch (renderPassAttachmentInfo.RenderAttachmentType)
        {
            case ColorRenderedTexture: renderedTextureList.emplace_back(textureSystem.CreateRenderPassTexture(renderPassAttachmentInfo, ivec2(renderPassLoader.RenderPassWidth, renderPassLoader.RenderPassHeight))); break;
            case InputAttachmentTexture: renderedTextureList.emplace_back(textureSystem.CreateRenderPassTexture(renderPassAttachmentInfo, ivec2(renderPassLoader.RenderPassWidth, renderPassLoader.RenderPassHeight))); break;
            case ResolveAttachmentTexture: renderedTextureList.emplace_back(textureSystem.CreateRenderPassTexture(renderPassAttachmentInfo, ivec2(renderPassLoader.RenderPassWidth, renderPassLoader.RenderPassHeight))); break;
            case DepthRenderedTexture: depthTexture = textureSystem.CreateRenderPassTexture(renderPassAttachmentInfo, ivec2(renderPassLoader.RenderPassWidth, renderPassLoader.RenderPassHeight)); break;
        };
    }
}

Vector<VkFramebuffer> RenderSystem::BuildFrameBuffer(const VulkanRenderPass& renderPass, Vector<Texture>& renderedTextureList, Texture& depthTexture)
{
    Vector<VkFramebuffer> frameBufferList = Vector<VkFramebuffer>(vulkanSystem.SwapChainImageCount);
    for (size_t x = 0; x < vulkanSystem.SwapChainImageCount; x++)
    {
        std::vector<VkImageView> TextureAttachmentList;
        for (int y = 0; y < renderedTextureList.size(); y++)
        {
            if (renderPass.IsRenderedToSwapchain)
            {
                TextureAttachmentList.emplace_back(vulkanSystem.SwapChainImageViews[x]);
            }
            else
            {
                TextureAttachmentList.emplace_back(renderedTextureList[y].textureView);
            }
        }
        if (depthTexture.TextureAllocation != VK_NULL_HANDLE)
        {
            TextureAttachmentList.emplace_back(depthTexture.textureView);
        }

        VkFramebufferCreateInfo framebufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass.RenderPass,
            .attachmentCount = static_cast<uint32_t>(TextureAttachmentList.size()),
            .pAttachments = TextureAttachmentList.data(),
            .width = static_cast<uint32_t>(renderPass.RenderPassResolution.x),
            .height = static_cast<uint32_t>(renderPass.RenderPassResolution.y),
            .layers = 1,
        };
        VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkanSystem.Device, &framebufferInfo, nullptr, &frameBufferList[x]));
    }

    return frameBufferList;
}