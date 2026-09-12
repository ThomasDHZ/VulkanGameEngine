#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "LevelSystem.h"
#include "LightSystem.h"
#include "LuaScriptingSystem.h"
#include "CSharpScriptSystem.h"
#include "EngineConfigSystem.h"
#include "CollisionSystem.h"
#include "ComponentSystem.h"
#include "GameObjectComponentRegistry.h"
#include <unordered_set>

GameObjectSystem& gameObjectSystem = GameObjectSystem::Get();

entt::entity GameObjectSystem::CreateGameObject(nlohmann::json& gameObjectJson, entt::entity parentGameObjectId)
{
    vec2 positionOverride = vec2(0.0f);
    if (gameObjectJson.contains("GameObjectPositionOverride"))
    {
        positionOverride = vec2(gameObjectJson["GameObjectPositionOverride"][0], gameObjectJson["GameObjectPositionOverride"][1]);
    }
    return CreateGameObject(gameObjectJson["GameObjectType"], positionOverride, parentGameObjectId);
}

entt::entity GameObjectSystem::CreateGameObject(nlohmann::json& gameObjectJson, vec2 gameObjectPosition, entt::entity parentGameObjectId)
{
    return CreateGameObject(gameObjectJson["GameObjectType"].get<GameObjectTypeEnum>(), gameObjectPosition, parentGameObjectId);
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

entt::entity GameObjectSystem::CreateGameObject(GameObjectTypeEnum gameObjectType, vec2 gameObjectPosition, entt::entity parentGameObjectId)
{
    entt::entity gameObjectEntity = EntityRegistry.create();
    GameObject& gameObject = EntityRegistry.emplace<GameObject>(gameObjectEntity, GameObject
        {
            .GameObjectId = static_cast<uint32>(gameObjectEntity),
            .GameObjectPtr = 0,
            .GameObjectType = gameObjectType,
            .GameObjectAlive = true
        });
    void* aafaf = (void*)&EntityRegistry;
    auto it = GameObjectBehaviorMap.find(gameObjectType);
    if (it != GameObjectBehaviorMap.end() && it->second.CreateObject)
    {
        gameObject.GameObjectPtr = FindGameObjectBehavior(gameObjectType).CreateObject();
    }

    if (parentGameObjectId != entt::null)
    {
        EntityRegistry.emplace<GameObjectHierarchy>(gameObjectEntity, GameObjectHierarchy
            {
                .Parent = parentGameObjectId
            });
    }

    const nlohmann::json gameObjectComponentJson = GameObjectComponentTempleteMap[gameObjectType];
    for (const auto& json : gameObjectComponentJson)
    {
        const auto componentType = static_cast<ComponentTypeEnum>(json["ComponentType"].get<uint64>());
        ComponentInitContext componentContext = ComponentInitContext
        {
            .Registry = EntityRegistry,
            .Entity = gameObjectEntity,
            .Json = json,
            .Position2DOverride = gameObjectPosition
        };
        gameObjectComponentRegistry.ApplyGameObjectComponent(componentType, componentContext);
    }

    if (GameObjectBehaviorMap.contains(gameObject.GameObjectType) &&
        gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].CreateObject)
    {
        if (gameObject.GameObjectPtr) gameObject.GameObjectPtr = gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].CreateObject();
        GameObjectBehaviorMap[gameObject.GameObjectType].Startup(gameObject.GameObjectPtr, gameObjectEntity, parentGameObjectId);
    }
    if (GameObjectVarTemplateMap.contains(gameObject.GameObjectType))
    {
        //  EntityRegistry.emplace<GameObjectStruct>(gameObject.GameObjectComponents, GameObjectVarTemplateMap[gameObject.GameObjectType]);
    }

    return gameObjectEntity;
}

void GameObjectSystem::LoadGameObjectTemplete(const Vector<String>& gameObjectJsonList)
{
    Vector<String> uniqueJsonSet;
    std::unordered_set<String> seen;
    std::for_each(gameObjectJsonList.begin(), gameObjectJsonList.end(),
        [&](const String& str)
        {
            if (!str.empty() && seen.insert(str).second)
            {
                uniqueJsonSet.emplace_back(str);
            }
        });

    for (const auto& jsonString : uniqueJsonSet)
    {
        LoadGameObjectTemplete(jsonString);
    }
}

void GameObjectSystem::LoadGameObjectTemplete(const String& gameObjectJson)
{
    nlohmann::json json = fileSystem.LoadJsonFile(gameObjectJson.c_str());
    if (GameObjectComponentTempleteMap.contains(json["GameObjectType"])) return;

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
            //luaScriptingSystem.CreateEntityFromScript(luaPath, "asdfad");
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
}

void GameObjectSystem::AddGameObjectBehavior(GameObjectTypeEnum gameObjectType, const GameObjectBehavior& gameObjectBehavior)
{
    GameObjectBehaviorMap[gameObjectType] = gameObjectBehavior;
}

void GameObjectSystem::Update(const float& deltaTime)
{
    auto view = gameObjectSystem.EntityRegistry.view<GameObject>();
    for (auto [entity, gameObject] : view.each())
    {
        if(gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].Update) gameObjectSystem.GameObjectBehaviorMap[gameObject.GameObjectType].Update(gameObject.GameObjectPtr, deltaTime);
        //luaScriptingSystem.Update(deltaTime);
    }
}

entt::entity GameObjectSystem::FindGameObject(uint32 gameObjectId)
{
    entt::entity entity = static_cast<entt::entity>(gameObjectId);
    if (EntityRegistry.valid(entity)) return entity;
    return entt::null;
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

const GameObjectRegistry* GameObjectSystem::GetEntityRegistry()
{
    return &EntityRegistry;
}

void GameObjectSystem::DestroyGameObject(entt::entity entity)
{
    if (!EntityRegistry.valid(entity)) return;

    auto* gameObject = EntityRegistry.try_get<GameObject>(entity);
    if (!gameObject)
    {
        return;
    }

    auto it = GameObjectBehaviorMap.find(gameObject->GameObjectType);
    if (it != GameObjectBehaviorMap.end() && it->second.Destroy)
    {
        it->second.Destroy(gameObject->GameObjectPtr);
    }

    if (auto* hierarchy = EntityRegistry.try_get<GameObjectHierarchy>(entity))
    {
        for (entt::entity child : hierarchy->Children)
        {
            DestroyGameObject(child); 
        }
    }
    EntityRegistry.destroy(entity);
}

entt::registry& GameObjectSystem_Registry()
{
     return gameObjectSystem.EntityRegistry; 
}
