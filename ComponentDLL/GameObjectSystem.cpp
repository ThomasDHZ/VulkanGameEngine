#include "pch.h"
#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "MegaManObject.h"
#include "LevelSystem.h"

GameObjectSystem& gameObjectSystem = GameObjectSystem::Get();

uint32 GameObjectSystem::GetNextGameObjectIndex()
{
    if (!FreeGameObjectIndices.empty())
    {
        uint index = FreeGameObjectIndices.back();
        FreeGameObjectIndices.pop_back();

        for (int x = index; x < GameObjectList.size(); x++)
        {
            GameObject& gameObject = FindGameObject(index);
            GameObjectList[x].GameObjectId++;
        }

        return index;
    }
    return GameObjectList.size();
}

void* GameObjectSystem::LoadObjectData(GameObjectTypeEnum gameObjectType)
{
    switch (gameObjectType)
    {
    //case GameObjectTypeEnum::kGameObjectMegaMan:
    //{
    //    MegaManObject megaManObject;
    //    MegaManObject* megaManObjectPtr = memorySystem.AddPtrBuffer<MegaManObject>(megaManObject, __FILE__, __LINE__, __func__);
    //    return static_cast<void*>(megaManObjectPtr);
    //}
    case GameObjectTypeEnum::kGameObjectMegaManShot:
    {
        MegaManShot megaManObject;
        MegaManShot* megaManObjectPtr = memorySystem.AddPtrBuffer<MegaManShot>(megaManObject, __FILE__, __LINE__, __func__);
        return static_cast<void*>(megaManObjectPtr);
    }
    }
}

void GameObjectSystem::LoadComponentTable(GameObject& gameObject, vec2& objectPosition, VkGuid& vramId)
{
    //uint64 mask = gameObject.GameObjectComponentMask;
  //  if (mask & kTransform2DComponent)
  //  {
  //      //gameObject.Transform2DComponentId = Transform2DComponentList.size();
  //      //Transform2DComponentList.emplace_back(Transform2DComponent
  //      //    {
  //      //        .GameObjectPosition = objectPosition,
  //      //        .GameObjectRotation = vec2(),
  //      //        .GameObjectScale = vec2()
  //      //    });
  //  }
  //  if (mask & kInputComponent)
  //  {
  //      //gameObject.InputComponentId = InputComponentList.size();
  //      //InputComponentList.emplace_back(InputComponent
  //      //    {
  //      //        .GameObjectId = gameObject.GameObjectId
  //      //    });
  //  }
  //  if (mask & kSpriteComponent)
  //  {
  ///*      gameObject.SpriteComponentId = spriteSystem.SpriteList.size();
  //      spriteSystem.AddSprite(gameObject, vramId);*/
  //  }
}

void GameObjectSystem::CreateGameObject(const String& gameObjectJson, vec2 gameObjectPosition)
{
    uint32 gameObjectComponentId = GameObjectComponentList.size();
    entt::entity& entity = GameObjectComponentList.emplace_back(levelSystem.EntityRegistry.create());
    levelSystem.EntityRegistry.emplace<GameObjectComponentLinker>(entity, GameObjectComponentLinker
        {
            .GameObjectId = static_cast<uint32>(GameObjectList.size())
        });
    GameObject& gameObject = GameObjectList.emplace_back(GameObject
        {
            .GameObjectComponentIndex = gameObjectComponentId,
            //.GameObjectType = GameObjectTypeEnum::kGameObjectMegaMan,
           // .GameObjectId = static_cast<uint32>(GameObjectList.size()),
         //   .GameObjectData = LoadObjectData(GameObjectTypeEnum::kGameObjectMegaMan),
        });

    nlohmann::json json = fileSystem.LoadJsonFile(gameObjectJson.c_str());
    switch (json["GameObjectType"].get<GameObjectTypeEnum>())
    {
        case kGameObjectMegaMan:     levelSystem.EntityRegistry.emplace<MegaManObject>(entity, MegaManObject{ }); break;
        case kGameObjectMegaManShot: levelSystem.EntityRegistry.emplace<MegaManShot>(entity, MegaManShot{ }); break;
    }
    for (const auto& componentJson : json["GameObjectComponentList"])
    {
        uint64 componentType = componentJson["ComponentType"].get<uint64>();
        switch (componentType)
        {
            case kInputComponent: levelSystem.EntityRegistry.emplace<InputComponent>(entity, InputComponent{ }); break;
            case kSpriteComponent:
            {
                String s = componentJson.dump().c_str();
                nlohmann::json json = json.parse(componentJson.dump().c_str());
                VkGuid vramId = VkGuid(json["VramId"].get<String>().c_str());
                spriteSystem.CreateSprite(gameObject, vramId);
                break;
            }
            case kTransform2DComponent:
            {
                String s = componentJson.dump().c_str();
                nlohmann::json json = json.parse(componentJson.dump().c_str());
                levelSystem.EntityRegistry.emplace<Transform2DComponent>(GameObjectComponentList[gameObject.GameObjectComponentIndex], Transform2DComponent
                    {
                        .GameObjectPosition = gameObjectPosition,
                        .GameObjectRotation = vec2{ json["GameObjectRotation"][0], json["GameObjectRotation"][1] },
                        .GameObjectScale = vec2{ json["GameObjectScale"][0], json["GameObjectScale"][1] }
                    });
                break;
            }
        }
    }
    LoadComponentBehavior(gameObject, json["GameObjectType"]);
}

void GameObjectSystem::CreateGameObject(const String& name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, uint64 gameObjectComponentMask, VkGuid vramId, vec2 objectPosition)
{
    uint32 gameObjectComponentId = GameObjectComponentList.size();
    entt::entity& entity = GameObjectComponentList.emplace_back(levelSystem.EntityRegistry.create());
    levelSystem.EntityRegistry.emplace<GameObjectComponentLinker>(entity, GameObjectComponentLinker
        {
            .GameObjectId = static_cast<uint32>(GameObjectList.size())
        });
    GameObject& gameObject = GameObjectList.emplace_back(GameObject
        {
            .GameObjectId = static_cast<uint32>(GameObjectList.size()),
            .ParentGameObjectId = parentGameObjectId,
            .GameObjectComponentIndex = gameObjectComponentId,
        });
    LoadComponentTable(gameObject, objectPosition, vramId);
    LoadComponentBehavior(gameObject, objectEnum);
}

void GameObjectSystem::Update(const float& deltaTime)
{
    //for (auto& gameObject : GameObjectList)
    //{
    //    GameObjectBehavior gameObjectBehavior = FindGameObjectBehavior(gameObject.GameObjectType);
    //    if (gameObjectBehavior.Update)
    //    {
    //        gameObjectBehavior.Update(gameObject.GameObjectId, deltaTime);
    //    }
    //}
}

void GameObjectSystem::LoadComponentBehavior(GameObject& gameObject, GameObjectTypeEnum objectEnum)
{
    GameObjectBehavior componentBehavior;
    if (!GameObjectBehaviorExists(objectEnum))
    {
        //switch (objectEnum)
        //{
        //case kGameObjectMegaMan: MegaMan_Behaviors(componentBehavior); break;
        //case kGameObjectMegaManShot: MegaManShot_Behaviors(componentBehavior); break;
        //}
        ComponentBehaviorMap[objectEnum] = componentBehavior;
    }
}

uint GameObjectSystem::LoadTransformComponent(GameObject& gameObject, const char* jsonString, const vec2& gameObjectPosition)
{
    nlohmann::json json = json.parse(jsonString);
    levelSystem.EntityRegistry.emplace<Transform2DComponent>(GameObjectComponentList[gameObject.GameObjectComponentIndex], Transform2DComponent
        {
            .GameObjectPosition = gameObjectPosition,
            .GameObjectRotation = vec2{ json["GameObjectRotation"][0], json["GameObjectRotation"][1] },
            .GameObjectScale = vec2{ json["GameObjectScale"][0], json["GameObjectScale"][1] }
        });
    return gameObject.GameObjectId;
}

uint GameObjectSystem::LoadInputComponent(const char* jsonString, uint gameObjectId)
{
    nlohmann::json json = json.parse(jsonString);

    return gameObjectId;
}

uint GameObjectSystem::LoadSpriteComponent(const char* jsonString, GameObject& gameObject)
{
    nlohmann::json json = json.parse(jsonString);
    VkGuid vramId = VkGuid(json["VramId"].get<String>().c_str());
    //spriteSystem.AddSprite(gameObject, vramId);
    return gameObject.GameObjectId;
}

GameObject& GameObjectSystem::FindGameObject(uint gameObjectId)
{
    auto it = std::find_if(GameObjectList.begin(), GameObjectList.end(),
        [gameObjectId](const GameObject& gameObject) {
            return gameObject.GameObjectId == gameObjectId;
        }
    );
    return *it;
}

bool GameObjectSystem::GameObjectBehaviorExists(const GameObjectTypeEnum objectEnum)
{
    return ComponentBehaviorMap.contains(objectEnum);
}

GameObjectBehavior& GameObjectSystem::FindGameObjectBehavior(const GameObjectTypeEnum& id)
{
    return ComponentBehaviorMap.at(id);
}

Vector<GameObject> GameObjectSystem::FindGameObjectByType(const GameObjectTypeEnum& gameObjectType)
{
    Vector<GameObject> gameObjectList;
    //std::copy_if(GameObjectList.begin(), GameObjectList.end(), std::back_inserter(gameObjectList),
    //    [gameObjectType](const GameObject& gameObject)
    //    {
    //        return gameObject.GameObjectType == gameObjectType;
    //    }
    //);
    return gameObjectList;
}

Vector<GameObject> GameObjectSystem::GetGameObjectList()
{
    return GameObjectList;
}

void GameObjectSystem::DestroyGameObject(uint gameObjectId)
{
    if (gameObjectId == 0 || gameObjectId > GameObjectList.size())
    {
        return;
    }

 /*   const GameObject& obj = GameObjectList[gameObjectId];
    if (ComponentBehaviorMap[obj.GameObjectType].Destroy)
    {
        ComponentBehaviorMap[obj.GameObjectType].Destroy(gameObjectId);
    }*/

    GameObjectList.erase(GameObjectList.begin() + gameObjectId);
    for (int x = gameObjectId; x < GameObjectList.size(); x++)
    {
        if (GameObjectList[x].ParentGameObjectId > gameObjectId)
        {
            GameObjectList[x].ParentGameObjectId--;
        }
        GameObjectList[x].GameObjectId--;
    }
}

void GameObjectSystem::DestroyGameObjects()
{
    GameObjectList.clear();
}

void GameObjectSystem::DestroyDeadGameObjects()
{
    if (GameObjectList.empty())
    {
        return;
    }

    Vector<SharedPtr<GameObject>> deadGameObjects;
    for (auto& gameObject : GameObjectList)
    {
        if (!gameObject.GameObjectAlive)
        {
            deadGameObjects.push_back(std::make_shared<GameObject>(gameObject));
        }
    }
    while (!deadGameObjects.empty())
    {
        DestroyGameObject(deadGameObjects[0]->GameObjectId);
        deadGameObjects.erase(deadGameObjects.begin());
    }
}

// void GameObjectSystem_CreateGameObjectFromJson(const char* gameObjectPath, vec2 gameObjectPosition)
//{
//     GameObjectSystem_CreateGameObjectFromJson(gameObjectPath, gameObjectPosition);
//}
//
// void GameObjectSystem_CreateGameObject(const char* name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, uint64 gameObjectComponentMask, VkGuid vramId, vec2 objectPosition)
//{
//     GameObjectSystem_CreateGameObject(name, parentGameObjectId, objectEnum, gameObjectComponentMask, vramId, objectPosition);
//}
//
// void GameObjectSystem_Update(const float& deltaTime)
// {
//     GameObjectSystem_Update(deltaTime);
//}
//
// uint GameObjectSystem_LoadTransformComponent(const char* jsonString, uint gameObjectId, const vec2& gameObjectPosition)
//{
//     return GameObjectSystem_LoadTransformComponent(jsonString, gameObjectId, gameObjectPosition);
//}
//
// uint GameObjectSystem_LoadInputComponent(const char* jsonString, uint gameObjectId)
//{
//     return  GameObjectSystem_LoadInputComponent(jsonString, gameObjectId);
//}
//
// uint GameObjectSystem_LoadSpriteComponent(const char* jsonString, GameObject& gameObject)
//{
//     return  GameObjectSystem_LoadSpriteComponent(jsonString, gameObject);
//}
//
// void GameObjectSystem_DestroyGameObject(uint gameObjectId)
//{
//     GameObjectSystem_DestroyGameObject(gameObjectId);
//}
//
// void GameObjectSystem_DestroyGameObjects()
//{
//     GameObjectSystem_DestroyGameObjects();
//}
//
// void GameObjectSystem_DestroyDeadGameObjects()
//{
//     GameObjectSystem_DestroyDeadGameObjects();
//}
//
// GameObject& GameObjectSystem_FindGameObject(uint gameObjectId)
//{
//     return  GameObjectSystem_FindGameObject(gameObjectId);
//}
//
// Transform2DComponent GameObjectSystem_FindTransform2DComponent(uint gameObjectId)
//{
//     return  GameObjectSystem_FindTransform2DComponent(gameObjectId);
// }
//
// InputComponent GameObjectSystem_FindInputComponent(uint gameObjectId)
// {
//     return  GameObjectSystem_FindInputComponent(gameObjectId);
// }
//
// GameObject* GameObjectSystem_GetGameObjectList(int& outCount)
// {
//     outCount = static_cast<int>(gameObjectSystem.GameObjectList.size());
//     return memorySystem.AddPtrBuffer<GameObject>(gameObjectSystem.GameObjectList.data(), gameObjectSystem.GameObjectList.size(), __FILE__, __LINE__, __func__);
// }
//
// Transform2DComponent* GameObjectSystem_GetTransform2DComponentList(int& outCount)
// {
//     outCount = static_cast<int>(gameObjectSystem.Transform2DComponentList.size());
//     return memorySystem.AddPtrBuffer<Transform2DComponent>(gameObjectSystem.Transform2DComponentList.data(), gameObjectSystem.Transform2DComponentList.size(), __FILE__, __LINE__, __func__);
// }
//
// InputComponent* GameObjectSystem_GetInputComponentList(int& outCount)
// {
//     outCount = static_cast<int>(gameObjectSystem.InputComponentList.size());
//     return memorySystem.AddPtrBuffer<InputComponent>(gameObjectSystem.InputComponentList.data(), gameObjectSystem.InputComponentList.size(), __FILE__, __LINE__, __func__);
// }
