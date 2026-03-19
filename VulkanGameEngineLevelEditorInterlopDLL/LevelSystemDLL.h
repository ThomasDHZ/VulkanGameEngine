#pragma once
#include <LevelSystem.h>

#ifdef __cplusplus
extern "C"
{
#endif
    DLL_EXPORT void                            LevelSystem_LoadLevel(const char* levelPath);
    DLL_EXPORT void                            LevelSystem_Update(const float& deltaTime);
    DLL_EXPORT void                            LevelSystem_RenderIrradianceMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime);
    DLL_EXPORT void                            LevelSystem_RenderPrefilterMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime);
    DLL_EXPORT void                            LevelSystem_RenderGBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime);
    DLL_EXPORT void                            LevelSystem_RenderGaussianBlurPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, uint blurDirection);
    DLL_EXPORT void                            LevelSystem_RenderBloomPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
    DLL_EXPORT void                            LevelSystem_RenderHdrPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
    DLL_EXPORT void                            LevelSystem_RenderFrameBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
    DLL_EXPORT void                            LevelSystem_Draw(VkCommandBuffer& commandBuffer, const float& deltaTime);
#ifdef __cplusplus
}
#endif

