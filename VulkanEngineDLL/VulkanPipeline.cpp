#include "VulkanPipeline.h"
#include "MemorySystem.h"
#include "json.h"
#include "VulkanShader.h"
#include "JsonLoader.h"

 VulkanPipeline VulkanPipeline_CreateRenderPipeline(VkDevice device, RenderPipelineLoader& renderPipelineLoader)
 {
     VkPipelineCache pipelineCache = VK_NULL_HANDLE;
     VkDescriptorPool descriptorPool = Pipeline_CreatePipelineDescriptorPool(device, renderPipelineLoader);
     Vector<VkDescriptorSetLayout> descriptorSetLayoutList = Pipeline_CreatePipelineDescriptorSetLayout(device, renderPipelineLoader);
     Vector<VkDescriptorSet> descriptorSetList = Pipeline_AllocatePipelineDescriptorSets(device, renderPipelineLoader, descriptorPool, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
     Pipeline_UpdatePipelineDescriptorSets(device, renderPipelineLoader, descriptorSetList.data(), descriptorSetList.size());
     VkPipelineLayout pipelineLayout = Pipeline_CreatePipelineLayout(device, renderPipelineLoader, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
     VkPipeline pipeline = Pipeline_CreatePipeline(device, renderPipelineLoader, pipelineCache, pipelineLayout, descriptorSetList.data(), descriptorSetList.size());

     return VulkanPipeline
     {
         .RenderPipelineId = renderPipelineLoader.PipelineId,
         .DescriptorSetLayoutCount = descriptorSetLayoutList.size(),
         .DescriptorSetCount = descriptorSetList.size(),
         .DescriptorPool = descriptorPool,
         .DescriptorSetLayoutList = memorySystem.AddPtrBuffer<VkDescriptorSetLayout>(descriptorSetLayoutList.data(), descriptorSetLayoutList.size(), __FILE__, __LINE__, __func__),
         .DescriptorSetList = memorySystem.AddPtrBuffer<VkDescriptorSet>(descriptorSetList.data(), descriptorSetList.size(), __FILE__, __LINE__, __func__),
         .Pipeline = pipeline,
         .PipelineLayout = pipelineLayout,
         .PipelineCache = pipelineCache
     };
 }

 VulkanPipeline VulkanPipeline_RebuildSwapChain(VkDevice device, RenderPipelineLoader& renderPipelineLoader, VulkanPipeline& oldPipeline)
 {
     VulkanPipeline_Destroy(device, oldPipeline);

     VkPipelineCache pipelineCache = VK_NULL_HANDLE;
     VkDescriptorPool descriptorPool = Pipeline_CreatePipelineDescriptorPool(device, renderPipelineLoader);
     Vector<VkDescriptorSetLayout> descriptorSetLayoutList = Pipeline_CreatePipelineDescriptorSetLayout(device, renderPipelineLoader);
     Vector<VkDescriptorSet> descriptorSetList = Pipeline_AllocatePipelineDescriptorSets(device, renderPipelineLoader, descriptorPool, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
     Pipeline_UpdatePipelineDescriptorSets(device, renderPipelineLoader, descriptorSetList.data(), descriptorSetList.size());
     VkPipelineLayout pipelineLayout = Pipeline_CreatePipelineLayout(device, renderPipelineLoader, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
     VkPipeline pipeline = Pipeline_CreatePipeline(device, renderPipelineLoader, pipelineCache, pipelineLayout, descriptorSetList.data(), descriptorSetList.size());

     return VulkanPipeline
     {
         .RenderPipelineId = renderPipelineLoader.PipelineId,
         .DescriptorSetLayoutCount = descriptorSetLayoutList.size(),
         .DescriptorSetCount = descriptorSetList.size(),
         .DescriptorPool = descriptorPool,
         .DescriptorSetLayoutList = memorySystem.AddPtrBuffer<VkDescriptorSetLayout>(descriptorSetLayoutList.data(), descriptorSetLayoutList.size(), __FILE__, __LINE__, __func__),
         .DescriptorSetList = memorySystem.AddPtrBuffer<VkDescriptorSet>(descriptorSetList.data(), descriptorSetList.size(), __FILE__, __LINE__, __func__),
         .Pipeline = pipeline,
         .PipelineLayout = pipelineLayout,
         .PipelineCache = pipelineCache
     };
 }

void VulkanPipeline_Destroy(VkDevice device, VulkanPipeline& vulkanPipeline)
{
    vulkanPipeline.RenderPipelineId = VkGuid();
    Renderer_DestroyPipeline(device, &vulkanPipeline.Pipeline);
    Renderer_DestroyPipelineLayout(device, &vulkanPipeline.PipelineLayout);
    Renderer_DestroyPipelineCache(device, &vulkanPipeline.PipelineCache);
    Renderer_DestroyDescriptorPool(device, &vulkanPipeline.DescriptorPool);

    for (size_t x = 0; x < vulkanPipeline.DescriptorSetLayoutCount; x++)
    {
        Renderer_DestroyDescriptorSetLayout(device, &vulkanPipeline.DescriptorSetLayoutList[x]);
    }

    memorySystem.RemovePtrBuffer<VkDescriptorSetLayout>(vulkanPipeline.DescriptorSetLayoutList);
    memorySystem.RemovePtrBuffer<VkDescriptorSet>(vulkanPipeline.DescriptorSetList);
}


VkDescriptorPool Pipeline_CreatePipelineDescriptorPool(VkDevice device, RenderPipelineLoader& renderPipelineLoader)
{
    Vector<VkDescriptorPoolSize> descriptorPoolSizeList = Vector<VkDescriptorPoolSize>();
    for (int x = 0; x < renderPipelineLoader.PipelineDescriptorModelsCount; x++)
    {
        descriptorPoolSizeList.emplace_back(VkDescriptorPoolSize
            {
                .type = renderPipelineLoader.VertexShaderModule.DescriptorBindingsList[x].DescripterType,
                .descriptorCount = static_cast<uint32>(renderPipelineLoader.VertexShaderModule.DescriptorBindingCount)
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
    VULKAN_RESULT(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &descriptorPool));
    return descriptorPool;
}

Vector<VkDescriptorSetLayout> Pipeline_CreatePipelineDescriptorSetLayout(VkDevice device, RenderPipelineLoader& renderPipelineLoader)
{
    Vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList = Vector<VkDescriptorSetLayoutBinding>();
    for (int x = 0; x < renderPipelineLoader.PipelineDescriptorModelsCount; x++)
    {
        descriptorSetLayoutBindingList.emplace_back(VkDescriptorSetLayoutBinding
            {
                .binding = renderPipelineLoader.PipelineDescriptorModelsList[x].BindingNumber,
                .descriptorType = renderPipelineLoader.PipelineDescriptorModelsList[x].DescriptorType,
                .descriptorCount = static_cast<uint32>(renderPipelineLoader.VertexShaderModule.DescriptorBindingCount),
                .stageFlags = renderPipelineLoader.PipelineDescriptorModelsList[x].StageFlags,
                .pImmutableSamplers = renderPipelineLoader.PipelineDescriptorModelsList[x].pImmutableSamplers
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
        VULKAN_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));
    }

    return descriptorSetLayoutList;
}

Vector<VkDescriptorSet> Pipeline_AllocatePipelineDescriptorSets(VkDevice device, RenderPipelineLoader& renderPipelineLoader,  const VkDescriptorPool& descriptorPool, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount)
{
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32>(descriptorSetLayoutCount),
        .pSetLayouts = descriptorSetLayoutList
    };

    Vector<VkDescriptorSet> descriptorSetList = Vector<VkDescriptorSet>(1);
    for (auto& descriptorSet : descriptorSetList)
    {
        VULKAN_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
    }
    return descriptorSetList;
}

void Pipeline_UpdatePipelineDescriptorSets(VkDevice device, RenderPipelineLoader& renderPipelineLoader, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount)
{
    Span<VkDescriptorSet> descriptorSetLayouts(descriptorSetList, descriptorSetCount);
    for (auto& descriptorSet : descriptorSetLayouts)
    {
        Vector<VkWriteDescriptorSet> writeDescriptorSet = Vector<VkWriteDescriptorSet>();
        for (int x = 0; x < renderPipelineLoader.VertexShaderModule.DescriptorBindingCount; x++)
        {

            writeDescriptorSet.emplace_back(VkWriteDescriptorSet
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = descriptorSet,
                    .dstBinding = renderPipelineLoader.VertexShaderModule.DescriptorBindingsList[x].Binding,
                    .dstArrayElement = 0,
                    .descriptorCount = static_cast<uint32>(renderPipelineLoader.VertexShaderModule.DescriptorBindingsList[x].DescriptorCount),
                    .descriptorType = renderPipelineLoader.VertexShaderModule.DescriptorBindingsList[x].DescripterType,
                    .pImageInfo = renderPipelineLoader.VertexShaderModule.DescriptorBindingsList[x].DescriptorImageInfo,
                    .pBufferInfo = renderPipelineLoader.VertexShaderModule.DescriptorBindingsList[x].DescriptorBufferInfo,
                    .pTexelBufferView = nullptr
                });
        }
        vkUpdateDescriptorSets(device, static_cast<uint32>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
    }
}

VkPipelineLayout Pipeline_CreatePipelineLayout(VkDevice device, RenderPipelineLoader& renderPipelineLoader, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount)
{
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    Vector<VkPushConstantRange> pushConstantRangeList = Vector<VkPushConstantRange>();
    if (renderPipelineLoader.PushConstant.PushConstantSize > 0)
    {
        pushConstantRangeList.emplace_back(VkPushConstantRange
            {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = static_cast<uint>(renderPipelineLoader.PushConstant.PushConstantSize)
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
    VULKAN_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));
    return pipelineLayout;
}

VkPipeline Pipeline_CreatePipeline(VkDevice device, RenderPipelineLoader& renderPipelineLoader, VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount)
{
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = VkPipelineVertexInputStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = static_cast<uint>(renderPipelineLoader.VertexShaderModule.VertexInputBindingCount),
        .pVertexBindingDescriptions = renderPipelineLoader.VertexShaderModule.VertexInputBindingList,
        .vertexAttributeDescriptionCount = static_cast<uint>(renderPipelineLoader.VertexShaderModule.VertexInputAttributeListCount),
        .pVertexAttributeDescriptions = renderPipelineLoader.VertexShaderModule.VertexInputAttributeList
    };

    Vector<VkViewport> viewPortList;
    Vector<VkRect2D> scissorList;
    Vector<VkDynamicState> dynamicStateList;
    if (renderPipelineLoader.ViewportList)
    {
        dynamicStateList = Vector<VkDynamicState>
        {
            VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
            VkDynamicState::VK_DYNAMIC_STATE_SCISSOR
        };
    }
    else
    {
        Vector<VkViewport> viewPortList = Vector<VkViewport>(renderPipelineLoader.ViewportList, renderPipelineLoader.ViewportList + renderPipelineLoader.ViewportCount);
        for (auto& viewPort : viewPortList)
        {
            viewPortList.emplace_back(VkViewport
                {
                    .x = viewPort.x,
                    .y = viewPort.y,
                    .width = static_cast<float>(viewPort.x),
                    .height = static_cast<float>(viewPort.y),
                    .minDepth = viewPort.minDepth,
                    .maxDepth = viewPort.maxDepth
                });
        }

        Vector<VkRect2D> scissorList = Vector<VkRect2D>(renderPipelineLoader.ScissorList, renderPipelineLoader.ScissorList + renderPipelineLoader.ScissorCount);
        for (auto& scissor : scissorList)
        {
            scissorList.emplace_back(VkRect2D
                {
                    .offset = VkOffset2D
                    {
                        .x = 0,
                        .y = 0
                    },
                    .extent = VkExtent2D
                    {
                        .width = static_cast<uint32>(scissor.extent.width),
                        .height = static_cast<uint32>(scissor.extent.height)
                    }
                });
        }
    }

    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = VkPipelineViewportStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = static_cast<uint32>(viewPortList.size() + (viewPortList.empty() ? 1 : 0)),
        .pViewports = viewPortList.data(),
        .scissorCount = static_cast<uint32>(scissorList.size() + (scissorList.empty() ? 1 : 0)),
        .pScissors = scissorList.data()
    };

    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = VkPipelineDynamicStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = static_cast<uint32>(dynamicStateList.size()),
        .pDynamicStates = dynamicStateList.data()
    };

    Vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfoList = Vector<VkPipelineShaderStageCreateInfo>
    {
        Shader_CreateShader(device, renderPipelineLoader.VertexShaderModule.ShaderPath, VK_SHADER_STAGE_VERTEX_BIT),
        Shader_CreateShader(device, renderPipelineLoader.FragmentShaderModule.ShaderPath, VK_SHADER_STAGE_FRAGMENT_BIT)
    };

    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfoModel = renderPipelineLoader.PipelineColorBlendStateCreateInfoModel;
    pipelineColorBlendStateCreateInfoModel.attachmentCount = renderPipelineLoader.PipelineColorBlendAttachmentStateCount;
    pipelineColorBlendStateCreateInfoModel.pAttachments = renderPipelineLoader.PipelineColorBlendAttachmentStateList;

    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = renderPipelineLoader.PipelineMultisampleStateCreateInfo;
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

    VULKAN_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline));
    for (auto& shader : pipelineShaderStageCreateInfoList)
    {
        vkDestroyShaderModule(device, shader.module, nullptr);
    }

    return pipeline;
}

