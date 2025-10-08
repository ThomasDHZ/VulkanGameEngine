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

void GameObjectSystem::LoadComponentBehavior(GameObjectID gameObjectId, GameObjectTypeEnum objectEnum)
{
    GameObject_LoadComponentBehavior(gameObjectId, objectEnum);
}

void GameObjectSystem::CreateGameObject(const String& name, GameObjectTypeEnum objectEnum, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, VkGuid vramId, vec2 objectPosition)
{
    GameObject_CreateGameObject(name, objectEnum, gameObjectComponentTypeList, vramId, objectPosition);
}

void GameObjectSystem::CreateGameObject(const String& gameObjectPath, const vec2& gameObjectPosition)
{
    GameObject_CreateGameObject(gameObjectPath, gameObjectPosition);
}

void GameObjectSystem::Update(const float deltaTime)
{
    GameObject_Update(deltaTime);
}

void GameObjectSystem::LoadTransformComponent(const nlohmann::json& json, GameObjectID id, const vec2& gameObjectPosition)
{
    GameObject_LoadTransformComponent(json, id, gameObjectPosition);
}

void GameObjectSystem::LoadInputComponent(const nlohmann::json& json, GameObjectID id)
{
    GameObject_LoadInputComponent(json, id);
}

void GameObjectSystem::LoadSpriteComponent(const nlohmann::json& json, GameObjectID id)
{
    GameObject_LoadSpriteComponent(json, id);
}

const GameObject& GameObjectSystem::FindGameObject(const GameObjectID& id)
{
    return GameObject_FindGameObject(id);
}

Transform2DComponent& GameObjectSystem::FindTransform2DComponent(const GameObjectID& id)
{
    return GameObject_FindTransform2DComponent(id);
}

const InputComponent& GameObjectSystem::FindInputComponent(const GameObjectID& id)
{
    return GameObject_FindInputComponent(id);
}

const Vector<GameObject> GameObjectSystem::GameObjectList()
{
    return GameObject_GameObjectList();
}

const Vector<Transform2DComponent> GameObjectSystem::Transform2DComponentList()
{
    return GameObject_Transform2DComponentList();
}

const Vector<InputComponent> GameObjectSystem::InputComponentList()
{
    return GameObject_InputComponentList();
}

void GameObjectSystem::DestroyGameObject(const GameObjectID& gameObjectId)
{
    GameObject_DestroyGameObject(gameObjectId);
}

void GameObjectSystem::DestroyGameObjects()
{
    GameObject_DestroyGameObjects();
}