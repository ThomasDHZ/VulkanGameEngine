#include "SpriteSystemDLL.h"

void SpriteSystem_CreateSprite(uint32 gameObjectId, VkGuid spriteVramId)
{
	spriteSystem.CreateSprite(gameObjectId, spriteVramId);
}

VkGuid SpriteSystem_LoadSpriteVRAM(const char* spriteVramPath)
{
	return spriteSystem.LoadSpriteVRAM(spriteVramPath);
}

void SpriteSystem_Update(float deltaTime)
{
	spriteSystem.Update(deltaTime);
}

void SpriteSystem_SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum)
{
	spriteSystem.SetSpriteAnimation(sprite, spriteAnimationEnum);
}

SpriteVram SpriteSystem_FindSpriteVram(VkGuid vramSpriteId)
{
	return spriteSystem.FindSpriteVram(vramSpriteId);
}

Animation2D SpriteSystem_FindSpriteAnimation(VkGuid vramId, AnimationListId animationId)
{
	return spriteSystem.FindSpriteAnimation(vramId, animationId);
}

bool SpriteSystem_SpriteVramExists(VkGuid vramId)
{
	return spriteSystem.SpriteVramExists(vramId);
}

void SpriteSystem_Destroy(Sprite* sprite)
{
	spriteSystem.Destroy(*sprite);
}
