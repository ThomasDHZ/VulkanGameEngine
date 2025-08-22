#include "VulkanPipeline.h"
#include "MemorySystem.h"
#include "json.h"
#include "ShaderCompiler.h"
#include "JsonLoader.h"

 VulkanPipeline VulkanPipeline_CreateRenderPipeline(VkDevice device, VkGuid& renderPassId, const char* pipelineJsonPath, VkRenderPass renderPass, ShaderPushConstant& pushConstant, ivec2& renderPassResolution, const GPUIncludes& includes)
 {
     const char* jsonDataString = File_Read(pipelineJsonPath).Data;
     RenderPipelineLoader renderPassLoader = nlohmann::json::parse(jsonDataString).get<RenderPipelineLoader>();

     VkPipelineCache pipelineCache = VK_NULL_HANDLE;
     VkDescriptorPool descriptorPool = Pipeline_CreatePipelineDescriptorPool(device, renderPassLoader, includes);
     Vector<VkDescriptorSetLayout> descriptorSetLayoutList = Pipeline_CreatePipelineDescriptorSetLayout(device, renderPassLoader, includes);
     Vector<VkDescriptorSet> descriptorSetList = Pipeline_AllocatePipelineDescriptorSets(device, descriptorPool, renderPassLoader, descriptorSetLayoutList);
     Pipeline_UpdatePipelineDescriptorSets(device, descriptorSetList, renderPassLoader, includes);
     VkPipelineLayout pipelineLayout = Pipeline_CreatePipelineLayout(device, descriptorSetLayoutList, pushConstant);
     VkPipeline pipeline = Pipeline_CreatePipeline(device, renderPass, pipelineLayout, pipelineCache, renderPassLoader, renderPassResolution);

     VulkanPipeline* vulkanRenderPipelinePtr = new VulkanPipeline
     {
         .RenderPipelineId = renderPassLoader.PipelineId,
         .DescriptorSetLayoutCount = descriptorSetLayoutList.size(),
         .DescriptorSetCount = descriptorSetList.size(),
         .DescriptorPool = descriptorPool,
         .Pipeline = pipeline,
         .PipelineLayout = pipelineLayout,
         .PipelineCache = pipelineCache
     };

     vulkanRenderPipelinePtr->DescriptorSetLayoutList = nullptr;
     if (vulkanRenderPipelinePtr->DescriptorSetLayoutCount > 0)
     {
         vulkanRenderPipelinePtr->DescriptorSetLayoutList = memorySystem.AddPtrBuffer<VkDescriptorSetLayout>(descriptorSetLayoutList.size(), __FILE__, __LINE__, __func__);
         std::memcpy(vulkanRenderPipelinePtr->DescriptorSetLayoutList, descriptorSetLayoutList.data(), vulkanRenderPipelinePtr->DescriptorSetLayoutCount * sizeof(VkFramebuffer));
     }

     vulkanRenderPipelinePtr->DescriptorSetList = nullptr;
     if (vulkanRenderPipelinePtr->DescriptorSetCount > 0)
     {
         vulkanRenderPipelinePtr->DescriptorSetList = memorySystem.AddPtrBuffer<VkDescriptorSet>(descriptorSetList.size(), __FILE__, __LINE__, __func__);
         std::memcpy(vulkanRenderPipelinePtr->DescriptorSetList, descriptorSetList.data(), vulkanRenderPipelinePtr->DescriptorSetCount * sizeof(VkClearValue));
     }

     VulkanPipeline vulkanPipeline = *vulkanRenderPipelinePtr;
     delete vulkanRenderPipelinePtr;
     return vulkanPipeline;
 }

 VulkanPipeline VulkanPipeline_RebuildSwapChain(VkDevice device, VkGuid& renderPassId, VulkanPipeline& oldVulkanPipeline, const char* pipelineJson, VkRenderPass renderPass, ShaderPushConstant& pushConstant, ivec2& renderPassResolution, const GPUIncludes& includes)
 {
     VulkanPipeline_Destroy(device, oldVulkanPipeline);

     RenderPipelineLoader renderPassLoader = nlohmann::json::parse(pipelineJson).get<RenderPipelineLoader>();

     VkPipelineCache pipelineCache = VK_NULL_HANDLE;
     VkDescriptorPool descriptorPool = Pipeline_CreatePipelineDescriptorPool(device, renderPassLoader, includes);
     Vector<VkDescriptorSetLayout> descriptorSetLayoutList = Pipeline_CreatePipelineDescriptorSetLayout(device, renderPassLoader, includes);
     Vector<VkDescriptorSet> descriptorSetList = Pipeline_AllocatePipelineDescriptorSets(device, descriptorPool, renderPassLoader, descriptorSetLayoutList);
     Pipeline_UpdatePipelineDescriptorSets(device, descriptorSetList, renderPassLoader, includes);
     VkPipelineLayout pipelineLayout = Pipeline_CreatePipelineLayout(device, descriptorSetLayoutList, pushConstant);
     VkPipeline pipeline = Pipeline_CreatePipeline(device, renderPass, pipelineLayout, pipelineCache, renderPassLoader, renderPassResolution);

     VulkanPipeline* vulkanRenderPipelinePtr = new VulkanPipeline
     {
         .RenderPipelineId = renderPassLoader.PipelineId,
         .DescriptorSetLayoutCount = descriptorSetLayoutList.size(),
         .DescriptorSetCount = descriptorSetList.size(),
         .DescriptorPool = descriptorPool,
         .Pipeline = pipeline,
         .PipelineLayout = pipelineLayout,
         .PipelineCache = pipelineCache
     };

     vulkanRenderPipelinePtr->DescriptorSetLayoutList = nullptr;
     if (vulkanRenderPipelinePtr->DescriptorSetLayoutCount > 0)
     {
         vulkanRenderPipelinePtr->DescriptorSetLayoutList = memorySystem.AddPtrBuffer<VkDescriptorSetLayout>(descriptorSetLayoutList.size(), __FILE__, __LINE__, __func__);
         std::memcpy(vulkanRenderPipelinePtr->DescriptorSetLayoutList, descriptorSetLayoutList.data(), vulkanRenderPipelinePtr->DescriptorSetLayoutCount * sizeof(VkFramebuffer));
     }

     vulkanRenderPipelinePtr->DescriptorSetList = nullptr;
     if (vulkanRenderPipelinePtr->DescriptorSetCount > 0)
     {
         vulkanRenderPipelinePtr->DescriptorSetList = memorySystem.AddPtrBuffer<VkDescriptorSet>(descriptorSetList.size(), __FILE__, __LINE__, __func__);
         std::memcpy(vulkanRenderPipelinePtr->DescriptorSetList, descriptorSetList.data(), vulkanRenderPipelinePtr->DescriptorSetCount * sizeof(VkClearValue));
     }

     VulkanPipeline vulkanPipeline = *vulkanRenderPipelinePtr;
     delete vulkanRenderPipelinePtr;
     return vulkanPipeline;
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


VkDescriptorPool Pipeline_CreatePipelineDescriptorPool(VkDevice device, const RenderPipelineLoader& model, const GPUIncludes& includes)
{
    const Vector<VkDescriptorBufferInfo> vertexPropertiesList = Vector<VkDescriptorBufferInfo>(includes.VertexProperties, includes.VertexProperties + includes.VertexPropertiesCount);
    const Vector<VkDescriptorBufferInfo> indexPropertiesList = Vector<VkDescriptorBufferInfo>(includes.IndexProperties, includes.IndexProperties + includes.IndexPropertiesCount);
    const Vector<VkDescriptorBufferInfo> transformPropertiesList = Vector<VkDescriptorBufferInfo>(includes.TransformProperties, includes.TransformProperties + includes.TransformPropertiesCount);
    const Vector<VkDescriptorBufferInfo> meshPropertiesList = Vector<VkDescriptorBufferInfo>(includes.MeshProperties, includes.MeshProperties + includes.MeshPropertiesCount);
    const Vector<VkDescriptorBufferInfo> levelLayerMeshPropertiesList = Vector<VkDescriptorBufferInfo>(includes.LevelLayerMeshProperties, includes.LevelLayerMeshProperties + includes.LevelLayerMeshPropertiesCount);
    const Vector<VkDescriptorImageInfo>  texturePropertiesList = Vector<VkDescriptorImageInfo>(includes.TexturePropertiesList, includes.TexturePropertiesList + includes.TexturePropertiesListCount);
    const Vector<VkDescriptorBufferInfo> materialPropertiesList = Vector<VkDescriptorBufferInfo>(includes.MaterialProperties, includes.MaterialProperties + includes.MaterialPropertiesCount);

    Vector<VkDescriptorPoolSize> descriptorPoolSizeList = Vector<VkDescriptorPoolSize>();
    for (auto binding : model.PipelineDescriptorModelsList)
    {
        switch (binding.BindingPropertiesList)
        {
        case kVertexDescsriptor:
        {
            descriptorPoolSizeList.emplace_back(VkDescriptorPoolSize
                {
                    .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = static_cast<uint32>(vertexPropertiesList.size())
                });
            break;
        }
        case kIndexDescriptor:
        {
            descriptorPoolSizeList.emplace_back(VkDescriptorPoolSize
                {
                    .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = static_cast<uint32>(indexPropertiesList.size())
                });
            break;
        }
        case kTransformDescriptor:
        {
            descriptorPoolSizeList.emplace_back(VkDescriptorPoolSize
                {
                    .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = static_cast<uint32>(transformPropertiesList.size())
                });
            break;
        }
        case kMeshPropertiesDescriptor:
        {
            descriptorPoolSizeList.emplace_back(VkDescriptorPoolSize
                {
                    .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = static_cast<uint32>(meshPropertiesList.size())
                });
            break;
        }
        case kTextureDescriptor:
        {
            descriptorPoolSizeList.emplace_back(VkDescriptorPoolSize
                {
                    .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = static_cast<uint32>(texturePropertiesList.size())
                });
            break;
        }
        case kMaterialDescriptor:
        {
            descriptorPoolSizeList.emplace_back(VkDescriptorPoolSize
                {
                    .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = static_cast<uint32>(materialPropertiesList.size())
                });
            break;
        }
        default:
        {
            throw std::runtime_error("Binding case hasn't been handled yet");
        }
        }
    }

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorPoolCreateInfo poolCreateInfo = VkDescriptorPoolCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = 500,
        .poolSizeCount = static_cast<uint32>(descriptorPoolSizeList.size()),
        .pPoolSizes = descriptorPoolSizeList.data()
    };
    VULKAN_RESULT(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &descriptorPool));
    return descriptorPool;
}

Vector<VkDescriptorSetLayout> Pipeline_CreatePipelineDescriptorSetLayout(VkDevice device, const RenderPipelineLoader& model, const GPUIncludes& includes)
{
    const Vector<VkDescriptorBufferInfo> vertexPropertiesList = Vector<VkDescriptorBufferInfo>(includes.VertexProperties, includes.VertexProperties + includes.VertexPropertiesCount);
    const Vector<VkDescriptorBufferInfo> indexPropertiesList = Vector<VkDescriptorBufferInfo>(includes.IndexProperties, includes.IndexProperties + includes.IndexPropertiesCount);
    const Vector<VkDescriptorBufferInfo> transformPropertiesList = Vector<VkDescriptorBufferInfo>(includes.TransformProperties, includes.TransformProperties + includes.TransformPropertiesCount);
    const Vector<VkDescriptorBufferInfo> meshPropertiesList = Vector<VkDescriptorBufferInfo>(includes.MeshProperties, includes.MeshProperties + includes.MeshPropertiesCount);
    const Vector<VkDescriptorBufferInfo> levelLayerMeshPropertiesList = Vector<VkDescriptorBufferInfo>(includes.LevelLayerMeshProperties, includes.LevelLayerMeshProperties + includes.LevelLayerMeshPropertiesCount);
    const Vector<VkDescriptorImageInfo>  texturePropertiesList = Vector<VkDescriptorImageInfo>(includes.TexturePropertiesList, includes.TexturePropertiesList + includes.TexturePropertiesListCount);
    const Vector<VkDescriptorBufferInfo> materialPropertiesList = Vector<VkDescriptorBufferInfo>(includes.MaterialProperties, includes.MaterialProperties + includes.MaterialPropertiesCount);

    
    Vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList = Vector<VkDescriptorSetLayoutBinding>();
    for (auto binding : model.PipelineDescriptorModelsList)
    {
        descriptorSetLayoutBindingList.emplace_back(VkDescriptorSetLayoutBinding
            {
                .binding = binding.BindingNumber,
                .descriptorType = binding.DescriptorType,
                .stageFlags = binding.StageFlags,
                .pImmutableSamplers = binding.pImmutableSamplers
            });

        switch (binding.BindingPropertiesList)
        {
            case kVertexDescsriptor: descriptorSetLayoutBindingList.back().descriptorCount = static_cast<uint32>(meshPropertiesList.size()); break;
            case kIndexDescriptor: descriptorSetLayoutBindingList.back().descriptorCount = static_cast<uint32>(meshPropertiesList.size()); break;
            case kTransformDescriptor: descriptorSetLayoutBindingList.back().descriptorCount = static_cast<uint32>(transformPropertiesList.size()); break;
            case kMeshPropertiesDescriptor: descriptorSetLayoutBindingList.back().descriptorCount = static_cast<uint32>(meshPropertiesList.size()); break;
            case kTextureDescriptor: descriptorSetLayoutBindingList.back().descriptorCount = static_cast<uint32>(texturePropertiesList.size()); break;
            case kMaterialDescriptor: descriptorSetLayoutBindingList.back().descriptorCount = static_cast<uint32>(materialPropertiesList.size()); break;
            default:
            {
                throw std::runtime_error("Binding case hasn't been handled yet");
            }
        }
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

Vector<VkDescriptorSet> Pipeline_AllocatePipelineDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, const RenderPipelineLoader& model, const Vector<VkDescriptorSetLayout>& descriptorSetLayoutList)
{
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32>(descriptorSetLayoutList.size()),
        .pSetLayouts = descriptorSetLayoutList.data()
    };

    Vector<VkDescriptorSet> descriptorSetList = Vector<VkDescriptorSet>(1);
    for (auto& descriptorSet : descriptorSetList)
    {
        VULKAN_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
    }
    return descriptorSetList;
}

void Pipeline_UpdatePipelineDescriptorSets(VkDevice device, const Vector<VkDescriptorSet>& descriptorSetList, const RenderPipelineLoader& model, const GPUIncludes& includes)
{
    const Vector<VkDescriptorBufferInfo> vertexPropertiesList = Vector<VkDescriptorBufferInfo>(includes.VertexProperties, includes.VertexProperties + includes.VertexPropertiesCount);
    const Vector<VkDescriptorBufferInfo> indexPropertiesList = Vector<VkDescriptorBufferInfo>(includes.IndexProperties, includes.IndexProperties + includes.IndexPropertiesCount);
    const Vector<VkDescriptorBufferInfo> transformPropertiesList = Vector<VkDescriptorBufferInfo>(includes.TransformProperties, includes.TransformProperties + includes.TransformPropertiesCount);
    const Vector<VkDescriptorBufferInfo> meshPropertiesList = Vector<VkDescriptorBufferInfo>(includes.MeshProperties, includes.MeshProperties + includes.MeshPropertiesCount);
    const Vector<VkDescriptorBufferInfo> levelLayerMeshPropertiesList = Vector<VkDescriptorBufferInfo>(includes.LevelLayerMeshProperties, includes.LevelLayerMeshProperties + includes.LevelLayerMeshPropertiesCount);
    const Vector<VkDescriptorImageInfo>  texturePropertiesList = Vector<VkDescriptorImageInfo>(includes.TexturePropertiesList, includes.TexturePropertiesList + includes.TexturePropertiesListCount);
    const Vector<VkDescriptorBufferInfo> materialPropertiesList = Vector<VkDescriptorBufferInfo>(includes.MaterialProperties, includes.MaterialProperties + includes.MaterialPropertiesCount);

    for (auto& descriptorSet : descriptorSetList)
    {
        Vector<VkWriteDescriptorSet> writeDescriptorSet = Vector<VkWriteDescriptorSet>();
        for (auto binding : model.PipelineDescriptorModelsList)
        {
            writeDescriptorSet.emplace_back(VkWriteDescriptorSet
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = descriptorSet,
                    .dstBinding = binding.BindingNumber,
                    .dstArrayElement = binding.DstArrayElement,
                    .descriptorCount = static_cast<uint32>(meshPropertiesList.size()),
                    .descriptorType = binding.DescriptorType,
                    .pImageInfo = nullptr,
                    .pBufferInfo = nullptr,
                    .pTexelBufferView = binding.pTexelBufferView
                });

            switch (binding.BindingPropertiesList)
            {
                case kVertexDescsriptor:
                {
                    writeDescriptorSet.back().descriptorCount = static_cast<uint32>(meshPropertiesList.size());
                    writeDescriptorSet.back().pBufferInfo = meshPropertiesList.data();
                    break;
                }
                case kIndexDescriptor:
                {
                    writeDescriptorSet.back().descriptorCount = static_cast<uint32>(meshPropertiesList.size());
                    writeDescriptorSet.back().pBufferInfo = meshPropertiesList.data();
                    break;
                }
                case kTransformDescriptor:
                {
                    writeDescriptorSet.back().descriptorCount = static_cast<uint32>(transformPropertiesList.size());
                    writeDescriptorSet.back().pBufferInfo = transformPropertiesList.data();
                    break;
                }
                case kMeshPropertiesDescriptor:
                {
                    writeDescriptorSet.back().descriptorCount = static_cast<uint32>(meshPropertiesList.size());
                    writeDescriptorSet.back().pBufferInfo = meshPropertiesList.data();
                    break;
                }
                case kTextureDescriptor:
                {
                    writeDescriptorSet.back().descriptorCount = static_cast<uint32>(texturePropertiesList.size());
                    writeDescriptorSet.back().pImageInfo = texturePropertiesList.data();
                    break;
                }
                case kMaterialDescriptor:
                {
                    writeDescriptorSet.back().descriptorCount = static_cast<uint32>(materialPropertiesList.size());
                    writeDescriptorSet.back().pBufferInfo = materialPropertiesList.data();
                    break;
                }
                default:
                {
                    throw std::runtime_error("Binding case hasn't been handled yet");
                }
            }
        }
        vkUpdateDescriptorSets(device, static_cast<uint32>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
    }
}

VkPipelineLayout Pipeline_CreatePipelineLayout(VkDevice device, const Vector<VkDescriptorSetLayout>& descriptorSetLayoutList, ShaderPushConstant& pushConstant)
{
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    Vector<VkPushConstantRange> pushConstantRangeList = Vector<VkPushConstantRange>();
    if (pushConstant.PushConstantSize > 0)
    {
        pushConstantRangeList.emplace_back(VkPushConstantRange
            {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = static_cast<uint>(pushConstant.PushConstantSize)
            });
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = VkPipelineLayoutCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = static_cast<uint32>(descriptorSetLayoutList.size()),
        .pSetLayouts = descriptorSetLayoutList.data(),
        .pushConstantRangeCount = static_cast<uint32>(pushConstantRangeList.size()),
        .pPushConstantRanges = pushConstantRangeList.data()
    };
    VULKAN_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));
    return pipelineLayout;
}

VkPipeline Pipeline_CreatePipeline(VkDevice device, VkRenderPass renderpass, VkPipelineLayout pipelineLayout, VkPipelineCache pipelineCache, const RenderPipelineLoader& model, ivec2& extent)
{
    VkPipeline pipeline = VK_NULL_HANDLE;
    Vector<VkVertexInputBindingDescription> vertexInputBindingList;
    Vector<VkVertexInputAttributeDescription> vertexInputAttributeList;
    SpvReflectShaderModule module = Shader_GetShaderData(model.VertexShaderPath);
    Shader_GetShaderInputVertexVariables(module, vertexInputBindingList, vertexInputAttributeList);
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = VkPipelineVertexInputStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = static_cast<uint>(vertexInputBindingList.size()),
        .pVertexBindingDescriptions = vertexInputBindingList.data(),
        .vertexAttributeDescriptionCount = static_cast<uint>(vertexInputAttributeList.size()),
        .pVertexAttributeDescriptions = vertexInputAttributeList.data()
    };

    Vector<VkViewport> viewPortList;
    Vector<VkRect2D> scissorList;
    Vector<VkDynamicState> dynamicStateList;
    if (model.ViewportList.empty())
    {
        dynamicStateList = Vector<VkDynamicState>
        {
            VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
            VkDynamicState::VK_DYNAMIC_STATE_SCISSOR
        };
    }
    else
    {
        for (auto& viewPort : model.ViewportList)
        {
            viewPortList.emplace_back(VkViewport
                {
                    .x = viewPort.x,
                    .y = viewPort.y,
                    .width = static_cast<float>(extent.x),
                    .height = static_cast<float>(extent.y),
                    .minDepth = viewPort.minDepth,
                    .maxDepth = viewPort.maxDepth
                });
        }
        for (auto& viewPort : model.ViewportList)
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
                        .width = static_cast<uint32>(extent.x),
                        .height = static_cast<uint32>(extent.y)
                    }
                });
        }
    }

    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = VkPipelineViewportStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = static_cast<uint32>(viewPortList.size() + (model.ViewportList.empty() ? 1 : 0)),
        .pViewports = viewPortList.data(),
        .scissorCount = static_cast<uint32>(scissorList.size() + (model.ScissorList.empty() ? 1 : 0)),
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
        Shader_CreateShader(device, model.VertexShaderPath, VK_SHADER_STAGE_VERTEX_BIT),
        Shader_CreateShader(device, model.FragmentShaderPath, VK_SHADER_STAGE_FRAGMENT_BIT)
    };

    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfoModel = model.PipelineColorBlendStateCreateInfoModel;
    pipelineColorBlendStateCreateInfoModel.attachmentCount = model.PipelineColorBlendAttachmentStateList.size();
    pipelineColorBlendStateCreateInfoModel.pAttachments = model.PipelineColorBlendAttachmentStateList.data();

    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = model.PipelineMultisampleStateCreateInfo;
    pipelineMultisampleStateCreateInfo.pSampleMask = nullptr;

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = VkGraphicsPipelineCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = static_cast<uint32>(pipelineShaderStageCreateInfoList.size()),
        .pStages = pipelineShaderStageCreateInfoList.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &model.PipelineInputAssemblyStateCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &pipelineViewportStateCreateInfo,
        .pRasterizationState = &model.PipelineRasterizationStateCreateInfo,
        .pMultisampleState = &pipelineMultisampleStateCreateInfo,
        .pDepthStencilState = &model.PipelineDepthStencilStateCreateInfo,
        .pColorBlendState = &pipelineColorBlendStateCreateInfoModel,
        .pDynamicState = &pipelineDynamicStateCreateInfo,
        .layout = pipelineLayout,
        .renderPass = renderpass,
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

