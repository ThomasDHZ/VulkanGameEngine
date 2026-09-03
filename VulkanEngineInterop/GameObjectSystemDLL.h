#pragma once
#include "DLL.h"
#include <GameObjectSystem.h>

struct GameObjectComponentContainer
{
    ComponentTypeEnum ComponentType;
    void* ComponentPtr;
};

struct GameObjectVariableDLL
{
    const char*                       VariableName;
    byte*                             Value;
    size_t                            VariableByteSize = 0;
    GameObjectMemberType              MemberTypeEnum;
    bool                              ConstVariable = false;
};

#ifdef __cplusplus
extern "C" {
#endif

    DLL_EXPORT uint                   GameObjectSystem_CreateGameObjectJson(const char* gameObjectJson, vec2 gameObjectPosition, uint32 parentGameObjectId = UINT32_MAX);
    DLL_EXPORT uint                   GameObjectSystem_CreateGameObject(GameObjectTypeEnum gameObjectType, vec2 gameObjectPosition, uint32 parentGameObjectId = UINT32_MAX);
    DLL_EXPORT void                   GameObjectSystem_Update(const float& deltaTime);
    DLL_EXPORT GameObjectVariableDLL* GameObjectSystem_GetGameObjectVariables(uint gameObjectId, size_t& returnCount);
    DLL_EXPORT GameObject*            GameObjectSystem_GetGameObjectList(size_t& returnGameObjectCount);
    DLL_EXPORT GameObject*            GameObjectSystem_GetGameObject(uint gameObjectId);
    DLL_EXPORT IntPtr                 GameObjectSystem_GetGameObjectPtr(uint gameObjectId);
    DLL_EXPORT void                   GameObjectSystem_DestroyGameObject(uint gameObjectId);
    DLL_EXPORT void*                  GameObjectSystem_UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType);
    DLL_EXPORT void                   GameObjectSystem_CreateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType, void* componentData);
    DLL_EXPORT void                   GameObjectSystem_RegisterBehavior(GameObjectTypeEnum gameObjectType, IntPtr(*CreateObject)(), void  (*startup)(IntPtr, entt::entity, entt::entity), void  (*update)(IntPtr, float), void (*onCollisionEnter)(IntPtr, entt::entity, entt::entity), void (*onCollisionStay)(IntPtr, entt::entity, entt::entity), void (*onCollisionExit)(IntPtr, entt::entity, entt::entity), void  (*destroy)(IntPtr));
    DLL_EXPORT ComponentTypeEnum*     GameObjectSystem_GetGameObjectComponentList(size_t gameObjectId, size_t& returnCount);
#ifdef __cplusplus
}
#endif
