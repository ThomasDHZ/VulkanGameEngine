#pragma once
#include "pch.h"
#include "GameObjectSystem.h"
#include "Transform2DComponent.h"
#include "VRAM.h"
#include <MemoryPoolSystem.h>

typedef uint32 SpriteLayerId;
struct SpriteLayer
{
    uint32 InstanceCount = 0;
    uint32 StartInstanceIndex = 0;
    uint32 SpriteDrawLayer = UINT32_MAX;
};

struct Sprite
{
    uint SpriteId = UINT32_MAX;
    uint GameObjectId = UINT32_MAX;
    uint SpriteInstanceId = 0;
    uint CurrentAnimationId = 0;
    uint CurrentFrame = 0;
    uint SpriteLayer = 0;
    vec2 SpritePosition = vec2(0.0f);
    vec2 SpriteRotation = vec2(0.0f);
    vec2 SpriteScale = vec2(1.0f);
    ivec2 FlipSprite = ivec2(0);
    VkGuid SpriteVramId = VkGuid();
    float CurrentFrameTime = 0.0f;
    bool SpriteAlive = true;
    bool IsSpriteTranformDirty = true;
    bool IsSpriteAnimationDirty = true;
    bool IsSpritePropertiesDirty = true;
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
    UnorderedMap<VramSpriteGuid, Vector<Animation2D>> SpriteAnimationMap;

    uint32     GetNextSpriteIndex();
    void       AddSpriteBatchLayer(RenderPassGuid& renderPassId, uint32 spriteDrawLayer);
    void       SyncSpritesWithSpriteInstances();
 
public:
    
    uint32                                            SpriteMeshId;
    Vector<Sprite>									  SpriteList;
    Vector<SpriteVram>                                SpriteVramList;
    Vector<SpriteLayer>                               SpriteLayerList;
    bool                                              SpriteListDirty = true;

    DLL_EXPORT void AddSprite(GameObject& gameObject, VkGuid& spriteVramId);
    DLL_EXPORT VkGuid LoadSpriteVRAM(const String& spriteVramPath);
    DLL_EXPORT void Update(const float& deltaTime);
    DLL_EXPORT void SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum);
    DLL_EXPORT void SortSpritesbyLayer();
    DLL_EXPORT Sprite FindSprite(uint gameObjectId);
    DLL_EXPORT SpriteVram& FindSpriteVram(VramSpriteGuid vramSpriteId);
    DLL_EXPORT Vector<Sprite> FindSpritesByLayer(const SpriteLayer& spriteLayer);
    DLL_EXPORT Animation2D& FindSpriteAnimation(const VramSpriteGuid& vramId, const AnimationListId& animationId);
    DLL_EXPORT uint32 GetSpriteCountofLayer(uint32 layerId);
    DLL_EXPORT void DestroySprite(uint32 spriteId);
    DLL_EXPORT void DestroyDeadSprites();
    DLL_EXPORT void Destroy();
};
extern DLL_EXPORT SpriteSystem& spriteSystem;
inline SpriteSystem& SpriteSystem::Get()
{
    static SpriteSystem instance;
    return instance;
}