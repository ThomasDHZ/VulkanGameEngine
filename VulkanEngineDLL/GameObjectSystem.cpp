#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "LevelSystem.h"
#include "LuaScriptingSystem.h"
#include "CSharpScriptSystem.h"

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

uint GameObjectSystem::CreateGameObject(vec2 gameObjectPosition, uint32 parentGameObjectId)
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
    return gameObject.GameObjectId;
}

uint GameObjectSystem::CreateGameObject(const String& gameObjectJson, vec2 gameObjectPosition, uint32 parentGameObjectId)
{
    nlohmann::json json = fileSystem.LoadJsonFile(gameObjectJson.c_str());
    GameObject gameObject = GameObjectList.emplace_back(GameObject
        {
            .GameObjectId = AllocateGameObject(),
            .ParentGameObjectId = parentGameObjectId,
            .GameObjectComponents = levelSystem.EntityRegistry.create(),
        });

    if (json.contains("GameObjectDLL") && json.contains("GameObjectDLLType"))
    {
        String dllPath = json["GameObjectDLL"].get<String>();
        String dllType = json["GameObjectDLLType"].get<String>();

        if (!GameObjectBehaviorExists(dllType))
        {
            cSharpScriptSystem.LoadGameObjectScript(dllPath, dllType);
        }

        if (GameObjectBehaviorExists(dllType))
        {
            auto& behavior = gameObjectSystem.GameObjectBehaviorMap[dllType];

            gameObject.GameObjectBehaviorKey = dllType;
            gameObject.ObjectPtr = behavior.CreateObject();
        }
        else
        {
            std::cerr << "[GameObject] Failed to load C# behavior: " << dllType << std::endl;
        }
    }
    else if (json.contains("GameObjectLuaScript"))
    {
        String luaPath = json["GameObjectLuaScript"].get<String>();

        if (fileSystem.GetFileExtention(luaPath.c_str()) == "lua")
        {
            gameObject.GameObjectBehaviorKey = luaPath;  // Store lua path as key

            // TODO: Load Lua script and store table in ScriptComponent
            // luaScriptingSystem.CreateEntityFromScript(luaPath, gameObject.GameObjectId);

            std::cout << "[GameObject] Created Lua object: " << luaPath << std::endl;
        }
    }
    GameObjectList.emplace_back(gameObject);
    levelSystem.EntityRegistry.emplace<GameObjectComponentLinker>(gameObject.GameObjectComponents, GameObjectComponentLinker
        {
            .GameObjectId = static_cast<uint32>(gameObject.GameObjectId)
        });

    //for (const auto& componentJson : json["GameObjectComponentList"])
    //{
    //    uint64 componentType = componentJson["ComponentType"].get<uint64>();
    //    switch (componentType)
    //    {
    //    case kInputComponent: levelSystem.EntityRegistry.emplace<InputComponent>(gameObject.GameObjectComponents, InputComponent{ }); break;
    //    case kSpriteComponent:
    //    {
    //        nlohmann::json json = json.parse(componentJson.dump().c_str());
    //        VkGuid vramId = VkGuid(json["VramId"].get<String>().c_str());
    //        spriteSystem.CreateSprite(gameObject, vramId);
    //        break;
    //    }
    //    case kTransform2DComponent:
    //    {
    //        nlohmann::json json = json.parse(componentJson.dump().c_str());
    //        levelSystem.EntityRegistry.emplace<Transform2DComponent>(gameObject.GameObjectComponents, Transform2DComponent
    //            {
    //                .GameObjectPosition = gameObjectPosition,
    //                .GameObjectRotation = vec2{ json["GameObjectRotation"][0], json["GameObjectRotation"][1] },
    //                .GameObjectScale = vec2{ json["GameObjectScale"][0], json["GameObjectScale"][1] }
    //            });
    //        break;
    //    }
    //    case kTransform3DComponent:
    //    {
    //        nlohmann::json json = json.parse(componentJson.dump().c_str());
    //        levelSystem.EntityRegistry.emplace<Transform3DComponent>(gameObject.GameObjectComponents, Transform3DComponent
    //            {
    //                .GameObjectPosition = vec3{ gameObjectPosition.x, gameObjectPosition.y, 0.0f },
    //                .GameObjectRotation = vec3{ json["GameObjectRotation"][0], json["GameObjectRotation"][1], 0.0f },
    //                .GameObjectScale = vec3{ json["GameObjectScale"][0], json["GameObjectScale"][1], 0.0f }
    //            });
    //        break;
    //    }
    //    case kCameraFollowComponent: levelSystem.EntityRegistry.emplace<CameraFollowComponent>(gameObject.GameObjectComponents, CameraFollowComponent{ }); break;
    //    case kDirectionalLightComponent: levelSystem.EntityRegistry.emplace<DirectionalLightComponent>(gameObject.GameObjectComponents, DirectionalLightComponent{ }); break;
    //    case kPointLightComponent: levelSystem.EntityRegistry.emplace<PointLightComponent>(gameObject.GameObjectComponents, PointLightComponent{ }); break;
    //    default:  std::cerr << "GameObjectComponent not implemented yet: " << componentType << std::endl;
    //    }
    //}

   // gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectBehaviorKey].Startup(gameObject.ObjectPtr);

    return gameObject.GameObjectId;
}

void GameObjectSystem::Update(const float& deltaTime)
{
    for (auto& gameObject : GameObjectList)
    {
        if(gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectBehaviorKey].Update)
        gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectBehaviorKey].Update(gameObject.ObjectPtr, deltaTime);
    }
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

const GameObjectBehavior GameObjectSystem::FindGameObjectBehavior(const String& gameObjectClass)
{
    auto it = GameObjectBehaviorMap.find(gameObjectClass);
    if (it == GameObjectBehaviorMap.end())
    {
        throw std::runtime_error("Game object behavior not found");
    }
    return it->second;
}

bool GameObjectSystem::GameObjectBehaviorExists(const String& gameObjectClass)
{
    return GameObjectBehaviorMap.contains(gameObjectClass);
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