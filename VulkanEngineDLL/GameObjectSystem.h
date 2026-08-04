#pragma once
#include <Platform.h>
#include "MemorySystem.h"
#include "Transform2DComponent.h"
#include "ComponentSystem.h"
#include <VulkanSystem.h>
#include <entt/entt.hpp>
#include "nethost.h"
#include "enum.h"
#include <InputEnum.h>
#include <glfw3.h>
#include <GameController.h>

struct InputComponent
{
    // Keyboard
    KeyState* KeyPressed{};

    // Mouse
    float MouseX = 0.0f;
    float MouseY = 0.0f;
    float MouseDeltaX = 0.0f;
    float MouseDeltaY = 0.0f;
    bool* MouseButtons{};
    bool  HasGamepad = false;
    byte* buttonsList{};
    float* axesList{};
    float  DeltaTime = 0.0f;
    uint32 Sequence = 0;
    size_t MaxKeyboardSize = MAXKEYBOARDKEY;
    size_t MaxMouseButton = MAXMOUSEKEY;
    size_t maxButtonCount = 15;
    size_t maxAxesCount = 6;

    InputComponent()
    {
        KeyPressed = memorySystem.AddPtrBuffer<KeyState>(MAXKEYBOARDKEY, __FILE__, __LINE__, __func__);
        MouseButtons = memorySystem.AddPtrBuffer<bool>(MAXMOUSEKEY, __FILE__, __LINE__, __func__);
        buttonsList = memorySystem.AddPtrBuffer<byte>(maxButtonCount, __FILE__, __LINE__, __func__);
        axesList = memorySystem.AddPtrBuffer<float>(maxAxesCount, __FILE__, __LINE__, __func__);
    }

    InputComponent(const InputComponent& other)
    {
        MaxKeyboardSize = other.MaxKeyboardSize;
        KeyPressed = memorySystem.AddPtrBuffer<KeyState>(MaxKeyboardSize, __FILE__, __LINE__, __func__);
        std::memcpy(KeyPressed, other.KeyPressed, sizeof(KeyState) * MaxKeyboardSize);

        MaxMouseButton = other.MaxMouseButton;
        MouseButtons = memorySystem.AddPtrBuffer<bool>(MaxMouseButton, __FILE__, __LINE__, __func__);
        std::memcpy(MouseButtons, other.MouseButtons, sizeof(bool) * MaxMouseButton);

        maxButtonCount = other.maxButtonCount;
        buttonsList = memorySystem.AddPtrBuffer<byte>(maxButtonCount, __FILE__, __LINE__, __func__);
        std::memcpy(buttonsList, other.buttonsList, sizeof(byte) * maxButtonCount);

        maxAxesCount = other.maxAxesCount;
        axesList = memorySystem.AddPtrBuffer<float>(maxAxesCount, __FILE__, __LINE__, __func__);
        std::memcpy(axesList, other.axesList, sizeof(float) * maxAxesCount);
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

    UnorderedMap<GameObjectTypeEnum, GameObjectBehavior>    GameObjectBehaviorMap;
    UnorderedMap<GameObjectTypeEnum, GameObjectStruct>      GameObjectVarTemplateMap;
    UnorderedMap<GameObjectTypeEnum, nlohmann::json>        GameObjectComponentTempleteMap;

public:
    GameObjectRegistry                                      EntityRegistry;

    DLL_EXPORT void                                         LoadGameObjectTempletes(Vector<String>& gameObjectJson);
    DLL_EXPORT void                                         CreateGameObjects(nlohmann::json& gameObjectJson);
    DLL_EXPORT entt::entity                                 CreateGameObject(GameObjectTypeEnum gameObjectType, vec2 gameObjectPosition, entt::entity parentGameObjectId = entt::null);

    DLL_EXPORT void                                         Update(const float& deltaTime);
    DLL_EXPORT void                                         DestroyGameObject(entt::entity gameObjectId);
    DLL_EXPORT entt::entity                                 FindGameObject(uint gameObjectId);
    DLL_EXPORT const GameObjectBehavior                     FindGameObjectBehavior(GameObjectTypeEnum gameObjectClass);
    DLL_EXPORT bool                                         GameObjectBehaviorExists(GameObjectTypeEnum gameObjectClass);


    template<typename T>
    T* GetGameObjectComponent(entt::entity entity)
    {
        if (!EntityRegistry.valid(entity)) return nullptr;
        return EntityRegistry.try_get<T>(entity);
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