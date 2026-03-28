#include "GameObjectSystemDLL.h"
#include <LevelSystem.h>

void GameObjectSystem_CreateGameObject(const char* gameObjectJson, vec2 gameObjectPosition, uint32 parentGameObjectId)
{
	gameObjectSystem.CreateGameObject(gameObjectJson, gameObjectPosition, parentGameObjectId);
}

void GameObjectSystem_Update(float deltaTime)
{
	gameObjectSystem.Update(deltaTime);
}

GameObject* GameObjectSystem_UpdateGameObject(uint gameObjectIndex)
{
	return &gameObjectSystem.GameObjectList[gameObjectIndex];
}

void* GameObjectSystem_UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType)
{
    switch (componentType)
    {
        case kInputComponent: return levelSystem.GetGameObjectComponent<InputComponent>(gameObjectId);
        case kSpriteComponent: return levelSystem.GetGameObjectComponent<SpriteComponent>(gameObjectId);
        case kTransform2DComponent:  return levelSystem.GetGameObjectComponent<Transform2DComponent>(gameObjectId);
        case kTransform3DComponent: return levelSystem.GetGameObjectComponent<Transform3DComponent>(gameObjectId);
        case kCameraFollowComponent: return levelSystem.GetGameObjectComponent<CameraFollowComponent>(gameObjectId);
        default: throw std::runtime_error("GameObject_GetComponent: unknown or unsupported component type: " + std::to_string(static_cast<int>(componentType)) + " (gameObjectId=" + std::to_string(gameObjectId) + ")");
    }
}

Transform2DComponent* GameObjectSystem_UpdateTransform2DComponent(uint gameObjectId)
{
    return levelSystem.GetGameObjectComponent<Transform2DComponent>(gameObjectId);
}

void GameObjectSystem_DestroyGameObject(uint gameObjectId)
{
	gameObjectSystem.DestroyGameObject(gameObjectId);
}

void GameObjectSystem_DestroyDeadGameObjects()
{
	gameObjectSystem.DestroyDeadGameObjects();
}
