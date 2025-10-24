#include "RenderSystem.h"
#include "MaterialSystem.h"
#include "MeshSystem.h"
#include "BufferSystem.h"

RenderSystem renderSystem = RenderSystem();

void Renderer_StartUp(void* windowHandle)
{
    Renderer_RendererSetUp(windowHandle, renderer);
    shaderSystem.StartUp();
}

void Renderer_Update(void* windowHandle, VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime)
{
    if (renderer.RebuildRendererFlag)
    {
        Renderer_RecreateSwapChain(windowHandle, spriteRenderPass2DId, levelId, deltaTime);
        renderer.RebuildRendererFlag = false;
    }
}

VkGuid Renderer_LoadRenderPass(VkGuid& levelId, const String& jsonPath, ivec2 renderPassResolution)
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

    Vector<VkDescriptorBufferInfo> vertexPropertiesList = Renderer_GetVertexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo> indexPropertiesList = Renderer_GetIndexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo> transformPropertiesList = Renderer_GetGameObjectTransformBuffer();
    Vector<VkDescriptorBufferInfo> meshPropertiesList = Renderer_GetMeshPropertiesBuffer(levelId);
    Vector<VkDescriptorImageInfo> texturePropertiesList = Renderer_GetTexturePropertiesBuffer(renderPassLoader.RenderPassId);
    Vector<VkDescriptorBufferInfo> materialPropertiesList = materialSystem.GetMaterialPropertiesBuffer();

    GPUIncludes gpuIncludes =
    {
        .VertexPropertiesCount = vertexPropertiesList.size(),
        .IndexPropertiesCount = indexPropertiesList.size(),
        .TransformPropertiesCount = transformPropertiesList.size(),
        .MeshPropertiesCount = meshPropertiesList.size(),
        .TexturePropertiesCount = texturePropertiesList.size(),
        .MaterialPropertiesCount = materialPropertiesList.size(),
        .VertexProperties = vertexPropertiesList.data(),
        .IndexProperties = indexPropertiesList.data(),
        .TransformProperties = transformPropertiesList.data(),
        .MeshProperties = meshPropertiesList.data(),
        .TextureProperties = texturePropertiesList.data(),
        .MaterialProperties = materialPropertiesList.data()
    };

    for (int x = 0; x < renderPassLoader.RenderPipelineList.size(); x++)
    {
        nlohmann::json pipelineJson = fileSystem.LoadJsonFile(renderPassLoader.RenderPipelineList[x]);
        ShaderPipelineData shaderPiplineInfo = shaderSystem.LoadShaderPipelineData(Vector<String> { pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] });
        renderSystem.RenderPipelineMap[renderPassLoader.RenderPassId].emplace_back(VulkanPipeline_CreateRenderPipeline(renderer.Device, renderSystem.RenderPassMap[vulkanRenderPass.RenderPassId], renderPassLoader.RenderPipelineList[x].c_str(), gpuIncludes, shaderPiplineInfo));

        Span<const char*> shaderListPtr(shaderPiplineInfo.ShaderList, shaderPiplineInfo.ShaderCount);
        for (auto& shaderString : shaderListPtr)
        {
            memorySystem.RemovePtrBuffer(shaderString);
        }
        memorySystem.RemovePtrBuffer(shaderPiplineInfo.ShaderList);
    }
    memorySystem.RemovePtrBuffer(renderPassAttachments.RenderPassTexture);
    memorySystem.RemovePtrBuffer(renderPassAttachments.DepthTexture);
    return renderPassLoader.RenderPassId;
}

void Renderer_RecreateSwapChain(void* windowHandle, VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime)
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

 VulkanRenderPass& Renderer_FindRenderPass(const RenderPassGuid& guid)
{
    return renderSystem.RenderPassMap.at(guid);
}

 Vector<VulkanPipeline>& Renderer_FindRenderPipelineList(const RenderPassGuid& guid)
{
    return renderSystem.RenderPipelineMap.at(guid);
}

void Renderer_DestroyRenderPasses()
{
    for (auto& renderPass : renderSystem.RenderPassMap)
    {
        VulkanRenderPass_DestroyRenderPass(renderer, renderPass.second);
    }
    renderSystem.RenderPassMap.clear();
}

void Renderer_DestroyRenderPipelines()
{
    for (auto& renderPipelineList : renderSystem.RenderPipelineMap)
    {
        for (auto& renderPipeline : renderPipelineList.second)
        {
            VulkanPipeline_Destroy(renderer.Device, renderPipeline);
        }
    }
    renderSystem.RenderPipelineMap.clear();
}

void Renderer_Destroy()
{
    Renderer_DestroyRenderPasses();
    Renderer_DestroyRenderPipelines();
    Renderer_DestroyRenderer(renderer);
}

const Vector<VkDescriptorBufferInfo> Renderer_GetVertexPropertiesBuffer()
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
}

const Vector<VkDescriptorBufferInfo> Renderer_GetIndexPropertiesBuffer()
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
}

const Vector<VkDescriptorBufferInfo> Renderer_GetGameObjectTransformBuffer()
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
}

const Vector<VkDescriptorBufferInfo> Renderer_GetMeshPropertiesBuffer(const VkGuid& levelLayerId)
{
    Vector<Mesh> meshList;
    if (levelLayerId == VkGuid())
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
}


const Vector<VkDescriptorImageInfo> Renderer_GetTexturePropertiesBuffer(const VkGuid& renderPassId)
{
    Vector<Texture> textureList;
    const VulkanRenderPass& renderPass = Renderer_FindRenderPass(renderPassId);
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