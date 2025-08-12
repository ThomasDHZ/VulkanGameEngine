#include "renderSystem.h"
#include "json.h"
#include "TextureSystem.h"
#include <ShaderCompiler.h>
#include "BufferSystem.h"
#include "MeshSystem.h"
#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "ShaderSystem.h"

RenderSystem renderSystem = RenderSystem();

RenderSystem::RenderSystem()
{

}

RenderSystem::~RenderSystem()
{

}

void RenderSystem::StartUp(WindowType windowType, void* windowHandle)
{
    renderer = Renderer_RendererSetUp(windowType, windowHandle);
    imGuiRenderer = ImGui_StartUp(renderer);
    shaderSystem.StartUp();
}

void RenderSystem::Update(VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime)
{
    if (renderer.RebuildRendererFlag)
    {
        RecreateSwapchain(spriteRenderPass2DId, levelId, deltaTime);
        renderer.RebuildRendererFlag = false;
    }
}

VkGuid RenderSystem::CreateVulkanRenderPass(const String& jsonPath, ivec2& renderPassResolution)
{
    Texture depthTexture = Texture();
    size_t renderedTextureCount = 1;
    Vector<Texture> renderedTextureList = Vector<Texture>(renderedTextureCount);

    VulkanRenderPass vulkanRenderPass = VulkanRenderPass_CreateVulkanRenderPass(renderer, jsonPath.c_str(), renderPassResolution, sizeof(SceneDataBuffer), renderedTextureList[0], renderedTextureCount, depthTexture);
    RenderPassMap[vulkanRenderPass.RenderPassId] = vulkanRenderPass;
    RenderPassLoaderJsonMap[vulkanRenderPass.RenderPassId] = jsonPath;

    textureSystem.AddRenderedTexture(vulkanRenderPass.RenderPassId, renderedTextureList);
    if (depthTexture.textureView != VK_NULL_HANDLE)
    {
        textureSystem.AddDepthTexture(vulkanRenderPass.RenderPassId, depthTexture);
    }

    return vulkanRenderPass.RenderPassId;
}

void RenderSystem::RecreateSwapchain(VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime)
{
    vkDeviceWaitIdle(renderer.Device);
    Renderer_RebuildSwapChain(vulkanWindow->WindowType, vulkanWindow->WindowHandle, renderer);
    for (auto& renderPassPair : RenderPassMap)
    {
        VulkanRenderPass& renderPass = renderPassPair.second;
        ivec2 swapChainResolution = ivec2(renderer.SwapChainResolution.width, renderer.SwapChainResolution.height);
        String renderPassJsonLoader = RenderPassLoaderJsonMap[renderPass.RenderPassId];
        Vector<Texture>& renderedTextureList = textureSystem.FindRenderedTextureList(renderPass.RenderPassId);

        Texture depthTexture = Texture();
        if (textureSystem.DepthTextureExists(renderPass.RenderPassId))
        {
            depthTexture = textureSystem.FindDepthTexture(renderPass.RenderPassId);
        }

        size_t size = renderedTextureList.size();
        renderPass = VulkanRenderPass_RebuildSwapChain(renderer, renderPass, RenderPassLoaderJsonMap[renderPass.RenderPassId].c_str(), swapChainResolution, renderedTextureList[0], size, depthTexture);
    }
    ImGui_RebuildSwapChain(renderer, imGuiRenderer);
}

VkCommandBuffer RenderSystem::RenderFrameBuffer(VkGuid& renderPassId, VkGuid& inputTextureRenderPassId)
{
    const VulkanRenderPass renderPass = FindRenderPass(renderPassId);
    const VulkanPipeline& pipeline = FindRenderPipelineList(renderPassId)[0];
     VkCommandBuffer commandBuffer = renderPass.CommandBuffer;
    Vector renderPassTexture = textureSystem.FindRenderedTextureList(levelSystem.spriteRenderPass2DId);

    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderer.SwapChainResolution.width),
        .height = static_cast<float>(renderer.SwapChainResolution.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = VkRect2D
    {
        .offset = VkOffset2D
        {
            .x = 0,
            .y = 0
        },
        .extent = VkExtent2D
        {
            .width = static_cast<uint32>(renderer.SwapChainResolution.width),
            .height = static_cast<uint32>(renderer.SwapChainResolution.height)
        }
    };


    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList[renderer.ImageIndex],
        .renderArea = scissor,
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueCount),
        .pClearValues = renderPass.ClearValueList
    };

    VULKAN_RESULT(vkBeginCommandBuffer(commandBuffer, &CommandBufferBeginInfo));
    textureSystem.UpdateTextureLayout(renderPassTexture[0], commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetCount, pipeline.DescriptorSetList, 0, nullptr);
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    vkEndCommandBuffer(commandBuffer);
    return commandBuffer;
}

VkCommandBuffer RenderSystem::RenderLevel(VkGuid& renderPassId, VkGuid& levelId, const float deltaTime, SceneDataBuffer& sceneDataBuffer)
{
    const VulkanRenderPass& renderPass = FindRenderPass(renderPassId);
    const VulkanPipeline& spritePipeline = FindRenderPipelineList(renderPassId)[0];
    const VulkanPipeline& levelPipeline = FindRenderPipelineList(renderPassId)[1];
    const Vector<SpriteBatchLayer>& spriteLayerList = spriteSystem.FindSpriteBatchLayer(renderPassId);
    const Vector<Mesh>& levelLayerList = meshSystem.FindLevelLayerMeshList(levelId);
    const VkCommandBuffer& commandBuffer = renderPass.CommandBuffer;

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList[renderer.ImageIndex],
        .renderArea = renderPass.RenderArea,
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueCount),
        .pClearValues = renderPass.ClearValueList
    };

    VULKAN_RESULT(vkBeginCommandBuffer(commandBuffer, &CommandBufferBeginInfo));
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    for (auto& levelLayer : levelLayerList)
    {
        const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshVertexBufferId).Buffer;
        const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshIndexBufferId).Buffer;

        VkDeviceSize offsets[] = { 0 };
        vkCmdPushConstants(commandBuffer, levelPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SceneDataBuffer), &sceneDataBuffer);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.Pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.PipelineLayout, 0, levelPipeline.DescriptorSetCount, levelPipeline.DescriptorSetList, 0, nullptr);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, levelLayer.IndexCount, 1, 0, 0, 0);
    }
    for (auto& spriteLayer : spriteLayerList)
    {
        const Vector<SpriteInstanceStruct>& spriteInstanceList = spriteSystem.FindSpriteInstanceList(spriteLayer.SpriteBatchLayerID);
        const Mesh& spriteMesh = meshSystem.FindSpriteMesh(spriteLayer.SpriteLayerMeshId);
        const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshVertexBufferId).Buffer;
        const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshIndexBufferId).Buffer;
        const VkBuffer& spriteInstanceBuffer = bufferSystem.FindVulkanBuffer(spriteSystem.FindSpriteInstanceBufferId(spriteLayer.SpriteBatchLayerID)).Buffer;

        VkDeviceSize offsets[] = { 0 };
        vkCmdPushConstants(commandBuffer, spritePipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SceneDataBuffer), &sceneDataBuffer);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.Pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.PipelineLayout, 0, spritePipeline.DescriptorSetCount, spritePipeline.DescriptorSetList, 0, nullptr);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
        vkCmdBindVertexBuffers(commandBuffer, 1, 1, &spriteInstanceBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, gameObjectSystem.SpriteIndexList.size(), spriteInstanceList.size(), 0, 0, 0);
    }
    vkCmdEndRenderPass(commandBuffer);
    vkEndCommandBuffer(commandBuffer);
    return commandBuffer;
}

VkGuid RenderSystem::LoadRenderPass(VkGuid& levelId, const String& jsonPath, ivec2 renderPassResolution)
{
    nlohmann::json json = Json::ReadJson(jsonPath);
    VkGuid renderPassId = CreateVulkanRenderPass(jsonPath, renderPassResolution);
    for (int x = 0; x < json["RenderPipelineList"].size(); x++)
    {
        String pipelineJson = json["RenderPipelineList"][x];

        Vector<VkDescriptorBufferInfo> vertexPropertiesList = renderSystem.GetVertexPropertiesBuffer();
        Vector<VkDescriptorBufferInfo> indexPropertiesList = renderSystem.GetIndexPropertiesBuffer();
        Vector<VkDescriptorBufferInfo> transformPropertiesList = renderSystem.GetGameObjectTransformBuffer();
        Vector<VkDescriptorBufferInfo> meshPropertiesList = renderSystem.GetMeshPropertiesBuffer(levelId);
        //  Vector<VkDescriptorBufferInfo> levelLayerMeshPropertiesList = Vector<VkDescriptorBufferInfo>(includes.LevelLayerMeshProperties, includes.LevelLayerMeshProperties + includes.LevelLayerMeshPropertiesCount);
        Vector<VkDescriptorImageInfo>  texturePropertiesList = renderSystem.GetTexturePropertiesBuffer(renderPassId);
        Vector<VkDescriptorBufferInfo> materialPropertiesList = materialSystem.GetMaterialPropertiesBuffer();
        GPUIncludes include =
        {
           .VertexProperties = vertexPropertiesList.data(),
           .IndexProperties = indexPropertiesList.data(),
           .TransformProperties = transformPropertiesList.data(),
           .MeshProperties = meshPropertiesList.data(),
           .TexturePropertiesList = texturePropertiesList.data(),
           .MaterialProperties = materialPropertiesList.data(),
           .VertexPropertiesCount = vertexPropertiesList.size(),
           .IndexPropertiesCount = indexPropertiesList.size(),
           .TransformPropertiesCount = transformPropertiesList.size(),
           .MeshPropertiesCount = meshPropertiesList.size(),
           .TexturePropertiesListCount = texturePropertiesList.size(),
           .MaterialPropertiesCount = materialPropertiesList.size()
        };

        VulkanPipeline vulkanPipelineDLL = VulkanPipeline_CreateRenderPipeline(renderer.Device, renderPassId, pipelineJson.c_str(), RenderPassMap[renderPassId].RenderPass, sizeof(SceneDataBuffer), renderPassResolution, include);
        RenderPipelineMap[renderPassId].emplace_back(vulkanPipelineDLL);
    }
    return renderPassId;
}

const Vector<VkDescriptorBufferInfo> RenderSystem::GetVertexPropertiesBuffer()
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

const Vector<VkDescriptorBufferInfo> RenderSystem::GetIndexPropertiesBuffer()
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

const Vector<VkDescriptorBufferInfo> RenderSystem::GetGameObjectTransformBuffer()
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

const Vector<VkDescriptorBufferInfo> RenderSystem::GetMeshPropertiesBuffer(VkGuid& levelLayerId)
{
    Vector<Mesh> meshList;
    if (levelLayerId == VkGuid())
    {
        for (auto& sprite : meshSystem.SpriteMeshList())
        {
            meshList.emplace_back(sprite);

        }
    }
    else
    {
        for (auto& layer : meshSystem.FindLevelLayerMeshList(levelLayerId))
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


const Vector<VkDescriptorImageInfo> RenderSystem::GetTexturePropertiesBuffer(VkGuid& renderPassId)
{
    Vector<Texture> textureList;
    const VulkanRenderPass& renderPass = RenderPassMap[renderPassId];
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

void RenderSystem::DestroyRenderPasses()
{
    for (auto& renderPass : RenderPassMap)
    {
       // VulkanRenderPass_DestroyRenderPass(renderer, renderPass.second, );
    }
    RenderPassMap.clear();
}

void RenderSystem::DestroyRenderPipelines()
{
    for (auto& renderPipelineList : RenderPipelineMap)
    {
        for (auto& renderPipeline : renderPipelineList.second)
        {
            VulkanPipeline_Destroy(renderer.Device, renderPipeline);
        }
    }
    RenderPipelineMap.clear();
}

const VulkanRenderPass& RenderSystem::FindRenderPass(const RenderPassGuid& guid)
{
    auto it = RenderPassMap.find(guid);
    if (it != RenderPassMap.end()) 
    {
        return it->second;
    }
    throw std::out_of_range("Render pass not found for given GUID");
}

const Vector<VulkanPipeline>& RenderSystem::FindRenderPipelineList(const RenderPassGuid& guid)
{
    auto it = RenderPipelineMap.find(guid);
    if (it != RenderPipelineMap.end())
    {
        return it->second;
    }
    throw std::out_of_range("Render Pipeline List not found for given GUID");
}

void RenderSystem::Destroy()
{
    ImGui_Destroy(renderer, imGuiRenderer);
    DestroyRenderPasses();
    DestroyRenderPipelines();
    Renderer_DestroyRenderer(renderer);
}

void RenderSystem::DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList)
{
    Renderer_DestroyFrameBuffers(renderSystem.renderer.Device, frameBufferList.data(), frameBufferList.size());
}

void RenderSystem::DestroyCommandBuffers(VkCommandBuffer& commandBuffer)
{
    Renderer_DestroyCommandBuffers(renderSystem.renderer.Device, &renderSystem.renderer.CommandPool, &commandBuffer, 1);
}

void RenderSystem::DestroyBuffer(VkBuffer& buffer)
{
    Renderer_DestroyBuffer(renderSystem.renderer.Device, &buffer);
}

VkCommandBuffer  RenderSystem::BeginSingleTimeCommands() 
{
    return Renderer_BeginSingleTimeCommands(renderSystem.renderer.Device, renderSystem.renderer.CommandPool);
}

VkCommandBuffer  RenderSystem::BeginSingleTimeCommands(VkCommandPool& commandPool) 
{
    return Renderer_BeginSingleTimeCommands(renderSystem.renderer.Device, renderSystem.renderer.CommandPool);
}

VkResult  RenderSystem::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    return Renderer_EndSingleTimeCommands(renderSystem.renderer.Device, renderSystem.renderer.CommandPool, renderSystem.renderer.GraphicsQueue, commandBuffer);
}

VkResult  RenderSystem::EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool& commandPool) 
{
    return Renderer_EndSingleTimeCommands(renderSystem.renderer.Device, commandPool, renderSystem.renderer.GraphicsQueue, commandBuffer);
}

VkResult RenderSystem::StartFrame()
{
    return Renderer_StartFrame(renderer.Device,
                               renderer.Swapchain,
                               renderer.InFlightFences,
                               renderer.AcquireImageSemaphores,
                               &renderer.ImageIndex,
                               &renderer.CommandIndex,
                               &renderer.RebuildRendererFlag);
}

VkResult RenderSystem::EndFrame(Vector<VkCommandBuffer> commandBufferSubmitList)
{
    return Renderer_EndFrame(renderSystem.renderer.Swapchain,
                             renderer.AcquireImageSemaphores,
                             renderer.PresentImageSemaphores,
                             renderer.InFlightFences,
                             renderer.GraphicsQueue,
                             renderer.PresentQueue,
                             renderer.CommandIndex,
                             renderer.ImageIndex,
                             commandBufferSubmitList.data(),
                             commandBufferSubmitList.size(),
                             &renderer.RebuildRendererFlag);
}