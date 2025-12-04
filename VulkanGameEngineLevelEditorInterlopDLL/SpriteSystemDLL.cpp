#include "pch.h"
#include "SpriteSystemDLL.h"

void SpriteSystem_AddSprite(GameObject& gameObject, VramSpriteGuid& spriteVramId)
{
    return spriteSystem.AddSprite(gameObject, spriteVramId);
}

VramSpriteGuid SpriteSystem_LoadSpriteVRAM(const char* spriteVramPath)
{
    return spriteSystem.LoadSpriteVRAM(spriteVramPath);
}

void SpriteSystem_Update(float deltaTime)
{
    return spriteSystem.Update(deltaTime);
}

void SpriteSystem_SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum)
{
    return spriteSystem.SetSpriteAnimation(sprite, spriteAnimationEnum);
}

Sprite SpriteSystem_FindSprite(uint gameObjectId)
{
    return spriteSystem.FindSprite(gameObjectId);
}

SpriteVram& SpriteSystem_FindSpriteVram(VramSpriteGuid VramSpriteID)
{
    return spriteSystem.FindSpriteVram(VramSpriteID);
}

Animation2D& SpriteSystem_FindSpriteAnimation(const VramSpriteGuid& vramId, const AnimationListId& animationId)
{
    return spriteSystem.FindSpriteAnimation(vramId, animationId);
}

void SpriteSystem_Destroy()
{
    spriteSystem.Destroy();
}

SpriteInstance* SpriteSystem_FindSpriteInstancesByLayer(const SpriteLayer& spriteLayer, int& outCount)
{
    Vector<SpriteInstance> spriteInstanceList = spriteSystem.FindSpriteInstancesByLayer(spriteLayer);

    outCount = static_cast<int>(spriteInstanceList.size());
    return memorySystem.AddPtrBuffer<SpriteInstance>(spriteInstanceList.data(), spriteInstanceList.size(), __FILE__, __LINE__, __func__);
}

Sprite* SpriteSystem_FindSpritesByLayer(const SpriteLayer& spriteLayer, int& outCount)
{
    Vector<Sprite> spriteList = spriteSystem.FindSpritesByLayer(spriteLayer);

    outCount = static_cast<int>(spriteList.size());
    return memorySystem.AddPtrBuffer<Sprite>(spriteList.data(), spriteList.size(), __FILE__, __LINE__, __func__);
}