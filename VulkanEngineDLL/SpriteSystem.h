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
typedef uint AnimationListId;
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
    UnorderedMap<VkGuid, Vector<Animation2D>>         SpriteAnimationMap;

    Vector<Animation2D>                               LoadSpriteAnimations(const nlohmann::json& json);
    void                                              AddSpriteBatchLayer();
    void                                              SortSpriteLayers();

public:
    uint32                                            SpriteMeshId;
    Vector<SpriteVram>                                SpriteVramList;
    Vector<SpriteLayer>                               SpriteLayerList;
    bool                                              SpriteListDirty = true;

    DLL_EXPORT VkGuid                                 LoadSpriteVRAM(const nlohmann::json& json);
    DLL_EXPORT void                                   CreateSprite(entt::entity& gameObjectId, VkGuid& spriteVramId);
    DLL_EXPORT void                                   Update(const float& deltaTime);
    DLL_EXPORT void                                   SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum);
    DLL_EXPORT SpriteVram&                            FindSpriteVram(VkGuid vramSpriteId);
    DLL_EXPORT Animation2D&                           FindSpriteAnimation(const VkGuid& vramId, const AnimationListId& animationId);
    DLL_EXPORT bool                                   SpriteVramExists(const VkGuid& vramId);
    DLL_EXPORT void                                   Destroy(Sprite& sprite);
};
extern DLL_EXPORT SpriteSystem& spriteSystem;
inline SpriteSystem& SpriteSystem::Get()
{
    static SpriteSystem instance;
    return instance;
}