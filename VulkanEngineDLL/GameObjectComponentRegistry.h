#pragma once
#include <Platform.h>
#include "JsonStruct.h"
#include "GameObjectSystem.h"

struct ComponentInitContext
{
    entt::registry&       Registry;
    entt::entity          Entity;
    const nlohmann::json& Json;
    vec2                  Position2DOverride;
    vec3                  Position3DOverride;
};

class GameObjectComponentRegistry
{
public:
    static GameObjectComponentRegistry& Get();

private:
    GameObjectComponentRegistry() = default;
    GameObjectComponentRegistry(const GameObjectComponentRegistry&) = delete;
    GameObjectComponentRegistry& operator=(const GameObjectComponentRegistry&) = delete;
    GameObjectComponentRegistry(GameObjectComponentRegistry&&) = delete;
    GameObjectComponentRegistry& operator=(GameObjectComponentRegistry&&) = delete;

    using UpdateFunc = std::function<void(const ComponentInitContext&)>;
    UnorderedMap<ComponentTypeEnum, UpdateFunc> _registry;

public:
    void RegisterGameObjectComponent(ComponentTypeEnum componentType, UpdateFunc func);
    void ApplyGameObjectComponent(ComponentTypeEnum componentType, const ComponentInitContext& context);
    void RegisterDefaultGameObjectComponents();
};

extern GameObjectComponentRegistry& gameObjectComponentRegistry;

inline GameObjectComponentRegistry& GameObjectComponentRegistry::Get()
{
    static GameObjectComponentRegistry instance;
    return instance;
}

