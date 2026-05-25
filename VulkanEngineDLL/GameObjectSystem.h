#pragma once
#include "Platform.h"
#include "Transform2DComponent.h"
#include "VulkanSystem.h"
#include <entt/entt.hpp>
#include "nethost.h"

struct InputComponent
{
    byte a = 0x00;
};

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
    uint32        GameObjectId = UINT32_MAX;
    uint32        ParentGameObjectId = UINT32_MAX;
    String        GameObjectBehaviorKey;
    entt::entity  GameObjectComponents; //Not accessible directly in level editor side
    intptr_t      ObjectPtr;
    bool          GameObjectAlive = true;
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
using PlayerCreateFn = intptr_t(*)();
using PlayerStartUpFn = void(*)(intptr_t instance);
using PlayerUpdateFn = void(*)(intptr_t instance, float deltaTime);
using PlayerDestroyFn = void(*)(intptr_t instance);
struct GameObjectBehavior
{
    intptr_t (*CreateObject)() = nullptr;
    void (*Startup)(intptr_t instance) = nullptr;
    //void (*KeyBoardInput)(uint gameObjectId, const float& deltaTime, const KeyState* keyBoardStateArray) = nullptr;
   // void (*ControllerInput)(uint gameObjectId, const float& deltaTime, const GLFWgamepadstate& controlelrState) = nullptr;
    void (*Update)(intptr_t instance, float deltaTime) = nullptr;
    void (*Destroy)(intptr_t instance) = nullptr;
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

    Vector<uint32>                      FreeGameObjectIndex;
    uint32                              AllocateGameObject();

public:
    Vector<GameObject> GameObjectList;
    UnorderedMap<String, GameObjectBehavior> GameObjectBehaviorMap;

    DLL_EXPORT uint                     CreateGameObject(vec2 gameObjectPosition, uint32 parentGameObjectId);
    DLL_EXPORT uint                     CreateGameObject(const String& gameObjectJson, vec2 gameObjectPosition, uint32 parentGameObjectId = UINT32_MAX);
    DLL_EXPORT void                     Update(const float& deltaTime);
    DLL_EXPORT void                     DestroyGameObject(uint gameObjectId);
    DLL_EXPORT GameObject&              FindGameObject(uint gameObjectId);
    DLL_EXPORT const GameObjectBehavior FindGameObjectBehavior(const String& gameObjectClass);
    DLL_EXPORT bool                     GameObjectBehaviorExists(const String& gameObjectClass);
};
extern DLL_EXPORT GameObjectSystem& gameObjectSystem;
inline GameObjectSystem& GameObjectSystem::Get()
{
    static GameObjectSystem instance;
    return instance;
}