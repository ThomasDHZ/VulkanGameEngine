#pragma once
#include <VulkanSystem.h>
#include <RenderSystem.h>

typedef void (*LogVulkanMessageCallback)(const char* message, int severity);
#ifdef __cplusplus
extern "C"
{
#endif
    DLL_EXPORT RenderPassGuid                                          RenderSystem_LoadRenderPass(LevelGuid& levelGuid, const char* jsonPath);     
    DLL_EXPORT void                                                    RenderSystem_Update(void* windowHandle, const float deltaTime);
    DLL_EXPORT VulkanRenderPass                                        RenderSystem_FindRenderPass(RenderPassGuid renderPassGuid);

    DLL_EXPORT void                                                    RenderSystem_Destroy();
    DLL_EXPORT void                                                    RenderSystem_DestroyRenderPass(VulkanRenderPass& renderPass);
    DLL_EXPORT void                                                    RenderSystem_DestroyRenderPasses();
    DLL_EXPORT void                                                    RenderSystem_DestroyRenderPipelines();
    DLL_EXPORT void                                                    RenderSystem_DestroyPipeline(VulkanPipeline& vulkanPipelineDLL);
    DLL_EXPORT void                                                    RenderSystem_DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList);
    DLL_EXPORT void                                                    RenderSystem_DestroyCommandBuffers(Vector<VkCommandBuffer>& commandBuffer);
    DLL_EXPORT void                                                    RenderSystem_DestroyBuffer(VkBuffer& buffer);
    DLL_EXPORT void                                                    RenderSystem_RenderTest(float deltaTime);
#ifdef __cplusplus
}
#endif