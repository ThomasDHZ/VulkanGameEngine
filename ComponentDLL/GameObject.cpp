#include "pch.h"
#include <FileSystem.h>
#include <Transform2DComponent.h>
#include "SpriteSystem.h"
#include "GameObject.h"
#include "MegaManObject.h"
#include "MemorySystem.h"
#include "Component.h"

GameObjectArchive gameObjectArchive = GameObjectArchive();


void GameObject_CreateGameObject(const String& gameObjectPath, const vec2& gameObjectPosition)
{
    nlohmann::json json = File_LoadJsonFile(gameObjectPath.c_str());
    GameObject& gameObject = gameObjectArchive.GameObjectList.emplace_back(GameObject
        {
            .GameObjectType = GameObjectTypeEnum::kGameObjectMegaMan,
            .GameObjectId = static_cast<uint32>(gameObjectArchive.GameObjectList.size()),
            .GameObjectData = GameObject_LoadObjectData(GameObjectTypeEnum::kGameObjectMegaMan),
        });

    for (size_t x = 0; x < json["GameObjectComponentList"].size(); x++)
    {
        uint64 componentType = json["GameObjectComponentList"][x]["ComponentType"];
        gameObject.GameObjectComponentMask |= componentType;
        switch (componentType)
        {
            case kTransform2DComponent: GameObject_LoadTransformComponent(json["GameObjectComponentList"][x], gameObject.GameObjectId, gameObjectPosition); break;
            case kInputComponent: GameObject_LoadInputComponent(json["GameObjectComponentList"][x], gameObject.GameObjectId); break;
            case kSpriteComponent: GameObject_LoadSpriteComponent(json["GameObjectComponentList"][x], gameObject); break;
        }
    }
    GameObject_LoadComponentBehavior(gameObject, json["GameObjectType"]);
}

void GameObject_CreateGameObject(const String& name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, uint64 gameObjectComponentMask, VkGuid vramId, vec2 objectPosition)
{
    GameObject& gameObject = gameObjectArchive.GameObjectList.emplace_back(GameObject
    {
        .GameObjectType = GameObjectTypeEnum::kGameObjectMegaManShot,
        .GameObjectComponentMask = gameObjectComponentMask,
        .GameObjectId = static_cast<uint32>(gameObjectArchive.GameObjectList.size()),
        .ParentGameObjectId = parentGameObjectId,
        .GameObjectData = GameObject_LoadObjectData(GameObjectTypeEnum::kGameObjectMegaManShot)
    });
    GameObject_LoadComponentTable(gameObject, objectPosition, vramId);
    GameObject_LoadComponentBehavior(gameObject, objectEnum);
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

void GameObject_LoadComponentBehavior(GameObject& gameObject, GameObjectTypeEnum objectEnum)
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

void GameObject_LoadSpriteComponent(const nlohmann::json& json, GameObject& gameObject)
{
    VkGuid vramId = VkGuid(json["VramId"].get<String>().c_str());
    Sprite_AddSprite(gameObject, vramId);
}

GameObject& GameObject_FindGameObject(uint gameObjectId)
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

//uint32 GameObject_FindTransform2DComponentIndex(uint gameObjectId)
//{
//    auto it = std::find(gameObjectArchive.Transform2DComponentList.begin(), gameObjectArchive.Transform2DComponentList.end(), [gameObjectId](const Transform2DComponent& transformComponent)
//        {
//            return transformComponent.GameObjectId == gameObjectId;
//        });
//    return std::distance(gameObjectArchive.Transform2DComponentList.begin(), it);
//}
//
//uint32 GameObject_FindInputComponentIndex(uint gameObjectId)
//{
//    auto it = std::find(gameObjectArchive.InputComponentList.begin(), gameObjectArchive.InputComponentList.end(), [gameObjectId](const InputComponent& inputComponent)
//        {
//            return inputComponent.GameObjectId == gameObjectId;
//        });
//    return std::distance(gameObjectArchive.InputComponentList.begin(), it);
//}

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
    if (gameObjectId == 0 || gameObjectId > gameObjectArchive.GameObjectList.size())
    {
        return;
    }

    const GameObject& obj = gameObjectArchive.GameObjectList[gameObjectId];
    if (obj.GameObjectComponentMask & kTransform2DComponent)
    {
        gameObjectArchive.Transform2DComponentList.erase(gameObjectArchive.Transform2DComponentList.begin() + gameObjectId);
        for (int x = gameObjectId; x < gameObjectArchive.Transform2DComponentList.size(); x++)
        {
            gameObjectArchive.Transform2DComponentList[x].GameObjectId--;
        }
    }

    if (obj.GameObjectComponentMask & kSpriteComponent)
    {
        spriteSystem.SpriteList.erase(spriteSystem.SpriteList.begin() + gameObjectId);
        for (int x = gameObjectId; x < spriteSystem.SpriteList.size(); x++)
        {
            spriteSystem.SpriteList[x].GameObjectId--;
        }
    }

    if (obj.GameObjectComponentMask & kInputComponent)
    {
        gameObjectArchive.InputComponentList.erase(gameObjectArchive.InputComponentList.begin() + gameObjectId);
        for (int x = gameObjectId; x < gameObjectArchive.InputComponentList.size(); x++)
        {
            gameObjectArchive.InputComponentList[x].GameObjectId--;
        }
    }

    if (gameObjectArchive.ComponentBehaviorMap[obj.GameObjectType].Destroy)
    {
        gameObjectArchive.ComponentBehaviorMap[obj.GameObjectType].Destroy(gameObjectId);
    }

    gameObjectArchive.GameObjectList.erase(gameObjectArchive.GameObjectList.begin() + gameObjectId);
    for (int x = gameObjectId; x < gameObjectArchive.GameObjectList.size(); x++)
    {
        if (gameObjectArchive.GameObjectList[x].ParentGameObjectId > gameObjectId)
        {
            gameObjectArchive.GameObjectList[x].ParentGameObjectId--;
        }
        if (gameObjectArchive.GameObjectList[x].InputComponentId != UINT32_MAX) gameObjectArchive.GameObjectList[x].InputComponentId--;
        if (gameObjectArchive.GameObjectList[x].SpriteComponentId != UINT32_MAX) gameObjectArchive.GameObjectList[x].SpriteComponentId--;
        if (gameObjectArchive.GameObjectList[x].Transform2DComponentId != UINT32_MAX) gameObjectArchive.GameObjectList[x].Transform2DComponentId--;
        gameObjectArchive.GameObjectList[x].GameObjectId--;
    }
}

void GameObject_DestroyGameObjects()
{
    gameObjectArchive.Transform2DComponentList.clear();
    gameObjectArchive.InputComponentList.clear();
    gameObjectArchive.GameObjectList.clear();
}

void GameObject_DestroyDeadGameObjects()
{
    if (gameObjectArchive.GameObjectList.empty())
    {
        return;
    }

    Vector<SharedPtr<GameObject>> deadGameObjects;
    for (auto& gameObject : gameObjectArchive.GameObjectList)
    {
        if (!gameObject.GameObjectAlive)
        {
            deadGameObjects.push_back(std::make_shared<GameObject>(gameObject));
        }
    }
    while(!deadGameObjects.empty())
    {
        GameObject_DestroyGameObject(deadGameObjects[0]->GameObjectId);
        deadGameObjects.erase(deadGameObjects.begin());
    }
}

uint32 GetNextGameObjectIndex()
{
    if (!gameObjectArchive.FreeGameObjectIndices.empty())
    {
        uint index = gameObjectArchive.FreeGameObjectIndices.back();
        gameObjectArchive.FreeGameObjectIndices.pop_back();

        for (int x = index; x < gameObjectArchive.GameObjectList.size(); x++)
        {
            GameObject& gameObject = GameObject_FindGameObject(index);

            if (gameObject.GameObjectComponentMask & kTransform2DComponent)
            {
                gameObjectArchive.Transform2DComponentList[x].GameObjectId++;
            }

            if (gameObject.GameObjectComponentMask & kSpriteComponent)
            {
                spriteSystem.SpriteList[x].GameObjectId++;
            }

            if (gameObject.GameObjectComponentMask & kInputComponent)
            {
                gameObjectArchive.InputComponentList[x].GameObjectId++;
            }

            if (gameObjectArchive.GameObjectList[x].ParentGameObjectId > index)
            {
                gameObjectArchive.GameObjectList[x].ParentGameObjectId++;
            }
            if (gameObjectArchive.GameObjectList[x].InputComponentId != UINT64_MAX) gameObjectArchive.GameObjectList[x].InputComponentId++;
            if (gameObjectArchive.GameObjectList[x].SpriteComponentId != UINT64_MAX) gameObjectArchive.GameObjectList[x].SpriteComponentId++;
            if (gameObjectArchive.GameObjectList[x].Transform2DComponentId != UINT64_MAX) gameObjectArchive.GameObjectList[x].Transform2DComponentId++;
            gameObjectArchive.GameObjectList[x].GameObjectId++;
        }

        return index;
    }
    return gameObjectArchive.GameObjectList.size();
}

void* GameObject_LoadObjectData(GameObjectTypeEnum gameObjectType)
{
    switch (gameObjectType)
    {
        case GameObjectTypeEnum::kGameObjectMegaMan:
        {
            MegaManObject megaManObject;
            MegaManObject* megaManObjectPtr = memorySystem.AddPtrBuffer<MegaManObject>(megaManObject, __FILE__, __LINE__, __func__);
            return static_cast<void*>(megaManObjectPtr);
        }
        case GameObjectTypeEnum::kGameObjectMegaManShot:
        {
            MegaManShot megaManObject;
            MegaManShot* megaManObjectPtr = memorySystem.AddPtrBuffer<MegaManShot>(megaManObject, __FILE__, __LINE__, __func__);
            return static_cast<void*>(megaManObjectPtr);
        }
    }
}

void GameObject_LoadComponentTable(GameObject& gameObject, vec2& objectPosition, VkGuid& vramId)
{
    uint64 mask = gameObject.GameObjectComponentMask;
    if (mask & kTransform2DComponent)
    {
        gameObject.Transform2DComponentId = gameObjectArchive.Transform2DComponentList.size();
        gameObjectArchive.Transform2DComponentList.emplace_back(Transform2DComponent
            {
                .GameObjectId = gameObject.GameObjectId,
                .GameObjectPosition = objectPosition,
                .GameObjectRotation = vec2(),
                .GameObjectScale = vec2()
            });
    }
    if (mask & kInputComponent)
    {
        gameObject.InputComponentId = gameObjectArchive.InputComponentList.size();
        gameObjectArchive.InputComponentList.emplace_back(InputComponent
            {
                .GameObjectId = gameObject.GameObjectId
            });
    }
    if (mask & kSpriteComponent)
    {
        gameObject.SpriteComponentId = spriteSystem.SpriteList.size();
        Sprite_AddSprite(gameObject, vramId);
    }
}
