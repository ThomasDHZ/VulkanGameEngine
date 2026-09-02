#pragma once
#include "DLL.h"
#include <RenderSystem.h>
#include "LevelSystemDLL.h"

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT RenderPassGuid               RenderSystem_LoadRenderPass(const char* jsonPath);
    DLL_EXPORT void                         RenderSystem_Update(void* windowHandle, const float& deltaTime);
    DLL_EXPORT void                         RenderSystem_Draw(VkCommandBuffer& commandBuffer, RenderPassNodeDLL* renderPassNodeListPtr, size_t renderPassNodeCount);
    DLL_EXPORT void                         RenderSystem_PresentToSwapChain(VkCommandBuffer& commandBuffer, const VkGuid* renderPassTextureGuid);
    DLL_EXPORT VkGuid*                      RenderSystem_FindRenderPassAttachmentList(const VkGuid& renderPassGuid, uint32* returnTextureCount);
    DLL_EXPORT uint32                       RenderSystem_SampleRenderPassPixel(const VkGuid& attachmentGuid, ivec2 mousePosition);
    DLL_EXPORT void                         RenderSystem_GetAttachmentSize(const VkGuid& attachmentGuid, int* outX, int* outY);
#ifdef __cplusplus
}
#endif