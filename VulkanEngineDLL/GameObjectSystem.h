#pragma once
#include "Platform.h"
#include "MemorySystem.h"
#include "Transform2DComponent.h"
#include "VulkanSystem.h"
#include <entt/entt.hpp>
#include "nethost.h"
#include "enum.h"

struct Collider2DComponent
{
    ivec2 Size = ivec2(32, 32);
    ivec2 Offset = ivec2(0, 0);
    bool Enabled = true;
    bool IsTrigger = false;
};

struct InputComponent
{
    KeyState* KeyPressed;
    size_t size;

    InputComponent()
    {
        KeyPressed = memorySystem.AddPtrBuffer<KeyState>(MAXKEYBOARDKEY, __FILE__, __LINE__, __func__);
        size = MAXKEYBOARDKEY;
    }

    InputComponent(const InputComponent& other)
    {
        size = other.size;
        KeyPressed = memorySystem.AddPtrBuffer<KeyState>(size, __FILE__, __LINE__, __func__);
        std::memcpy(KeyPressed, other.KeyPressed, sizeof(KeyState) * size);
    }

    ~InputComponent()
    {
    }
};

enum GameObjectTypeEnum
{
    kGameObjectNone,
    kGameObjectMegaMan,
    kGameObjectMegaManShot,
    kGameObjectDirectionalLight,
    kGameObjectPointLight
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
    kCollisionComponent,
    kDebugObjectComponent,
    kEndOfEnum
};

struct CameraFollowComponent { int a = 0; };


struct GameObjectComponentLinker
{
    uint32 GameObjectId = UINT32_MAX;
};

struct GameObject
{
    uint32                    GameObjectId = UINT32_MAX;
    uint32                    ParentGameObjectId = UINT32_MAX;
    GameObjectTypeEnum        GameObjectType;
    entt::entity              GameObjectComponents; //Not accessible directly in level editor side
    intptr_t                  ObjectPtr;
    bool                      GameObjectAlive = true;
};

struct GameObjectBehavior
{
    intptr_t (*CreateObject)() = nullptr;
    void (*Startup)(intptr_t instance, uint32 gameObjectId, uint32 parentGameObject) = nullptr;
    void (*OnCollisionEnter)(intptr_t instance, uint gameObjectId, uint collidingGameObjectId) = nullptr;
    void (*OnCollisionStay)(intptr_t instance, uint gameObjectId, uint collidingGameObjectId) = nullptr;
    void (*OnCollisionExit)(intptr_t instance, uint gameObjectId, uint collidingGameObjectId) = nullptr;
    void (*Update)(intptr_t instance, float deltaTime) = nullptr;
    void (*Destroy)(intptr_t instance) = nullptr;
};

struct GameObjectVariable
{
    String                          VariableName;
    Vector<byte>                    Value;
    GameObjectMemberType            MemberTypeEnum = GameObjectVarUnknown;
    size_t                          VariableByteSize = 0;
    bool                            ConstVariable = false;
};

struct GameObjectStruct
{
     UnorderedMap<String, GameObjectVariable>      GameObjectVariableMap;
};

struct DebugObjectComponent
{
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

    UnorderedMap<GameObjectTypeEnum, GameObject>            GameObjectTempleteMap;
    UnorderedMap<GameObjectTypeEnum, GameObjectBehavior>    GameObjectBehaviorMap;
    UnorderedMap<GameObjectTypeEnum, GameObjectStruct>      GameObjectVarTemplateMap;
    UnorderedMap<GameObjectTypeEnum, nlohmann::json>        GameObjectComponentTempleteMap;

    Vector<uint32>                                          FreeGameObjectIndex;
    uint32                                                  AllocateGameObject();

public:
    entt::registry                                          EntityRegistry;
    Vector<GameObject>                                      GameObjectList;


    DLL_EXPORT void                                         LoadGameObjectTempletes(Vector<String>& gameObjectJson);
    DLL_EXPORT void                                         CreateGameObjects(nlohmann::json& gameObjectJson);
    DLL_EXPORT uint                                         CreateGameObject(GameObjectTypeEnum gameObjectJson, vec2 gameObjectPosition, uint32 parentGameObjectId = UINT32_MAX);

    DLL_EXPORT void                                         Update(const float& deltaTime);
    DLL_EXPORT void                                         DestroyGameObject(uint gameObjectId);
    DLL_EXPORT GameObject&                                  FindGameObject(uint gameObjectId);
    DLL_EXPORT const GameObjectBehavior                     FindGameObjectBehavior(GameObjectTypeEnum gameObjectClass);
    DLL_EXPORT bool                                         GameObjectBehaviorExists(GameObjectTypeEnum gameObjectClass);

    template <typename T>
    T* GetGameObjectComponent(uint gameObjectId)
    {
        if (gameObjectId == UINT32_MAX) return nullptr;

        GameObject& gameObject = GameObjectList[gameObjectId];
        auto view = EntityRegistry.view<GameObjectComponentLinker, T>();
        for (auto [entity, linker, component] : view.each())
        {
            if (linker.GameObjectId == gameObjectId)
            {
                return &component;
            }
        }
        return nullptr;
    }

    template <typename T>
    void CreateGameObjectComponent(uint32 gameObjectId, T* gameObjectComponent)
    {
        GameObject& gameObject = GameObjectList[gameObjectId];
        EntityRegistry.emplace<T>(gameObject.GameObjectComponents, *gameObjectComponent);
    }
};
extern DLL_EXPORT GameObjectSystem& gameObjectSystem;
inline GameObjectSystem& GameObjectSystem::Get()
{
    static GameObjectSystem instance;
    return instance;
}