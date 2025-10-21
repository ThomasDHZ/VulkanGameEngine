#pragma once
#include <DLL.h>
#include <Typedef.h>
#include <VkGuid.h>
#include <ECSid.h>
#include <MaterialSystem.h>
#include <MeshSystem.h>
#include <Transform2DComponent.h>
#include "VRAM.h"
#include "GameObject.h"
#include <BufferSystem.h>

struct Sprite
{
    uint GameObjectId;
    uint SpriteID = 0;
    uint CurrentAnimationID = 0;
    uint CurrentFrame = 0;
    uint SpriteLayer = 0;
    uint SpriteInstance = 0;
    VkGuid SpriteVramId;
    float CurrentFrameTime = 0.0f;
    bool SpriteAlive = true;
    ivec2 FlipSprite = ivec2(0);
    vec2 LastSpritePosition = vec2(0.0f);
    vec2 LastSpriteRotation = vec2(0.0f);
    vec2 LastSpriteScale = vec2(1.0f);
    vec2 SpritePosition = vec2(0.0f);
    vec2 SpriteRotation = vec2(0.0f);
    vec2 SpriteScale = vec2(1.0f);
};

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


typedef uint32 SpriteLayerId;
struct SpriteArchive
{
    Vector<uint32>				                      FreeSpriteIndicesList;
    Vector<Sprite>									  SpriteList;
    Vector<SpriteInstance>                            SpriteInstanceList;
    Vector<SpriteVram>                                SpriteVramList;
    UnorderedMap<SpriteLayerId, SpriteLayer>          SpriteLayerList;
    UnorderedMap<VramSpriteGuid, Vector<Animation2D>> SpriteAnimationMap;
};
DLL_EXPORT SpriteArchive spriteArchive;

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void Sprite_UpdateBatchSprites(SpriteInstance* spriteInstanceList, Sprite* spriteList, const Transform2DComponent* transform2DList, const SpriteVram* vramList, const Animation2D* animationList, const Material* materialList, size_t spriteCount, float deltaTime);
    DLL_EXPORT void Sprite_SetSpriteAnimation(Sprite& sprite, uint spriteAnimationEnum);
#ifdef __cplusplus
}
#endif

uint32 GetNextSpriteIndex();
uint32 GetNextSpriteLayerIndex();
DLL_EXPORT void Sprite_AddSprite(GameObject& gameObject, VkGuid& spriteVramId);
DLL_EXPORT VkGuid Sprite_LoadSpriteVRAM(const String& spriteVramPath);

DLL_EXPORT void Sprite_UpdateSprites(const float& deltaTime);
DLL_EXPORT void Sprite_Update(const GraphicsRenderer& renderer, const float& deltaTime);
DLL_EXPORT void Sprite_Destroy();

DLL_EXPORT void Sprite_SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum);

DLL_EXPORT Sprite* Sprite_FindSprite(uint gameObjectId);
DLL_EXPORT uint32 Sprite_FindSpriteComponentIndex(uint gameObjectId);
DLL_EXPORT Vector<std::reference_wrapper<Sprite>> Sprite_FindSpritesByLayer(const SpriteLayer& spriteLayer);
DLL_EXPORT const Vector<Mesh>& Sprite_FindSpriteLayerMeshList();

DLL_EXPORT SpriteVram& Sprite_FindSpriteVram(VkGuid VramSpriteID);
DLL_EXPORT Vector<SpriteInstance> Sprite_FindSpriteInstancesByLayer(const SpriteLayer& spriteLayer);
DLL_EXPORT Animation2D& Sprite_FindSpriteAnimation(const VramSpriteGuid& vramId, const UM_AnimationListID& animationId);
DLL_EXPORT const bool Sprite_SpriteLayerExists(const uint32 spriteDrawLayer);

DLL_EXPORT void Sprite_AddSpriteBatchLayer(const GraphicsRenderer& renderer, RenderPassGuid& renderPassId, uint32 spriteDrawLayer);

class SpriteSystem
{
private:
    void UpdateSprites(const float& deltaTime) { Sprite_UpdateSprites(deltaTime); }
    void UpdateSpriteBatchLayers(const float& deltaTime) 
    {
        for (auto& spriteLayer : SpriteLayerList)
        {
            Vector<SpriteInstance> spriteInstanceList = FindSpriteInstancesByLayer(spriteLayer.second);
            bufferSystem.UpdateBufferMemory(renderer, spriteLayer.second.SpriteLayerBufferId, spriteInstanceList);
        }
    }

public:

    Vector<uint32>				                      FreeSpriteIndicesList;
    Vector<Sprite>									  SpriteList;
    Vector<SpriteInstance>                            SpriteInstanceList;
    Vector<SpriteVram>                                SpriteVramList;
    UnorderedMap<SpriteLayerId, SpriteLayer>          SpriteLayerList;
    UnorderedMap<VramSpriteGuid, Vector<Animation2D>> SpriteAnimationMap;

    SpriteSystem() 
    {
        SpriteList.reserve(5);
        SpriteInstanceList.reserve(5);
        SpriteLayerList.reserve(5);
    }

    ~SpriteSystem() { }


    void AddSprite(GameObject& gameObject, VkGuid& spriteVramId) { Sprite_AddSprite(gameObject, spriteVramId); }
    void Update(const float& deltaTime) { Sprite_Update(renderer, deltaTime); }
    void SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum) { Sprite_SetSpriteAnimation(*sprite, spriteAnimationEnum); }
    Sprite* FindSprite(uint gameObjectId) { return Sprite_FindSprite(gameObjectId); }
    Vector<std::reference_wrapper<Sprite>> FindSpritesByLayer(const SpriteLayer& spriteLayer) { return Sprite_FindSpritesByLayer(spriteLayer); }
    const Vector<SpriteInstance> FindSpriteInstancesByLayer(const SpriteLayer& spriteLayer) { return Sprite_FindSpriteInstancesByLayer(spriteLayer); }
    const SpriteVram& FindSpriteVram(VkGuid vramSpriteId) { return Sprite_FindSpriteVram(vramSpriteId); }
    const Animation2D& FindSpriteAnimation(const VramSpriteGuid& vramId, const UM_AnimationListID& animationId) { return Sprite_FindSpriteAnimation(vramId, animationId); }
    VkGuid LoadSpriteVRAM(const String& spriteVramPath) { return Sprite_LoadSpriteVRAM(spriteVramPath); }
    void Destroy() { Sprite_Destroy(); }
};
DLL_EXPORT SpriteSystem spriteSystem;