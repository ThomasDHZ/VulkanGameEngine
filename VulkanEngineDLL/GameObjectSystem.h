#pragma once
#include "Platform.h"
#include "MemorySystem.h"
#include "Transform2DComponent.h"
#include "ComponentSystem.h"
#include "VulkanSystem.h"
#include <entt/entt.hpp>
#include "nethost.h"
#include "enum.h"


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
    kGameObjectPointLight,
    kGameObjectEnemy
};


struct CameraFollowComponent { int a = 0; };


struct GameObjectComponentLinker
{
    uint32 GameObjectId = UINT32_MAX;
};

//struct GameObject
//{
//    uint32                    GameObjectId = UINT32_MAX;
//    uint32                    ParentGameObjectId = UINT32_MAX;
//    GameObjectTypeEnum        GameObjectType;
//    entt::entity              GameObjectComponents; //Not accessible directly in level editor side
//    intptr_t                  ObjectPtr;
//    bool                      GameObjectAlive = true;
//};

struct GameObject
{
    uint                      ParentGameObjectId;
    uint                      GameObjectId;
    IntPtr                    GameObjectPtr;
    GameObjectTypeEnum        GameObjectType;
    bool                      GameObjectAlive = true;
};

struct GameObjectHierarchy
{
    entt::entity Parent = entt::null;
    Vector<entt::entity> Children;
};

struct GameObjectBehavior
{
    IntPtr (*CreateObject)     ()                                                                               = nullptr;
    void   (*Startup)          (IntPtr instance, entt::entity gameObjectId, entt::entity parentGameObject)      = nullptr;
    void   (*OnCollisionEnter) (IntPtr instance, entt::entity gameObjectId, entt::entity collidingGameObjectId) = nullptr;
    void   (*OnCollisionStay)  (IntPtr instance, entt::entity gameObjectId, entt::entity collidingGameObjectId) = nullptr;
    void   (*OnCollisionExit)  (IntPtr instance, entt::entity gameObjectId, entt::entity collidingGameObjectId) = nullptr;
    void   (*Update)           (IntPtr instance, float deltaTime)                                               = nullptr;
    void   (*Destroy)          (IntPtr instance)                                                                = nullptr;
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

typedef entt::registry GameObjectRegistry;
typedef entt::entity   Entity;
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
    GameObjectRegistry                                      EntityRegistry;
    Vector<GameObject>                                      GameObjectList;


    DLL_EXPORT void                                         LoadGameObjectTempletes(Vector<String>& gameObjectJson);
    DLL_EXPORT void                                         CreateGameObjects(nlohmann::json& gameObjectJson);
    DLL_EXPORT entt::entity                                 CreateGameObject(GameObjectTypeEnum gameObjectType, vec2 gameObjectPosition, entt::entity parentGameObjectId = entt::null);

    DLL_EXPORT void                                         Update(const float& deltaTime);
    DLL_EXPORT void                                         DestroyGameObject(entt::entity gameObjectId);
    DLL_EXPORT entt::entity                                 FindGameObject(uint gameObjectId);
    DLL_EXPORT const GameObjectBehavior                     FindGameObjectBehavior(GameObjectTypeEnum gameObjectClass);
    DLL_EXPORT bool                                         GameObjectBehaviorExists(GameObjectTypeEnum gameObjectClass);

    template <typename T>
    T* GetGameObjectComponent(entt::entity gameObjectId)
    {
        if (gameObjectId == entt::null) return nullptr;

        auto view = EntityRegistry.view<T>();
        for (auto [entity, component] : view.each())
        {
            if (entity == gameObjectId)
            {
                return &component;
            }
        }
        return nullptr;
    }

    template <typename T>
    void CreateGameObjectComponent(entt::entity gameObjectId, T* gameObjectComponent)
    {
        EntityRegistry.emplace<T>(gameObjectId, *gameObjectComponent);
    }
};
extern DLL_EXPORT GameObjectSystem& gameObjectSystem;
inline GameObjectSystem& GameObjectSystem::Get()
{
    static GameObjectSystem instance;
    return instance;
}