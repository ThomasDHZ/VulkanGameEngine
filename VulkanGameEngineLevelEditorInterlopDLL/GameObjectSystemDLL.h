#pragma once
#include <GameObjectSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void        GameObjectSystem_CreateGameObject(const char* gameObjectJson, vec2 gameObjectPosition, uint32 parentGameObjectId = UINT32_MAX);
    DLL_EXPORT void        GameObjectSystem_Update(float deltaTime);
    DLL_EXPORT GameObject* GameObjectSystem_UpdateGameObject(uint gameObjectIndex);
    DLL_EXPORT void*       GameObjectSystem_UpdateGameObjectComponent(uint gameObjectId, ComponentTypeEnum componentType);
    DLL_EXPORT void        GameObjectSystem_DestroyGameObject(uint gameObjectId);
    DLL_EXPORT void        GameObjectSystem_DestroyDeadGameObjects();
#ifdef __cplusplus
}
#endif

