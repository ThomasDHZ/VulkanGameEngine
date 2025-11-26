#include "VulkanPipeline.h"
#include "MemorySystem.h"
#include "RenderSystem.h"
#include "MaterialSystem.h"
#include "ShaderSystem.h"
#include "JsonLoader.h"
#include "GPUSystem.h"
#include "JsonStruct.h"
#include "FileSystem.h"

VulkanPipeline VulkanPipeline_CreateRenderPipeline(VulkanRenderPass& vulkanRenderPass, const char* pipelineJsonFilePath, ShaderPipelineDataDLL& shaderPipelineData)
{
    nlohmann::json pipelineJson = File_LoadJsonFile(pipelineJsonFilePath);
    RenderPipelineLoader renderPipelineLoader = pipelineJson.get<RenderPipelineLoader>();
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = vulkanRenderPass.SampleCount;
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = vulkanRenderPass.SampleCount;
    renderPipelineLoader.RenderPassId = vulkanRenderPass.RenderPassId;
    renderPipelineLoader.RenderPass = vulkanRenderPass.RenderPass;
    renderPipelineLoader.RenderPassResolution = vulkanRenderPass.RenderPassResolution;
    renderPipelineLoader.ShaderPiplineInfo = shaderPipelineData;

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    Pipeline_PipelineBindingData(renderPipelineLoader);
    VkDescriptorPool descriptorPool = Pipeline_CreatePipelineDescriptorPool(renderPipelineLoader);
    Vector<VkDescriptorSetLayout> descriptorSetLayoutList = Pipeline_CreatePipelineDescriptorSetLayout(renderPipelineLoader);
    Vector<VkDescriptorSet> descriptorSetList = Pipeline_AllocatePipelineDescriptorSets(renderPipelineLoader, descriptorPool, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
    Pipeline_UpdatePipelineDescriptorSets(renderPipelineLoader, descriptorSetList.data(), descriptorSetList.size());
    VkPipelineLayout pipelineLayout = Pipeline_CreatePipelineLayout(renderPipelineLoader, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
    VkPipeline pipeline = Pipeline_CreatePipeline(renderPipelineLoader, pipelineCache, pipelineLayout, descriptorSetList.data(), descriptorSetList.size());

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

VulkanPipeline VulkanPipeline_RebuildSwapChain(VulkanPipeline& oldPipeline, VulkanRenderPass& vulkanRenderPass, const char* pipelineJsonFilePath, ShaderPipelineDataDLL& shaderPipelineData)
{
    VulkanPipeline_Destroy(oldPipeline);
    return VulkanPipeline_CreateRenderPipeline(vulkanRenderPass, pipelineJsonFilePath, shaderPipelineData);
}

void VulkanPipeline_Destroy(VulkanPipeline& vulkanPipeline)
{
    vulkanPipeline.RenderPipelineId = VkGuid();
    Renderer_DestroyPipeline(renderer.Device, &vulkanPipeline.Pipeline);
    Renderer_DestroyPipelineLayout(renderer.Device, &vulkanPipeline.PipelineLayout);
    Renderer_DestroyPipelineCache(renderer.Device, &vulkanPipeline.PipelineCache);
    Renderer_DestroyDescriptorPool(renderer.Device, &vulkanPipeline.DescriptorPool);
}


VkDescriptorPool Pipeline_CreatePipelineDescriptorPool(RenderPipelineLoader& renderPipelineLoader)
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
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorPool(renderer.Device, &poolCreateInfo, nullptr, &descriptorPool));
    return descriptorPool;
}

Vector<VkDescriptorSetLayout> Pipeline_CreatePipelineDescriptorSetLayout(RenderPipelineLoader& renderPipelineLoader)
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
        VULKAN_THROW_IF_FAIL(vkCreateDescriptorSetLayout(renderer.Device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout));
    }

    return descriptorSetLayoutList;
}

Vector<VkDescriptorSet> Pipeline_AllocatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader, const VkDescriptorPool& descriptorPool, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount)
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
        VULKAN_THROW_IF_FAIL(vkAllocateDescriptorSets(renderer.Device, &allocInfo, &descriptorSet));
    }
    return descriptorSetList;
}

void Pipeline_UpdatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount)
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
        vkUpdateDescriptorSets(renderer.Device, static_cast<uint32>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
    }
}

VkPipelineLayout Pipeline_CreatePipelineLayout(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount)
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
    VULKAN_THROW_IF_FAIL(vkCreatePipelineLayout(renderer.Device, &pipelineLayoutInfo, nullptr, &pipelineLayout));
    return pipelineLayout;
}

VkPipeline Pipeline_CreatePipeline(RenderPipelineLoader& renderPipelineLoader, VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount)
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

    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = VkPipelineViewportStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = static_cast<uint32>(renderPipelineLoader.ViewportList.size() + (renderPipelineLoader.ViewportList.empty() ? 1 : 0)),
        .pViewports = renderPipelineLoader.ViewportList.data(),
        .scissorCount = static_cast<uint32>(renderPipelineLoader.ScissorList.size() + (renderPipelineLoader.ScissorList.empty() ? 1 : 0)),
        .pScissors = renderPipelineLoader.ScissorList.data()
    };

    Vector<VkDynamicState> dynamicStateList = renderPipelineLoader.ViewportList.empty() ? Vector<VkDynamicState> { VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT, VkDynamicState::VK_DYNAMIC_STATE_SCISSOR} : Vector<VkDynamicState>();
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
        shaderSystem.LoadShader(renderPipelineLoader.ShaderPiplineInfo.ShaderList[0].c_str(), VK_SHADER_STAGE_VERTEX_BIT),
        shaderSystem.LoadShader(renderPipelineLoader.ShaderPiplineInfo.ShaderList[1].c_str(), VK_SHADER_STAGE_FRAGMENT_BIT)
    };

     VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfoModel = renderPipelineLoader.PipelineColorBlendStateCreateInfoModel;
    pipelineColorBlendStateCreateInfoModel.attachmentCount = renderPipelineLoader.PipelineColorBlendAttachmentStateList.size();
    pipelineColorBlendStateCreateInfoModel.pAttachments = renderPipelineLoader.PipelineColorBlendAttachmentStateList.data();

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

    VULKAN_THROW_IF_FAIL(vkCreateGraphicsPipelines(renderer.Device, pipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline));
    for (auto& shader : pipelineShaderStageCreateInfoList)
    {
        vkDestroyShaderModule(renderer.Device, shader.module, nullptr);
    }

    return pipeline;
}

void Pipeline_PipelineBindingData(RenderPipelineLoader& renderPipelineLoader)
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