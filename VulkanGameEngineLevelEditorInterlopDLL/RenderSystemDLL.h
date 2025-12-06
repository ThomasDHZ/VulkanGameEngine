#pragma once
#include <RenderSystem.h>

#ifdef __cplusplus
extern "C"
{
#endif
    DLL_EXPORT GraphicsRenderer       RenderSystem_StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface, VkDebugUtilsMessengerEXT& debugMessenger);
    DLL_EXPORT void                   RenderSystem_Update(void* windowHandle, VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime);
    DLL_EXPORT RenderPassGuid         RenderSystem_LoadRenderPass(VkGuid& levelGuid, const String& jsonPath, ivec2 renderPassResolution);
    DLL_EXPORT void                   RenderSystem_RecreateSwapChain(void* windowHandle, VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime);
    DLL_EXPORT VulkanRenderPass       RenderSystem_FindRenderPass(const RenderPassGuid& guid);
    //DLL_EXPORT Vector<VulkanPipeline> RenderSystem_FindRenderPipelineList(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT void                   RenderSystem_StartFrame();
    DLL_EXPORT void                   RenderSystem_EndFrame(VkCommandBuffer* commandBufferListPtr, size_t commandBufferCount);
    DLL_EXPORT void                   RenderSystem_DestroyRenderPasses();
    DLL_EXPORT void                   RenderSystem_DestroyRenderPipelines();
    DLL_EXPORT void                   RenderSystem_Destroy();
#ifdef __cplusplus
}
#endif