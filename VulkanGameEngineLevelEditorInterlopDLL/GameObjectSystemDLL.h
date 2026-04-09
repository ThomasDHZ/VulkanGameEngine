#pragma once
#include <GameObjectSystem.h>

struct GameObjectComponentContainer
{
    ComponentTypeEnum ComponentType;
    void*             ComponentPtr;
};

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT uint32                             GameObjectSystem_CreateGameObjectBase(vec2 gameObjectPosition, uint32 parentGameObjectId = UINT32_MAX);
    DLL_EXPORT uint32                             GameObjectSystem_CreateGameObject(const char* gameObjectJson, vec2 gameObjectPosition, uint32 parentGameObjectId = UINT32_MAX);
    DLL_EXPORT void                               GameObjectSystem_CreateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType, void* componentData);
    DLL_EXPORT void                               GameObjectSystem_Update(float deltaTime);
    DLL_EXPORT GameObject*                        GameObjectSystem_UpdateGameObject(uint gameObjectIndex);
    DLL_EXPORT void*                              GameObjectSystem_UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType);
    DLL_EXPORT void                               GameObjectSystem_DestroyGameObject(uint gameObjectId);
    DLL_EXPORT void                               GameObjectSystem_DestroyDeadGameObjects();
    DLL_EXPORT GameObject*                        GameObjectSystem_GetGameObject(size_t gameGameObjectId);
    DLL_EXPORT GameObject*                        GameObjectSystem_GetGameObjectList(size_t& returnCount);
    DLL_EXPORT ComponentTypeEnum*                 GameObjectSystem_GetGameObjectComponentList(size_t gameObjectId, size_t& returnCount);
#ifdef __cplusplus
}
#endif

