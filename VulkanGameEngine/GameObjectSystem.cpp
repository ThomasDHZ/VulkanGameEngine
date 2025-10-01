#include "GameObjectSystem.h"
#include "LevelSystem.h"
#include "SpriteSystem.h"
#include <MegaManObject.h>
#include "VulkanFileSystem.h"

GameObjectSystem gameObjectSystem = GameObjectSystem();

GameObjectSystem::GameObjectSystem()
{

}

GameObjectSystem::~GameObjectSystem()
{

}

void GameObjectSystem::LoadComponentBehavior(GameObjectID gameObjectId, ObjectEnum objectEnum)
{
    ComponentBehavior componentBehavior;
    switch (objectEnum)
    {
        case kMegaMan: MegaMan_Behaviors(componentBehavior); break;
    }
    ComponentBehaviorMap[gameObjectId] = componentBehavior;
}

void GameObjectSystem::CreateGameObject(const String& name, ObjectEnum objectEnum, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, VkGuid vramId, vec2 objectPosition)
{
    GameObjectID id;
    id.id = static_cast<uint>(GameObjectMap.size() + 1);
    GameObjectMap[id] = GameObject(id);

    for (auto component : gameObjectComponentTypeList)
    {
        switch (component)
        {
            case kTransform2DComponent: Transform2DComponentMap[id] = Transform2DComponent(objectPosition); break;
            case kInputComponent: InputComponentMap[id] = InputComponent(id); break;
            case kSpriteComponent: spriteSystem.AddSprite(id, vramId); break;
        }
    }
    LoadComponentBehavior(id, objectEnum);
}

void GameObjectSystem::CreateGameObject(const String& gameObjectPath, const vec2& gameObjectPosition)
{
    GameObjectID id;
    id.id = static_cast<uint>(GameObjectMap.size() + 1);
    GameObjectMap[id] = GameObject(id);

    nlohmann::json json = vulkanFileSystem.LoadJsonFile(gameObjectPath);
    for (size_t x = 0; x < json["GameObjectComponentList"].size(); x++)
    {
        int componentType = json["GameObjectComponentList"][x]["ComponentType"];
        switch (componentType)
        {
            case kTransform2DComponent: LoadTransformComponent(json["GameObjectComponentList"][x], id, gameObjectPosition); break;
            case kInputComponent: LoadInputComponent(json["GameObjectComponentList"][x], id); break;
            case kSpriteComponent: LoadSpriteComponent(json["GameObjectComponentList"][x], id); break;
        }
    }
    LoadComponentBehavior(id, json["GameObjectType"]);
}

void GameObjectSystem::LoadTransformComponent(const nlohmann::json& json, GameObjectID id, const vec2& gameObjectPosition)
{
    Transform2DComponent transform2D;
    transform2D.GameObjectPosition = gameObjectPosition;
    transform2D.GameObjectRotation = vec2{ json["GameObjectRotation"][0], json["GameObjectRotation"][1] };
    transform2D.GameObjectScale = vec2{ json["GameObjectScale"][0], json["GameObjectScale"][1] };
    Transform2DComponentMap[id] = transform2D;
}

void GameObjectSystem::LoadInputComponent(const nlohmann::json& json, GameObjectID id)
{
    InputComponentMap[id] = InputComponent(id);
}

void GameObjectSystem::LoadSpriteComponent(const nlohmann::json& json, GameObjectID id)
{
    VkGuid vramId = VkGuid(json["VramId"].get<String>().c_str());
    spriteSystem.AddSprite(id, vramId);
}

const GameObject& GameObjectSystem::FindGameObject(const GameObjectID& id)
{
    return GameObjectMap.at(id);
}

Transform2DComponent& GameObjectSystem::FindTransform2DComponent(const GameObjectID& id)
{
    return Transform2DComponentMap.at(id);
}

const InputComponent& GameObjectSystem::FindInputComponent(const GameObjectID& id)
{
    return InputComponentMap.at(id);
}

const Vector<GameObject> GameObjectSystem::GameObjectList()
{
    Vector<GameObject> list;
    for (const auto& pair : GameObjectMap)
    {
        list.emplace_back(pair.second);
    }
    return list;
}

const Vector<Transform2DComponent> GameObjectSystem::Transform2DComponentList()
{
    Vector<Transform2DComponent> list;
    for (const auto& pair : Transform2DComponentMap)
    {
        list.emplace_back(pair.second);
    }
    return list;
}

const Vector<InputComponent> GameObjectSystem::InputComponentList()
{
    Vector<InputComponent> list;
    for (const auto& pair : InputComponentMap)
    {
        list.emplace_back(pair.second);
    }
    return list;
}

void GameObjectSystem::DestroyGameObject(const GameObjectID& gameObjectId)
{
    GameObjectMap.erase(gameObjectId);
    Transform2DComponentMap.erase(gameObjectId);
    InputComponentMap.erase(gameObjectId);
}

void GameObjectSystem::DestroyGameObjects()
{
    GameObjectMap.clear();
    Transform2DComponentMap.clear();
    InputComponentMap.clear();
}