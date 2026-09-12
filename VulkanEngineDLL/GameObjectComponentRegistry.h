#pragma once

#include "DLL.h"
#include <Platform.h>
#include "JsonStruct.h"
#include "GameObjectSystem.h"
#include <functional>

struct ComponentInitContext
{
    entt::registry& Registry;
    entt::entity          Entity;
    const nlohmann::json& Json;
    vec2                  Position2DOverride;
    vec3                  Position3DOverride;
};

class GameObjectComponentRegistry
{
public:
    static GameObjectComponentRegistry& Get();
    using UpdateFunc = std::function<void(const ComponentInitContext&)>;

    void RegisterGameObjectComponent(ComponentTypeEnum componentType, UpdateFunc func);
    void ApplyGameObjectComponent(ComponentTypeEnum componentType, const ComponentInitContext& context);
    void RegisterDefaultGameObjectComponents();

private:
    GameObjectComponentRegistry() = default;
    GameObjectComponentRegistry(const GameObjectComponentRegistry&) = delete;
    GameObjectComponentRegistry& operator=(const GameObjectComponentRegistry&) = delete;
    GameObjectComponentRegistry(GameObjectComponentRegistry&&) = delete;
    GameObjectComponentRegistry& operator=(GameObjectComponentRegistry&&) = delete;

    UnorderedMap<ComponentTypeEnum, UpdateFunc> _registry;
};
ENGINE_DLL_EXPORT extern GameObjectComponentRegistry& gameObjectComponentRegistry;
inline GameObjectComponentRegistry& GameObjectComponentRegistry::Get()
{
    static GameObjectComponentRegistry instance;
    return instance;
}

