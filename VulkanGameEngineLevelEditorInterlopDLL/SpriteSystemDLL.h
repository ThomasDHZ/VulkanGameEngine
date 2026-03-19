#pragma once
#include <SpriteSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void                                   SpriteSystem_CreateSprite(uint32 gameObjectId, VkGuid spriteVramId);
    DLL_EXPORT VkGuid                                 SpriteSystem_LoadSpriteVRAM(const char* spriteVramPath);
    DLL_EXPORT void                                   SpriteSystem_Update(float deltaTime);
    DLL_EXPORT void                                   SpriteSystem_SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum);
    DLL_EXPORT SpriteVram                             SpriteSystem_FindSpriteVram(VkGuid vramSpriteId);
    DLL_EXPORT Animation2D                            SpriteSystem_FindSpriteAnimation(VkGuid vramId, AnimationListId animationId);
    DLL_EXPORT bool                                   SpriteSystem_SpriteVramExists(VkGuid vramId);
    DLL_EXPORT void                                   SpriteSystem_Destroy(Sprite* sprite);
#ifdef __cplusplus
}
#endif

