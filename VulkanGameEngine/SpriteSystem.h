#pragma once
#include "ECSid.h"
#include "Sprite.h"
#include <VRAM.h>
#include "Transform2DComponent.h"
#include "RenderSystem.h"

static uint32 NextSpriteId;
static uint32 NextSpriteLayerID;
struct SpriteLayer
{
    VkGuid RenderPassId;
    uint SpriteLayerId = 0;
    uint SpriteLayerMeshId = 0;
    uint SpriteLayerBufferId = 0;
};

class SpriteSystem
{
private:
    Vector<Sprite>										      SpriteList;
    Vector<SpriteInstance>                                    SpriteInstanceList;
    Vector<SpriteLayer>                                       SpriteLayerList;
    Vector<SpriteVram>                                        SpriteVramList;
    UnorderedMap<VramSpriteGuid, Vector<Animation2D>>         SpriteAnimationMap;

    void UpdateSprites(const float& deltaTime);
    void UpdateSpriteBatchLayers(const float& deltaTime);

public:
   
    SpriteSystem();
    ~SpriteSystem();

    void AddSprite(GameObjectID gameObjectId, VkGuid& spriteVramId);
    void AddSpriteBatchLayer(RenderPassGuid& renderPassId);
    VkGuid LoadSpriteVRAM(const String& spriteVramPath);

    void Update(const float& deltaTime);
    void Destroy();

    void SetSpriteAnimation(Sprite* sprite, Sprite::SpriteAnimationEnum spriteAnimation);

    Sprite* FindSprite(GameObjectID gameObjectId);
    Vector<SpriteLayer> FindSpriteLayer(RenderPassGuid& guid);
    Vector<std::reference_wrapper<Sprite>> FindSpritesByLayer(const SpriteLayer& spriteLayer);
    
    const SpriteVram& FindSpriteVram(VkGuid VramSpriteID);
    const Vector<SpriteInstance> FindSpriteInstancesByLayer(const SpriteLayer& spriteLayer);
    const Animation2D& FindSpriteAnimation(const VramSpriteGuid& vramId, const UM_AnimationListID& animationId);
};
extern SpriteSystem spriteSystem;