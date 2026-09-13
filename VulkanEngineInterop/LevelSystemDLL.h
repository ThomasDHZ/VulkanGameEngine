#pragma once
#include "DLL.h"
#include <LevelSystem.h>
#include "ToDLL.h"

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void                           LevelSystem_LoadLevel(const char* levelPath);
    DLL_EXPORT void                           LevelSystem_Update(const float& deltaTime);
    DLL_EXPORT void                           LevelSystem_LevelEditorRenderPass(const char* levelPath);
    DLL_EXPORT RenderPassNodeDLL*             LevelSystem_CreateDrawCommands(VkCommandBuffer& commandBuffer, const float& deltaTime, size_t* renderPassNodeCount);
    DLL_EXPORT void                           LevelSystem_FreeDrawCommands(RenderPassNodeDLL* dllList, size_t renderPassNodeCount);
#ifdef __cplusplus
}
#endif