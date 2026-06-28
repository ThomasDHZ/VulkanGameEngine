#pragma once
#include <VulkanSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT const uint  LevelEditorSystem_SampleRenderPassPixel(const TextureGuid& textureGuid, ivec2 mousePosition);
    DLL_EXPORT void        LevelEditorSystem_SetSelectedGameObject(uint gameObjectId);
#ifdef __cplusplus
}
#endif

