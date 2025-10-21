#pragma once
#include "GameObject.h"

class GameObjectSystem
{
private:

public:

    GameObjectSystem()
    {

    }

    ~GameObjectSystem()
    {

    }

    void CreateGameObject(const String& name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, uint64 gameObjectComponentMask, VkGuid vramId, vec2 objectPosition)
    {
        GameObject_CreateGameObject(name, parentGameObjectId, objectEnum, gameObjectComponentMask, vramId, objectPosition);
    }

    void CreateGameObject(const String& gameObjectPath, const vec2& gameObjectPosition)
    {
        GameObject_CreateGameObject(gameObjectPath, gameObjectPosition);
    }

    void Update(const float deltaTime)
    {
        GameObject_Update(deltaTime);
    }

    void LoadTransformComponent(const nlohmann::json& json, uint gameObjectId, const vec2& gameObjectPosition)
    {
        GameObject_LoadTransformComponent(json, gameObjectId, gameObjectPosition);
    }

    void LoadInputComponent(const nlohmann::json& json, uint gameObjectId)
    {
        GameObject_LoadInputComponent(json, gameObjectId);
    }

    void LoadSpriteComponent(const nlohmann::json& json, GameObject& gameObject)
    {
        GameObject_LoadSpriteComponent(json, gameObject);
    }

    const GameObject& FindGameObject(uint gameObjectId)
    {
        return GameObject_FindGameObject(gameObjectId);
    }

    Transform2DComponent& FindTransform2DComponent(uint gameObjectId)
    {
        return GameObject_FindTransform2DComponent(gameObjectId);
    }

    const InputComponent& FindInputComponent(uint gameObjectId)
    {
        return GameObject_FindInputComponent(gameObjectId);
    }

    const Vector<GameObject> GameObjectList()
    {
        return GameObject_GameObjectList();
    }

    const Vector<Transform2DComponent> Transform2DComponentList()
    {
        return GameObject_Transform2DComponentList();
    }

    const Vector<InputComponent> InputComponentList()
    {
        return GameObject_InputComponentList();
    }

    void DestroyGameObject(uint gameObjectId)
    {
        GameObject_DestroyGameObject(gameObjectId);
    }

    void DestroyGameObjects()
    {
        GameObject_DestroyGameObjects();
    }
};
DLL_EXPORT GameObjectSystem gameObjectSystem;