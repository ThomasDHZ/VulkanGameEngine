#pragma once
#include "pch.h"
#include "GameObjectSystem.h"
#include "Transform2DComponent.h"
#include <MemoryPoolSystem.h>

typedef uint32 SpriteLayerId;
typedef Vector<ivec2> AnimationFrames;

struct SpriteLayer
{
    uint32 InstanceCount = 0;
    uint32 StartInstanceIndex = 0;
    uint32 SpriteDrawLayer = UINT32_MAX;
};

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

struct Sprite
{
    uint32 GameObjectId = UINT32_MAX;
    uint32 SpriteInstanceId = 0;
    uint32 CurrentAnimationId = 0;
    uint32 CurrentFrame = 0;
    uint32 SpriteLayer = 0;
    ivec2  FlipSprite = ivec2(0);
    VkGuid SpriteVramId = VkGuid();
    float  CurrentFrameTime = 0.0f;
};

struct SpriteComponent 
{
    VkGuid spriteVramId;
    uint32 currentAnimationId = 0;
    uint32 currentFrame = 0;
    float frameTimeAccumulator = 0.0f;
    bool flipX = false;
    bool flipY = false;
    int layer = 0;
    glm::vec4 tint{ 1.0f };
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
        GameObjectComponentLinker gameObject;
        Sprite                    sprite;
        Transform2DComponent      transform2D;
    };

private:

    Vector<uint32>				                      FreeSpriteIndicesList;
    UnorderedMap<VramSpriteGuid, Vector<Animation2D>> SpriteAnimationMap;

    SpriteVram                                        LoadSpriteVRAM(const char* spritePath, const Material& material, const Texture& texture);
    Vector<Animation2D>                               LoadSpriteAnimations(const char* spritePath);
    void                                              AddSpriteBatchLayer();
    void                                              SortSpriteLayers();

public:
    uint32                                            SpriteMeshId;
    Vector<SpriteVram>                                SpriteVramList;
    Vector<SpriteLayer>                               SpriteLayerList;
    bool                                              SpriteListDirty = true;

    DLL_EXPORT void                                   CreateSprite(GameObject& gameObject, VkGuid& spriteVramId);
    DLL_EXPORT VkGuid                                 LoadSpriteVRAM(const String& spriteVramPath);
    DLL_EXPORT void                                   Update(const float& deltaTime);
    DLL_EXPORT void                                   SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum);
    DLL_EXPORT SpriteVram&                            FindSpriteVram(VramSpriteGuid vramSpriteId);
    DLL_EXPORT Animation2D&                           FindSpriteAnimation(const VramSpriteGuid& vramId, const AnimationListId& animationId);
    DLL_EXPORT bool                                   SpriteVramExists(const VkGuid& vramId);
    DLL_EXPORT void                                   Destroy(Sprite& sprite);
};
extern DLL_EXPORT SpriteSystem& spriteSystem;
inline SpriteSystem& SpriteSystem::Get()
{
    static SpriteSystem instance;
    return instance;
}