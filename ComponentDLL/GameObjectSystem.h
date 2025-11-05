#pragma once
#include "pch.h"
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

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void GameObjectSystem_CreateGameObjectFromJson(const char* gameObjectPath, vec2 gameObjectPosition);
    DLL_EXPORT void GameObjectSystem_CreateGameObject(const char* name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, uint64 gameObjectComponentMask, VkGuid vramId, vec2 objectPosition);
    DLL_EXPORT void GameObjectSystem_Update(const float& deltaTime);
    DLL_EXPORT uint GameObjectSystem_LoadTransformComponent(const char* jsonString, uint gameObjectId, const vec2& gameObjectPosition);
    DLL_EXPORT uint GameObjectSystem_LoadInputComponent(const char* jsonString, uint gameObjectId);
    DLL_EXPORT uint GameObjectSystem_LoadSpriteComponent(const char* jsonString, GameObject& gameObject);
    DLL_EXPORT void GameObjectSystem_DestroyGameObject(uint gameObjectId);
    DLL_EXPORT void GameObjectSystem_DestroyGameObjects();
    DLL_EXPORT void GameObjectSystem_DestroyDeadGameObjects(); 
    DLL_EXPORT GameObject& GameObjectSystem_FindGameObject(uint gameObjectId);
    DLL_EXPORT Transform2DComponent GameObjectSystem_FindTransform2DComponent(uint gameObjectId);
    DLL_EXPORT InputComponent GameObjectSystem_FindInputComponent(uint gameObjectId);
    DLL_EXPORT GameObject* GameObjectSystem_GameObjectList(int& outCount);
    DLL_EXPORT Transform2DComponent* GameObjectSystem_Transform2DComponentList(int& outCount);
    DLL_EXPORT InputComponent* GameObjectSystem_InputComponentList(int& outCount);
#ifdef __cplusplus
}
#endif

  uint32 GameObject_GetNextGameObjectIndex();
  void*  GameObject_LoadObjectData(GameObjectTypeEnum gameObjectType);
  void   GameObject_LoadComponentTable(GameObject& gameObject, vec2& objectPosition, VkGuid& vramId);
  void   GameObject_LoadComponentBehavior(GameObject& gameObject, GameObjectTypeEnum objectEnum);
  void   GameObject_LoadMegaManShotComponent(uint gameObjectId, const vec2& gameObjectPosition);
  bool   GameObject_GameObjectBehaviorExists(const GameObjectTypeEnum objectEnum);
  GameObjectBehavior& GameObject_FindGameObjectBehavior(const GameObjectTypeEnum& id);
  Vector<GameObject> GameObject_FindGameObjectByType(const GameObjectTypeEnum& gameObjectType);

class GameObjectSystem
{
private:

public:
    Vector<GameObject> GameObjectList;
    Vector<uint32> FreeGameObjectIndices;
    Vector<InputComponent> InputComponentList;
    Vector<Transform2DComponent> Transform2DComponentList;
    UnorderedMap<GameObjectTypeEnum, GameObjectBehavior> ComponentBehaviorMap;

    GameObjectSystem()
    {

    }

    ~GameObjectSystem()
    {

    }

    void CreateGameObject(const String& name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, uint64 gameObjectComponentMask, VkGuid vramId, vec2 objectPosition)
    {
        GameObjectSystem_CreateGameObject(name.c_str(), parentGameObjectId, objectEnum, gameObjectComponentMask, vramId, objectPosition);
    }

    void CreateGameObject(const String& gameObjectPath, const vec2& gameObjectPosition)
    {
        GameObjectSystem_CreateGameObjectFromJson(gameObjectPath.c_str(), gameObjectPosition);
    }

    void Update(const float& deltaTime)
    {
        GameObjectSystem_Update(deltaTime);
    }

    void LoadTransformComponent(const String& json, uint gameObjectId, const vec2& gameObjectPosition)
    {
        GameObjectSystem_LoadTransformComponent(json.c_str(), gameObjectId, gameObjectPosition);
    }

    void LoadInputComponent(const String& json, uint gameObjectId)
    {
        GameObjectSystem_LoadInputComponent(json.c_str(), gameObjectId);
    }

    void LoadSpriteComponent(const String& json, GameObject& gameObject)
    {
        GameObjectSystem_LoadSpriteComponent(json.c_str(), gameObject);
    }

    const GameObject& FindGameObject(uint gameObjectId)
    {
        return GameObjectSystem_FindGameObject(gameObjectId);
    }

    Transform2DComponent FindTransform2DComponent(uint gameObjectId)
    {
        return GameObjectSystem_FindTransform2DComponent(gameObjectId);
    }

    const InputComponent FindInputComponent(uint gameObjectId)
    {
        return GameObjectSystem_FindInputComponent(gameObjectId);
    }

    Vector<GameObject> GetGameObjectList()
    {
        int count = INT32_MAX;
        GameObject* gameObjectList = GameObjectSystem_GameObjectList(count);
        return Vector<GameObject>(gameObjectList, gameObjectList + count);
    }

    Vector<Transform2DComponent> GetTransform2DComponentList()
    {
        int count = INT32_MAX;
        Transform2DComponent* transform2DComponentList = GameObjectSystem_Transform2DComponentList(count);
        return Vector<Transform2DComponent>(transform2DComponentList, transform2DComponentList + count);
    }

    Vector<InputComponent> GetInputComponentList()
    {
        int count = INT32_MAX;
        InputComponent* inputComponentComponentList = GameObjectSystem_InputComponentList(count);
        return Vector<InputComponent>(inputComponentComponentList, inputComponentComponentList + count);
    }

    void DestroyGameObject(uint gameObjectId)
    {
        GameObjectSystem_DestroyGameObject(gameObjectId);
    }

    void DestroyGameObjects()
    {
        GameObjectSystem_DestroyGameObjects();
    }

    void DestroyDeadGameObjects()
    {
        GameObjectSystem_DestroyDeadGameObjects();
    }
};
DLL_EXPORT GameObjectSystem gameObjectSystem;