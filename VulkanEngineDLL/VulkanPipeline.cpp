#include "VulkanPipeline.h"
#include "MemorySystem.h"
#include "json.h"
#include "VulkanShader.h"
#include "JsonLoader.h"

 VulkanPipeline VulkanPipeline_CreateRenderPipeline(VkDevice device, RenderPipelineLoader& renderPipelineLoader)
 {
     VkPipelineCache pipelineCache = VK_NULL_HANDLE;
     Pipeline_PipelineBindingData(renderPipelineLoader);
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
     return VulkanPipeline_CreateRenderPipeline(device, renderPipelineLoader);
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
    for (int x = 0; x < renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingCount; x++)
    {
        descriptorPoolSizeList.emplace_back(VkDescriptorPoolSize
            {
                .type = renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescripterType,
                .descriptorCount = static_cast<uint32>(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingCount)
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
    Span<ShaderDescriptorBinding> descriptorBindingList(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList, renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingCount);
    for (auto& descriptorBinding : descriptorBindingList)
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
        Span<ShaderDescriptorBinding> descriptorSetBindingList(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList, renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingCount);
        for (auto& descriptorSetBinding : descriptorSetBindingList)
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
                    .pImageInfo = descriptorSetBinding.DescriptorImageInfo,
                    .pBufferInfo = descriptorSetBinding.DescriptorBufferInfo,
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
    if (renderPipelineLoader.ShaderPiplineInfo.PushConstantList != nullptr)
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
        .vertexBindingDescriptionCount = static_cast<uint>(renderPipelineLoader.ShaderPiplineInfo.VertexInputBindingCount),
        .pVertexBindingDescriptions = renderPipelineLoader.ShaderPiplineInfo.VertexInputBindingList,
        .vertexAttributeDescriptionCount = static_cast<uint>(renderPipelineLoader.ShaderPiplineInfo.VertexInputAttributeListCount),
        .pVertexAttributeDescriptions = renderPipelineLoader.ShaderPiplineInfo.VertexInputAttributeList
    };

    Vector<VkViewport> viewPortList(renderPipelineLoader.ViewportList, renderPipelineLoader.ViewportList + renderPipelineLoader.ViewportCount);
    for (auto& viewPort : viewPortList)
    {
        viewPort.width = static_cast<float>(renderPipelineLoader.RenderPassResolution.x);
        viewPort.height = static_cast<float>(renderPipelineLoader.RenderPassResolution.y);
    }

    Vector<VkRect2D> scissorList(renderPipelineLoader.ScissorList, renderPipelineLoader.ScissorList + renderPipelineLoader.ScissorCount);
    for (auto& scissor : scissorList)
    {
        scissor.extent.width = static_cast<float>(renderPipelineLoader.RenderPassResolution.x);
        scissor.extent.height = static_cast<float>(renderPipelineLoader.RenderPassResolution.y);
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

    Vector<VkDynamicState> dynamicStateList = viewPortList.empty() ? Vector<VkDynamicState> { VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT, VkDynamicState::VK_DYNAMIC_STATE_SCISSOR} : Vector<VkDynamicState>();
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
        Shader_CreateShader(device, renderPipelineLoader.ShaderPiplineInfo.ShaderList[0], VK_SHADER_STAGE_VERTEX_BIT),
        Shader_CreateShader(device, renderPipelineLoader.ShaderPiplineInfo.ShaderList[1], VK_SHADER_STAGE_FRAGMENT_BIT)
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

void Pipeline_PipelineBindingData(RenderPipelineLoader& renderPipelineLoader)
{
    Vector<ShaderDescriptorBinding> bindingList;
     for (int x = 0; x < renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingCount; x++)
    {
        switch (renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBindingType)
        {
        case kVertexDescsriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderPipelineLoader.gpuIncludes.VertexPropertiesCount;
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(renderPipelineLoader.gpuIncludes.VertexPropertiesCount, __FILE__, __LINE__, __func__);
            std::memcpy(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo, renderPipelineLoader.gpuIncludes.VertexProperties, renderPipelineLoader.gpuIncludes.VertexPropertiesCount * sizeof(VkDescriptorBufferInfo));
            break;
        }
        case kIndexDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderPipelineLoader.gpuIncludes.IndexPropertiesCount;
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(renderPipelineLoader.gpuIncludes.IndexPropertiesCount, __FILE__, __LINE__, __func__);
            std::memcpy(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo, renderPipelineLoader.gpuIncludes.IndexProperties, renderPipelineLoader.gpuIncludes.IndexPropertiesCount * sizeof(VkDescriptorBufferInfo));
            break;
        }
        case kTransformDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderPipelineLoader.gpuIncludes.TransformPropertiesCount;
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(renderPipelineLoader.gpuIncludes.TransformPropertiesCount, __FILE__, __LINE__, __func__);
            std::memcpy(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo, renderPipelineLoader.gpuIncludes.TransformProperties, renderPipelineLoader.gpuIncludes.TransformPropertiesCount * sizeof(VkDescriptorBufferInfo));
            break;
        }
        case kMeshPropertiesDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderPipelineLoader.gpuIncludes.MeshPropertiesCount;
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(renderPipelineLoader.gpuIncludes.MeshPropertiesCount, __FILE__, __LINE__, __func__);
            std::memcpy(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo, renderPipelineLoader.gpuIncludes.MeshProperties, renderPipelineLoader.gpuIncludes.MeshPropertiesCount * sizeof(VkDescriptorBufferInfo));
            break;
        }
        case kTextureDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderPipelineLoader.gpuIncludes.TexturePropertiesCount;
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = memorySystem.AddPtrBuffer<VkDescriptorImageInfo>(renderPipelineLoader.gpuIncludes.TexturePropertiesCount, __FILE__, __LINE__, __func__);
            std::memcpy(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo, renderPipelineLoader.gpuIncludes.TextureProperties, renderPipelineLoader.gpuIncludes.TexturePropertiesCount * sizeof(VkDescriptorImageInfo));
            break;
        }
        case kMaterialDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderPipelineLoader.gpuIncludes.MaterialPropertiesCount;
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(renderPipelineLoader.gpuIncludes.MaterialPropertiesCount, __FILE__, __LINE__, __func__);
            std::memcpy(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo, renderPipelineLoader.gpuIncludes.MaterialProperties, renderPipelineLoader.gpuIncludes.MaterialPropertiesCount * sizeof(VkDescriptorBufferInfo));
            break;
        }
        default:
        {
            throw std::runtime_error("Binding case hasn't been handled yet");
        }
        }
    }
}

 void DebugRenderPipelineLoader(const RenderPipelineLoader& model, const std::string& indent) {
    std::cout << indent << "=== RenderPipelineLoader ===" << std::endl;
    std::cout << indent << "PipelineId: " << model.PipelineId.ToString() << std::endl;
    std::cout << indent << "RenderPassId: " << model.RenderPassId.ToString() << std::endl;
    std::cout << indent << "RenderPass: 0x" << std::hex << std::setw(16) << std::setfill('0') << (uint64_t)model.RenderPass << std::dec << std::endl;
    std::cout << indent << "RenderPassResolution: (" << model.RenderPassResolution.x << ", " << model.RenderPassResolution.y << ")" << std::endl;

    std::cout << indent << "--- GPUIncludes ---" << std::endl;
    DebugGPUIncludes(model.gpuIncludes, indent + "  ");

    std::cout << indent << "--- ShaderPipelineData ---" << std::endl;
    DebugShaderPipelineData(model.ShaderPiplineInfo, indent + "  ");

    std::cout << indent << "ViewportCount: " << model.ViewportCount << std::endl;
    DebugViewportList(model.ViewportList, model.ViewportCount, indent + "  ");

    std::cout << indent << "ScissorCount: " << model.ScissorCount << std::endl;
    DebugScissorList(model.ScissorList, model.ScissorCount, indent + "  ");

    std::cout << indent << "PipelineColorBlendAttachmentStateCount: " << model.PipelineColorBlendAttachmentStateCount << std::endl;
    DebugPipelineColorBlendAttachmentStateList(model.PipelineColorBlendAttachmentStateList, model.PipelineColorBlendAttachmentStateCount, indent + "  ");

    std::cout << indent << "PipelineInputAssemblyStateCreateInfo:" << std::endl;
    DebugPipelineInputAssemblyState(model.PipelineInputAssemblyStateCreateInfo, indent + "  ");

    std::cout << indent << "PipelineRasterizationStateCreateInfo:" << std::endl;
    DebugPipelineRasterizationState(model.PipelineRasterizationStateCreateInfo, indent + "  ");

    std::cout << indent << "PipelineMultisampleStateCreateInfo:" << std::endl;
    DebugPipelineMultisampleState(model.PipelineMultisampleStateCreateInfo, indent + "  ");

    std::cout << indent << "PipelineDepthStencilStateCreateInfo:" << std::endl;
    DebugPipelineDepthStencilState(model.PipelineDepthStencilStateCreateInfo, indent + "  ");

    std::cout << indent << "PipelineColorBlendStateCreateInfo:" << std::endl;
    DebugPipelineColorBlendState(model.PipelineColorBlendStateCreateInfoModel, indent + "  ");
}

     void DebugGPUIncludes(const GPUIncludes& includes, const std::string& indent) {
        std::cout << indent << "VertexPropertiesCount: " << includes.VertexPropertiesCount << std::endl;
        DebugDescriptorBufferInfoList(includes.VertexProperties, includes.VertexPropertiesCount, "VertexProperties", indent + "  ");

        std::cout << indent << "IndexPropertiesCount: " << includes.IndexPropertiesCount << std::endl;
        DebugDescriptorBufferInfoList(includes.IndexProperties, includes.IndexPropertiesCount, "IndexProperties", indent + "  ");

        std::cout << indent << "TransformPropertiesCount: " << includes.TransformPropertiesCount << std::endl;
        DebugDescriptorBufferInfoList(includes.TransformProperties, includes.TransformPropertiesCount, "TransformProperties", indent + "  ");

        std::cout << indent << "MeshPropertiesCount: " << includes.MeshPropertiesCount << std::endl;
        DebugDescriptorBufferInfoList(includes.MeshProperties, includes.MeshPropertiesCount, "MeshProperties", indent + "  ");

        std::cout << indent << "TexturePropertiesCount: " << includes.TexturePropertiesCount << std::endl;
        DebugDescriptorImageInfoList(includes.TextureProperties, includes.TexturePropertiesCount, indent + "  ");

        std::cout << indent << "MaterialPropertiesCount: " << includes.MaterialPropertiesCount << std::endl;
        DebugDescriptorBufferInfoList(includes.MaterialProperties, includes.MaterialPropertiesCount, "MaterialProperties", indent + "  ");
    }

     void DebugShaderPipelineData(const ShaderPipelineData& data, const std::string& indent) {
        std::cout << indent << "ShaderCount: " << data.ShaderCount << std::endl;
        DebugShaderList(data.ShaderList, data.ShaderCount, indent + "  ");

        std::cout << indent << "DescriptorBindingCount: " << data.DescriptorBindingCount << std::endl;
        DebugDescriptorBindingsList(data.DescriptorBindingsList, data.DescriptorBindingCount, indent + "  ");

        std::cout << indent << "ShaderStructCount: " << data.ShaderStructCount << std::endl;
        DebugShaderStructList(data.ShaderStructList, data.ShaderStructCount, indent + "  ");

        std::cout << indent << "VertexInputBindingCount: " << data.VertexInputBindingCount << std::endl;
        DebugVertexInputBindingList(data.VertexInputBindingList, data.VertexInputBindingCount, indent + "  ");

        std::cout << indent << "VertexInputAttributeListCount: " << data.VertexInputAttributeListCount << std::endl;
        DebugVertexInputAttributeList(data.VertexInputAttributeList, data.VertexInputAttributeListCount, indent + "  ");

        std::cout << indent << "ShaderOutputCount: " << data.ShaderOutputCount << std::endl;
        DebugShaderVariableList(data.ShaderOutputList, data.ShaderOutputCount, "ShaderOutput", indent + "  ");

        std::cout << indent << "PushConstantCount: " << data.PushConstantCount << std::endl;
        DebugPushConstantList(data.PushConstantList, data.PushConstantCount, indent + "  ");
    }

     void DebugShaderList(const char** shaderList, size_t count, const std::string& indent) {
        if (!shaderList || count == 0) {
            std::cout << indent << "ShaderList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            std::string shaderName = shaderList[i] ? shaderList[i] : "(null)";
            std::cout << indent << "[" << i << "] Shader: " << shaderName << std::endl;
        }
    }

     void DebugDescriptorBindingsList(const ShaderDescriptorBinding* bindings, size_t count, const std::string& indent) {
        if (!bindings || count == 0) {
            std::cout << indent << "DescriptorBindingsList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const ShaderDescriptorBinding& binding = bindings[i];
            std::cout << indent << "[" << i << "] DescriptorBinding:" << std::endl;
            std::cout << indent << "  Name: " << (binding.Name ? binding.Name : "(null)") << std::endl;
            std::cout << indent << "  Binding: " << binding.Binding << std::endl;
            std::cout << indent << "  DescriptorCount: " << binding.DescriptorCount << std::endl;
            std::cout << indent << "  ShaderStageFlags: " << binding.ShaderStageFlags << std::endl;
            std::cout << indent << "  DescriptorBindingType: " << binding.DescriptorBindingType << std::endl;
            std::cout << indent << "  DescripterType: " << binding.DescripterType << std::endl;
            std::cout << indent << "  DescriptorImageInfo: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)binding.DescriptorImageInfo << std::dec << std::endl;
            std::cout << indent << "  DescriptorBufferInfo: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)binding.DescriptorBufferInfo << std::dec << std::endl;
        }
    }

     void DebugShaderStructList(const ShaderStruct* structs, size_t count, const std::string& indent) {
        if (!structs || count == 0) {
            std::cout << indent << "ShaderStructList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const ShaderStruct& shaderStruct = structs[i];
            std::cout << indent << "[" << i << "] ShaderStruct:" << std::endl;
            std::cout << indent << "  Name: " << (shaderStruct.Name ? shaderStruct.Name : "(null)") << std::endl;
            std::cout << indent << "  ShaderBufferSize: " << shaderStruct.ShaderBufferSize << std::endl;
            std::cout << indent << "  ShaderBufferVariableListCount: " << shaderStruct.ShaderBufferVariableListCount << std::endl;
            std::cout << indent << "  ShaderStructBufferId: " << shaderStruct.ShaderStructBufferId << std::endl;
            std::cout << indent << "  ShaderStructBuffer: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)shaderStruct.ShaderStructBuffer << std::dec << std::endl;
            DebugShaderVariableList(shaderStruct.ShaderBufferVariableList, shaderStruct.ShaderBufferVariableListCount,
                "ShaderBufferVariable", indent + "    ");
        }
    }

     void DebugShaderVariableList(const ShaderVariable* variables, size_t count, const std::string& listName,
        const std::string& indent) {
        if (!variables || count == 0) {
            std::cout << indent << listName << "List: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const ShaderVariable& variable = variables[i];
            std::cout << indent << "[" << i << "] " << listName << ":" << std::endl;
            std::cout << indent << "  Name: " << (variable.Name ? variable.Name : "(null)") << std::endl;
            std::cout << indent << "  Size: " << variable.Size << std::endl;
            std::cout << indent << "  ByteAlignment: " << variable.ByteAlignment << std::endl;
            std::cout << indent << "  Value: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)variable.Value << std::dec << std::endl;
            std::cout << indent << "  MemberTypeEnum: " << variable.MemberTypeEnum << std::endl;
        }
    }

     void DebugPushConstantList(const ShaderPushConstant* pushConstants, size_t count, const std::string& indent) {
        if (!pushConstants || count == 0) {
            std::cout << indent << "PushConstantList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const ShaderPushConstant& pushConstant = pushConstants[i];
            std::cout << indent << "[" << i << "] PushConstant:" << std::endl;
            std::cout << indent << "  Name: " << (pushConstant.PushConstantName ? pushConstant.PushConstantName : "(null)") << std::endl;
            std::cout << indent << "  PushConstantSize: " << pushConstant.PushConstantSize << std::endl;
            std::cout << indent << "  PushConstantVariableListCount: " << pushConstant.PushConstantVariableListCount << std::endl;
            std::cout << indent << "  ShaderStageFlags: " << pushConstant.ShaderStageFlags << std::endl;
            std::cout << indent << "  PushConstantBuffer: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)pushConstant.PushConstantBuffer << std::dec << std::endl;
            std::cout << indent << "  GlobalPushContant: " << (pushConstant.GlobalPushContant ? "true" : "false") << std::endl;
            DebugShaderVariableList(pushConstant.PushConstantVariableList, pushConstant.PushConstantVariableListCount,
                "PushConstantVariable", indent + "    ");
        }
    }

     void DebugVertexInputBindingList(const VkVertexInputBindingDescription* bindings, size_t count,
        const std::string& indent) {
        if (!bindings || count == 0) {
            std::cout << indent << "VertexInputBindingList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkVertexInputBindingDescription& binding = bindings[i];
            std::cout << indent << "[" << i << "] VertexInputBinding:" << std::endl;
            std::cout << indent << "  Binding: " << binding.binding << std::endl;
            std::cout << indent << "  Stride: " << binding.stride << std::endl;
            std::cout << indent << "  InputRate: " << binding.inputRate << std::endl;
        }
    }

     void DebugVertexInputAttributeList(const VkVertexInputAttributeDescription* attributes, size_t count,
        const std::string& indent) {
        if (!attributes || count == 0) {
            std::cout << indent << "VertexInputAttributeList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkVertexInputAttributeDescription& attribute = attributes[i];
            std::cout << indent << "[" << i << "] VertexInputAttribute:" << std::endl;
            std::cout << indent << "  Location: " << attribute.location << std::endl;
            std::cout << indent << "  Binding: " << attribute.binding << std::endl;
            std::cout << indent << "  Format: " << attribute.format << std::endl;
            std::cout << indent << "  Offset: " << attribute.offset << std::endl;
        }
    }

     void DebugDescriptorBufferInfoList(const VkDescriptorBufferInfo* infos, size_t count, const std::string& name,
        const std::string& indent) {
        if (!infos || count == 0) {
            std::cout << indent << name << "List: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkDescriptorBufferInfo& info = infos[i];
            std::cout << indent << "[" << i << "] " << name << ":" << std::endl;
            std::cout << indent << "  Buffer: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)info.buffer << std::dec << std::endl;
            std::cout << indent << "  Offset: " << info.offset << std::endl;
            std::cout << indent << "  Range: " << info.range << std::endl;
        }
    }

     void DebugDescriptorImageInfoList(const VkDescriptorImageInfo* infos, size_t count, const std::string& indent) {
        if (!infos || count == 0) {
            std::cout << indent << "TexturePropertiesList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkDescriptorImageInfo& info = infos[i];
            std::cout << indent << "[" << i << "] TextureProperties:" << std::endl;
            std::cout << indent << "  Sampler: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)info.sampler << std::dec << std::endl;
            std::cout << indent << "  ImageView: 0x" << std::hex << std::setw(16) << std::setfill('0')
                << (uint64_t)info.imageView << std::dec << std::endl;
            std::cout << indent << "  ImageLayout: " << info.imageLayout << std::endl;
        }
    }

     void DebugViewportList(const VkViewport* viewports, size_t count, const std::string& indent) {
        if (!viewports || count == 0) {
            std::cout << indent << "ViewportList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkViewport& viewport = viewports[i];
            std::cout << indent << "[" << i << "] Viewport:" << std::endl;
            std::cout << indent << "  X: " << viewport.x << std::endl;
            std::cout << indent << "  Y: " << viewport.y << std::endl;
            std::cout << indent << "  Width: " << viewport.width << std::endl;
            std::cout << indent << "  Height: " << viewport.height << std::endl;
            std::cout << indent << "  MinDepth: " << viewport.minDepth << std::endl;
            std::cout << indent << "  MaxDepth: " << viewport.maxDepth << std::endl;
        }
    }

     void DebugScissorList(const VkRect2D* scissors, size_t count, const std::string& indent) {
        if (!scissors || count == 0) {
            std::cout << indent << "ScissorList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkRect2D& scissor = scissors[i];
            std::cout << indent << "[" << i << "] Scissor:" << std::endl;
            std::cout << indent << "  Offset: (" << scissor.offset.x << ", " << scissor.offset.y << ")" << std::endl;
            std::cout << indent << "  Extent: (" << scissor.extent.width << ", " << scissor.extent.height << ")" << std::endl;
        }
    }

     void DebugPipelineColorBlendAttachmentStateList(const VkPipelineColorBlendAttachmentState* states, size_t count,
        const std::string& indent) {
        if (!states || count == 0) {
            std::cout << indent << "PipelineColorBlendAttachmentStateList: (empty)" << std::endl;
            return;
        }

        for (size_t i = 0; i < count; ++i) {
            const VkPipelineColorBlendAttachmentState& state = states[i];
            std::cout << indent << "[" << i << "] PipelineColorBlendAttachmentState:" << std::endl;
            std::cout << indent << "  BlendEnable: " << (state.blendEnable ? "true" : "false") << std::endl;
            std::cout << indent << "  SrcColorBlendFactor: " << state.srcColorBlendFactor << std::endl;
            std::cout << indent << "  DstColorBlendFactor: " << state.dstColorBlendFactor << std::endl;
            std::cout << indent << "  ColorBlendOp: " << state.colorBlendOp << std::endl;
            std::cout << indent << "  SrcAlphaBlendFactor: " << state.srcAlphaBlendFactor << std::endl;
            std::cout << indent << "  DstAlphaBlendFactor: " << state.dstAlphaBlendFactor << std::endl;
            std::cout << indent << "  AlphaBlendOp: " << state.alphaBlendOp << std::endl;
            std::cout << indent << "  ColorWriteMask: " << state.colorWriteMask << std::endl;
        }
    }

     void DebugPipelineInputAssemblyState(const VkPipelineInputAssemblyStateCreateInfo& state, const std::string& indent) {
        std::cout << indent << "Topology: " << state.topology << std::endl;
        std::cout << indent << "PrimitiveRestartEnable: " << (state.primitiveRestartEnable ? "true" : "false") << std::endl;
    }

     void DebugPipelineRasterizationState(const VkPipelineRasterizationStateCreateInfo& state, const std::string& indent) {
        std::cout << indent << "DepthClampEnable: " << (state.depthClampEnable ? "true" : "false") << std::endl;
        std::cout << indent << "RasterizerDiscardEnable: " << (state.rasterizerDiscardEnable ? "true" : "false") << std::endl;
        std::cout << indent << "PolygonMode: " << state.polygonMode << std::endl;
        std::cout << indent << "CullMode: " << state.cullMode << std::endl;
        std::cout << indent << "FrontFace: " << state.frontFace << std::endl;
        std::cout << indent << "DepthBiasEnable: " << (state.depthBiasEnable ? "true" : "false") << std::endl;
        std::cout << indent << "DepthBiasConstantFactor: " << state.depthBiasConstantFactor << std::endl;
        std::cout << indent << "DepthBiasClamp: " << state.depthBiasClamp << std::endl;
        std::cout << indent << "DepthBiasSlopeFactor: " << state.depthBiasSlopeFactor << std::endl;
        std::cout << indent << "LineWidth: " << state.lineWidth << std::endl;
    }

     void DebugPipelineMultisampleState(const VkPipelineMultisampleStateCreateInfo& state, const std::string& indent) {
        std::cout << indent << "RasterizationSamples: " << state.rasterizationSamples << std::endl;
        std::cout << indent << "SampleShadingEnable: " << (state.sampleShadingEnable ? "true" : "false") << std::endl;
        std::cout << indent << "MinSampleShading: " << state.minSampleShading << std::endl;
        std::cout << indent << "AlphaToCoverageEnable: " << (state.alphaToCoverageEnable ? "true" : "false") << std::endl;
        std::cout << indent << "AlphaToOneEnable: " << (state.alphaToOneEnable ? "true" : "false") << std::endl;
    }

     void DebugPipelineDepthStencilState(const VkPipelineDepthStencilStateCreateInfo& state, const std::string& indent) {
        std::cout << indent << "DepthTestEnable: " << (state.depthTestEnable ? "true" : "false") << std::endl;
        std::cout << indent << "DepthWriteEnable: " << (state.depthWriteEnable ? "true" : "false") << std::endl;
        std::cout << indent << "DepthCompareOp: " << state.depthCompareOp << std::endl;
        std::cout << indent << "DepthBoundsTestEnable: " << (state.depthBoundsTestEnable ? "true" : "false") << std::endl;
        std::cout << indent << "StencilTestEnable: " << (state.stencilTestEnable ? "true" : "false") << std::endl;
        std::cout << indent << "Front: [FailOp: " << state.front.failOp << ", PassOp: " << state.front.passOp
            << ", DepthFailOp: " << state.front.depthFailOp << ", CompareOp: " << state.front.compareOp << "]" << std::endl;
        std::cout << indent << "Back: [FailOp: " << state.back.failOp << ", PassOp: " << state.back.passOp
            << ", DepthFailOp: " << state.back.depthFailOp << ", CompareOp: " << state.back.compareOp << "]" << std::endl;
        std::cout << indent << "MinDepthBounds: " << state.minDepthBounds << std::endl;
        std::cout << indent << "MaxDepthBounds: " << state.maxDepthBounds << std::endl;
    }

     void DebugPipelineColorBlendState(const VkPipelineColorBlendStateCreateInfo& state, const std::string& indent) {
        std::cout << indent << "LogicOpEnable: " << (state.logicOpEnable ? "true" : "false") << std::endl;
        std::cout << indent << "LogicOp: " << state.logicOp << std::endl;
        std::cout << indent << "BlendConstants: [" << state.blendConstants[0] << ", " << state.blendConstants[1]
            << ", " << state.blendConstants[2] << ", " << state.blendConstants[3] << "]" << std::endl;
    }
