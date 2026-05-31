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

void GameObjectSystem::LoadGameObjectTempletes(Vector<String>& gameObjectJson)
{
    Vector<String> uniqueJsonSet;
    std::unordered_set<String> seen;
    std::for_each(gameObjectJson.begin(), gameObjectJson.end(),
        [&](const String& str)
        {
            if (!str.empty() && seen.insert(str).second)
            {
                uniqueJsonSet.emplace_back(str);
            }
        });

    for (const auto& jsonString : uniqueJsonSet)
    {
        nlohmann::json json = fileSystem.LoadJsonFile(jsonString.c_str());
        GameObject gameObject = GameObjectList.emplace_back(GameObject
            {
                .GameObjectId = AllocateGameObject(),
                .GameObjectType = json["GameObjectType"]
            });

        if (json.contains("GameObjectDLL") && json.contains("GameObjectDLLType"))
        {
            String dllPath = json["GameObjectDLL"].get<String>();
            String dllType = json["GameObjectDLLType"].get<String>();
            if (!GameObjectBehaviorExists(gameObject.GameObjectType))
            {
                gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType] = cSharpScriptSystem.LoadGameObjectScript(dllPath, dllType);
            }

            if (GameObjectBehaviorExists(gameObject.GameObjectType))
            {
                gameObject.ObjectPtr = gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].CreateObject();
                gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].Startup(gameObject.ObjectPtr, gameObject.GameObjectId);
            }
            else
            {
                std::cerr << "[GameObject] Failed to load C# behavior: " << dllType << std::endl;
            }

            GameObjectStruct gameObjectStruct;
            if (json.contains("GameObjectVariableStruct"))
            {
                for (auto& gameObjectVar : json["GameObjectVariableStruct"])
                {
                    gameObjectStruct.GameObjectVariableMap[gameObjectVar["VariableName"]] = GameObjectVariable
                    {
                        .VariableName = gameObjectVar["VariableName"],
                        .MemberTypeEnum = gameObjectVar["MemberTypeEnum"],
                        .VariableByteSize = gameObjectVar["VariableByteSize"].get<size_t>(),
                        .ConstVariable = gameObjectVar["ConstVariable"]
                    };

                    if (!gameObjectVar.is_null())
                    {
                        float value = gameObjectVar["Value"].get<float>();
                        gameObjectStruct.GameObjectVariableMap[gameObjectVar["VariableName"]].Value.resize(gameObjectStruct.GameObjectVariableMap[gameObjectVar["VariableName"]].VariableByteSize);
                        std::memcpy(gameObjectStruct.GameObjectVariableMap[gameObjectVar["VariableName"]].Value.data(), &value, sizeof(float));
                    }
                }
            }
            GameObjectVarTemplateMap[gameObject.GameObjectType] = gameObjectStruct;
        }
        else if (json.contains("GameObjectLuaScript"))
        {
            String luaPath = json["GameObjectLuaScript"].get<String>();

            if (fileSystem.GetFileExtention(luaPath.c_str()) == "lua")
            {
                //   gameObject.GameObjectTypeNameString = luaPath;  // Store lua path as key

                   // TODO: Load Lua script and store table in ScriptComponent
                   // luaScriptingSystem.CreateEntityFromScript(luaPath, gameObject.GameObjectId);

                std::cout << "[GameObject] Created Lua object: " << luaPath << std::endl;
            }
        }
        GameObjectComponentTempleteMap = json["GameObjectComponentList"];
    }
}

//uint GameObjectSystem::CreateGameObject(vec2 gameObjectPosition, uint32 parentGameObjectId)
//{
//    GameObject& gameObject = GameObjectList.emplace_back(GameObject
//        {
//            .GameObjectId = AllocateGameObject(),
//            .ParentGameObjectId = parentGameObjectId,
//            .GameObjectComponents = EntityRegistry.create(),
//        });
//    EntityRegistry.emplace<GameObjectComponentLinker>(gameObject.GameObjectComponents, GameObjectComponentLinker
//        {
//            .GameObjectId = static_cast<uint32>(gameObject.GameObjectId)
//        });
//    return gameObject.GameObjectId;
//}
//
//uint GameObjectSystem::CreateGameObject(const String& gameObjectJson, vec2 gameObjectPosition, uint32 parentGameObjectId)
//{
//    nlohmann::json json = fileSystem.LoadJsonFile(gameObjectJson.c_str());
//    GameObject gameObject = GameObjectList.emplace_back(GameObject
//        {
//            .GameObjectId = AllocateGameObject(),
//            .ParentGameObjectId = parentGameObjectId,
//            .GameObjectType = json["GameObjectType"],
//            .GameObjectComponents = EntityRegistry.create(),
//        });
//
//    if (json.contains("GameObjectDLL") && json.contains("GameObjectDLLType"))
//    {
//        String dllPath = json["GameObjectDLL"].get<String>();
//        String dllType = json["GameObjectDLLType"].get<String>();
//        if (!GameObjectBehaviorExists(gameObject.GameObjectType))
//        {
//            gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType] = cSharpScriptSystem.LoadGameObjectScript(dllPath, dllType);
//        }
//
//        if (GameObjectBehaviorExists(gameObject.GameObjectType))
//        {
//            gameObject.ObjectPtr = gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].CreateObject();
//            gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].Startup(gameObject.ObjectPtr, gameObject.GameObjectId);
//        }
//        else
//        {
//            std::cerr << "[GameObject] Failed to load C# behavior: " << dllType << std::endl;
//        }
//
//        GameObjectStruct gameObjectStruct;
//        if (json.contains("GameObjectVariableStruct"))
//        {
//            for (auto& gameObjectVar : json["GameObjectVariableStruct"])
//            {
//                gameObjectStruct.GameObjectVariableMap[gameObjectVar["VariableName"]] = GameObjectVariable
//                {
//                    .VariableName = gameObjectVar["VariableName"],
//                    .MemberTypeEnum = gameObjectVar["MemberTypeEnum"],
//                    .VariableByteSize = gameObjectVar["VariableByteSize"].get<size_t>(),
//                    .ConstVariable = gameObjectVar["ConstVariable"]
//                };
//
//                if (!gameObjectVar.is_null())
//                {
//                    float value = gameObjectVar["Value"].get<float>();
//                    gameObjectStruct.GameObjectVariableMap[gameObjectVar["VariableName"]].Value.resize(gameObjectStruct.GameObjectVariableMap[gameObjectVar["VariableName"]].VariableByteSize);
//                    std::memcpy(gameObjectStruct.GameObjectVariableMap[gameObjectVar["VariableName"]].Value.data(), &value, sizeof(float));
//                }
//            }
//        }
//        GameObjectVarTemplateMap[gameObject.GameObjectType] = gameObjectStruct;
//    }
//    else if (json.contains("GameObjectLuaScript"))
//    {
//        String luaPath = json["GameObjectLuaScript"].get<String>();
//
//        if (fileSystem.GetFileExtention(luaPath.c_str()) == "lua")
//        {
//         //   gameObject.GameObjectTypeNameString = luaPath;  // Store lua path as key
//
//            // TODO: Load Lua script and store table in ScriptComponent
//            // luaScriptingSystem.CreateEntityFromScript(luaPath, gameObject.GameObjectId);
//
//            std::cout << "[GameObject] Created Lua object: " << luaPath << std::endl;
//        }
//    }
//    GameObjectList.emplace_back(gameObject);
//    EntityRegistry.emplace<GameObjectComponentLinker>(gameObject.GameObjectComponents, GameObjectComponentLinker
//        {
//            .GameObjectId = static_cast<uint32>(gameObject.GameObjectId)
//        });
//
//    for (const auto& componentJson : json["GameObjectComponentList"])
//    {
//        uint64 componentType = componentJson["ComponentType"].get<uint64>();
//        switch (componentType)
//        {
//            case kInputComponent: EntityRegistry.emplace<InputComponent>(gameObject.GameObjectComponents, InputComponent{ }); break;
//            case kSpriteComponent:
//            {
//                nlohmann::json json = json.parse(componentJson.dump().c_str());
//                VkGuid vramId = VkGuid(json["VramId"].get<String>().c_str());
//                spriteSystem.CreateSprite(gameObject, vramId);
//                break;
//            }
//            case kTransform2DComponent:
//            {
//                nlohmann::json json = json.parse(componentJson.dump().c_str());
//                EntityRegistry.emplace<Transform2DComponent>(gameObject.GameObjectComponents, Transform2DComponent
//                    {
//                        .GameObjectPosition = gameObjectPosition,
//                        .GameObjectRotation = vec2{ json["GameObjectRotation"][0], json["GameObjectRotation"][1] },
//                        .GameObjectScale = vec2{ json["GameObjectScale"][0], json["GameObjectScale"][1] }
//                    });
//                break;
//            }
//            case kTransform3DComponent:
//            {
//                nlohmann::json json = json.parse(componentJson.dump().c_str());
//                EntityRegistry.emplace<Transform3DComponent>(gameObject.GameObjectComponents, Transform3DComponent
//                    {
//                        .GameObjectPosition = vec3{ gameObjectPosition.x, gameObjectPosition.y, 0.0f },
//                        .GameObjectRotation = vec3{ json["GameObjectRotation"][0], json["GameObjectRotation"][1], 0.0f },
//                        .GameObjectScale = vec3{ json["GameObjectScale"][0], json["GameObjectScale"][1], 0.0f }
//                    });
//                break;
//            }
//            case kCameraFollowComponent: EntityRegistry.emplace<CameraFollowComponent>(gameObject.GameObjectComponents, CameraFollowComponent{ }); break;
//            case kDirectionalLightComponent: EntityRegistry.emplace<DirectionalLightComponent>(gameObject.GameObjectComponents, DirectionalLightComponent{ }); break;
//            case kPointLightComponent: EntityRegistry.emplace<PointLightComponent>(gameObject.GameObjectComponents, PointLightComponent{ }); break;
//            default:  std::cerr << "GameObjectComponent not implemented yet: " << componentType << std::endl;
//        }
//    }
//
//    if (GameObjectVarTemplateMap.contains(gameObject.GameObjectType))
//    {
//        EntityRegistry.emplace<GameObjectStruct>(gameObject.GameObjectComponents, GameObjectVarTemplateMap[gameObject.GameObjectType]);
//    }
//
//    return gameObject.GameObjectId;
//}

uint GameObjectSystem::CreateGameObject(GameObjectTypeEnum gameObjectType, vec2 gameObjectPosition, uint32 parentGameObjectId)
{
    GameObject gameObject = GameObject
    {
        .GameObjectId = AllocateGameObject(),
        .ParentGameObjectId = parentGameObjectId,
        .GameObjectType = gameObjectType,
        .GameObjectComponents = EntityRegistry.create(),
        .ObjectPtr = gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].CreateObject()
    };
    EntityRegistry.emplace<GameObjectComponentLinker>(gameObject.GameObjectComponents, GameObjectComponentLinker
        {
            .GameObjectId = static_cast<uint32>(gameObject.GameObjectId)
        });

    nlohmann::json json = GameObjectComponentTempleteMap[gameObjectType];
    for (const auto& componentJson : json["GameObjectComponentList"])
    {
        uint64 componentType = componentJson["ComponentType"].get<uint64>();
        switch (componentType)
        {
            case kInputComponent: EntityRegistry.emplace<InputComponent>(gameObject.GameObjectComponents, InputComponent{ }); break;
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
                EntityRegistry.emplace<Transform2DComponent>(gameObject.GameObjectComponents, Transform2DComponent
                    {
                        .GameObjectPosition = gameObjectPosition,
                        .GameObjectRotation = vec2{ json["GameObjectRotation"][0], json["GameObjectRotation"][1] },
                        .GameObjectScale = vec2{ json["GameObjectScale"][0], json["GameObjectScale"][1] }
                    });
                break;
            }
            case kTransform3DComponent:
            {
                nlohmann::json json = json.parse(componentJson.dump().c_str());
                EntityRegistry.emplace<Transform3DComponent>(gameObject.GameObjectComponents, Transform3DComponent
                    {
                        .GameObjectPosition = vec3{ gameObjectPosition.x, gameObjectPosition.y, 0.0f },
                        .GameObjectRotation = vec3{ json["GameObjectRotation"][0], json["GameObjectRotation"][1], 0.0f },
                        .GameObjectScale = vec3{ json["GameObjectScale"][0], json["GameObjectScale"][1], 0.0f }
                    });
                break;
            }
            case kCameraFollowComponent: EntityRegistry.emplace<CameraFollowComponent>(gameObject.GameObjectComponents, CameraFollowComponent{ }); break;
            case kDirectionalLightComponent: EntityRegistry.emplace<DirectionalLightComponent>(gameObject.GameObjectComponents, DirectionalLightComponent{ }); break;
            case kPointLightComponent: EntityRegistry.emplace<PointLightComponent>(gameObject.GameObjectComponents, PointLightComponent{ }); break;
            default:  std::cerr << "GameObjectComponent not implemented yet: " << componentType << std::endl;
        }
    }
    return gameObject.GameObjectId;
}

void GameObjectSystem::Update(const float& deltaTime)
{
    for (auto& gameObject : GameObjectList)
    {
        if(gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].Update)
        gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].Update(gameObject.ObjectPtr, deltaTime);
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

const GameObjectBehavior GameObjectSystem::FindGameObjectBehavior(GameObjectTypeEnum gameObjectType)
{
    auto it = GameObjectBehaviorMap.find(gameObjectType);
    if (it == GameObjectBehaviorMap.end())
    {
        throw std::runtime_error("Game object behavior not found");
    }
    return it->second;
}

bool GameObjectSystem::GameObjectBehaviorExists(GameObjectTypeEnum gameObjectType)
{
    return GameObjectBehaviorMap.contains(gameObjectType);
}

void GameObjectSystem::DestroyGameObject(uint gameObjectId)
{
    GameObject& gameObject = GameObjectList[gameObjectId];
    FreeGameObjectIndex.push_back(gameObjectId);

    if (EntityRegistry.all_of<Sprite>(gameObject.GameObjectComponents)) spriteSystem.Destroy(EntityRegistry.get<Sprite>(gameObject.GameObjectComponents));

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