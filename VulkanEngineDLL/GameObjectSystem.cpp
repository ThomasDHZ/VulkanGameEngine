#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "LevelSystem.h"
#include "LightSystem.h"
#include "LuaScriptingSystem.h"
#include "CSharpScriptSystem.h"
#include "EngineConfigSystem.h"

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
        GameObject gameObject = GameObject
        {
            .GameObjectType = json["GameObjectType"]
        };

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

        if (json.contains("GameObjectMaterial")) materialSystem.LoadMaterial(json["GameObjectMaterial"]);
        if (json.contains("GameObjectSprite"))   spriteSystem.LoadSpriteVRAM(json);
        if (json.contains("GameObjectDLLType"))
        {
            String dllType = json["GameObjectDLLType"].get<String>();
            if (!GameObjectBehaviorExists(gameObject.GameObjectType)) gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType] = cSharpScriptSystem.LoadGameObjectScript(configSystem.GameScriptLibraryDLL, dllType);

        }
        else if (json.contains("GameObjectLuaScript"))
        {
            String luaPath = json["GameObjectLuaScript"].get<String>();
            if (fileSystem.GetFileExtention(luaPath.c_str()) == "lua")
            {
                luaScriptingSystem.CreateEntityFromScript(luaPath, "asdfad");
            }
        }

        const auto& componentList = json["GameObjectComponentList"];
        if (componentList.is_array())
        {
            GameObjectComponentTempleteMap[gameObject.GameObjectType] = componentList;
        }
        else if (componentList.is_object())
        {
            nlohmann::json wrappedArray = nlohmann::json::array();
            wrappedArray.push_back(componentList);
            GameObjectComponentTempleteMap[gameObject.GameObjectType] = wrappedArray;
        }
        GameObjectTempleteMap[gameObject.GameObjectType] = gameObject;
    }
}

void GameObjectSystem::CreateGameObjects(nlohmann::json& gameObjectJson)
{
    for (const auto& json : gameObjectJson)
    {
        vec2 positionOverride = vec2(0.0f);
        if (json.contains("GameObjectPositionOverride"))
        {
            positionOverride = vec2(json["GameObjectPositionOverride"][0], json["GameObjectPositionOverride"][1]);
        }
        CreateGameObject(json["GameObjectType"], positionOverride);
    }
}

uint GameObjectSystem::CreateGameObject(GameObjectTypeEnum gameObjectType, vec2 gameObjectPosition, uint32 parentGameObjectId)
{
    GameObject gameObject = GameObject
    {
        .GameObjectId = AllocateGameObject(),
        .ParentGameObjectId = parentGameObjectId,
        .GameObjectType = gameObjectType,
        .GameObjectComponents = EntityRegistry.create(),
        .ObjectPtr = gameObjectSystem.GameObjectBehaviorMap[gameObjectType].CreateObject ? gameObjectSystem.GameObjectBehaviorMap[gameObjectType].CreateObject() : 0
    };
    EntityRegistry.emplace<GameObjectComponentLinker>(gameObject.GameObjectComponents, GameObjectComponentLinker { .GameObjectId = static_cast<uint32>(gameObject.GameObjectId) });

    nlohmann::json gameObjectComponentJson = GameObjectComponentTempleteMap[gameObjectType];
    for (const auto& json : gameObjectComponentJson)
    {
        uint64 componentType = json["ComponentType"].get<uint64>();
        switch (componentType)
        {
            case kInputComponent: EntityRegistry.emplace<InputComponent>(gameObject.GameObjectComponents, InputComponent{ }); break;
            case kSpriteComponent:
            {
                VkGuid vramId = VkGuid(json["VramSpriteId"].get<String>().c_str());
                spriteSystem.CreateSprite(gameObject, vramId);
                break;
            }
            case kTransform2DComponent:
            {
                EntityRegistry.emplace<Transform2DComponent>(gameObject.GameObjectComponents, Transform2DComponent
                    {
                        .GameObjectPosition = gameObjectPosition,
                        .GameObjectRotation = vec2{ json["GameObjectRotation"][0], json["GameObjectRotation"][1] },
                        .GameObjectScale    = vec2{ json["GameObjectScale"][0], json["GameObjectScale"][1] }
                    });
                break;
            }
            case kTransform3DComponent:
            {
                EntityRegistry.emplace<Transform3DComponent>(gameObject.GameObjectComponents, Transform3DComponent
                    {
                        .GameObjectPosition = vec3{ gameObjectPosition.x, gameObjectPosition.y, 0.0f },
                        .GameObjectRotation = vec3{ json["GameObjectRotation"][0], json["GameObjectRotation"][1], 0.0f },
                        .GameObjectScale    = vec3{ json["GameObjectScale"][0], json["GameObjectScale"][1], 0.0f }
                    });
                break;
            }
            case kCollisionComponent:        EntityRegistry.emplace<Collider2DComponent>(gameObject.GameObjectComponents, Collider2DComponent
                {
                    .Size = vec2()
                }); break;

            case kCameraFollowComponent:     EntityRegistry.emplace<CameraFollowComponent>(gameObject.GameObjectComponents, CameraFollowComponent{ }); break;
            case kDirectionalLightComponent: EntityRegistry.emplace<DirectionalLightComponent>(gameObject.GameObjectComponents, lightSystem.GetDirectionalLight(lightSystem.LoadLight(json))); break;            
            case kPointLightComponent:       EntityRegistry.emplace<PointLightComponent>(gameObject.GameObjectComponents, lightSystem.GetPointLight(lightSystem.LoadLight(json))); break;
            case kDebugObjectComponent:      EntityRegistry.emplace<DebugObjectComponent>(gameObject.GameObjectComponents); break;
            default:  std::cerr << "GameObjectComponent not implemented yet: " << componentType << std::endl;
        }
    }
    if (GameObjectBehaviorMap.contains(gameObject.GameObjectType) &&
        gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].CreateObject)
    {
        if (gameObject.ObjectPtr) gameObject.ObjectPtr = gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].CreateObject();
        GameObjectBehaviorMap[gameObject.GameObjectType].Startup(gameObject.ObjectPtr, gameObject.GameObjectId, parentGameObjectId);
    }
    if (GameObjectVarTemplateMap.contains(gameObject.GameObjectType))
    {
        EntityRegistry.emplace<GameObjectStruct>(gameObject.GameObjectComponents, GameObjectVarTemplateMap[gameObject.GameObjectType]);
    }
    GameObjectList.emplace_back(gameObject);
    return gameObject.GameObjectId;
}

void GameObjectSystem::Update(const float& deltaTime)
{
    for (auto& gameObject : GameObjectList)
    {
        if(gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].Update) gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].Update(gameObject.ObjectPtr, deltaTime);
        luaScriptingSystem.Update(deltaTime);
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