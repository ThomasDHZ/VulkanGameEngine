#include "renderSystem.h"
#include "json.h"
#include "TextureSystem.h"
#include "ShaderSystem.h"
#include "BufferSystem.h"
#include "MeshSystem.h"
#include "GameObjectSystem.h"

RenderSystem renderSystem = RenderSystem();

RenderSystem::RenderSystem()
{

}

RenderSystem::~RenderSystem()
{

}

void RenderSystem::StartUp(void* windowHandle)
{
    renderer = Renderer_RendererSetUp(windowHandle);
    imGuiRenderer = ImGui_StartUp(renderer);
    shaderSystem.StartUp();
}

void RenderSystem::Update(const float& deltaTime)
{
    if (renderer.RebuildRendererFlag)
    {
        int width = renderer.SwapChainResolution.width;
        int height = renderer.SwapChainResolution.height;
        RecreateSwapchain();
        renderer.RebuildRendererFlag = false;
    }

    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    for (auto& renderPass : RenderPassMap)
    {
        if (levelSystem.SpriteBatchLayerListMap.find(renderPass.second.RenderPassId) != levelSystem.SpriteBatchLayerListMap.end())
        {
            for (auto& spriteLayer : levelSystem.SpriteBatchLayerListMap[renderPass.second.RenderPassId])
            {
                spriteLayer.Update(commandBuffer, deltaTime);
            }
        }
    }
    EndSingleTimeCommands(commandBuffer);
}

VkGuid RenderSystem::CreateVulkanRenderPass(const String& jsonPath, ivec2& renderPassResolution)
{
    Texture depthTexture = Texture();
    size_t renderedTextureCount = 1;
    Vector<Texture> renderedTextureList = Vector<Texture>(renderedTextureCount);

    VulkanRenderPass vulkanRenderPass = VulkanRenderPass_CreateVulkanRenderPass(renderer, jsonPath.c_str(), renderPassResolution, sizeof(SceneDataBuffer), renderedTextureList[0], renderedTextureCount, depthTexture);
    RenderPassMap[vulkanRenderPass.RenderPassId] = vulkanRenderPass;

    textureSystem.AddRenderedTexture(vulkanRenderPass.RenderPassId, renderedTextureList);
    if (depthTexture.textureView != VK_NULL_HANDLE)
    {
        textureSystem.AddDepthTexture(vulkanRenderPass.RenderPassId, depthTexture);
    }

    return vulkanRenderPass.RenderPassId;
}

void RenderSystem::RecreateSwapchain()
{
  /*  int width = 0;
    int height = 0;

    vkDeviceWaitIdle(*Device.get());

    vulkanWindow->GetFrameBufferSize(vulkanWindow, &width, &height);
    renderer.DestroySwapChainImageView();
    renderer.DestroySwapChain();
    renderer.SetUpSwapChain();

    RenderPassID id;
    id.id = 2;

    RenderPassList[id].RecreateSwapchain(width, height);*/
}

VkCommandBuffer RenderSystem::RenderFrameBuffer(VkGuid& renderPassId)
{
    const VulkanRenderPass renderPass = FindRenderPass(renderPassId);
    const VulkanPipeline& pipeline = FindRenderPipelineList(renderPassId)[0];
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

    VULKAN_RESULT(vkResetCommandBuffer(commandBuffer, 0));
    VULKAN_RESULT(vkBeginCommandBuffer(commandBuffer, &CommandBufferBeginInfo));
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
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
    const Vector<SpriteBatchLayer>& spriteLayerList = levelSystem.SpriteBatchLayerListMap[renderPassId];
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

    VULKAN_RESULT(vkResetCommandBuffer(commandBuffer, 0));
    VULKAN_RESULT(vkBeginCommandBuffer(commandBuffer, &CommandBufferBeginInfo));
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    for (auto levelLayer : levelLayerList)
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
    for (auto spriteLayer : spriteLayerList)
    {
        const Vector<SpriteInstanceStruct>& spriteInstanceList = levelSystem.SpriteInstanceListMap[spriteLayer.SpriteBatchLayerID];
        const Mesh& spriteMesh = meshSystem.FindSpriteMesh(spriteLayer.SpriteLayerMeshId);
        const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshVertexBufferId).Buffer;
        const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshIndexBufferId).Buffer;
        const VkBuffer& spriteInstanceBuffer = bufferSystem.FindVulkanBuffer(levelSystem.SpriteInstanceBufferMap[spriteLayer.SpriteBatchLayerID]).Buffer;

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
        uint pipeLineId = renderSystem.RenderPassMap.size();
        String asdf = json["RenderPipelineList"][x];
        nlohmann::json json = Json::ReadJson(asdf);
        RenderPipelineModel renderPipelineModel = RenderPipelineModel::from_json(json);
        GPUIncludes include =
        {
            .vertexProperties = renderSystem.GetVertexPropertiesBuffer(),
            .indexProperties = renderSystem.GetIndexPropertiesBuffer(),
            .transformProperties = renderSystem.GetGameObjectTransformBuffer(),
            .meshProperties = renderSystem.GetMeshPropertiesBuffer(levelId),
            .texturePropertiesList = renderSystem.GetTexturePropertiesBuffer(renderPassId, nullptr),
            .materialProperties = materialSystem.GetMaterialPropertiesBuffer()
        };

        Vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfoList = Vector<VkPipelineShaderStageCreateInfo>
        {
            shaderSystem.CreateShader(renderer.Device, renderPipelineModel.VertexShaderPath, VK_SHADER_STAGE_VERTEX_BIT),
            shaderSystem.CreateShader(renderer.Device, renderPipelineModel.FragmentShaderPath, VK_SHADER_STAGE_FRAGMENT_BIT)
        };

        VkPipelineShaderStageCreateInfo* createInfo  = pipelineShaderStageCreateInfoList.data();
        VulkanPipeline vulkanPipelineDLL = VulkanPipeline_CreateRenderPipeline(renderer.Device, renderPassId, pipeLineId, renderPipelineModel, RenderPassMap[renderPassId].RenderPass, sizeof(SceneDataBuffer), renderPassResolution, include, *createInfo, pipelineShaderStageCreateInfoList.size());
        RenderPipelineMap[renderPassId].emplace_back(vulkanPipelineDLL);
    }

    return renderPassId;
}

VkGuid RenderSystem::LoadRenderPass(VkGuid& levelId, const String& jsonPath, Texture& inputTexture, ivec2 renderPassResolution)
{
    nlohmann::json json = Json::ReadJson(jsonPath);
    VkGuid renderPassId = CreateVulkanRenderPass(jsonPath, renderPassResolution);
    for (int x = 0; x < json["RenderPipelineList"].size(); x++)
    {
        uint pipeLineId = renderSystem.RenderPipelineMap.size();
        String asdf = json["RenderPipelineList"][x];
        nlohmann::json json = Json::ReadJson(asdf);
        RenderPipelineModel renderPipelineModel = RenderPipelineModel::from_json(json);

        GPUIncludes include =
        {
            .vertexProperties = renderSystem.GetVertexPropertiesBuffer(),
            .indexProperties = renderSystem.GetIndexPropertiesBuffer(),
            .transformProperties = renderSystem.GetGameObjectTransformBuffer(),
            .meshProperties = renderSystem.GetMeshPropertiesBuffer(levelId),
            .texturePropertiesList = renderSystem.GetTexturePropertiesBuffer(renderPassId, &inputTexture),
            .materialProperties = materialSystem.GetMaterialPropertiesBuffer()
        };

        Vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfoList = Vector<VkPipelineShaderStageCreateInfo>
        {
            shaderSystem.CreateShader(renderer.Device, renderPipelineModel.VertexShaderPath, VK_SHADER_STAGE_VERTEX_BIT),
            shaderSystem.CreateShader(renderer.Device, renderPipelineModel.FragmentShaderPath, VK_SHADER_STAGE_FRAGMENT_BIT)
        };

        VkPipelineShaderStageCreateInfo* createInfo = pipelineShaderStageCreateInfoList.data();
        VulkanPipeline vulkanPipelineDLL = VulkanPipeline_CreateRenderPipeline(renderer.Device, renderPassId, pipeLineId, renderPipelineModel, RenderPassMap[renderPassId].RenderPass, sizeof(SceneDataBuffer), renderPassResolution, include, *createInfo, pipelineShaderStageCreateInfoList.size());
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


const Vector<VkDescriptorImageInfo> RenderSystem::GetTexturePropertiesBuffer(VkGuid& renderPassId, const Texture* renderedTexture)
{
    Vector<Texture> textureList;
    if (renderedTexture != nullptr)
    {
        textureList.emplace_back(*renderedTexture);
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

void RenderSystem::DestroyRenderPass()
{
    for (auto& renderPass : RenderPassMap)
    {
        VulkanRenderPass_DestroyRenderPass(renderer, renderPass.second);
    }
    RenderPassMap.clear();
}

void RenderSystem::DestroyRenderPipeline()
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
    DestroyRenderPass();
    DestroyRenderPipeline();
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
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = renderSystem.renderer.CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(renderSystem.renderer.Device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

VkCommandBuffer  RenderSystem::BeginSingleTimeCommands(VkCommandPool& commandPool) 
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(renderSystem.renderer.Device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

VkResult  RenderSystem::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkResult result = vkQueueSubmit(renderSystem.renderer.GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    result = vkQueueWaitIdle(renderSystem.renderer.GraphicsQueue);

    vkFreeCommandBuffers(renderSystem.renderer.Device, renderSystem.renderer.CommandPool, 1, &commandBuffer);

    return result;
}

VkResult  RenderSystem::EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool& commandPool) 
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkResult result = vkQueueSubmit(renderSystem.renderer.GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    result = vkQueueWaitIdle(renderSystem.renderer.GraphicsQueue);

    vkFreeCommandBuffers(renderSystem.renderer.Device, commandPool, 1, &commandBuffer);

    return result;
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