#pragma once
#include <VulkanSystem.h>
#include <RenderSystem.h>

typedef void (*LogVulkanMessageCallback)(const char* message, int severity);
#ifdef __cplusplus
extern "C"
{
#endif
    DLL_EXPORT RenderPassGuid                                          RenderSystem_LoadRenderPass(const char* jsonPath);     
    DLL_EXPORT void                                                    RenderSystem_Update(void* windowHandle, const float deltaTime);
    DLL_EXPORT VulkanRenderPass                                        RenderSystem_FindRenderPass(RenderPassGuid renderPassGuid);
    DLL_EXPORT void                                                    RenderSystem_RenderTest(float deltaTime);
#ifdef __cplusplus
}
#endif