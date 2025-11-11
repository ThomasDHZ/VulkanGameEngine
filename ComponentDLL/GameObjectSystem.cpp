#include "pch.h"
#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "MemorySystem.h"
#include "MegaManObject.h"
#include "Component.h"

GameObjectSystem gameObjectSystem = GameObjectSystem();

uint32 GameObject_GetNextGameObjectIndex()
{
    if (!gameObjectSystem.FreeGameObjectIndices.empty())
    {
        uint index = gameObjectSystem.FreeGameObjectIndices.back();
        gameObjectSystem.FreeGameObjectIndices.pop_back();

        for (int x = index; x < gameObjectSystem.GameObjectList.size(); x++)
        {
            GameObject& gameObject = GameObjectSystem_FindGameObject(index);

            if (gameObject.GameObjectComponentMask & kTransform2DComponent)
            {
                gameObjectSystem.Transform2DComponentList[x].GameObjectId++;
            }

            if (gameObject.GameObjectComponentMask & kSpriteComponent)
            {
                spriteSystem.SpriteList[x].GameObjectId++;
            }

            if (gameObject.GameObjectComponentMask & kInputComponent)
            {
                gameObjectSystem.InputComponentList[x].GameObjectId++;
            }

            if (gameObjectSystem.GameObjectList[x].ParentGameObjectId > index)
            {
                gameObjectSystem.GameObjectList[x].ParentGameObjectId++;
            }
            if (gameObjectSystem.GameObjectList[x].InputComponentId != UINT64_MAX) gameObjectSystem.GameObjectList[x].InputComponentId++;
            if (gameObjectSystem.GameObjectList[x].SpriteComponentId != UINT64_MAX) gameObjectSystem.GameObjectList[x].SpriteComponentId++;
            if (gameObjectSystem.GameObjectList[x].Transform2DComponentId != UINT64_MAX) gameObjectSystem.GameObjectList[x].Transform2DComponentId++;
            gameObjectSystem.GameObjectList[x].GameObjectId++;
        }

        return index;
    }
    return gameObjectSystem.GameObjectList.size();
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
        gameObject.Transform2DComponentId = gameObjectSystem.Transform2DComponentList.size();
        gameObjectSystem.Transform2DComponentList.emplace_back(Transform2DComponent
            {
                .GameObjectId = gameObject.GameObjectId,
                .GameObjectPosition = objectPosition,
                .GameObjectRotation = vec2(),
                .GameObjectScale = vec2()
            });
    }
    if (mask & kInputComponent)
    {
        gameObject.InputComponentId = gameObjectSystem.InputComponentList.size();
        gameObjectSystem.InputComponentList.emplace_back(InputComponent
            {
                .GameObjectId = gameObject.GameObjectId
            });
    }
    if (mask & kSpriteComponent)
    {
        gameObject.SpriteComponentId = spriteSystem.SpriteList.size();
        spriteSystem.AddSprite(gameObject, vramId);
    }
}

void GameObjectSystem_CreateGameObjectFromJson(const char* gameObjectPath, vec2 gameObjectPosition)
{
    GameObject& gameObject = gameObjectSystem.GameObjectList.emplace_back(GameObject
        {
            .GameObjectType = GameObjectTypeEnum::kGameObjectMegaMan,
            .GameObjectId = static_cast<uint32>(gameObjectSystem.GameObjectList.size()),
            .GameObjectData = GameObject_LoadObjectData(GameObjectTypeEnum::kGameObjectMegaMan),
        });

    nlohmann::json json = File_LoadJsonFile(gameObjectPath);
    for (const auto& componentJson : json["GameObjectComponentList"])
    {
        uint64 componentType = componentJson["ComponentType"].get<uint64>();
        gameObject.GameObjectComponentMask |= (1ULL << componentType);
        switch (componentType)
        {
            case kInputComponent: gameObject.InputComponentId = GameObjectSystem_LoadInputComponent(componentJson.dump().c_str(), gameObject.GameObjectId); break;
            case kSpriteComponent: gameObject.SpriteComponentId = GameObjectSystem_LoadSpriteComponent(componentJson.dump().c_str(), gameObject); break;
            case kTransform2DComponent: gameObject.Transform2DComponentId = GameObjectSystem_LoadTransformComponent(componentJson.dump().c_str(), gameObject.GameObjectId, gameObjectPosition); break;
        }
    }
    GameObject_LoadComponentBehavior(gameObject, json["GameObjectType"]);
}

void GameObjectSystem_CreateGameObject(const char* name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, uint64 gameObjectComponentMask, VkGuid vramId, vec2 objectPosition)
{
    GameObject& gameObject = gameObjectSystem.GameObjectList.emplace_back(GameObject
        {
            .GameObjectType = GameObjectTypeEnum::kGameObjectMegaManShot,
            .GameObjectComponentMask = gameObjectComponentMask,
            .GameObjectId = static_cast<uint32>(gameObjectSystem.GameObjectList.size()),
            .ParentGameObjectId = parentGameObjectId,
            .GameObjectData = GameObject_LoadObjectData(GameObjectTypeEnum::kGameObjectMegaManShot)
        });
    GameObject_LoadComponentTable(gameObject, objectPosition, vramId);
    GameObject_LoadComponentBehavior(gameObject, objectEnum);
}

void GameObjectSystem_Update(const float& deltaTime)
{
    for (auto& gameObject : gameObjectSystem.GameObjectList)
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
    if (!GameObject_GameObjectBehaviorExists(objectEnum))
    {
        switch (objectEnum)
        {
        case kGameObjectMegaMan: MegaMan_Behaviors(componentBehavior); break;
        case kGameObjectMegaManShot: MegaManShot_Behaviors(componentBehavior); break;
        }
        gameObjectSystem.ComponentBehaviorMap[objectEnum] = componentBehavior;
    }
}

uint GameObjectSystem_LoadTransformComponent(const char* jsonString, uint gameObjectId, const vec2& gameObjectPosition)
{
    nlohmann::json json = json.parse(jsonString);
    gameObjectSystem.Transform2DComponentList.emplace_back(Transform2DComponent
        {
            .GameObjectId = gameObjectId,
            .GameObjectPosition = gameObjectPosition,
            .GameObjectRotation = vec2{ json["GameObjectRotation"][0], json["GameObjectRotation"][1] },
            .GameObjectScale = vec2{ json["GameObjectScale"][0], json["GameObjectScale"][1] }
        });
    return gameObjectId;
}

uint GameObjectSystem_LoadInputComponent(const char* jsonString, uint gameObjectId)
{
    nlohmann::json json = json.parse(jsonString);
    gameObjectSystem.InputComponentList.emplace_back(InputComponent
        {
            .GameObjectId = gameObjectId
        });
    return gameObjectId;
}

uint GameObjectSystem_LoadSpriteComponent(const char* jsonString, GameObject& gameObject)
{
    nlohmann::json json = json.parse(jsonString);
    VkGuid vramId = VkGuid(json["VramId"].get<String>().c_str());
    spriteSystem.AddSprite(gameObject, vramId);
    return gameObject.GameObjectId;
}

GameObject& GameObjectSystem_FindGameObject(uint gameObjectId)
{
    auto it = std::find_if(gameObjectSystem.GameObjectList.begin(), gameObjectSystem.GameObjectList.end(),
        [gameObjectId](const GameObject& gameObject) {
            return gameObject.GameObjectId == gameObjectId;
        }
    );
    return *it;
}

bool GameObject_GameObjectBehaviorExists(const GameObjectTypeEnum objectEnum)
{
    return gameObjectSystem.ComponentBehaviorMap.contains(objectEnum);
}

GameObjectBehavior& GameObject_FindGameObjectBehavior(const GameObjectTypeEnum& id)
{
    return gameObjectSystem.ComponentBehaviorMap.at(id);
}

Transform2DComponent GameObjectSystem_FindTransform2DComponent(uint gameObjectId)
{
    auto it = std::find_if(gameObjectSystem.Transform2DComponentList.begin(), gameObjectSystem.Transform2DComponentList.end(),
        [gameObjectId](const Transform2DComponent& transformComponent) {
            return transformComponent.GameObjectId == gameObjectId;
        }
    );
    return *it;
}

InputComponent GameObjectSystem_FindInputComponent(uint gameObjectId)
{
    auto it = std::find_if(gameObjectSystem.InputComponentList.begin(), gameObjectSystem.InputComponentList.end(),
        [gameObjectId](const InputComponent& inputComponent) {
            return inputComponent.GameObjectId == gameObjectId;
        }
    );
    return *it;
}

Vector<GameObject> GameObject_FindGameObjectByType(const GameObjectTypeEnum& gameObjectType)
{
    Vector<GameObject> gameObjectList;
    std::copy_if(gameObjectSystem.GameObjectList.begin(), gameObjectSystem.GameObjectList.end(), std::back_inserter(gameObjectList),
        [gameObjectType](const GameObject& gameObject)
        {
            return gameObject.GameObjectType == gameObjectType;
        }
    );
    return gameObjectList;
}

GameObject* GameObjectSystem_GameObjectList(int& outCount)
{
    outCount = static_cast<int>(gameObjectSystem.GameObjectList.size());
    return memorySystem.AddPtrBuffer<GameObject>(gameObjectSystem.GameObjectList.data(), gameObjectSystem.GameObjectList.size(), __FILE__, __LINE__, __func__);
}

Transform2DComponent* GameObject_Transform2DComponentList(int& outCount)
{
    outCount = static_cast<int>(gameObjectSystem.Transform2DComponentList.size());
    return memorySystem.AddPtrBuffer<Transform2DComponent>(gameObjectSystem.Transform2DComponentList.data(), gameObjectSystem.Transform2DComponentList.size(), __FILE__, __LINE__, __func__);
}

InputComponent* GameObject_InputComponentList(int& outCount)
{
    outCount = static_cast<int>(gameObjectSystem.InputComponentList.size());
    return memorySystem.AddPtrBuffer<InputComponent>(gameObjectSystem.InputComponentList.data(), gameObjectSystem.InputComponentList.size(), __FILE__, __LINE__, __func__);
}

void GameObjectSystem_DestroyGameObject(uint gameObjectId)
{
    if (gameObjectId == 0 || gameObjectId > gameObjectSystem.GameObjectList.size())
    {
        return;
    }

    const GameObject& obj = gameObjectSystem.GameObjectList[gameObjectId];
    if (obj.GameObjectComponentMask & kTransform2DComponent)
    {
        gameObjectSystem.Transform2DComponentList.erase(gameObjectSystem.Transform2DComponentList.begin() + gameObjectId);
        for (int x = gameObjectId; x < gameObjectSystem.Transform2DComponentList.size(); x++)
        {
            gameObjectSystem.Transform2DComponentList[x].GameObjectId--;
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
        gameObjectSystem.InputComponentList.erase(gameObjectSystem.InputComponentList.begin() + gameObjectId);
        for (int x = gameObjectId; x < gameObjectSystem.InputComponentList.size(); x++)
        {
            gameObjectSystem.InputComponentList[x].GameObjectId--;
        }
    }

    if (gameObjectSystem.ComponentBehaviorMap[obj.GameObjectType].Destroy)
    {
        gameObjectSystem.ComponentBehaviorMap[obj.GameObjectType].Destroy(gameObjectId);
    }

    gameObjectSystem.GameObjectList.erase(gameObjectSystem.GameObjectList.begin() + gameObjectId);
    for (int x = gameObjectId; x < gameObjectSystem.GameObjectList.size(); x++)
    {
        if (gameObjectSystem.GameObjectList[x].ParentGameObjectId > gameObjectId)
        {
            gameObjectSystem.GameObjectList[x].ParentGameObjectId--;
        }
        if (gameObjectSystem.GameObjectList[x].InputComponentId != UINT32_MAX) gameObjectSystem.GameObjectList[x].InputComponentId--;
        if (gameObjectSystem.GameObjectList[x].SpriteComponentId != UINT32_MAX) gameObjectSystem.GameObjectList[x].SpriteComponentId--;
        if (gameObjectSystem.GameObjectList[x].Transform2DComponentId != UINT32_MAX) gameObjectSystem.GameObjectList[x].Transform2DComponentId--;
        gameObjectSystem.GameObjectList[x].GameObjectId--;
    }
}

void GameObjectSystem_DestroyGameObjects()
{
    gameObjectSystem.Transform2DComponentList.clear();
    gameObjectSystem.InputComponentList.clear();
    gameObjectSystem.GameObjectList.clear();
}

void GameObjectSystem_DestroyDeadGameObjects()
{
    if (gameObjectSystem.GameObjectList.empty())
    {
        return;
    }

    Vector<SharedPtr<GameObject>> deadGameObjects;
    for (auto& gameObject : gameObjectSystem.GameObjectList)
    {
        if (!gameObject.GameObjectAlive)
        {
            deadGameObjects.push_back(std::make_shared<GameObject>(gameObject));
        }
    }
    while (!deadGameObjects.empty())
    {
        gameObjectSystem.DestroyGameObject(deadGameObjects[0]->GameObjectId);
        deadGameObjects.erase(deadGameObjects.begin());
    }
}