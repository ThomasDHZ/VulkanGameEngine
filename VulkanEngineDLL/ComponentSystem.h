#pragma once

#include "DLL.h"
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <iostream>
#include <nlohmann/json.hpp>
#include <entt/entt.hpp>

enum ComponentTypeEnum : uint64_t
{
    kInputComponent,
    kSpriteComponent,
    kTransform2DComponent,
    kTransform3DComponent,
    kCameraFollowComponent,
    kDirectionalLightComponent,
    kPointLightComponent,
    kDebugObjectComponent,
    kCollisionComponent,
    kEndOfEnum
};

class IComponentLoader
{
public:
    virtual ~IComponentLoader() = default;
    virtual void Load(entt::registry& registry, entt::entity entity, const nlohmann::json& json) = 0;
};

class ENGINE_DLL_EXPORT ComponentFactory
{
private:
    std::unordered_map<uint64_t, std::unique_ptr<IComponentLoader>> m_loaders;
    ComponentFactory() = default;

public:
    ComponentFactory(const ComponentFactory&) = delete;
    ComponentFactory& operator=(const ComponentFactory&) = delete;

    static ComponentFactory& Get();

    void RegisterLoader(uint64_t componentType, std::unique_ptr<IComponentLoader> loader);
    void Load(entt::registry& registry, entt::entity entity, uint64_t componentType, const nlohmann::json& json);
};