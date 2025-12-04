#pragma once
#include <SpriteSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void             SpriteSystem_AddSprite(GameObject& gameObject, VramSpriteGuid& spriteVramId);
    DLL_EXPORT VramSpriteGuid   SpriteSystem_LoadSpriteVRAM(const char* spriteVramPath);
    DLL_EXPORT void             SpriteSystem_Update(float deltaTime);
    DLL_EXPORT void             SpriteSystem_SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum);
    DLL_EXPORT Sprite           SpriteSystem_FindSprite(uint gameObjectId);
    DLL_EXPORT SpriteVram& SpriteSystem_FindSpriteVram(VramSpriteGuid VramSpriteID);
    DLL_EXPORT Animation2D& SpriteSystem_FindSpriteAnimation(const VramSpriteGuid& vramId, const AnimationListId& animationId);
    DLL_EXPORT void             SpriteSystem_Destroy();
    DLL_EXPORT SpriteInstance* SpriteSystem_FindSpriteInstancesByLayer(const SpriteLayer& spriteLayer, int& outCount);
    DLL_EXPORT Sprite* SpriteSystem_FindSpritesByLayer(const SpriteLayer& spriteLayer, int& outCount);
#ifdef __cplusplus
}
#endif

