#pragma once
#include "ECSid.h"
#include "Sprite.h"
#include <VRAM.h>
#include "Transform2DComponent.h"
#include "RenderSystem.h"

static uint32 NextSpriteBatchLayerID;
struct SpriteBatchLayer
{
    VkGuid RenderPassId;
    uint SpriteBatchLayerID = 0;
    uint SpriteLayerMeshId = 0;
};

class SpriteSystem
{
private:
    Vector<Sprite>										    		SpriteList;
    Vector<SpriteInstanceStruct>                                    SpriteInstanceList;
    Vector<SpriteBatchLayer>                                        SpriteBatchLayerList;
    Vector<SpriteVram>                                              SpriteVramList;

    UnorderedMap<GameObjectID, size_t>                              SpriteIdToListIndexMap;
    UnorderedMap<UM_SpriteBatchID, int>                             SpriteInstanceBufferIdMap;
    UnorderedMap<VramSpriteGuid, Vector<Animation2D>>               SpriteAnimationMap;
    UnorderedMap<UM_SpriteBatchID, Vector<SpriteInstanceStruct>>    SpriteInstanceListMap;
    UnorderedMap<UM_SpriteBatchID, Vector<GameObjectID>>            SpriteBatchObjectListMap;

    void UpdateSprites(const float& deltaTime);
    void UpdateBatchSprites(const float& deltaTime);
    void UpdateSpriteBatchLayers(const float& deltaTime);

public:
   
    SpriteSystem();
    ~SpriteSystem();

    void AddSprite(GameObjectID gameObjectId, VkGuid& spriteVramId);
    void AddSpriteBatchLayer(RenderPassGuid& renderPassId);
    void AddSpriteInstanceBufferId(UM_SpriteBatchID spriteInstanceBufferId, int BufferId);
    void AddSpriteInstanceLayerList(UM_SpriteBatchID spriteBatchId, Vector<SpriteInstanceStruct>& spriteInstanceList);
    void AddSpriteBatchObjectList(UM_SpriteBatchID spriteBatchId, GameObjectID spriteBatchObject);
    VkGuid LoadSpriteVRAM(const String& spriteVramPath);

    void Update(const float& deltaTime);
    void Destroy();

    void SetSpriteAnimation(Sprite* sprite, Sprite::SpriteAnimationEnum spriteAnimation);

    Sprite* FindSprite(GameObjectID gameObjectId);
    const SpriteVram& FindSpriteVram(VkGuid VramSpriteID);
    const Animation2D& FindSpriteAnimation(const VramSpriteGuid& vramId, const UM_AnimationListID& animationId);
    const SpriteInstanceStruct* FindSpriteInstance(GameObjectID gameObjectId);

    const int FindSpriteInstanceBufferId(UM_SpriteBatchID spriteInstanceBufferId);
    Vector<SpriteInstanceStruct>& FindSpriteInstanceList(UM_SpriteBatchID spriteAnimation);
    const Vector<GameObjectID>& FindSpriteBatchObjectListMap(UM_SpriteBatchID spriteBatchObjectListId);
    Vector<SpriteBatchLayer> FindSpriteBatchLayer(RenderPassGuid& guid);

    const Vector<Sprite>& SpriteListRef() { return SpriteList; }
};
extern SpriteSystem spriteSystem;