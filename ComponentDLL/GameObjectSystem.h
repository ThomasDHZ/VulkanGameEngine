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
    DLL_EXPORT void GameObjectSystem_CreateGameObjectFromJson(const GraphicsRenderer& renderer, const char* gameObjectPath, vec2 gameObjectPosition);
    DLL_EXPORT void GameObjectSystem_CreateGameObject(const char* name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, uint64 gameObjectComponentMask, VkGuid vramId, vec2 objectPosition);
    DLL_EXPORT void GameObjectSystem_Update(const float& deltaTime);
    DLL_EXPORT void GameObjectSystem_LoadTransformComponent(const nlohmann::json& json, uint gameObjectId, const vec2& gameObjectPosition);
    DLL_EXPORT void GameObjectSystem_LoadInputComponent(const nlohmann::json& json, uint gameObjectId);
    DLL_EXPORT void GameObjectSystem_LoadSpriteComponent(const GraphicsRenderer& renderer, const nlohmann::json& json, GameObject& gameObject);
    DLL_EXPORT void GameObjectSystem_DestroyGameObject(uint gameObjectId);
    DLL_EXPORT void GameObjectSystem_DestroyGameObjects();
    DLL_EXPORT void GameObjectSystem_DestroyDeadGameObjects(); 
    DLL_EXPORT GameObject& GameObjectSystem_FindGameObject(uint gameObjectId);
    DLL_EXPORT Transform2DComponent& GameObjectSystem_FindTransform2DComponent(uint gameObjectId);
    DLL_EXPORT InputComponent& GameObjectSystem_FindInputComponent(uint gameObjectId);
    DLL_EXPORT GameObject* GameObjectSystem_GameObjectList(size_t& returnListCount);
    DLL_EXPORT Transform2DComponent* GameObjectSystem_Transform2DComponentList(size_t& returnListCount);
    DLL_EXPORT InputComponent* GameObjectSystem_InputComponentList(size_t& returnListCount);
#ifdef __cplusplus
}
#endif

  uint32 GameObject_GetNextGameObjectIndex();
  void*  GameObject_LoadObjectData(GameObjectTypeEnum gameObjectType);
  void   GameObject_LoadComponentTable(const GraphicsRenderer& renderer, GameObject& gameObject, vec2& objectPosition, VkGuid& vramId);
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

    void CreateGameObject(const GraphicsRenderer& renderer, const String& gameObjectPath, const vec2& gameObjectPosition)
    {
        GameObjectSystem_CreateGameObjectFromJson(renderer, gameObjectPath.c_str(), gameObjectPosition);
    }

    void Update(const float& deltaTime)
    {
        GameObjectSystem_Update(deltaTime);
    }

    void LoadTransformComponent(const nlohmann::json& json, uint gameObjectId, const vec2& gameObjectPosition)
    {
        GameObjectSystem_LoadTransformComponent(json, gameObjectId, gameObjectPosition);
    }

    void LoadInputComponent(const nlohmann::json& json, uint gameObjectId)
    {
        GameObjectSystem_LoadInputComponent(json, gameObjectId);
    }

    void LoadSpriteComponent(const GraphicsRenderer& renderer, const nlohmann::json& json, GameObject& gameObject)
    {
        GameObjectSystem_LoadSpriteComponent(renderer, json, gameObject);
    }

    const GameObject& FindGameObject(uint gameObjectId)
    {
        return GameObjectSystem_FindGameObject(gameObjectId);
    }

    Transform2DComponent& FindTransform2DComponent(uint gameObjectId)
    {
        return GameObjectSystem_FindTransform2DComponent(gameObjectId);
    }

    const InputComponent& FindInputComponent(uint gameObjectId)
    {
        return GameObjectSystem_FindInputComponent(gameObjectId);
    }

    Vector<GameObject> GetGameObjectList()
    {
        size_t count = 0;
        GameObject* gameObjectList = GameObjectSystem_GameObjectList(count);
        return Vector<GameObject>(gameObjectList, gameObjectList + count);
    }

    Vector<Transform2DComponent> GetTransform2DComponentList()
    {
        size_t count = 0;
        Transform2DComponent* transform2DComponentList = GameObjectSystem_Transform2DComponentList(count);
        return Vector<Transform2DComponent>(transform2DComponentList, transform2DComponentList + count);
    }

    Vector<InputComponent> GetInputComponentList()
    {
        size_t count = 0;
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