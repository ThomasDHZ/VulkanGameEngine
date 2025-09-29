#pragma once
#include "ECSid.h"
#include "Sprite.h"
#include <VRAM.h>
#include "Transform2DComponent.h"
#include "RenderSystem.h"

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

    UnorderedMap<GameObjectID, size_t>                        SpriteIdToListIndexMap;
    UnorderedMap<VramSpriteGuid, Vector<Animation2D>>         SpriteAnimationMap;
    UnorderedMap<UM_SpriteBatchID, Vector<SpriteInstance>>    SpriteInstanceListMap;
    UnorderedMap<UM_SpriteBatchID, Vector<GameObjectID>>      SpriteBatchObjectListMap;

    void UpdateSprites(const float& deltaTime);
    void UpdateBatchSprites(const float& deltaTime);
    void UpdateSpriteBatchLayers(const float& deltaTime);

    const Vector<GameObjectID>& FindSpriteBatchObjectListMap(UM_SpriteBatchID spriteBatchObjectListId);
public:
   
    SpriteSystem();
    ~SpriteSystem();

    void AddSprite(GameObjectID gameObjectId, VkGuid& spriteVramId);
    void AddSpriteBatchLayer(RenderPassGuid& renderPassId);
    void AddSpriteInstanceLayerList(UM_SpriteBatchID spriteBatchId, Vector<SpriteInstance>& spriteInstanceList);
    void AddSpriteBatchObjectList(UM_SpriteBatchID spriteBatchId, GameObjectID spriteBatchObject);
    VkGuid LoadSpriteVRAM(const String& spriteVramPath);

    void Update(const float& deltaTime);
    void Destroy();

    void SetSpriteAnimation(Sprite* sprite, Sprite::SpriteAnimationEnum spriteAnimation);

    Sprite* FindSprite(GameObjectID gameObjectId);
    const SpriteVram& FindSpriteVram(VkGuid VramSpriteID);
    const Animation2D& FindSpriteAnimation(const VramSpriteGuid& vramId, const UM_AnimationListID& animationId);
    const SpriteInstance* FindSpriteInstance(GameObjectID gameObjectId);

    Vector<SpriteInstance>& FindSpriteInstanceList(UM_SpriteBatchID spriteAnimation);
    Vector<SpriteLayer> FindSpriteLayer(RenderPassGuid& guid);

    const Vector<Sprite>& SpriteListRef() { return SpriteList; }
};
extern SpriteSystem spriteSystem;