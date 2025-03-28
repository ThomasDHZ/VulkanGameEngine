#include "JsonRenderPass.h"
#include "VulkanRenderPass.h"

JsonRenderPass::JsonRenderPass()
{
}

JsonRenderPass::JsonRenderPass(const String& jsonPath, GPUImport renderGraphics, ivec2 renderPassResolution, SceneDataBuffer& sceneDataBuffer)
{
    RenderPassResolution = renderPassResolution;
    SampleCount = VK_SAMPLE_COUNT_1_BIT;

    FrameBufferList.resize(cRenderer.SwapChain.SwapChainImageCount);
    VULKAN_RESULT(renderer.CreateCommandBuffer(CommandBuffer));

    nlohmann::json json= Json::ReadJson(jsonPath);
    RenderPassBuildInfoModel renderPassBuildInfo = RenderPassBuildInfoModel::from_json(json, RenderPassResolution);

    BuildRenderPass(renderPassBuildInfo);
    BuildFrameBuffer(renderPassBuildInfo);
    BuildRenderPipelines(renderPassBuildInfo, renderGraphics, sceneDataBuffer);
}

JsonRenderPass::JsonRenderPass(const String& jsonPath, GPUImport renderGraphics, VkExtent2D renderPassResolution, SceneDataBuffer& sceneDataBuffer)
{
    RenderPassResolution = ivec2(renderPassResolution.width, renderPassResolution.height);
    SampleCount = VK_SAMPLE_COUNT_1_BIT;

    FrameBufferList.resize(cRenderer.SwapChain.SwapChainImageCount);
    VULKAN_RESULT(renderer.CreateCommandBuffer(CommandBuffer));

    nlohmann::json json = Json::ReadJson(jsonPath);
    RenderPassBuildInfoModel renderPassBuildInfo = RenderPassBuildInfoModel::from_json(json, RenderPassResolution);

    BuildRenderPass(renderPassBuildInfo);
    BuildFrameBuffer(renderPassBuildInfo);
    BuildRenderPipelines(renderPassBuildInfo, renderGraphics, sceneDataBuffer);
}


JsonRenderPass::~JsonRenderPass()
{
}

void JsonRenderPass::Update(const float& deltaTime)
{
}

void JsonRenderPass::BuildRenderPipelines(const RenderPassBuildInfoModel& renderPassBuildInfo, GPUImport& renderGraphics, SceneDataBuffer& sceneDataBuffer)
{
    for (int x = 0; x < renderPassBuildInfo.RenderPipelineList.size(); x++)
    {
        Vector<VkVertexInputBindingDescription> vertexBinding = NullVertex::GetBindingDescriptions();
        Vector<VkVertexInputAttributeDescription> vertexAttribute = NullVertex::GetAttributeDescriptions();
        JsonPipelineList.emplace_back(std::make_shared<JsonPipeline>(JsonPipeline(renderPassBuildInfo.RenderPipelineList[x], RenderPass, renderGraphics, vertexBinding, vertexAttribute, sizeof(SceneDataBuffer))));
    }
}

void JsonRenderPass::BuildRenderPass(const RenderPassBuildInfoModel& renderPassBuildInfo)
{
    VkRenderPass& renderPass = RenderPass;
    Vector<SharedPtr<RenderedTexture>>& renderedColorTextureList = RenderedColorTextureList;
    RenderPass_BuildRenderPass(cRenderer.Device, renderPass, renderPassBuildInfo, renderedColorTextureList, depthTexture);
}

void JsonRenderPass::BuildFrameBuffer(const RenderPassBuildInfoModel& renderPassBuildInfo)
{
    Vector<VkImageView> imageViewList;
    for (int x = 0; x < RenderedColorTextureList.size(); x++)
    {
        imageViewList.emplace_back(RenderedColorTextureList[x]->View);
    }

    SharedPtr<VkImageView> depthTextureView = nullptr;
    if (depthTexture)
    {
        depthTextureView = std::make_shared<VkImageView>(depthTexture->View);
    }

    VkRenderPass& renderPass = RenderPass;
    Vector<VkFramebuffer>& frameBufferList = FrameBufferList;
    RenderPass_BuildFrameBuffer(cRenderer.Device, renderPass, renderPassBuildInfo, frameBufferList, imageViewList, depthTextureView, cRenderer.SwapChain.SwapChainImageViews, RenderPassResolution);
}

VkCommandBuffer JsonRenderPass::Draw(Vector<SharedPtr<GameObject>> meshList, SceneDataBuffer& sceneDataBuffer)
{
    std::vector<VkClearValue> clearValues
    {
        VkClearValue{.color = { {0.0f, 0.0f, 0.0f, 1.0f} } },
        VkClearValue{.depthStencil = { 1.0f, 0 } }
    };

    VkRenderPassBeginInfo renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = RenderPass,
        .framebuffer = FrameBufferList[cRenderer.ImageIndex],
        .renderArea
        {
            .offset = {0, 0},
            .extent =
            {
                .width = static_cast<uint32>(RenderPassResolution.x),
                .height = static_cast<uint32>(RenderPassResolution.y)
            }
        },
        .clearValueCount = static_cast<uint32>(clearValues.size()),
        .pClearValues = clearValues.data()
    };

    VkViewport viewport = VkViewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(RenderPassResolution.x),
        .height = static_cast<float>(RenderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = VkRect2D
    {
        .offset = VkOffset2D(0, 0),
        .extent = VkExtent2D
        {
          .width = static_cast<uint32>(RenderPassResolution.x),
          .height = static_cast<uint32>(RenderPassResolution.y)
        }
    };

    VkCommandBufferBeginInfo CommandBufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
    };

    VULKAN_RESULT(vkResetCommandBuffer(CommandBuffer, 0));
    VULKAN_RESULT(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo));
    vkCmdBeginRenderPass(CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(CommandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(CommandBuffer, 0, 1, &scissor);
    for (auto mesh : meshList)
    {
        mesh->Draw(CommandBuffer, JsonPipelineList[0]->Pipeline, JsonPipelineList[0]->PipelineLayout, JsonPipelineList[0]->DescriptorSetList[0]);
    }
    vkCmdEndRenderPass(CommandBuffer);
    vkEndCommandBuffer(CommandBuffer);
    return CommandBuffer;
}

void JsonRenderPass::Destroy()
{
    for (auto renderedTexture : RenderedColorTextureList)
    {
        renderedTexture->Destroy();
    }
    for (auto pipeline : JsonPipelineList)
    {
        pipeline->Destroy();
    }
    depthTexture->Destroy();
    renderer.DestroyRenderPass(RenderPass);
    renderer.DestroyCommandBuffers(CommandBuffer);
    renderer.DestroyFrameBuffers(FrameBufferList);
    FrameBufferList.clear();
}