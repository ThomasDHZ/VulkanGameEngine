#include "pch.h"
#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "MegaManObject.h"
#include "LevelSystem.h"

GameObjectSystem& gameObjectSystem = GameObjectSystem::Get();

uint32 GameObjectSystem::AllocateGameObject()
{
    if (!FreeGameObjectIndex.empty())
    {
        uint32 index = FreeGameObjectIndex.back();
        FreeGameObjectIndex.pop_back();
        return index;
    }
    return GameObjectList.size();
}

void GameObjectSystem::CreateGameObject(const String& gameObjectJson, vec2 gameObjectPosition, uint32 parentGameObjectId)
{
    GameObject& gameObject = GameObjectList.emplace_back(GameObject
        {
            .GameObjectId = AllocateGameObject(),
            .ParentGameObjectId = parentGameObjectId,
            .GameObjectComponents = levelSystem.EntityRegistry.create(),
        });
    levelSystem.EntityRegistry.emplace<GameObjectComponentLinker>(gameObject.GameObjectComponents, GameObjectComponentLinker
        {
            .GameObjectId = static_cast<uint32>(gameObject.GameObjectId)
        });

    nlohmann::json json = fileSystem.LoadJsonFile(gameObjectJson.c_str());
    switch (json["GameObjectType"].get<GameObjectTypeEnum>())
    {
        case kGameObjectMegaMan:     levelSystem.EntityRegistry.emplace<MegaManObject>(gameObject.GameObjectComponents, MegaManObject{ }); break;
        case kGameObjectMegaManShot: levelSystem.EntityRegistry.emplace<MegaManShot>(gameObject.GameObjectComponents, MegaManShot{ }); break;
    }
    for (const auto& componentJson : json["GameObjectComponentList"])
    {
        uint64 componentType = componentJson["ComponentType"].get<uint64>();
        switch (componentType)
        {
            case kInputComponent: levelSystem.EntityRegistry.emplace<InputComponent>(gameObject.GameObjectComponents, InputComponent{ }); break;
            case kSpriteComponent:
            {
                nlohmann::json json = json.parse(componentJson.dump().c_str());
                VkGuid vramId = VkGuid(json["VramId"].get<String>().c_str());
                spriteSystem.CreateSprite(gameObject, vramId);
                break;
            }
            case kTransform2DComponent:
            {
                nlohmann::json json = json.parse(componentJson.dump().c_str());
                levelSystem.EntityRegistry.emplace<Transform2DComponent>(gameObject.GameObjectComponents, Transform2DComponent
                    {
                        .GameObjectPosition = gameObjectPosition,
                        .GameObjectRotation = vec2{ json["GameObjectRotation"][0], json["GameObjectRotation"][1] },
                        .GameObjectScale = vec2{ json["GameObjectScale"][0], json["GameObjectScale"][1] }
                    });
                break;
            }
            case kCameraFollowComponent: levelSystem.EntityRegistry.emplace<CameraFollowComponent>(gameObject.GameObjectComponents, CameraFollowComponent{ }); break;
        }
    }
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

GameObject& GameObjectSystem::FindGameObject(uint gameObjectId)
{
    auto it = std::find_if(GameObjectList.begin(), GameObjectList.end(),
        [gameObjectId](const GameObject& gameObject) {
            return gameObject.GameObjectId == gameObjectId;
        }
    );
    return *it;
}

void GameObjectSystem::DestroyGameObject(uint gameObjectId)
{
    GameObject& gameObject = GameObjectList[gameObjectId];
    FreeGameObjectIndex.push_back(gameObjectId);

    if (levelSystem.EntityRegistry.all_of<Sprite>(gameObject.GameObjectComponents)) spriteSystem.Destroy(levelSystem.EntityRegistry.get<Sprite>(gameObject.GameObjectComponents));

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