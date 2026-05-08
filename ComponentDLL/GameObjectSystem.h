#pragma once
#include "pch.h"
#include "InputComponent.h"
#include "Transform2DComponent.h"
#include "MegaManShot.h"
#include "VulkanSystem.h"
#include <entt/entt.hpp>

enum GameObjectTypeEnum
{
    kGameObjectNone,
    kGameObjectMegaMan,
    kGameObjectMegaManShot,
    kGameObjectDebug
};

enum ComponentTypeEnum : uint
{
    kInputComponent,
    kSpriteComponent,
    kTransform2DComponent,
    kTransform3DComponent,
    kCameraFollowComponent,
    kDirectionalLightComponent,
    kPointLightComponent,
    kEndOfEnum
};

struct CameraFollowComponent { int a = 0; };

struct GameObject
{
    uint32       GameObjectId = UINT32_MAX;
    uint32       ParentGameObjectId = UINT32_MAX;
    bool         GameObjectAlive = true;
    entt::entity GameObjectComponents; //Not accessible directly in level editor side
};

struct GameObjectComponentLinker
{
    uint32 GameObjectId = UINT32_MAX;
};

struct DirectionalLightComponent
{
    uint32 GameObjectId = UINT32_MAX;
    uint32 DirectionalLightId = UINT32_MAX;
};

struct PointLightComponent
{
    uint32 GameObjectId = UINT32_MAX;
    uint32 PointLightId = UINT32_MAX;
};

class LevelSystem;
class GameObjectSystem
{
public:
    static GameObjectSystem& Get();
    
private:
    GameObjectSystem() = default;
    ~GameObjectSystem() = default;
    GameObjectSystem(const GameObjectSystem&) = delete;
    GameObjectSystem& operator=(const GameObjectSystem&) = delete;
    GameObjectSystem(GameObjectSystem&&) = delete;
    GameObjectSystem& operator=(GameObjectSystem&&) = delete;

    Vector<uint32> FreeGameObjectIndex;
    uint32 AllocateGameObject();

public:
    Vector<GameObject> GameObjectList;

    DLL_EXPORT uint CreateGameObject(vec2 gameObjectPosition, uint32 parentGameObjectId);
    DLL_EXPORT uint CreateGameObject(const String& gameObjectJson, vec2 gameObjectPosition, uint32 parentGameObjectId = UINT32_MAX);
    DLL_EXPORT void Update(const float& deltaTime);
    DLL_EXPORT void DestroyGameObject(uint gameObjectId);
    DLL_EXPORT void DestroyDeadGameObjects(); 
    DLL_EXPORT GameObject& FindGameObject(uint gameObjectId);
    DLL_EXPORT const Vector<GameObject>& FindGameObjectWithComponent();
    DLL_EXPORT const Vector<GameObject>& FindGameObjectByMeshType(MeshTypeEnum meshType);
};
extern DLL_EXPORT GameObjectSystem& gameObjectSystem;
inline GameObjectSystem& GameObjectSystem::Get()
{
    static GameObjectSystem instance;
    return instance;
}