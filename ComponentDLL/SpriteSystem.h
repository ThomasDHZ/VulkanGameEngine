#pragma once
#include "pch.h"
#include "GameObjectSystem.h"
#include "Transform2DComponent.h"
#include "VRAM.h"

typedef uint32 SpriteLayerId;
struct SpriteInstance
{
    vec2  SpritePosition;
    vec4  UVOffset;
    vec2  SpriteSize;
    ivec2 FlipSprite;
    vec4  Color;
    mat4  InstanceTransform;
    uint  MaterialID;
};

struct SpriteLayer
{
    VkGuid RenderPassId;
    uint SpriteDrawLayer = UINT32_MAX;
    uint SpriteLayerMeshId = UINT32_MAX;
    uint SpriteLayerBufferId = UINT32_MAX;
};

struct Sprite
{
    uint GameObjectId;
    uint SpriteID = 0;
    uint CurrentAnimationID = 0;
    uint CurrentFrame = 0;
    uint SpriteLayer = 0;
    uint SpriteInstance = 0;
    VkGuid SpriteVramId = VkGuid();
    vec2 SpritePosition = vec2(0.0f);
    vec2 SpriteRotation = vec2(0.0f);
    vec2 SpriteScale = vec2(1.0f);
    ivec2 FlipSprite = ivec2(0);
    float CurrentFrameTime = 0.0f;
    bool SpriteAlive = true;
};

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

private:

    Vector<uint32>				                      FreeSpriteIndicesList;
    Vector<SpriteInstance>                            SpriteInstanceList;
    UnorderedMap<VramSpriteGuid, Vector<Animation2D>> SpriteAnimationMap;

    uint32     GetNextSpriteIndex();
    void       AddSpriteBatchLayer(RenderPassGuid& renderPassId, uint32 spriteDrawLayer);
    void       UpdateSprites(const float& deltaTime);
    void       UpdateSpriteBatchLayers(const float& deltaTime);
    void       UpdateBatchSprites(SpriteInstance* spriteInstanceList, Sprite* spriteList, const Transform2DComponent* transform2DList, const SpriteVram* vramList, const Animation2D* animationList, const Material* materialList, size_t spriteCount, float deltaTime);
    uint32     FindSpriteComponentIndex(uint gameObjectId);
    const bool SpriteLayerExists(const uint32 spriteDrawLayer);
 
public:

    Vector<Sprite>									  SpriteList;
    Vector<SpriteVram>                                SpriteVramList;
    UnorderedMap<SpriteLayerId, SpriteLayer>          SpriteLayerList;

    DLL_EXPORT void AddSprite(GameObject& gameObject, VkGuid& spriteVramId);
    DLL_EXPORT VkGuid LoadSpriteVRAM(const String& spriteVramPath);
    DLL_EXPORT void Update(float deltaTime);
    DLL_EXPORT void SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum);
    DLL_EXPORT Sprite FindSprite(uint gameObjectId);
    DLL_EXPORT Vector<Sprite> FindSpritesByLayer(const SpriteLayer& spriteLayer);
    DLL_EXPORT Vector<SpriteInstance> FindSpriteInstancesByLayer(const SpriteLayer& spriteLayer);
    DLL_EXPORT SpriteVram& FindSpriteVram(VkGuid vramSpriteId);
    DLL_EXPORT Animation2D& FindSpriteAnimation(const VramSpriteGuid& vramId, const AnimationListId& animationId);
    DLL_EXPORT void Destroy();
};
extern DLL_EXPORT SpriteSystem& spriteSystem;
inline SpriteSystem& SpriteSystem::Get()
{
    static SpriteSystem instance;
    return instance;
}