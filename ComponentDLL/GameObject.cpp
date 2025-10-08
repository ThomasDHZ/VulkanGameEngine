#include "pch.h"
#include <File.h>
#include <Transform2DComponent.h>
#include "Sprite.h"
#include "GameObject.h"
#include "MegaManObject.h"

GameObjectArchive gameObjectArchive = GameObjectArchive();


void GameObject_CreateGameObject(const String& gameObjectPath, const vec2& gameObjectPosition)
{
    GameObjectID id;
    id.id = static_cast<uint>(gameObjectArchive.GameObjectMap.size() + 1);
    
    nlohmann::json json = File_LoadJsonFile(gameObjectPath.c_str());
    gameObjectArchive.GameObjectMap[id] = GameObject
    {
        .GameObjectId = id,
        .GameObjectType = json["GameObjectType"]
    };

    for (size_t x = 0; x < json["GameObjectComponentList"].size(); x++)
    {
        int componentType = json["GameObjectComponentList"][x]["ComponentType"];
        switch (componentType)
        {
        case kTransform2DComponent: GameObject_LoadTransformComponent(json["GameObjectComponentList"][x], id, gameObjectPosition); break;
        case kInputComponent: GameObject_LoadInputComponent(json["GameObjectComponentList"][x], id); break;
        case kSpriteComponent: GameObject_LoadSpriteComponent(json["GameObjectComponentList"][x], id); break;
        }
    }
    GameObject_LoadComponentBehavior(id, json["GameObjectType"]);
}

void GameObject_CreateGameObject(const String& name, GameObjectTypeEnum objectEnum, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, VkGuid vramId, vec2 objectPosition)
{
    GameObjectID id;
    id.id = static_cast<uint>(gameObjectArchive.GameObjectMap.size() + 1);
    
    gameObjectArchive.GameObjectMap[id] = GameObject
    {
        .GameObjectId = id,
        .GameObjectType = objectEnum
    };
    GameObject_LoadComponentTable(id, gameObjectComponentTypeList, objectPosition, vramId);
    GameObject_LoadComponentBehavior(id, objectEnum);
}

void GameObject_CreateGameObject(const String& name, GameObjectID parentGameObjectId, GameObjectTypeEnum objectEnum, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, VkGuid vramId, vec2 objectPosition)
{
    GameObjectID id;
    id.id = static_cast<uint>(gameObjectArchive.GameObjectMap.size() + 1);

    gameObjectArchive.GameObjectMap[id] = GameObject
    {
        .GameObjectId = id,
        .GameObjectType = objectEnum
    };

    for (auto component : gameObjectComponentTypeList)
    {
        switch (component)
        {
        case kTransform2DComponent: gameObjectArchive.Transform2DComponentMap[id] = Transform2DComponent(objectPosition); break;
        case kInputComponent: gameObjectArchive.InputComponentMap[id] = InputComponent(id); break;
        case kSpriteComponent: Sprite_AddSprite(id, vramId); break;
        }
    }
    GameObject_LoadComponentBehavior(id, objectEnum);
}

void GameObject_Update(const float deltaTime)
{
    for (auto& gameObject : gameObjectArchive.GameObjectMap)
    {
        GameObjectBehavior gameObjectBehavior = GameObject_FindGameObjectBehavior(gameObject.second.GameObjectType);
        if (gameObjectBehavior.Update)
        {
            gameObjectBehavior.Update(gameObject.second.GameObjectId, deltaTime);
        }
    }
}

void GameObject_LoadComponentBehavior(GameObjectID gameObjectId, GameObjectTypeEnum objectEnum)
{
    GameObjectBehavior componentBehavior;
    if (!GameObjectBehaviorExists(objectEnum))
    {
        switch (objectEnum)
        {
        case kGameObjectMegaMan: MegaMan_Behaviors(componentBehavior); break;
        case kGameObjectMegaManShot: MegaManShot_Behaviors(componentBehavior); break;
        }
        gameObjectArchive.ComponentBehaviorMap[objectEnum] = componentBehavior;
    }
}

void GameObject_LoadTransformComponent(const nlohmann::json& json, GameObjectID id, const vec2& gameObjectPosition)
{
    Transform2DComponent transform2D;
    transform2D.GameObjectPosition = gameObjectPosition;
    transform2D.GameObjectRotation = vec2{ json["GameObjectRotation"][0], json["GameObjectRotation"][1] };
    transform2D.GameObjectScale = vec2{ json["GameObjectScale"][0], json["GameObjectScale"][1] };
    gameObjectArchive.Transform2DComponentMap[id] = transform2D;
}

void GameObject_LoadInputComponent(const nlohmann::json& json, GameObjectID id)
{
    gameObjectArchive.InputComponentMap[id] = InputComponent(id);
}

void GameObject_LoadSpriteComponent(const nlohmann::json& json, GameObjectID id)
{
    VkGuid vramId = VkGuid(json["VramId"].get<String>().c_str());
    Sprite_AddSprite(id, vramId);
}

const GameObject& GameObject_FindGameObject(const GameObjectID& id)
{
    return gameObjectArchive.GameObjectMap.at(id);
}

const GameObjectBehavior& GameObject_FindGameObjectBehavior(const GameObjectTypeEnum& id)
{
    return gameObjectArchive.ComponentBehaviorMap.at(id);
}

Transform2DComponent& GameObject_FindTransform2DComponent(const GameObjectID& id)
{
    return gameObjectArchive.Transform2DComponentMap.at(id);
}

const InputComponent& GameObject_FindInputComponent(const GameObjectID& id)
{
    return gameObjectArchive.InputComponentMap.at(id);
}

const bool GameObjectExists(const GameObjectID& id)
{
    return gameObjectArchive.GameObjectMap.contains(id);
}

const bool GameObjectBehaviorExists(const GameObjectTypeEnum objectEnum)
{
    return gameObjectArchive.ComponentBehaviorMap.contains(objectEnum);
}

GameObject& GameObject_FindGameObjectById(const GameObjectID& id)
{
    return gameObjectArchive.GameObjectMap.at(id);
}

Vector<std::reference_wrapper<GameObject>> GameObject_FindGameObjectByType(const GameObjectTypeEnum& gameObjectType)
{
    Vector<std::reference_wrapper<GameObject>> result;
    for (auto& pair : gameObjectArchive.GameObjectMap)
    {
        GameObject& obj = pair.second;
        if (obj.GameObjectType == gameObjectType)
        {
            result.emplace_back(std::ref(obj));
        }
    }
    return result;
}

const Vector<GameObject> GameObject_GameObjectList()
{
    Vector<GameObject> list;
    for (const auto& pair : gameObjectArchive.GameObjectMap)
    {
        list.emplace_back(pair.second);
    }
    return list;
}

const Vector<Transform2DComponent> GameObject_Transform2DComponentList()
{
    Vector<Transform2DComponent> list;
    for (const auto& pair : gameObjectArchive.Transform2DComponentMap)
    {
        list.emplace_back(pair.second);
    }
    return list;
}

const Vector<InputComponent> GameObject_InputComponentList()
{
    Vector<InputComponent> list;
    for (const auto& pair : gameObjectArchive.InputComponentMap)
    {
        list.emplace_back(pair.second);
    }
    return list;
}

void GameObject_DestroyGameObject(const GameObjectID& gameObjectId)
{
    gameObjectArchive.GameObjectMap.erase(gameObjectId);
    gameObjectArchive.Transform2DComponentMap.erase(gameObjectId);
    gameObjectArchive.InputComponentMap.erase(gameObjectId);
}

void GameObject_DestroyGameObjects()
{
    gameObjectArchive.GameObjectMap.clear();
    gameObjectArchive.Transform2DComponentMap.clear();
    gameObjectArchive.InputComponentMap.clear();
}

void GameObject_LoadComponentTable(GameObjectID gameObjectId, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, vec2& objectPosition, VkGuid& vramId)
{
    for (auto component : gameObjectComponentTypeList)
    {
        switch (component)
        {
            case kTransform2DComponent: gameObjectArchive.Transform2DComponentMap[gameObjectId] = Transform2DComponent(objectPosition); break;
            case kInputComponent: gameObjectArchive.InputComponentMap[gameObjectId] = InputComponent(gameObjectId); break;
            case kSpriteComponent: Sprite_AddSprite(gameObjectId, vramId); break;
        }
    }
}
