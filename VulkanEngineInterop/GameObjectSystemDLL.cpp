#include "GameObjectSystemDLL.h"
#include <SpriteSystem.h>
#include <LightSystem.h>

void GameObjectSystem_Update(const float& deltaTime)
{
    gameObjectSystem.Update(deltaTime);
}

uint32 GameObjectSystem_CreateGameObject(GameObjectTypeEnum gameObjectType, vec2 gameObjectPosition, uint32 parentGameObjectId)
{
    entt::entity parentGameObject = static_cast<entt::entity>(parentGameObjectId);
    uint gameObjectId = static_cast<uint32>(gameObjectSystem.CreateGameObject(gameObjectType, gameObjectPosition, parentGameObject));
    return gameObjectId;
}

GameObjectVariableDLL* GameObjectSystem_GetGameObjectVariables(uint gameObjectId, size_t& returnCount)
{
    Vector<GameObjectVariableDLL> tempList;
    entt::entity gameObjectEntity = static_cast<entt::entity>(gameObjectId);
    GameObjectStruct* gameObjectStruct = gameObjectSystem.GetGameObjectComponent<GameObjectStruct>(gameObjectEntity);
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
            nameCopy = (char*)memorySystem.AddPtrBuffer(var.VariableName.c_str(), var.VariableName.length() + 1, __FILE__, __LINE__, __func__);
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

IntPtr GameObjectSystem_GetGameObjectPtr(uint gameObjectId)
{
    entt::entity gameObjectEntity = gameObjectSystem.FindGameObject(gameObjectId);
    GameObject gameObject = gameObjectSystem.EntityRegistry.get<GameObject>(gameObjectEntity);
    return gameObject.GameObjectPtr;
}

void GameObjectSystem_DestroyGameObject(uint gameObjectId)
{
    entt::entity gameObjectEntity = static_cast<entt::entity>(gameObjectId);
    gameObjectSystem.DestroyGameObject(gameObjectEntity);
}

void GameObjectSystem_CreateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType, void* componentData)
{
    entt::entity gameObjectEntity = static_cast<entt::entity>(gameObjectId);
    switch (componentType)
    {
        case kInputComponent:            gameObjectSystem.CreateGameObjectComponent<InputComponent>(gameObjectEntity, static_cast<InputComponent*>(componentData)); break;
        case kSpriteComponent:           gameObjectSystem.CreateGameObjectComponent<Sprite>(gameObjectEntity, static_cast<Sprite*>(componentData)); break;
        case kTransform2DComponent:      gameObjectSystem.CreateGameObjectComponent<Transform2DComponent>(gameObjectEntity, static_cast<Transform2DComponent*>(componentData)); break;
        case kTransform3DComponent:      gameObjectSystem.CreateGameObjectComponent<Transform3DComponent>(gameObjectEntity, static_cast<Transform3DComponent*>(componentData)); break;
        case kCameraFollowComponent:     gameObjectSystem.CreateGameObjectComponent<CameraFollowComponent>(gameObjectEntity, static_cast<CameraFollowComponent*>(componentData)); break;
        case kDirectionalLightComponent: gameObjectSystem.CreateGameObjectComponent<DirectionalLightComponent>(gameObjectEntity, static_cast<DirectionalLightComponent*>(componentData)); break;
        case kPointLightComponent:       gameObjectSystem.CreateGameObjectComponent<PointLightComponent>(gameObjectEntity, static_cast<PointLightComponent*>(componentData)); break;
        default: throw std::runtime_error("GameObject_GetComponent: unknown or unsupported component type: " + std::to_string(static_cast<int>(componentType)) + " (gameObjectId=" + std::to_string(gameObjectId) + ")");
    }
}

void* GameObjectSystem_UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType)
{
    entt::entity gameObjectEntity = static_cast<entt::entity>(gameObjectId);
    switch (componentType)
    {
    case kInputComponent:            return gameObjectSystem.GetGameObjectComponent<InputComponent>(gameObjectEntity);
    case kSpriteComponent:           return gameObjectSystem.GetGameObjectComponent<Sprite>(gameObjectEntity);
    case kTransform2DComponent:      return gameObjectSystem.GetGameObjectComponent<Transform2DComponent>(gameObjectEntity);
    case kTransform3DComponent:      return gameObjectSystem.GetGameObjectComponent<Transform3DComponent>(gameObjectEntity);
    case kCameraFollowComponent:     return gameObjectSystem.GetGameObjectComponent<CameraFollowComponent>(gameObjectEntity);
    case kDirectionalLightComponent: return gameObjectSystem.GetGameObjectComponent<DirectionalLightComponent>(gameObjectEntity);
    case kPointLightComponent:       return gameObjectSystem.GetGameObjectComponent<PointLightComponent>(gameObjectEntity);
    default: throw std::runtime_error("GameObject_GetComponent: unknown or unsupported component type: " + std::to_string(static_cast<int>(componentType)) + " (gameObjectId=" + std::to_string(gameObjectId) + ")");
    }
}