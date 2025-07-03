#include "VulkanPipeline.h"
#include "MemorySystem.h"
#include "json.h"
#include "ShaderCompiler.h"
#include "JsonLoader.h"

 VulkanPipeline VulkanPipeline_CreateRenderPipeline(VkDevice device, VkGuid& renderPassId, uint renderPipelineId, const char* pipelineJson, VkRenderPass renderPass, size_t constBufferSize, ivec2& renderPassResolution, const GPUIncludes& includes)
 {
     RenderPipelineLoader model = JsonLoader_LoadRenderPipelineLoaderInfo(pipelineJson, renderPassResolution);
     Vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfoList = Vector<VkPipelineShaderStageCreateInfo>
     {
         Shader_CreateShader(device, model.VertexShaderPath, VK_SHADER_STAGE_VERTEX_BIT),
         Shader_CreateShader(device, model.FragmentShaderPath, VK_SHADER_STAGE_FRAGMENT_BIT)
     };

     VkPipelineCache pipelineCache = VK_NULL_HANDLE;
     VkDescriptorPool descriptorPool = Pipeline_CreatePipelineDescriptorPool(device, model, includes);
     Vector<VkDescriptorSetLayout> descriptorSetLayoutList = Pipeline_CreatePipelineDescriptorSetLayout(device, model, includes);
     Vector<VkDescriptorSet> descriptorSetList = Pipeline_AllocatePipelineDescriptorSets(device, descriptorPool, model, descriptorSetLayoutList);
     Pipeline_UpdatePipelineDescriptorSets(device, descriptorSetList, model, includes);
     VkPipelineLayout pipelineLayout = Pipeline_CreatePipelineLayout(device, descriptorSetLayoutList, constBufferSize);
     VkPipeline pipeline = Pipeline_CreatePipeline(device, renderPass, pipelineLayout, pipelineCache, model, renderPassResolution, pipelineShaderStageCreateInfoList);

     VulkanPipeline* vulkanRenderPipelinePtr = new VulkanPipeline
     {
         .RenderPipelineId = renderPipelineId,
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

void VulkanPipeline_RecreateSwapchain(VkRenderPass renderPass, uint constBufferSize, int newWidth, int newHeight)
{
    //GPUIncludes include =
    //{
    //    .vertexProperties = renderSystem.GetVertexPropertiesBuffer(),
    //    .indexProperties = renderSystem.GetIndexPropertiesBuffer(),
    //    //        .transformProperties = renderSystem.GetTransformPropertiesBuffer(gpuImport.MeshList),
    //    .meshProperties = renderSystem.GetMeshPropertiesBuffer(),
    //    .texturePropertiesList = renderSystem.GetTexturePropertiesBuffer(renderSystem.InputTextureList[RenderPipelineId]),
    //    .materialProperties = renderSystem.GetMaterialPropertiesBuffer()
    //};

    //Destroy();

    //ivec2 renderPassResolution = ivec2(newWidth, newHeight);
    //DescriptorPool = Pipeline_CreateDescriptorPool(*renderSystem.Device.get(), renderSystem.renderPipelineModelList[RenderPipelineId], include);
    //DescriptorSetLayoutList = Pipeline_CreateDescriptorSetLayout(*renderSystem.Device.get(), renderSystem.renderPipelineModelList[RenderPipelineId], include);
    //DescriptorSetList = Pipeline_AllocateDescriptorSets(*renderSystem.Device.get(), DescriptorPool, renderSystem.renderPipelineModelList[RenderPipelineId], DescriptorSetLayoutList);
    //Pipeline_UpdateDescriptorSets(*renderSystem.Device.get(), DescriptorSetList, renderSystem.renderPipelineModelList[RenderPipelineId], include);
    //PipelineLayout = Pipeline_CreatePipelineLayout(*renderSystem.Device.get(), DescriptorSetLayoutList, constBufferSize);
    //Pipeline = Pipeline_CreatePipeline(*renderSystem.Device.get(), renderPass, PipelineLayout, PipelineCache, renderSystem.renderPipelineModelList[RenderPipelineId], renderPassResolution);
}

void VulkanPipeline_Destroy(VkDevice device, VulkanPipeline& vulkanPipeline)
{
    vulkanPipeline.RenderPipelineId = 0;
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
    Vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingList = Vector<VkDescriptorSetLayoutBinding>();
    const Vector<VkDescriptorBufferInfo> vertexPropertiesList = Vector<VkDescriptorBufferInfo>(includes.VertexProperties, includes.VertexProperties + includes.VertexPropertiesCount);
    const Vector<VkDescriptorBufferInfo> indexPropertiesList = Vector<VkDescriptorBufferInfo>(includes.IndexProperties, includes.IndexProperties + includes.IndexPropertiesCount);
    const Vector<VkDescriptorBufferInfo> transformPropertiesList = Vector<VkDescriptorBufferInfo>(includes.TransformProperties, includes.TransformProperties + includes.TransformPropertiesCount);
    const Vector<VkDescriptorBufferInfo> meshPropertiesList = Vector<VkDescriptorBufferInfo>(includes.MeshProperties, includes.MeshProperties + includes.MeshPropertiesCount);
    const Vector<VkDescriptorBufferInfo> levelLayerMeshPropertiesList = Vector<VkDescriptorBufferInfo>(includes.LevelLayerMeshProperties, includes.LevelLayerMeshProperties + includes.LevelLayerMeshPropertiesCount);
    const Vector<VkDescriptorImageInfo>  texturePropertiesList = Vector<VkDescriptorImageInfo>(includes.TexturePropertiesList, includes.TexturePropertiesList + includes.TexturePropertiesListCount);
    const Vector<VkDescriptorBufferInfo> materialPropertiesList = Vector<VkDescriptorBufferInfo>(includes.MaterialProperties, includes.MaterialProperties + includes.MaterialPropertiesCount);
    for (auto binding : model.PipelineDescriptorModelsList)
    {
        switch (binding.BindingPropertiesList)
        {
        case kVertexDescsriptor:
        {
            descriptorSetLayoutBindingList.emplace_back(VkDescriptorSetLayoutBinding
                {
                    .binding = binding.BindingNumber,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = static_cast<uint32>(meshPropertiesList.size()),
                    .stageFlags = VK_SHADER_STAGE_ALL,
                    .pImmutableSamplers = nullptr
                });
            break;
        }
        case kIndexDescriptor:
        {
            descriptorSetLayoutBindingList.emplace_back(VkDescriptorSetLayoutBinding
                {
                    .binding = binding.BindingNumber,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = static_cast<uint32>(meshPropertiesList.size()),
                    .stageFlags = VK_SHADER_STAGE_ALL,
                    .pImmutableSamplers = nullptr
                });
            break;
        }
        case kTransformDescriptor:
        {
            descriptorSetLayoutBindingList.emplace_back(VkDescriptorSetLayoutBinding
                {
                    .binding = binding.BindingNumber,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = static_cast<uint32>(transformPropertiesList.size()),
                    .stageFlags = VK_SHADER_STAGE_ALL,
                    .pImmutableSamplers = nullptr
                });
            break;
        }
        case kMeshPropertiesDescriptor:
        {
            descriptorSetLayoutBindingList.emplace_back(VkDescriptorSetLayoutBinding
                {
                    .binding = binding.BindingNumber,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = static_cast<uint32>(meshPropertiesList.size()),
                    .stageFlags = VK_SHADER_STAGE_ALL,
                    .pImmutableSamplers = nullptr
                });
            break;
        }
        case kTextureDescriptor:
        {
            descriptorSetLayoutBindingList.emplace_back(VkDescriptorSetLayoutBinding
                {
                    .binding = binding.BindingNumber,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = static_cast<uint32>(texturePropertiesList.size()),
                    .stageFlags = VK_SHADER_STAGE_ALL,
                    .pImmutableSamplers = nullptr
                });
            break;
        }
        case kMaterialDescriptor:
        {
            descriptorSetLayoutBindingList.emplace_back(VkDescriptorSetLayoutBinding
                {
                    .binding = binding.BindingNumber,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = static_cast<uint32>(materialPropertiesList.size()),
                    .stageFlags = VK_SHADER_STAGE_ALL,
                    .pImmutableSamplers = nullptr
                });
            break;
        }
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

    Vector<VkDescriptorSetLayout> descriptorSetLayoutList = Vector<VkDescriptorSetLayout>(model.DescriptorSetLayoutCount);
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

    Vector<VkDescriptorSet> descriptorSetList = Vector<VkDescriptorSet>(model.DescriptorSetCount);
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
            switch (binding.BindingPropertiesList)
            {
            case kVertexDescsriptor:
            {
                writeDescriptorSet.emplace_back(VkWriteDescriptorSet
                    {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = descriptorSet,
                        .dstBinding = binding.BindingNumber,
                        .dstArrayElement = 0,
                        .descriptorCount = static_cast<uint32>(meshPropertiesList.size()),
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = meshPropertiesList.data(),
                        .pTexelBufferView = nullptr
                    });
                break;
            }
            case kIndexDescriptor:
            {
                writeDescriptorSet.emplace_back(VkWriteDescriptorSet
                    {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = descriptorSet,
                        .dstBinding = binding.BindingNumber,
                        .dstArrayElement = 0,
                        .descriptorCount = static_cast<uint32>(meshPropertiesList.size()),
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = meshPropertiesList.data(),
                        .pTexelBufferView = nullptr
                    });
                break;
            }
            case kTransformDescriptor:
            {
                writeDescriptorSet.emplace_back(VkWriteDescriptorSet
                    {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = descriptorSet,
                        .dstBinding = binding.BindingNumber,
                        .dstArrayElement = 0,
                        .descriptorCount = static_cast<uint32>(transformPropertiesList.size()),
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = transformPropertiesList.data(),
                        .pTexelBufferView = nullptr
                    });
                break;
            }
            case kMeshPropertiesDescriptor:
            {
                writeDescriptorSet.emplace_back(VkWriteDescriptorSet
                    {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = descriptorSet,
                        .dstBinding = binding.BindingNumber,
                        .dstArrayElement = 0,
                        .descriptorCount = static_cast<uint32>(meshPropertiesList.size()),
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = meshPropertiesList.data(),
                        .pTexelBufferView = nullptr
                    });
                break;
            }
            case kTextureDescriptor:
            {
                writeDescriptorSet.emplace_back(VkWriteDescriptorSet
                    {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = descriptorSet,
                        .dstBinding = binding.BindingNumber,
                        .dstArrayElement = 0,
                        .descriptorCount = static_cast<uint32>(texturePropertiesList.size()),
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .pImageInfo = texturePropertiesList.data(),
                        .pBufferInfo = nullptr,
                        .pTexelBufferView = nullptr
                    });

                break;
            }
            case kMaterialDescriptor:
            {
                writeDescriptorSet.emplace_back(VkWriteDescriptorSet
                    {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = descriptorSet,
                        .dstBinding = binding.BindingNumber,
                        .dstArrayElement = 0,
                        .descriptorCount = static_cast<uint32>(materialPropertiesList.size()),
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = materialPropertiesList.data(),
                        .pTexelBufferView = nullptr
                    });

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

VkPipelineLayout Pipeline_CreatePipelineLayout(VkDevice device, const Vector<VkDescriptorSetLayout>& descriptorSetLayoutList, uint constBufferSize)
{
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    Vector<VkPushConstantRange> pushConstantRangeList = Vector<VkPushConstantRange>();
    if (constBufferSize > 0)
    {
        pushConstantRangeList.emplace_back(VkPushConstantRange
            {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = constBufferSize
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

VkPipeline Pipeline_CreatePipeline(VkDevice device, VkRenderPass renderpass, VkPipelineLayout pipelineLayout, VkPipelineCache pipelineCache, const RenderPipelineLoader& model, ivec2& extent, Vector<VkPipelineShaderStageCreateInfo>& pipelineShaders)
{

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = VkPipelineVertexInputStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = static_cast<uint>(model.VertexInputBindingDescriptionList.size()),
        .pVertexBindingDescriptions = model.VertexInputBindingDescriptionList.data(),
        .vertexAttributeDescriptionCount = static_cast<uint>(model.VertexInputAttributeDescriptionList.size()),
        .pVertexAttributeDescriptions = model.VertexInputAttributeDescriptionList.data()
    };

    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfoModel = model.PipelineColorBlendStateCreateInfoModel;
    pipelineColorBlendStateCreateInfoModel.attachmentCount = model.PipelineColorBlendAttachmentStateList.size();
    pipelineColorBlendStateCreateInfoModel.pAttachments = model.PipelineColorBlendAttachmentStateList.data();

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

    Vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfoList = pipelineShaders;

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