#include "GameObjectSystemDLL.h"
#include <LevelSystem.h>

uint32 GameObjectSystem_CreateGameObjectBase(vec2 gameObjectPosition, uint32 parentGameObjectId)
{
    gameObjectSystem.CreateGameObject(gameObjectPosition, parentGameObjectId);
    return gameObjectSystem.GameObjectList.back().GameObjectId;
}

uint32 GameObjectSystem_CreateGameObject(const char* gameObjectJson, vec2 gameObjectPosition, uint32 parentGameObjectId)
{
	gameObjectSystem.CreateGameObject(gameObjectJson, gameObjectPosition, parentGameObjectId);
    return gameObjectSystem.GameObjectList.back().GameObjectId;
}

void GameObjectSystem_CreateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType, void* componentData)
{
    switch (componentType)
    {
        case kInputComponent:        levelSystem.CreateGameObjectComponent<InputComponent>(       gameObjectId, static_cast<InputComponent*>(componentData));
        case kSpriteComponent:       levelSystem.CreateGameObjectComponent<SpriteComponent>(      gameObjectId, static_cast<SpriteComponent*>(componentData));
        case kTransform2DComponent:  levelSystem.CreateGameObjectComponent<Transform2DComponent>( gameObjectId, static_cast<Transform2DComponent*>(componentData));
        case kTransform3DComponent:  levelSystem.CreateGameObjectComponent<Transform3DComponent>( gameObjectId, static_cast<Transform3DComponent*>(componentData));
        case kCameraFollowComponent: levelSystem.CreateGameObjectComponent<CameraFollowComponent>(gameObjectId, static_cast<CameraFollowComponent*>(componentData));
        default: throw std::runtime_error("GameObject_GetComponent: unknown or unsupported component type: " + std::to_string(static_cast<int>(componentType)) + " (gameObjectId=" + std::to_string(gameObjectId) + ")");
    }
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

void GameObjectSystem_DestroyGameObject(uint gameObjectId)
{
	gameObjectSystem.DestroyGameObject(gameObjectId);
}

void GameObjectSystem_DestroyDeadGameObjects()
{
	gameObjectSystem.DestroyDeadGameObjects();
}

GameObject* GameObjectSystem_GetGameObject(size_t gameObjectId)
{
    return &gameObjectSystem.FindGameObject(gameObjectId);
}

GameObject* GameObjectSystem_GetGameObjectList(size_t& returnCount)
{
    returnCount = gameObjectSystem.GameObjectList.size();
    return gameObjectSystem.GameObjectList.data();
}

ComponentTypeEnum* GameObjectSystem_GetGameObjectComponentList(size_t gameObjectId, size_t& returnCount)
{
    Vector<ComponentTypeEnum> componentList;
    GameObject gameObject = gameObjectSystem.FindGameObject(gameObjectId);
    for (uint x = 0; x < ComponentTypeEnum::kEndOfEnum; x++)
    {
        void* componentPtr = GameObjectSystem_UpdateGameObjectComponent(gameObjectId, (ComponentTypeEnum)x);
        if (componentPtr)
        {
            componentList.emplace_back(static_cast<ComponentTypeEnum>(x));
        }
    }

    returnCount = componentList.size();
    return memorySystem.AddPtrBuffer(componentList.data(), componentList.size(), __FILE__, __LINE__, __func__);
}
