#include "GameObjectSystem.h"
#include "LevelSystem.h"
#include "SpriteSystem.h"
#include <MegaManObject.h>
#include <FileSystem.h>

GameObjectSystem gameObjectSystem = GameObjectSystem();

GameObjectSystem::GameObjectSystem()
{

}

GameObjectSystem::~GameObjectSystem()
{

}

void GameObjectSystem::CreateGameObject(const String& name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, uint64 gameObjectComponentMask, VkGuid vramId, vec2 objectPosition)
{
    GameObject_CreateGameObject(name, parentGameObjectId, objectEnum, gameObjectComponentMask, vramId, objectPosition);
}

void GameObjectSystem::CreateGameObject(const String& gameObjectPath, const vec2& gameObjectPosition)
{
    GameObject_CreateGameObject(gameObjectPath, gameObjectPosition);
}

void GameObjectSystem::Update(const float deltaTime)
{
    GameObject_Update(deltaTime);
}

void GameObjectSystem::LoadTransformComponent(const nlohmann::json& json, uint gameObjectId, const vec2& gameObjectPosition)
{
    GameObject_LoadTransformComponent(json, gameObjectId, gameObjectPosition);
}

void GameObjectSystem::LoadInputComponent(const nlohmann::json& json, uint gameObjectId)
{
    GameObject_LoadInputComponent(json, gameObjectId);
}

void GameObjectSystem::LoadSpriteComponent(const nlohmann::json& json, GameObject& gameObject)
{
    GameObject_LoadSpriteComponent(json, gameObject);
}

const GameObject& GameObjectSystem::FindGameObject(uint gameObjectId)
{
    return GameObject_FindGameObject(gameObjectId);
}

Transform2DComponent& GameObjectSystem::FindTransform2DComponent(uint gameObjectId)
{
    return GameObject_FindTransform2DComponent(gameObjectId);
}

const InputComponent& GameObjectSystem::FindInputComponent(uint gameObjectId)
{
    return GameObject_FindInputComponent(gameObjectId);
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

void GameObjectSystem::DestroyGameObject(uint gameObjectId)
{
    GameObject_DestroyGameObject(gameObjectId);
}

void GameObjectSystem::DestroyGameObjects()
{
    GameObject_DestroyGameObjects();
}