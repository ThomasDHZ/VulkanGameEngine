#include "GameObjectSystemDLL.h"

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