#include "renderSystem.h"
#include "json.h"
#include "TextureSystem.h"
#include <VulkanShader.h>
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
    RenderPassAttachementTextures renderPassAttachments;
    VulkanRenderPass vulkanRenderPass = VulkanRenderPass_CreateVulkanRenderPass(renderer, jsonPath.c_str(), renderPassAttachments, renderPassResolution);
    RenderPassMap[vulkanRenderPass.RenderPassId] = vulkanRenderPass;
    RenderPassLoaderJsonMap[vulkanRenderPass.RenderPassId] = jsonPath;

    Vector<Texture> renderTextureList(renderPassAttachments.RenderPassTexture, renderPassAttachments.RenderPassTexture + renderPassAttachments.RenderPassTextureCount);
    textureSystem.AddRenderedTexture(vulkanRenderPass.RenderPassId, renderTextureList);
    if (renderPassAttachments.DepthTexture != VK_NULL_HANDLE)
    {
        textureSystem.AddDepthTexture(vulkanRenderPass.RenderPassId, *renderPassAttachments.DepthTexture);
    }

    memorySystem.RemovePtrBuffer(renderPassAttachments.RenderPassTexture);
    memorySystem.RemovePtrBuffer(renderPassAttachments.DepthTexture);
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
        renderPass = VulkanRenderPass_RebuildSwapChain(renderer, renderPass, RenderPassLoaderJsonMap[renderPass.RenderPassId].c_str(), swapChainResolution, *renderedTextureList.data(), size, depthTexture);
    }
    ImGui_RebuildSwapChain(renderer, imGuiRenderer);
}

VkCommandBuffer RenderSystem::RenderBloomPass(VkGuid& renderPassId)
{
    const VulkanRenderPass renderPass = FindRenderPass(renderPassId);
    const VulkanPipeline& pipeline = FindRenderPipelineList(renderPassId)[0];
    VkCommandBuffer commandBuffer = renderPass.CommandBuffer;
    Texture blurTexture = textureSystem.FindRenderedTextureList(levelSystem.spriteRenderPass2DId)[0];
    ShaderPushConstant pushConstant = *shaderSystem.GetGlobalShaderPushConstant("bloomSettings");

    uint mipWidth = renderer.SwapChainResolution.width;
    uint mipHeight = renderer.SwapChainResolution.height;
    for (uint x = 0; x < blurTexture.mipMapLevels; x++)
    {
        VkDescriptorImageInfo imageInfo =
        {
            .sampler = blurTexture.textureSampler,
            .imageView = blurTexture.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = pipeline.DescriptorSetList[0],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo
        };
        vkUpdateDescriptorSets(renderer.Device, 1, &descriptorWrite, 0, nullptr);

        VkViewport viewport
        {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(mipWidth > 1 ? mipWidth : 1),
            .height = static_cast<float>(mipHeight > 1 ? mipHeight : 1),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        VkRect2D scissor = VkRect2D
        {
            .offset = VkOffset2D {.x = 0, .y = 0},
            .extent = {mipWidth > 1 ? mipWidth : 1, mipHeight > 1 ? mipHeight : 1}
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

        float blurStrength = 1.0f + x * 0.5f;
        float lodLevel = static_cast<float>(x);
        memcpy(shaderSystem.SearchGlobalShaderConstantVar(&pushConstant, "blurScale")->Value, &lodLevel, sizeof(lodLevel));
        memcpy(shaderSystem.SearchGlobalShaderConstantVar(&pushConstant, "blurStrength")->Value, &blurStrength, sizeof(blurStrength));

        VULKAN_RESULT(vkBeginCommandBuffer(commandBuffer, &CommandBufferBeginInfo));
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        vkCmdPushConstants(commandBuffer, pipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetCount, pipeline.DescriptorSetList, 0, nullptr);
        vkCmdDraw(commandBuffer, 6, 1, 0, 0);
        vkCmdEndRenderPass(commandBuffer);
        VULKAN_RESULT(vkEndCommandBuffer(commandBuffer));
    }
    return commandBuffer;
}


VkCommandBuffer RenderSystem::RenderFrameBuffer(VkGuid& renderPassId)
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
        .offset = VkOffset2D {.x = 0, .y = 0},
        .extent = VkExtent2D {.width = (uint32)renderer.SwapChainResolution.width, .height = (uint32)renderer.SwapChainResolution.height}
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
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetCount, pipeline.DescriptorSetList, 0, nullptr);
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    VULKAN_RESULT(vkEndCommandBuffer(commandBuffer));
    return commandBuffer;
}

VkCommandBuffer RenderSystem::RenderLevel(VkGuid& renderPassId, VkGuid& levelId, const float deltaTime)
{
    const VulkanRenderPass& renderPass = FindRenderPass(renderPassId);
    const VulkanPipeline& spritePipeline = FindRenderPipelineList(renderPassId)[0];
    const VulkanPipeline& levelPipeline = FindRenderPipelineList(renderPassId)[1];
    const Vector<SpriteBatchLayer>& spriteLayerList = spriteSystem.FindSpriteBatchLayer(renderPassId);
    const Vector<Mesh>& levelLayerList = meshSystem.FindLevelLayerMeshList(levelId);
    const VkCommandBuffer& commandBuffer = renderPass.CommandBuffer;
    ShaderPushConstant pushConstant = *shaderSystem.GetGlobalShaderPushConstant("sceneData");

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList[renderer.ImageIndex],
        .renderArea = renderPass.RenderArea,
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueCount),
        .pClearValues = renderPass.ClearValueList
    };

    VkDeviceSize offsets[] = { 0 };
    VULKAN_RESULT(vkBeginCommandBuffer(commandBuffer, &CommandBufferBeginInfo));
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    for (auto& levelLayer : levelLayerList)
    {
        const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshVertexBufferId).Buffer;
        const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshIndexBufferId).Buffer;

        uint meshIndex = 0;
        // memcpy(shaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "MeshBufferIndex")->Value, &meshIndex, sizeof(meshIndex));
        vkCmdPushConstants(commandBuffer, levelPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.Pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.PipelineLayout, 0, levelPipeline.DescriptorSetCount, levelPipeline.DescriptorSetList, 0, nullptr);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, levelLayer.IndexCount, 1, 0, 0, 0);
    }
    for (auto& spriteLayer : spriteLayerList)
    {
        const Mesh& spriteMesh = meshSystem.FindSpriteMesh(spriteLayer.SpriteLayerMeshId);
        const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshVertexBufferId).Buffer;
        const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshIndexBufferId).Buffer;
        const Vector<SpriteInstanceStruct>& spriteInstanceList = spriteSystem.FindSpriteInstanceList(spriteLayer.SpriteBatchLayerID);
        const VkBuffer& spriteInstanceBuffer = bufferSystem.FindVulkanBuffer(spriteSystem.FindSpriteInstanceBufferId(spriteLayer.SpriteBatchLayerID)).Buffer;

        // memcpy(shaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "MeshBufferIndex")->Value, &spriteLayer.SpriteLayerMeshId, sizeof(spriteLayer.SpriteLayerMeshId));
        vkCmdPushConstants(commandBuffer, spritePipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.Pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.PipelineLayout, 0, spritePipeline.DescriptorSetCount, spritePipeline.DescriptorSetList, 0, nullptr);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &spriteInstanceBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, gameObjectSystem.SpriteIndexList.size(), spriteInstanceList.size(), 0, 0, 0);
    }
    vkCmdEndRenderPass(commandBuffer);
    VULKAN_RESULT(vkEndCommandBuffer(commandBuffer));
    return commandBuffer;
}

VkGuid RenderSystem::LoadRenderPass(VkGuid& levelId, const String& jsonPath, ivec2 renderPassResolution)
{
    nlohmann::json renderPassJson = Json::ReadJson(jsonPath);
    const char* jsonDataString = File_Read(jsonPath.c_str()).Data;
    RenderPassLoader renderPassLoader = nlohmann::json::parse(jsonDataString).get<RenderPassLoader>();
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
    RenderPassMap[vulkanRenderPass.RenderPassId] = vulkanRenderPass;
    RenderPassLoaderJsonMap[vulkanRenderPass.RenderPassId] = jsonPath;

    Vector<Texture> renderTextureList(renderPassAttachments.RenderPassTexture, renderPassAttachments.RenderPassTexture + renderPassAttachments.RenderPassTextureCount);
    textureSystem.AddRenderedTexture(vulkanRenderPass.RenderPassId, renderTextureList);
    if (renderPassAttachments.DepthTexture != VK_NULL_HANDLE)
    {
        textureSystem.AddDepthTexture(vulkanRenderPass.RenderPassId, *renderPassAttachments.DepthTexture);
    }

    Vector<VkDescriptorBufferInfo> vertexPropertiesList = GetVertexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo> indexPropertiesList = GetIndexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo> transformPropertiesList = GetGameObjectTransformBuffer();
    Vector<VkDescriptorBufferInfo> meshPropertiesList = GetMeshPropertiesBuffer(levelId);
    Vector<VkDescriptorImageInfo> texturePropertiesList = GetTexturePropertiesBuffer(vulkanRenderPass.RenderPassId);
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

    for (int x = 0; x < renderPassJson["RenderPipelineList"].size(); x++)
    {

        VkSampleCountFlagBits sampleCountOverride = renderPassJson.at("RenderedTextureInfoModelList")[x]["SampleCountOverride"].get<VkSampleCountFlagBits>();

        nlohmann::json pipelineJson = Json::ReadJson(renderPassJson["RenderPipelineList"][x]);
        RenderPipelineLoader renderPipelineLoader = pipelineJson.get<RenderPipelineLoader>();
        renderPipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = sampleCountOverride > VK_SAMPLE_COUNT_1_BIT ? sampleCountOverride : VK_SAMPLE_COUNT_1_BIT;
        renderPipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = sampleCountOverride > VK_SAMPLE_COUNT_1_BIT ? true : false;
        renderPipelineLoader.RenderPassId = vulkanRenderPass.RenderPassId;
        renderPipelineLoader.RenderPass = RenderPassMap[vulkanRenderPass.RenderPassId].RenderPass;
        renderPipelineLoader.gpuIncludes = gpuIncludes;
        renderPipelineLoader.RenderPassResolution = renderPassResolution;
        renderPipelineLoader.ShaderPiplineInfo = shaderSystem.LoadShaderPipelineData(Vector<String> { pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] });

        RenderPipelineMap[vulkanRenderPass.RenderPassId].emplace_back(VulkanPipeline_CreateRenderPipeline(renderer.Device, renderPipelineLoader));
        memorySystem.RemovePtrBuffer(renderPipelineLoader.PipelineColorBlendAttachmentStateList);
        memorySystem.RemovePtrBuffer(renderPipelineLoader.ViewportList);
        memorySystem.RemovePtrBuffer(renderPipelineLoader.ScissorList);
    }
    memorySystem.RemovePtrBuffer(renderPassAttachments.RenderPassTexture);
    memorySystem.RemovePtrBuffer(renderPassAttachments.DepthTexture);

    return vulkanRenderPass.RenderPassId;
}

const VulkanRenderPass& RenderSystem::FindRenderPass(const RenderPassGuid& guid)
{
    return RenderPassMap.at(guid);
}

const Vector<VulkanPipeline>& RenderSystem::FindRenderPipelineList(const RenderPassGuid& guid)
{
    return RenderPipelineMap.at(guid);
}

void RenderSystem::DestroyRenderPasses()
{
    for (auto& renderPass : RenderPassMap)
    {
        VulkanRenderPass_DestroyRenderPass(renderer, renderPass.second);
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

VkCommandBuffer RenderSystem::BeginSingleTimeCommands()
{
    return Renderer_BeginSingleTimeCommands(renderSystem.renderer.Device, renderSystem.renderer.CommandPool);
}

VkCommandBuffer RenderSystem::BeginSingleTimeCommands(VkCommandPool& commandPool)
{
    return Renderer_BeginSingleTimeCommands(renderSystem.renderer.Device, renderSystem.renderer.CommandPool);
}

VkResult RenderSystem::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    return Renderer_EndSingleTimeCommands(renderSystem.renderer.Device, renderSystem.renderer.CommandPool, renderSystem.renderer.GraphicsQueue, commandBuffer);
}

VkResult RenderSystem::EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool& commandPool)
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

const Vector<VkDescriptorBufferInfo> RenderSystem::GetMeshPropertiesBuffer(const VkGuid& levelLayerId)
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


const Vector<VkDescriptorImageInfo> RenderSystem::GetTexturePropertiesBuffer(const VkGuid& renderPassId)
{
    Vector<Texture> textureList;
    const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
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
        if (vkCreateSampler(renderSystem.renderer.Device, &NullSamplerInfo, nullptr, &nullSampler))
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