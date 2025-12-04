#pragma once
#include "Platform.h"
#include "VulkanRenderer.h"
#include "VulkanRenderPass.h"
#include "VulkanPipeline.h"

class RenderSystem
{
    friend class JsonRenderPass;
private:
    UnorderedMap<RenderPassGuid, VulkanRenderPass>                RenderPassMap;
    UnorderedMap<RenderPassGuid, Vector<VulkanPipeline>>          RenderPipelineMap;
    UnorderedMap<RenderPassGuid, String>                          RenderPassLoaderJsonMap;

    void RecreateSwapchain(void* windowHandle, RenderPassGuid& renderPassGuid, LevelGuid& levelGuid, const float& deltaTime);


public:
    VkCommandBufferBeginInfo                                      CommandBufferBeginInfo;

    RenderSystem();
    ~RenderSystem();

    DLL_EXPORT GraphicsRenderer              StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface);
    DLL_EXPORT RenderPassGuid                LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath, ivec2 renderPassResolution);
    DLL_EXPORT void                          Update(void* windowHandle, RenderPassGuid& spriteRenderPass2DGuid, LevelGuid& levelGuid, const float& deltaTime);
    DLL_EXPORT VkCommandBuffer               BeginSingleUseCommand();
    DLL_EXPORT VkCommandBuffer               BeginSingleUseCommand(VkCommandPool& commandPool);
    DLL_EXPORT void                          EndSingleUseCommand(VkCommandBuffer commandBuffer);
    DLL_EXPORT void                          EndSingleUseCommand(VkCommandBuffer commandBuffer, VkCommandPool& commandPool);
    DLL_EXPORT void                          StartFrame();
    DLL_EXPORT void                          EndFrame(Vector<VkCommandBuffer> commandBufferSubmitList);
    DLL_EXPORT const VulkanRenderPass        FindRenderPass(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT const Vector<VulkanPipeline>  FindRenderPipelineList(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT void                          Destroy();
    DLL_EXPORT void                          DestroyRenderPasses();
    DLL_EXPORT void                          DestroyRenderPipelines();
    DLL_EXPORT void                          DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList);
    DLL_EXPORT void                          DestroyCommandBuffers(VkCommandBuffer& commandBuffer);
    DLL_EXPORT void                          DestroyBuffer(VkBuffer& buffer);

    Vector<VkDescriptorBufferInfo>    GetVertexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo>    GetIndexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo>    GetGameObjectTransformBuffer();
    Vector<VkDescriptorBufferInfo>    GetMeshPropertiesBuffer(const  LevelGuid& levelGuid);
    Vector<VkDescriptorImageInfo>     GetTexturePropertiesBuffer(const RenderPassGuid& renderPassGuid);
};
extern DLL_EXPORT RenderSystem renderSystem;