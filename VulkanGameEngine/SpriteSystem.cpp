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
#include "VulkanFileSystem.h"


SpriteSystem spriteSystem = SpriteSystem();

SpriteSystem::SpriteSystem()
{
    spriteContainerPtr = &spriteArchive;
    spriteContainerPtr->SpriteList.reserve(10000);
    spriteContainerPtr->SpriteInstanceList.reserve(10000);
    spriteContainerPtr->SpriteLayerList.reserve(10000);
}

SpriteSystem::~SpriteSystem()
{

}

void SpriteSystem::UpdateSprites(const float& deltaTime)
{
    for (auto& sprite : spriteContainerPtr->SpriteList)
    {
        const auto& transform2D = gameObjectSystem.FindTransform2DComponent(sprite.GameObjectId);
        const auto& vram = FindSpriteVram(sprite.SpriteVramId);
        const auto& animation = FindSpriteAnimation(vram.VramSpriteID, sprite.CurrentAnimationID);
        const auto& material = materialSystem.FindMaterial(vram.SpriteMaterialID);
        const auto& currentFrame = animation.FrameList[sprite.CurrentFrame];
        spriteContainerPtr->SpriteInstanceList[sprite.SpriteInstance] = Sprite_UpdateSprites(transform2D, vram, animation, material, currentFrame, sprite, animation.FrameCount, deltaTime);
    }
}

void SpriteSystem::UpdateSpriteBatchLayers(const float& deltaTime)
{
    for (auto& spriteLayer : spriteContainerPtr->SpriteLayerList)
    {
        Vector<SpriteInstance> spriteInstanceList = FindSpriteInstancesByLayer(spriteLayer);
        bufferSystem.UpdateBufferMemory(renderSystem.renderer, spriteLayer.SpriteLayerBufferId, spriteInstanceList);
    }
}

void SpriteSystem::AddSprite(GameObjectID gameObjectId, VkGuid& spriteVramId)
{
    Sprite_AddSprite(gameObjectId, spriteVramId);
}

void SpriteSystem::AddSpriteBatchLayer(RenderPassGuid& renderPassId)
{
    Sprite_AddSpriteBatchLayer(renderSystem.renderer, renderPassId);
}

void SpriteSystem::Update(const float& deltaTime)
{
    Sprite_Update(renderSystem.renderer, deltaTime);
}

void SpriteSystem::SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum)
{
    Sprite_SetSpriteAnimation(*sprite, spriteAnimationEnum);
}

Sprite* SpriteSystem::FindSprite(GameObjectID gameObjectId)
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

Vector<SpriteLayer> SpriteSystem::FindSpriteLayer(RenderPassGuid& guid)
{
    return Sprite_FindSpriteLayer(guid);
}

VkGuid SpriteSystem::LoadSpriteVRAM(const String& spriteVramPath)
{
    return Sprite_LoadSpriteVRAM(spriteVramPath);
}

void SpriteSystem::Destroy()
{
    Sprite_Destroy();
}