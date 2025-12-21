#pragma once
#include "pch.h"
#include "InputComponent.h"
#include "Transform2DComponent.h"
#include "MegaManShot.h"
#include "VulkanSystem.h"
#include <entt/entt.hpp>

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

class GameObjectSystem
{
public:
    static GameObjectSystem& Get();

private:
    GameObjectSystem() = default;
    ~GameObjectSystem() = default;
    GameObjectSystem(const GameObjectSystem&) = delete;
    GameObjectSystem& operator=(const GameObjectSystem&) = delete;
    GameObjectSystem(GameObjectSystem&&) = delete;
    GameObjectSystem& operator=(GameObjectSystem&&) = delete;

public:
    Vector<GameObject> GameObjectList;
    Vector<uint32> FreeGameObjectIndices;
    Vector<InputComponent> InputComponentList;
    Vector<Transform2DComponent> Transform2DComponentList;
    UnorderedMap<GameObjectTypeEnum, GameObjectBehavior> ComponentBehaviorMap;

    DLL_EXPORT uint32 GetNextGameObjectIndex();
    DLL_EXPORT void*  LoadObjectData(GameObjectTypeEnum gameObjectType);
    DLL_EXPORT void   LoadComponentTable(GameObject& gameObject, vec2& objectPosition, VkGuid& vramId);
    DLL_EXPORT void   LoadComponentBehavior(GameObject& gameObject, GameObjectTypeEnum objectEnum);
    DLL_EXPORT void   LoadMegaManShotComponent(uint gameObjectId, const vec2& gameObjectPosition);
    DLL_EXPORT bool   GameObjectBehaviorExists(const GameObjectTypeEnum objectEnum);
    DLL_EXPORT GameObjectBehavior& FindGameObjectBehavior(const GameObjectTypeEnum& id);
    DLL_EXPORT Vector<GameObject> FindGameObjectByType(const GameObjectTypeEnum& gameObjectType);

    DLL_EXPORT void CreateGameObject(const String& gameObjectJson, vec2 gameObjectPosition);
    DLL_EXPORT void CreateGameObject(const String& name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, uint64 gameObjectComponentMask, VkGuid vramId, vec2 objectPosition);
    DLL_EXPORT void Update(const float& deltaTime);
    DLL_EXPORT uint LoadTransformComponent(const char* jsonString, uint gameObjectId, const vec2& gameObjectPosition);
    DLL_EXPORT uint LoadInputComponent(const char* jsonString, uint gameObjectId);
    DLL_EXPORT uint LoadSpriteComponent(const char* jsonString, GameObject& gameObject);
    DLL_EXPORT void DestroyGameObject(uint gameObjectId);
    DLL_EXPORT void DestroyGameObjects();
    DLL_EXPORT void DestroyDeadGameObjects(); 
    DLL_EXPORT GameObject& FindGameObject(uint gameObjectId);
    DLL_EXPORT Transform2DComponent FindTransform2DComponent(uint gameObjectId);
    DLL_EXPORT InputComponent FindInputComponent(uint gameObjectId);
    DLL_EXPORT Vector<GameObject> GetGameObjectList();
    DLL_EXPORT Vector<Transform2DComponent> GetTransform2DComponentList();
    DLL_EXPORT Vector<InputComponent> GetInputComponentList();
};
extern DLL_EXPORT GameObjectSystem& gameObjectSystem;
inline GameObjectSystem& GameObjectSystem::Get()
{
    static GameObjectSystem instance;
    return instance;
}