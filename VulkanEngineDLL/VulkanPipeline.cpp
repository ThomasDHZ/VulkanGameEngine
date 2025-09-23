#include "VulkanPipeline.h"
#include "MemorySystem.h"
#include "json.h"
#include "VulkanShader.h"
#include "JsonLoader.h"
#include "GPUSystem.h"
#include "JsonStruct.h"

VulkanPipeline VulkanPipeline_CreateRenderPipeline(VkDevice device, VulkanRenderPass& vulkanRenderPass, const char* pipelineJsonFilePath, GPUIncludes& gpuIncludes, ShaderPipelineData& shaderPipelineData)
{
    nlohmann::json pipelineJson = Json::ReadJson(pipelineJsonFilePath);
    RenderPipelineLoader renderPipelineLoader = pipelineJson.get<RenderPipelineLoader>();
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = vulkanRenderPass.SampleCount;
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = vulkanRenderPass.SampleCount > VK_SAMPLE_COUNT_1_BIT ? VK_TRUE : VK_FALSE;
    renderPipelineLoader.RenderPassId = vulkanRenderPass.RenderPassId;
    renderPipelineLoader.RenderPass = vulkanRenderPass.RenderPass;
    renderPipelineLoader.gpuIncludes = gpuIncludes;
    renderPipelineLoader.RenderPassResolution = vulkanRenderPass.RenderPassResolution;
    renderPipelineLoader.ShaderPipelineInfo = shaderPipelineData;

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    Pipeline_PipelineBindingData(renderPipelineLoader);
    VkDescriptorPool descriptorPool = Pipeline_CreatePipelineDescriptorPool(device, renderPipelineLoader);
    Vector<VkDescriptorSetLayout> descriptorSetLayoutList = Pipeline_CreatePipelineDescriptorSetLayout(device, renderPipelineLoader);
    Vector<VkDescriptorSet> descriptorSetList = Pipeline_AllocatePipelineDescriptorSets(device, renderPipelineLoader, descriptorPool, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
    Pipeline_UpdatePipelineDescriptorSets(device, renderPipelineLoader, descriptorSetList.data(), descriptorSetList.size());
    VkPipelineLayout pipelineLayout = Pipeline_CreatePipelineLayout(device, renderPipelineLoader, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
    VkPipeline pipeline = Pipeline_CreatePipeline(device, renderPipelineLoader, pipelineCache, pipelineLayout, descriptorSetList.data(), descriptorSetList.size());

    VulkanPipeline vulkanPipeline = VulkanPipeline
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

    memorySystem.RemovePtrBuffer(renderPipelineLoader.PipelineColorBlendAttachmentStateList);
    memorySystem.RemovePtrBuffer(renderPipelineLoader.ViewportList);
    memorySystem.RemovePtrBuffer(renderPipelineLoader.ScissorList);
    return vulkanPipeline;
}

VulkanPipeline VulkanPipeline_CreateRenderPipelineCS(VkDevice device, VulkanRenderPass& vulkanRenderPass, const char* pipelineJsonFilePath, GPUIncludes& gpuIncludes, ShaderPipelineDataDLL& shaderPipelineDataDLL)
{
    Vector<ShaderDescriptorBinding> shaderDescriptorBindingList;
    Span<ShaderDescriptorBindingDLL> shaderDescriptorBindingSpan(shaderPipelineDataDLL.DescriptorBindingsList, shaderPipelineDataDLL.DescriptorBindingsList + shaderPipelineDataDLL.DescriptorBindingCount);
    for (auto& shaderDescirptorBinding : shaderDescriptorBindingSpan)
    {
        shaderDescriptorBindingList.emplace_back(ShaderDescriptorBinding
            {
                .Name = String(shaderDescirptorBinding.Name),
                .Binding = shaderDescirptorBinding.Binding,
                .ShaderStageFlags = shaderDescirptorBinding.ShaderStageFlags,
                .DescriptorBindingType = shaderDescirptorBinding.DescriptorBindingType,
                .DescripterType = shaderDescirptorBinding.DescripterType,
                .DescriptorBufferInfo = shaderDescirptorBinding.DescriptorBufferInfo,
            });
    }
    ShaderDescriptorBinding* shaderDescriptorBindingListPtr = memorySystem.AddPtrBuffer<ShaderDescriptorBinding>(shaderDescriptorBindingList.data(), shaderDescriptorBindingList.size(), __FILE__, __LINE__, __func__, "DescriptorBindings");

    Vector<ShaderStruct> shaderStructList;
    ShaderStruct* shaderStructListPtr = nullptr;
    Span<ShaderStructDLL> shaderStructSpan(shaderPipelineDataDLL.ShaderStructList, shaderPipelineDataDLL.ShaderStructList + shaderPipelineDataDLL.ShaderStructCount);
    if (shaderPipelineDataDLL.ShaderStructCount > 0)
    {
        for (auto& shaderStruct : shaderStructSpan)
        {
            Vector<ShaderVariable> shaderVariableList;
            Span<ShaderVariableDLL> shaderVariableSpan(shaderStruct.ShaderBufferVariableList, shaderStruct.ShaderBufferVariableList + shaderStruct.ShaderBufferVariableCount);
            for (auto& shaderVariable : shaderVariableSpan)
            {
                shaderVariableList.emplace_back(ShaderVariable
                    {
                        .Name = String(shaderVariable.Name),
                        .ByteAlignment = shaderVariable.ByteAlignment,
                        .Value = shaderVariable.Value,
                        .MemberTypeEnum = shaderVariable.MemberTypeEnum
                    });
            }

            shaderStructList.emplace_back(ShaderStruct
                {
                    .Name = String(shaderStruct.Name),
                    .ShaderBufferSize = shaderStruct.ShaderBufferSize,
                    .ShaderBufferVariableList = shaderVariableList,
                    .ShaderStructBufferId = shaderStruct.ShaderStructBufferId,
                    .ShaderStructBuffer = shaderStruct.ShaderStructBuffer,
                });
        }
        shaderStructListPtr = memorySystem.AddPtrBuffer<ShaderStruct>(shaderStructList.data(), shaderStructList.size(), __FILE__, __LINE__, __func__, "ShaderStruct");
    }

    Vector<ShaderPushConstant> shaderPushConstantList;
    ShaderPushConstant* shaderPushConstantListPtr = nullptr;
    Span<ShaderPushConstantDLL> shaderPushConstantSpan(shaderPipelineDataDLL.PushConstantList, shaderPipelineDataDLL.PushConstantList + shaderPipelineDataDLL.PushConstantCount);
    if (shaderPipelineDataDLL.PushConstantCount > 0)
    {
        for (auto& shaderPushConstant : shaderPushConstantSpan)
        {
            Vector<ShaderVariable> shaderVariableList;
            Span<ShaderVariableDLL> shaderVariableSpan(shaderPushConstant.VariableList, shaderPushConstant.VariableList + shaderPushConstant.VariableCount);
            for (auto& shaderVariable : shaderVariableSpan)
            {
                shaderVariableList.emplace_back(ShaderVariable
                    {
                        .Name = String(shaderVariable.Name),
                        .Size = shaderVariable.Size,
                        .ByteAlignment = shaderVariable.ByteAlignment,
                        .Value = shaderVariable.Value,
                        .MemberTypeEnum = shaderVariable.MemberTypeEnum
                    });
            }

            shaderPushConstantList.emplace_back(ShaderPushConstant
                {
                    .Name = String(shaderPushConstant.Name),
                    .Size = shaderPushConstant.Size,
                    .VariableList = shaderVariableList,
                    .Buffer = shaderPushConstant.Buffer,
                    .ShaderStageFlags = shaderPushConstant.ShaderStageFlags,
                    .GlobalPushContant = shaderPushConstant.GlobalPushContant
                });
        }
        shaderPushConstantListPtr = memorySystem.AddPtrBuffer<ShaderPushConstant>(shaderPushConstantList.data(), shaderPushConstantList.size(), __FILE__, __LINE__, __func__, "ShaderPushConstant");
    }

    ShaderPipelineData shaderPipelineData = ShaderPipelineData
    {
         .ShaderCount = shaderPipelineDataDLL.ShaderCount,
         .DescriptorBindingCount = shaderPipelineDataDLL.DescriptorBindingCount,
         .ShaderStructCount = shaderPipelineDataDLL.ShaderStructCount,
         .VertexInputBindingCount = shaderPipelineDataDLL.VertexInputBindingCount,  // Fixed: Use correct count for bindings
         .VertexInputAttributeListCount = shaderPipelineDataDLL.VertexInputAttributeListCount,
         .PushConstantCount = shaderPipelineDataDLL.PushConstantCount,
         .ShaderList = shaderPipelineDataDLL.ShaderList,
         .DescriptorBindingsList = shaderDescriptorBindingListPtr,
         .ShaderStructList = shaderStructListPtr,
         .VertexInputBindingList = shaderPipelineDataDLL.VertexInputBindingList,
         .VertexInputAttributeList = shaderPipelineDataDLL.VertexInputAttributeList,
         .PushConstantList = shaderPushConstantListPtr
    };

    Span<ShaderDescriptorBinding> descriptorBindingsList(shaderPipelineData.DescriptorBindingsList, shaderPipelineData.DescriptorBindingsList + shaderPipelineData.DescriptorBindingCount);
    Span<ShaderStruct> shaderStructList2(shaderPipelineData.ShaderStructList, shaderPipelineData.ShaderStructList + shaderPipelineData.ShaderStructCount);
    Span<VkVertexInputBindingDescription> vertexInputBindingList(shaderPipelineData.VertexInputBindingList, shaderPipelineData.VertexInputBindingList + shaderPipelineData.VertexInputBindingCount);
    Span<VkVertexInputAttributeDescription> vertexInputAttributeList(shaderPipelineData.VertexInputAttributeList, shaderPipelineData.VertexInputAttributeList + shaderPipelineData.VertexInputAttributeListCount);
    // Removed commented-out incorrect span
    Span<ShaderPushConstant> pushConstantList(shaderPipelineData.PushConstantList, shaderPipelineData.PushConstantList + shaderPipelineData.PushConstantCount);

    nlohmann::json pipelineJson = Json::ReadJson(pipelineJsonFilePath);
    RenderPipelineLoader renderPipelineLoader = pipelineJson.get<RenderPipelineLoader>();
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = vulkanRenderPass.SampleCount;
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = vulkanRenderPass.SampleCount > VK_SAMPLE_COUNT_1_BIT ? VK_TRUE : VK_FALSE;
    renderPipelineLoader.RenderPassId = vulkanRenderPass.RenderPassId;
    renderPipelineLoader.RenderPass = vulkanRenderPass.RenderPass;
    renderPipelineLoader.gpuIncludes = gpuIncludes;
    renderPipelineLoader.RenderPassResolution = vulkanRenderPass.RenderPassResolution;
    renderPipelineLoader.ShaderPipelineInfo = shaderPipelineData;  // Fixed typo: ShaderPiplineInfo -> ShaderPipelineInfo

    VulkanPipeline vulkanPipeline = VulkanPipeline_CreateRenderPipeline(device, vulkanRenderPass, pipelineJsonFilePath, gpuIncludes, shaderPipelineData);
    return vulkanPipeline;
}

VulkanPipeline VulkanPipeline_RebuildSwapChain(VkDevice device, VulkanPipeline& oldPipeline, VulkanRenderPass& vulkanRenderPass, const char* pipelineJsonFilePath, GPUIncludes& gpuIncludes, ShaderPipelineData& shaderPipelineData)
{
    VulkanPipeline_Destroy(device, oldPipeline);
    return VulkanPipeline_CreateRenderPipeline(device, vulkanRenderPass, pipelineJsonFilePath, gpuIncludes, shaderPipelineData);
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
    for (int x = 0; x < renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingCount; x++)
    {
        descriptorPoolSizeList.emplace_back(VkDescriptorPoolSize
            {
                .type = renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescripterType,
                .descriptorCount = static_cast<uint32>(renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingCount)
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
    Span<ShaderDescriptorBinding> descriptorBindingList(renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList, renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingCount);
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

Vector<VkDescriptorSet> Pipeline_AllocatePipelineDescriptorSets(VkDevice device, RenderPipelineLoader& renderPipelineLoader, const VkDescriptorPool& descriptorPool, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount)
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
        Span<ShaderDescriptorBinding> descriptorSetBindingList(renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList, renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingCount);
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
    if (renderPipelineLoader.ShaderPipelineInfo.PushConstantList != nullptr)
    {
        pushConstantRangeList.emplace_back(VkPushConstantRange
            {
                .stageFlags = renderPipelineLoader.ShaderPipelineInfo.PushConstantList[0].ShaderStageFlags,
                .offset = 0,
                .size = static_cast<uint>(renderPipelineLoader.ShaderPipelineInfo.PushConstantList[0].Size)
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
        .vertexBindingDescriptionCount = static_cast<uint>(renderPipelineLoader.ShaderPipelineInfo.VertexInputBindingCount),
        .pVertexBindingDescriptions = renderPipelineLoader.ShaderPipelineInfo.VertexInputBindingList,
        .vertexAttributeDescriptionCount = static_cast<uint>(renderPipelineLoader.ShaderPipelineInfo.VertexInputAttributeListCount),
        .pVertexAttributeDescriptions = renderPipelineLoader.ShaderPipelineInfo.VertexInputAttributeList
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
        Shader_LoadShader(device, renderPipelineLoader.ShaderPipelineInfo.ShaderList[0], VK_SHADER_STAGE_VERTEX_BIT),
        Shader_LoadShader(device, renderPipelineLoader.ShaderPipelineInfo.ShaderList[1], VK_SHADER_STAGE_FRAGMENT_BIT)
    };

    Span<VkPipelineColorBlendAttachmentState> attachments(renderPipelineLoader.PipelineColorBlendAttachmentStateList, renderPipelineLoader.PipelineColorBlendAttachmentStateList + renderPipelineLoader.PipelineColorBlendAttachmentStateCount);
    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfoModel = renderPipelineLoader.PipelineColorBlendStateCreateInfoModel;
    pipelineColorBlendStateCreateInfoModel.attachmentCount = renderPipelineLoader.PipelineColorBlendAttachmentStateCount;
    pipelineColorBlendStateCreateInfoModel.pAttachments = renderPipelineLoader.PipelineColorBlendAttachmentStateList;

    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = renderPipelineLoader.PipelineMultisampleStateCreateInfo;
    pipelineMultisampleStateCreateInfo.rasterizationSamples = pipelineMultisampleStateCreateInfo.rasterizationSamples >= gpuSystem.MaxSampleCount ? gpuSystem.MaxSampleCount : pipelineMultisampleStateCreateInfo.rasterizationSamples;
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
    for (int x = 0; x < renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingCount; x++)
    {
        switch (renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorBindingType)
        {
        case kVertexDescsriptor:
        {
            renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorCount = renderPipelineLoader.gpuIncludes.VertexPropertiesCount;
            renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(renderPipelineLoader.gpuIncludes.VertexPropertiesCount, __FILE__, __LINE__, __func__);
            std::memcpy(renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorBufferInfo, renderPipelineLoader.gpuIncludes.VertexProperties, renderPipelineLoader.gpuIncludes.VertexPropertiesCount * sizeof(VkDescriptorBufferInfo));
            break;
        }
        case kIndexDescriptor:
        {
            renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorCount = renderPipelineLoader.gpuIncludes.IndexPropertiesCount;
            renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(renderPipelineLoader.gpuIncludes.IndexPropertiesCount, __FILE__, __LINE__, __func__);
            std::memcpy(renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorBufferInfo, renderPipelineLoader.gpuIncludes.IndexProperties, renderPipelineLoader.gpuIncludes.IndexPropertiesCount * sizeof(VkDescriptorBufferInfo));
            break;
        }
        case kTransformDescriptor:
        {
            renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorCount = renderPipelineLoader.gpuIncludes.TransformPropertiesCount;
            renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(renderPipelineLoader.gpuIncludes.TransformPropertiesCount, __FILE__, __LINE__, __func__);
            std::memcpy(renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorBufferInfo, renderPipelineLoader.gpuIncludes.TransformProperties, renderPipelineLoader.gpuIncludes.TransformPropertiesCount * sizeof(VkDescriptorBufferInfo));
            break;
        }
        case kMeshPropertiesDescriptor:
        {
            renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorCount = renderPipelineLoader.gpuIncludes.MeshPropertiesCount;
            renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(renderPipelineLoader.gpuIncludes.MeshPropertiesCount, __FILE__, __LINE__, __func__);
            std::memcpy(renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorBufferInfo, renderPipelineLoader.gpuIncludes.MeshProperties, renderPipelineLoader.gpuIncludes.MeshPropertiesCount * sizeof(VkDescriptorBufferInfo));
            break;
        }
        case kTextureDescriptor:
        {
            renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorCount = renderPipelineLoader.gpuIncludes.TexturePropertiesCount;
            renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorImageInfo = memorySystem.AddPtrBuffer<VkDescriptorImageInfo>(renderPipelineLoader.gpuIncludes.TexturePropertiesCount, __FILE__, __LINE__, __func__);
            std::memcpy(renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorImageInfo, renderPipelineLoader.gpuIncludes.TextureProperties, renderPipelineLoader.gpuIncludes.TexturePropertiesCount * sizeof(VkDescriptorImageInfo));
            break;
        }
        case kMaterialDescriptor:
        {
            renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorCount = renderPipelineLoader.gpuIncludes.MaterialPropertiesCount;
            renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = memorySystem.AddPtrBuffer<VkDescriptorBufferInfo>(renderPipelineLoader.gpuIncludes.MaterialPropertiesCount, __FILE__, __LINE__, __func__);
            std::memcpy(renderPipelineLoader.ShaderPipelineInfo.DescriptorBindingsList[x].DescriptorBufferInfo, renderPipelineLoader.gpuIncludes.MaterialProperties, renderPipelineLoader.gpuIncludes.MaterialPropertiesCount * sizeof(VkDescriptorBufferInfo));
            break;
        }
        default:
        {
            throw std::runtime_error("Binding case hasn't been handled yet");
        }
        }
    }
}