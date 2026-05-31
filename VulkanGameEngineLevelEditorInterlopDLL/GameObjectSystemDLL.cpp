#include "GameObjectSystemDLL.h"
#include <SpriteSystem.h>

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
        case kInputComponent:            gameObjectSystem.CreateGameObjectComponent<InputComponent>(           gameObjectId, static_cast<InputComponent*>(componentData)); break;
        case kSpriteComponent:           gameObjectSystem.CreateGameObjectComponent<Sprite>(          gameObjectId, static_cast<Sprite*>(componentData)); break;
        case kTransform2DComponent:      gameObjectSystem.CreateGameObjectComponent<Transform2DComponent>(     gameObjectId, static_cast<Transform2DComponent*>(componentData)); break;
        case kTransform3DComponent:      gameObjectSystem.CreateGameObjectComponent<Transform3DComponent>(     gameObjectId, static_cast<Transform3DComponent*>(componentData)); break;
        case kCameraFollowComponent:     gameObjectSystem.CreateGameObjectComponent<CameraFollowComponent>(    gameObjectId, static_cast<CameraFollowComponent*>(componentData)); break;
        case kDirectionalLightComponent: gameObjectSystem.CreateGameObjectComponent<DirectionalLightComponent>(gameObjectId, static_cast<DirectionalLightComponent*>(componentData)); break;
        case kPointLightComponent:       gameObjectSystem.CreateGameObjectComponent<PointLightComponent>(      gameObjectId, static_cast<PointLightComponent*>(componentData)); break;
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
        case kInputComponent:            return gameObjectSystem.GetGameObjectComponent<InputComponent>(gameObjectId);
        case kSpriteComponent:           return gameObjectSystem.GetGameObjectComponent<Sprite>(gameObjectId);
        case kTransform2DComponent:      return gameObjectSystem.GetGameObjectComponent<Transform2DComponent>(gameObjectId);
        case kTransform3DComponent:      return gameObjectSystem.GetGameObjectComponent<Transform3DComponent>(gameObjectId);
        case kCameraFollowComponent:     return gameObjectSystem.GetGameObjectComponent<CameraFollowComponent>(gameObjectId);
        case kDirectionalLightComponent: return gameObjectSystem.GetGameObjectComponent<DirectionalLightComponent>(gameObjectId);
        case kPointLightComponent:       return gameObjectSystem.GetGameObjectComponent<PointLightComponent>(gameObjectId);
        default: throw std::runtime_error("GameObject_GetComponent: unknown or unsupported component type: " + std::to_string(static_cast<int>(componentType)) + " (gameObjectId=" + std::to_string(gameObjectId) + ")");
    }
}

void GameObjectSystem_DestroyGameObject(uint gameObjectId)
{
	gameObjectSystem.DestroyGameObject(gameObjectId);
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

GameObjectVariableDLL* GameObjectSystem_GetGameObjectVariables(uint gameObjectId, size_t& returnCount)
{
    Vector<GameObjectVariableDLL> tempList;
    GameObjectStruct* gameObjectStruct = gameObjectSystem.GetGameObjectComponent<GameObjectStruct>(gameObjectId);
    if (!gameObjectStruct || gameObjectStruct->GameObjectVariableMap.empty())
    {
        returnCount = 0;
        return nullptr;
    }

    for (const auto& [varName, var] : gameObjectStruct->GameObjectVariableMap)
    {
        char* nameCopy = nullptr;
        if (!var.VariableName.empty())
        {
            nameCopy = (char*)memorySystem.AddPtrBuffer(var.VariableName.c_str(),  var.VariableName.length() + 1,  __FILE__, __LINE__, __func__ );
        }

        byte* valuePtr = nullptr;
        if (!var.Value.empty())
        {
            valuePtr = memorySystem.AddPtrBuffer(var.Value.data(), var.Value.size(), __FILE__, __LINE__, __func__);
        }

        tempList.emplace_back(GameObjectVariableDLL
            {
                .VariableName = nameCopy,
                .Value = valuePtr,
                .VariableByteSize = var.Value.size(),
                .MemberTypeEnum = var.MemberTypeEnum,
                .ConstVariable = var.ConstVariable
            });
    }
    returnCount = tempList.size();

    if (returnCount == 0) return nullptr;
    return memorySystem.AddPtrBuffer(tempList.data(), returnCount * sizeof(GameObjectVariableDLL), __FILE__, __LINE__, __func__);
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
