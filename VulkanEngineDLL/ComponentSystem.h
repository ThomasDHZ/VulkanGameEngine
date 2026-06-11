#pragma once
#include "Platform.h"
#include "Transform2DComponent.h"
#include "Collider2DComponent.h"

enum ComponentTypeEnum : uint64
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

class ComponentFactory
{
private:
    std::unordered_map<uint64_t, std::unique_ptr<IComponentLoader>> m_loaders;

public:
    static ComponentFactory& Get();

    void RegisterLoader(uint64_t componentType, std::unique_ptr<IComponentLoader> loader);
    void Load(entt::registry& registry, entt::entity entity, uint64_t componentType, const nlohmann::json& json);
};

