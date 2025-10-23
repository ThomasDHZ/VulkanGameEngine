#pragma once
#include "pch.h"
#include "GameObject.h"
#include <Transform2DComponent.h>
#include "InputComponent.h"
#include "MegaManShot.h"
#include "VulkanRenderer.h"

enum GameObjectTypeEnum
{
    kGameObjectNone,
    kGameObjectMegaMan,
    kGameObjectMegaManShot
};

enum ComponentTypeEnum 
{
    kUndefined = 0,
    kInputComponent = 1ULL << 0,
    kSpriteComponent = 1ULL << 1,
    kTransform2DComponent = 1ULL << 2,
    kTransform3DComponent = 1ULL << 3,
};

struct GameObject
{
    GameObjectTypeEnum GameObjectType = kGameObjectNone;
    uint64 GameObjectComponentMask = kUndefined;
    uint32 GameObjectId = UINT32_MAX;
    uint32 ParentGameObjectId = UINT32_MAX;
    uint32 Transform2DComponentId = UINT32_MAX;
    uint32 InputComponentId = UINT32_MAX;
    uint32 SpriteComponentId = UINT32_MAX;
    void* GameObjectData = nullptr;
    bool  GameObjectAlive = true;
};

struct GameObjectArchive
{
    Vector<GameObject> GameObjectList;
    Vector<uint32> FreeGameObjectIndices;
    Vector<InputComponent> InputComponentList;
    Vector<Transform2DComponent> Transform2DComponentList;
    UnorderedMap<GameObjectTypeEnum, GameObjectBehavior> ComponentBehaviorMap;
};
DLL_EXPORT GameObjectArchive gameObjectArchive;

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void GameObject_CreateGameObjectFromJson(const GraphicsRenderer& renderer, const char* gameObjectPath, vec2 gameObjectPosition);
    DLL_EXPORT void GameObject_CreateGameObject(const char* name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, uint64 gameObjectComponentMask, VkGuid vramId, vec2 objectPosition);
#ifdef __cplusplus
}
#endif

DLL_EXPORT void GameObject_Update(const float deltaTime);

DLL_EXPORT void GameObject_LoadComponentBehavior(GameObject& gameObject, GameObjectTypeEnum objectEnum);
DLL_EXPORT void GameObject_LoadTransformComponent(const nlohmann::json& json, uint gameObjectId, const vec2& gameObjectPosition);
DLL_EXPORT void GameObject_LoadInputComponent(const nlohmann::json& json, uint gameObjectId);
DLL_EXPORT void GameObject_LoadSpriteComponent(const GraphicsRenderer& renderer, const nlohmann::json& json, GameObject& gameObject);
DLL_EXPORT void GameObject_LoadMegaManShotComponent(uint gameObjectId, const vec2& gameObjectPosition);

DLL_EXPORT GameObject& GameObject_FindGameObject(uint gameObjectId);
DLL_EXPORT const GameObjectBehavior& GameObject_FindGameObjectBehavior(const GameObjectTypeEnum& id);
DLL_EXPORT Transform2DComponent& GameObject_FindTransform2DComponent(uint gameObjectId);
DLL_EXPORT const InputComponent& GameObject_FindInputComponent(uint gameObjectId);

DLL_EXPORT Vector<GameObject> GameObject_FindGameObjectByType(const GameObjectTypeEnum& gameObjectType);

DLL_EXPORT const bool GameObjectExists(uint gameObjectId);
DLL_EXPORT const bool GameObjectBehaviorExists(const GameObjectTypeEnum objectEnum);

DLL_EXPORT const Vector<GameObject> GameObject_GameObjectList();
DLL_EXPORT const Vector<Transform2DComponent> GameObject_Transform2DComponentList();
DLL_EXPORT const Vector<InputComponent> GameObject_InputComponentList();

DLL_EXPORT void GameObject_DestroyGameObject(uint gameObjectId);
DLL_EXPORT void GameObject_DestroyGameObjects();

DLL_EXPORT void GameObject_DestroyDeadGameObjects();

uint32 GetNextGameObjectIndex();
void* GameObject_LoadObjectData(GameObjectTypeEnum gameObjectType);
void GameObject_LoadComponentTable(const GraphicsRenderer& renderer, GameObject& gameObject, vec2& objectPosition, VkGuid& vramId);