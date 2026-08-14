#pragma once

#include <Platform.h>
#include "GameObjectSystem.h"
#include "Transform2DComponent.h"
#include "MeshSystem.h"
#include "MaterialSystem.h"

typedef uint32 SpriteLayerId;
typedef Vector<ivec2> AnimationFrames;

struct SpriteVram
{
    VkGuid VramSpriteID = VkGuid();
    VkGuid SpriteMaterialID = VkGuid();
    uint SpriteLayer = 0;
    vec4 SpriteColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    ivec2 SpritePixelSize = ivec2();
    vec2 SpriteScale = vec2(1.0f, 1.0f);
    ivec2 SpriteCells = ivec2(0, 0);
    vec2 SpriteUVSize = vec2();
    vec2 SpriteSize = vec2(50.0f);
    uint AnimationListID = 0;
};

//struct Animation2D
//{
//    uint  AnimationId;
//    float FrameHoldTime;
//};

struct Sprite
{
    entt::entity GameObjectId = entt::null;
    uint32 SpriteInstanceId = 0;
    uint32 CurrentAnimationId = 0;
    uint32 CurrentFrame = 0;
    uint32 SpriteLayer = 0;
    ivec2  FlipSprite = ivec2(0);
    VkGuid SpriteVramId = VkGuid();
    float  CurrentFrameTime = 0.0f;
};

struct Animation2D
{
    uint          AnimationId;
    Vector<ivec2> FrameList;
    float         FrameHoldTime;
};

enum TileColliderTypeEnum
{
    kTileColliderNone,
    kTileCollidable
};

struct RenderableTag {};

class SpriteSystem
{
public:
    static SpriteSystem& Get();

private:
    SpriteSystem() = default;
    ~SpriteSystem() = default;
    SpriteSystem(const SpriteSystem&) = delete;
    SpriteSystem operator=(const SpriteSystem&) = delete;
    SpriteSystem(SpriteSystem&&) = delete;
    SpriteSystem& operator=(SpriteSystem&&) = delete;

    struct SpritesToUpdate
    {
        entt::entity              entity;
        Sprite                    sprite;
        Transform2DComponent      transform2D;
    };

private:

    Vector<uint32>				                      FreeSpriteIndicesList;
    UnorderedMap<VramSpriteGuid, Vector<Animation2D>> SpriteAnimationMap;

    Vector<Animation2D>                               LoadSpriteAnimations(const nlohmann::json& json);
    void                                              AddSpriteBatchLayer();
    void                                              SortSpriteLayers();

public:
    uint32                                            SpriteMeshId;
    Vector<SpriteVram>                                SpriteVramList;
    Vector<SpriteLayer>                               SpriteLayerList;
    bool                                              SpriteListDirty = true;

     VramSpriteGuid                         LoadSpriteVRAM(const nlohmann::json& json);
     void                                   CreateSprite(entt::entity& gameObjectId, VkGuid& spriteVramId);
     void                                   Update(const float& deltaTime);
     void                                   SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum);
     SpriteVram&                            FindSpriteVram(VramSpriteGuid vramSpriteId);
     Animation2D&                           FindSpriteAnimation(const VramSpriteGuid& vramId, const AnimationListId& animationId);
     bool                                   SpriteVramExists(const VkGuid& vramId);
     void                                   Destroy(Sprite& sprite);
};
extern  SpriteSystem& spriteSystem;
inline SpriteSystem& SpriteSystem::Get()
{
    static SpriteSystem instance;
    return instance;
}