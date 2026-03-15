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
    kGameObjectMegaManShot
};

enum ComponentTypeEnum
{
    kUndefined,
    kInputComponent,
    kSpriteComponent,
    kTransform2DComponent,
    kTransform3DComponent,
};

struct GameObject
{
    uint32 GameObjectId = UINT32_MAX;
    uint32 ParentGameObjectId = UINT32_MAX;
    uint32 GameObjectComponentIndex = UINT32_MAX;
    bool   GameObjectAlive = true;
};

struct GameObjectComponentLinker
{
    uint32 GameObjectId = UINT32_MAX;
};

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
    Vector<uint32> FreeGameObjectComponentIndex;

    uint32 AllocateGameObject();
    uint32 AllocateGameObjectComponent();

public:
    Vector<GameObject> GameObjectList;
    Vector<entt::entity> GameObjectComponentList;

    DLL_EXPORT void CreateGameObject(const String& gameObjectJson, vec2 gameObjectPosition, uint32 parentGameObjectId = UINT32_MAX);
    DLL_EXPORT void Update(const float& deltaTime);
    DLL_EXPORT void DestroyGameObject(uint gameObjectId);
    DLL_EXPORT void DestroyDeadGameObjects(); 
    DLL_EXPORT GameObject& FindGameObject(uint gameObjectId);
};
extern DLL_EXPORT GameObjectSystem& gameObjectSystem;
inline GameObjectSystem& GameObjectSystem::Get()
{
    static GameObjectSystem instance;
    return instance;
}