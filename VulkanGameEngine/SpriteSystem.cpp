#include "SpriteSystem.h"
#include "RenderSystem.h"
#include "BufferSystem.h"
#include "TextureSystem.h"
#include "GameObjectSystem.h"
#include "LevelSystem.h"
#include "MaterialSystem.h"
#include "MeshSystem.h"
#include <limits>
#include <algorithm>
#include <FileSystem.h>


SpriteSystem spriteSystem = SpriteSystem();

SpriteSystem::SpriteSystem()
{
    spriteContainerPtr = &spriteArchive;
    spriteContainerPtr->SpriteList.reserve(5);
    spriteContainerPtr->SpriteInstanceList.reserve(5);
    spriteContainerPtr->SpriteLayerList.reserve(5);
}

SpriteSystem::~SpriteSystem()
{

}

void SpriteSystem::UpdateSprites(const float& deltaTime)
{
    Sprite_UpdateSprites(deltaTime);
}

void SpriteSystem::UpdateSpriteBatchLayers(const float& deltaTime)
{
    for (auto& spriteLayer : spriteContainerPtr->SpriteLayerList)
    {
        Vector<SpriteInstance> spriteInstanceList = FindSpriteInstancesByLayer(spriteLayer.second);
        bufferSystem.UpdateBufferMemory(renderer, spriteLayer.second.SpriteLayerBufferId, spriteInstanceList);
    }
}

void SpriteSystem::AddSprite(GameObject& gameObject, VkGuid& spriteVramId)
{
    Sprite_AddSprite(gameObject, spriteVramId);
}

void SpriteSystem::Update(const float& deltaTime)
{
    Sprite_Update(renderer, deltaTime);
}

void SpriteSystem::SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum)
{
    Sprite_SetSpriteAnimation(*sprite, spriteAnimationEnum);
}

Sprite* SpriteSystem::FindSprite(uint gameObjectId)
{
    return Sprite_FindSprite(gameObjectId);
}

Vector<std::reference_wrapper<Sprite>> SpriteSystem::FindSpritesByLayer(const SpriteLayer& spriteLayer) 
{
    return Sprite_FindSpritesByLayer(spriteLayer);
}

const Vector<SpriteInstance> SpriteSystem::FindSpriteInstancesByLayer(const SpriteLayer& spriteLayer)
{
    return Sprite_FindSpriteInstancesByLayer(spriteLayer);
}

const SpriteVram& SpriteSystem::FindSpriteVram(VkGuid vramSpriteId)
{
    return Sprite_FindSpriteVram(vramSpriteId);
}

const Animation2D& SpriteSystem::FindSpriteAnimation(const VramSpriteGuid& vramId, const UM_AnimationListID& animationId)
{
    return Sprite_FindSpriteAnimation(vramId, animationId);
}

VkGuid SpriteSystem::LoadSpriteVRAM(const String& spriteVramPath)
{
    return Sprite_LoadSpriteVRAM(spriteVramPath);
}

void SpriteSystem::Destroy()
{
    Sprite_Destroy();
}