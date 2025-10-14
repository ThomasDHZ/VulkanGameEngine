#pragma once
#include "ECSid.h"
#include "Sprite.h"
#include <VRAM.h>
#include "Transform2DComponent.h"
#include "RenderSystem.h"


class SpriteSystem
{
private:
    SpriteArchive* spriteContainerPtr;

    void UpdateSprites(const float& deltaTime);
    void UpdateSpriteBatchLayers(const float& deltaTime);

public:
   
    SpriteSystem();
    ~SpriteSystem();

    void AddSprite(GameObject& gameObject, VkGuid& spriteVramId);
    void AddSpriteBatchLayer(RenderPassGuid& renderPassId);
    VkGuid LoadSpriteVRAM(const String& spriteVramPath);

    void Update(const float& deltaTime);
    void Destroy();

    void SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum);

    Sprite* FindSprite(uint gameObjectId);
    Vector<SpriteLayer> FindSpriteLayer(RenderPassGuid& guid);
    Vector<std::reference_wrapper<Sprite>> FindSpritesByLayer(const SpriteLayer& spriteLayer);
    
    const SpriteVram& FindSpriteVram(VkGuid VramSpriteID);
    const Vector<SpriteInstance> FindSpriteInstancesByLayer(const SpriteLayer& spriteLayer);
    const Animation2D& FindSpriteAnimation(const VramSpriteGuid& vramId, const UM_AnimationListID& animationId);
};
extern SpriteSystem spriteSystem;