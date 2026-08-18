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
    //DLL_EXPORT const VulkanRenderPass&      RenderSystem_FindRenderPass(const RenderPassGuid& renderPassGuid);
    //DLL_EXPORT const VulkanPipelinePackage& RenderSystem_FindPipelinePackage(const VkGuid& pipelinePackageGuid);
    //DLL_EXPORT const VulkanPipeline&        RenderSystem_FindRenderPipeline(const VkGuid& pipelineGuid);
    //DLL_EXPORT bool                         RenderSystem_FindPipelinePackageByPipelineType(const VkGuid& pipelinePackageGuid, PipelineType pipelineType);
    //DLL_EXPORT uint32                       RenderSystem_SampleRenderPassPixel(const TextureGuid& textureGuid, ivec2 mousePosition);

#ifdef __cplusplus
}
#endif