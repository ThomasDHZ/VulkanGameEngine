#pragma once
#include <RenderSystem.h>

#ifdef __cplusplus
extern "C"
{
#endif
    DLL_EXPORT void                                                    RenderSystem_StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface);
    DLL_EXPORT RenderPassGuid                                          RenderSystem_LoadRenderPass(LevelGuid& levelGuid, const char* jsonPath, bool useGlobalDescriptorSet);
    DLL_EXPORT RenderPassGuid                                          RenderSystem_LoadRenderPass(LevelGuid& levelGuid, RenderPassLoader& renderPassLoader, bool useGlobalDescriptorSet);
    DLL_EXPORT void                                                    RenderSystem_RebuildSwapChain(VulkanRenderPass& vulkanRenderPass);
    DLL_EXPORT void                                                    RenderSystem_Update(void* windowHandle, LevelGuid& levelGuid, const float& deltaTime);
    DLL_EXPORT VulkanRenderPass                                        RenderSystem_FindRenderPass(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT const Vector<VulkanPipeline>                            RenderSystem_FindRenderPipelineList(const RenderPassGuid& renderPassGuid);

    DLL_EXPORT void                                                    RenderSystem_Destroy();
    DLL_EXPORT void                                                    RenderSystem_DestroyRenderPass(VulkanRenderPass& renderPass);
    DLL_EXPORT void                                                    RenderSystem_DestroyRenderPasses();
    DLL_EXPORT void                                                    RenderSystem_DestroyRenderPipelines();
    DLL_EXPORT void                                                    RenderSystem_DestroyPipeline(VulkanPipeline& vulkanPipelineDLL);
    DLL_EXPORT void                                                    RenderSystem_DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList);
    DLL_EXPORT void                                                    RenderSystem_DestroyCommandBuffers(VkCommandBuffer& commandBuffer);
    DLL_EXPORT void                                                    RenderSystem_DestroyBuffer(VkBuffer& buffer);
#ifdef __cplusplus
}
#endif