#pragma once
#include "ECSid.h"
#include "Sprite.h"
#include <VRAM.h>
#include "SpriteBatchLayer.h"
#include "Transform2DComponent.h"
#include <Sprite.h>

class SpriteSystem
{
private:
    Vector<SpriteInstanceStruct>                                                  SpriteInstanceList;
    Vector<SpriteBatchLayer>                                        SpriteBatchLayerList;
    Vector<SpriteVram>                                              SpriteVramList;

    UnorderedMap<GameObjectID, size_t>                              SpriteIdToListIndexMap;
    UnorderedMap<UM_SpriteBatchID, int>                               SpriteInstanceBufferIdMap;
    UnorderedMap<UM_AnimationListID, Animation2D>                     SpriteAnimationMap;
    UnorderedMap<VkGuid, Vector<AnimationFrames>>                     SpriteAnimationFrameListMap;
    UnorderedMap<UM_SpriteBatchID, Vector<SpriteInstanceStruct>>    SpriteInstanceListMap;
    UnorderedMap<UM_SpriteBatchID, Vector<GameObjectID>>            SpriteBatchObjectListMap;

    void UpdateBatchSprites(const float& deltaTime);
    void UpdateSprites(const float& deltaTime);
    void UpdateSpriteBatchLayers(const float& deltaTime);

public:
    Vector<Sprite>												  SpriteList;
   
    SpriteSystem();
    ~SpriteSystem();

    void AddSprite(GameObjectID gameObjectId, VkGuid& spriteVramId);
    void AddSpriteBatchLayer(RenderPassGuid& renderPassId);
    void AddSpriteInstanceBufferId(UM_SpriteBatchID spriteInstanceBufferId, int BufferId);
    void AddSpriteInstanceLayerList(UM_SpriteBatchID spriteBatchId, Vector<SpriteInstanceStruct>& spriteInstanceList);
    void AddSpriteBatchObjectList(UM_SpriteBatchID spriteBatchId, GameObjectID spriteBatchObject);

    void Update(const float& deltaTime);

    void SetSpriteAnimation(Sprite& sprite, Sprite::SpriteAnimationEnum spriteAnimation);

    Sprite* FindSprite(GameObjectID gameObjectId);
    const SpriteVram& FindVramSprite(VkGuid VramSpriteID);
    const Animation2D& FindSpriteAnimation(const UM_AnimationListID& animationId);
    Vector<AnimationFrames>& FindSpriteAnimationFrames(const VkGuid& vramSpriteId);
    const SpriteInstanceStruct* FindSpriteInstance(GameObjectID gameObjectId);
    const int FindSpriteInstanceBufferId(UM_SpriteBatchID spriteInstanceBufferId);
    Vector<SpriteInstanceStruct>& FindSpriteInstanceList(UM_SpriteBatchID spriteAnimation);
    const Vector<GameObjectID>& FindSpriteBatchObjectListMap(UM_SpriteBatchID spriteBatchObjectListId);

    Vector<SpriteBatchLayer> FindSpriteBatchLayer(RenderPassGuid& guid);
    size_t FindSpriteIndex(GameObjectID gameObjectId);
    VkGuid LoadSpriteVRAM(const String& spriteVramPath);

    const Vector<Sprite>& SpriteListRef() { return SpriteList; }
};
extern SpriteSystem spriteSystem;