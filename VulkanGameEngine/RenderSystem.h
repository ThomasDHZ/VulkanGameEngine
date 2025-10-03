#pragma once

#include <VulkanRenderer.h>
#include <VulkanRenderPass.h>
#include "ImGuiRenderer.h"
#include <nlohmann/json.hpp>
#include "ShaderSystem.h"
#include <VulkanPipeline.h>

class RenderSystem
{
    friend class JsonRenderPass;

private:
    UnorderedMap<RenderPassGuid, VulkanRenderPass>                RenderPassMap;
    UnorderedMap<RenderPassGuid, Vector<VulkanPipeline>>          RenderPipelineMap;
    UnorderedMap<RenderPassGuid, String>                          RenderPassLoaderJsonMap;
    VkCommandBufferBeginInfo                                      CommandBufferBeginInfo;

    void RecreateSwapchain(VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime);

    const Vector<VkDescriptorBufferInfo> GetVertexPropertiesBuffer();
    const Vector<VkDescriptorBufferInfo> GetIndexPropertiesBuffer();
    const Vector<VkDescriptorBufferInfo> GetGameObjectTransformBuffer();
    const Vector<VkDescriptorBufferInfo> GetMeshPropertiesBuffer(const VkGuid& levelLayerId);
    const Vector<VkDescriptorImageInfo>  GetTexturePropertiesBuffer(const VkGuid& renderPassId);

public:
    GraphicsRenderer                                              renderer;

    RenderSystem();
    ~RenderSystem();

    void DestroyRenderPasses();
    void DestroyRenderPipelines();
    void DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList);
    void DestroyCommandBuffers(VkCommandBuffer& commandBuffer);
    void DestroyBuffer(VkBuffer& buffer);

    void StartUp(WindowType windowType, void* windowHandle);
    void Update(VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime);

    VkCommandBuffer RenderBloomPass(VkGuid& renderPassId);
    VkCommandBuffer RenderFrameBuffer(VkGuid& renderPassId);
    VkCommandBuffer RenderLevel(VkGuid& renderPassId, VkGuid& levelId, const float deltaTime);

    VkGuid LoadRenderPass(VkGuid& levelId, const String& jsonPath, ivec2 renderPassResolution);

    const VulkanRenderPass& FindRenderPass(const RenderPassGuid& guid);
    const Vector<VulkanPipeline>& FindRenderPipelineList(const RenderPassGuid& guid);

    VkResult StartFrame();
    VkResult EndFrame(Vector<VkCommandBuffer> commandBufferSubmitList);
    void Destroy();

    static VkCommandBuffer BeginSingleTimeCommands();
    static VkCommandBuffer BeginSingleTimeCommands(VkCommandPool& commandPool);
    static VkResult EndSingleTimeCommands(VkCommandBuffer commandBuffer);
    static VkResult EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool& commandPool);
};
extern RenderSystem renderSystem;