#include "pch.h"
#include <File.h>
#include <Transform2DComponent.h>
#include "Sprite.h"
#include "GameObject.h"
#include "MegaManObject.h"

GameObjectArchive gameObjectArchive = GameObjectArchive();


void GameObject_CreateGameObject(const String& gameObjectPath, const vec2& gameObjectPosition)
{
    uint32 newIndex = GetNextGameObjectIndex();
    nlohmann::json json = File_LoadJsonFile(gameObjectPath.c_str());
    gameObjectArchive.GameObjectList.emplace_back(GameObject
    {
        .GameObjectId = GetNextGameObjectIndex(),
        .GameObjectType = json["GameObjectType"]
    });

    for (size_t x = 0; x < json["GameObjectComponentList"].size(); x++)
    {
        int componentType = json["GameObjectComponentList"][x]["ComponentType"];
        switch (componentType)
        {
        case kTransform2DComponent: GameObject_LoadTransformComponent(json["GameObjectComponentList"][x], newIndex, gameObjectPosition); break;
        case kInputComponent: GameObject_LoadInputComponent(json["GameObjectComponentList"][x], newIndex); break;
        case kSpriteComponent: GameObject_LoadSpriteComponent(json["GameObjectComponentList"][x], newIndex); break;
        }
    }
    GameObject_LoadComponentBehavior(newIndex, json["GameObjectType"]);
}

void GameObject_CreateGameObject(const String& name, GameObjectTypeEnum objectEnum, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, VkGuid vramId, vec2 objectPosition)
{
    uint32 newIndex = GetNextGameObjectIndex();
    gameObjectArchive.GameObjectList.emplace_back(GameObject
    {
        .GameObjectId = newIndex,
        .GameObjectType = objectEnum
    });
    GameObject_LoadComponentTable(newIndex, gameObjectComponentTypeList, objectPosition, vramId);
    GameObject_LoadComponentBehavior(newIndex, objectEnum);
}

void GameObject_CreateGameObject(const String& name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, VkGuid vramId, vec2 objectPosition)
{
    uint32 newIndex = GetNextGameObjectIndex();
    gameObjectArchive.GameObjectList.emplace_back(GameObject
    {
        .GameObjectId = newIndex,
        .ParentGameObjectId = parentGameObjectId,
        .GameObjectType = objectEnum
    });
    GameObject_LoadComponentTable(newIndex, gameObjectComponentTypeList, objectPosition, vramId);
    GameObject_LoadComponentBehavior(newIndex, objectEnum);
}

void GameObject_Update(const float deltaTime)
{
    for (auto& gameObject : gameObjectArchive.GameObjectList)
    {
        GameObjectBehavior gameObjectBehavior = GameObject_FindGameObjectBehavior(gameObject.GameObjectType);
        if (gameObjectBehavior.Update)
        {
            gameObjectBehavior.Update(gameObject.GameObjectId, deltaTime);
        }
    }
}

void GameObject_LoadComponentBehavior(uint gameObjectId, GameObjectTypeEnum objectEnum)
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

void GameObject_LoadTransformComponent(const nlohmann::json& json, uint gameObjectId, const vec2& gameObjectPosition)
{
    gameObjectArchive.Transform2DComponentList.emplace_back(Transform2DComponent
        {
            .GameObjectId = gameObjectId,
            .GameObjectPosition = gameObjectPosition,
            .GameObjectRotation = vec2{ json["GameObjectRotation"][0], json["GameObjectRotation"][1] },
            .GameObjectScale = vec2{ json["GameObjectScale"][0], json["GameObjectScale"][1] }
        });
}

void GameObject_LoadInputComponent(const nlohmann::json& json, uint gameObjectId)
{
    gameObjectArchive.InputComponentList.emplace_back(InputComponent
        {
            .GameObjectId = gameObjectId
        });
}

void GameObject_LoadSpriteComponent(const nlohmann::json& json, uint id)
{
    VkGuid vramId = VkGuid(json["VramId"].get<String>().c_str());
    Sprite_AddSprite(id, vramId);
}

const GameObject& GameObject_FindGameObject(uint gameObjectId)
{
    auto it = std::find_if(gameObjectArchive.GameObjectList.begin(), gameObjectArchive.GameObjectList.end(),
        [gameObjectId](const GameObject& gameObject) {
            return gameObject.GameObjectId == gameObjectId;
        }
    );
    return *it;
}

const GameObjectBehavior& GameObject_FindGameObjectBehavior(const GameObjectTypeEnum& id)
{
    return gameObjectArchive.ComponentBehaviorMap.at(id);
}

Transform2DComponent& GameObject_FindTransform2DComponent(uint gameObjectId)
{
    auto it = std::find_if(gameObjectArchive.Transform2DComponentList.begin(), gameObjectArchive.Transform2DComponentList.end(),
        [gameObjectId](const Transform2DComponent& transformComponent) {
            return transformComponent.GameObjectId == gameObjectId;
        }
    );
    return *it;
}

const InputComponent& GameObject_FindInputComponent(uint gameObjectId)
{
    auto it = std::find_if(gameObjectArchive.InputComponentList.begin(), gameObjectArchive.InputComponentList.end(),
        [gameObjectId](const InputComponent& inputComponent) {
            return inputComponent.GameObjectId == gameObjectId;
        }
    );
    return *it;
}

const bool GameObjectExists(uint gameObjectId)
{
    return  std::any_of(gameObjectArchive.GameObjectList.begin(), gameObjectArchive.GameObjectList.end(), [gameObjectId](const GameObject& gameObject) { return gameObject.GameObjectId == gameObjectId; });
}

const bool GameObjectBehaviorExists(const GameObjectTypeEnum objectEnum)
{
    return gameObjectArchive.ComponentBehaviorMap.contains(objectEnum);
}

Vector<GameObject> GameObject_FindGameObjectByType(const GameObjectTypeEnum& gameObjectType)
{
    Vector<GameObject> gameObjectList;
    std::copy_if(gameObjectArchive.GameObjectList.begin(), gameObjectArchive.GameObjectList.end(), std::back_inserter(gameObjectList),
        [gameObjectType](const GameObject& gameObject) 
        {
            return gameObject.GameObjectType == gameObjectType;
        }
    );
    return gameObjectList;
}

const Vector<GameObject> GameObject_GameObjectList()
{
    return gameObjectArchive.GameObjectList;
}

const Vector<Transform2DComponent> GameObject_Transform2DComponentList()
{
    return gameObjectArchive.Transform2DComponentList;
}

const Vector<InputComponent> GameObject_InputComponentList()
{
    return gameObjectArchive.InputComponentList;
}

void GameObject_DestroyGameObject(uint gameObjectId)
{
    //gameObjectArchive.GameObjectList.erase(gameObjectId);
    //gameObjectArchive.Transform2DComponentList.erase(gameObjectId);
    //gameObjectArchive.InputComponentList.erase(gameObjectId);
}

void GameObject_DestroyGameObjects()
{
    gameObjectArchive.GameObjectList.clear();
    gameObjectArchive.Transform2DComponentList.clear();
    gameObjectArchive.InputComponentList.clear();
}

uint32 GetNextGameObjectIndex()
{
    if (!gameObjectArchive.FreeGameObjectIndices.empty())
    {
        uint index = gameObjectArchive.FreeGameObjectIndices.back();
        gameObjectArchive.FreeGameObjectIndices.pop_back();
        return index;
    }
    return gameObjectArchive.GameObjectList.size();
}

void GameObject_LoadComponentTable(uint gameObjectId, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, vec2& objectPosition, VkGuid& vramId)
{
    for (auto component : gameObjectComponentTypeList)
    {
        switch (component)
        {
        case kTransform2DComponent:
        {
            gameObjectArchive.Transform2DComponentList.emplace_back(Transform2DComponent
                {
                    .GameObjectId = gameObjectId,
                    .GameObjectPosition = objectPosition,
                    .GameObjectRotation = vec2(),
                    .GameObjectScale = vec2()
                });
            break;
        }
        case kInputComponent:
        {
            gameObjectArchive.InputComponentList.emplace_back(InputComponent
                {
                    .GameObjectId = gameObjectId
                });
            break;
        }
            case kSpriteComponent: Sprite_AddSprite(gameObjectId, vramId); break;
        }
    }
}
