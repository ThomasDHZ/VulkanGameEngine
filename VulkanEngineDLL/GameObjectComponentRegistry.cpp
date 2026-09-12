#include "GameObjectComponentRegistry.h"
#include "GameObjectSystem.h"
#include "CollisionSystem.h"
#include "SpriteSystem.h"
#include "LightSystem.h"

GameObjectComponentRegistry& gameObjectComponentRegistry = GameObjectComponentRegistry::Get();

void GameObjectComponentRegistry::RegisterGameObjectComponent(ComponentTypeEnum componentType, UpdateFunc func)
{
    _registry[componentType] = std::move(func);
}

void GameObjectComponentRegistry::ApplyGameObjectComponent(ComponentTypeEnum componentType, const ComponentInitContext& context)
{
    auto it = _registry.find(componentType);
    if (it != _registry.end())
    {
        it->second(context);
        return;
    }
    std::cerr << "GameObjectComponent not implemented yet: " << static_cast<uint64>(componentType) << '\n';
}

void GameObjectComponentRegistry::RegisterDefaultGameObjectComponents()
{

    RegisterGameObjectComponent(kInputComponent, [](const ComponentInitContext& ctx)
        {
            ctx.Registry.emplace<InputComponent>(ctx.Entity);
        });

    RegisterGameObjectComponent(kSpriteComponent, [](const ComponentInitContext& ctx)
        {
            VkGuid vramId{ ctx.Json["VramSpriteId"].get<String>().c_str() };
            spriteSystem.CreateSprite(ctx.Entity, vramId);
        });

    RegisterGameObjectComponent(kTransform2DComponent, [](const ComponentInitContext& ctx)
        {
            ctx.Registry.emplace<Transform2DComponent>(ctx.Entity, Transform2DComponent{
                .GameObjectPosition = ctx.Position2DOverride,
                .GameObjectRotation = vec2{ ctx.Json["GameObjectRotation"][0], ctx.Json["GameObjectRotation"][1] },
                .GameObjectScale = vec2{ ctx.Json["GameObjectScale"][0],    ctx.Json["GameObjectScale"][1] }
                });
        });

    RegisterGameObjectComponent(kTransform3DComponent, [](const ComponentInitContext& ctx)
        {
            ctx.Registry.emplace<Transform3DComponent>(ctx.Entity, Transform3DComponent{
                .GameObjectPosition = vec3{ ctx.Position2DOverride.x, ctx.Position2DOverride.y, 0.0f },
                .GameObjectRotation = vec3{ ctx.Json["GameObjectRotation"][0], ctx.Json["GameObjectRotation"][1], 0.0f },
                .GameObjectScale = vec3{ ctx.Json["GameObjectScale"][0],    ctx.Json["GameObjectScale"][1], 0.0f }
                });
        });

    RegisterGameObjectComponent(kCollisionComponent, [](const ComponentInitContext& ctx)
        {
            Collider2DComponent collider{
                .Size = ivec2{ ctx.Json["ColliderSize"][0], ctx.Json["ColliderSize"][1] },
                .Offset = ivec2{ ctx.Json["ColliderOffset"][0], ctx.Json["ColliderOffset"][1] },
                .Enabled = ctx.Json.value("Enabled", true),
                .IsTrigger = ctx.Json.value("IsTrigger", false)
            };
            ctx.Registry.emplace<Collider2DComponent>(ctx.Entity, collider);
            collisionSystem.AddListener(ctx.Entity, collider);
        });

    RegisterGameObjectComponent(kCameraFollowComponent, [](const ComponentInitContext& ctx)
        {
            ctx.Registry.emplace<CameraFollowComponent>(ctx.Entity);
        });

    RegisterGameObjectComponent(kDirectionalLightComponent, [](const ComponentInitContext& ctx)
        {
            DirectionalLightComponent directionalLightComponent = DirectionalLightComponent
            {
                .DirectionalLightMemoryPoolIndex = memoryPoolSystem.AllocateObject(kDirectionalLightBuffer)
            };
            ctx.Registry.emplace<DirectionalLightComponent>(ctx.Entity, directionalLightComponent);

            DirectionalLight& directionalLight = memoryPoolSystem.UpdateDirectionalLight(directionalLightComponent.DirectionalLightMemoryPoolIndex);
            directionalLight = DirectionalLight
            {
                .LightColor =     vec3(ctx.Json["LightColor"][0], ctx.Json["LightColor"][1], ctx.Json["LightColor"][2]),
                .LightDirection = vec3(ctx.Json["LightDirection"][0], ctx.Json["LightDirection"][1], ctx.Json["LightDirection"][2]),
                .LightIntensity = ctx.Json["LightIntensity"],
                .ShadowStrength = ctx.Json["ShadowStrength"],
                .ShadowBias =     ctx.Json["ShadowBias"],
                .ShadowSoftness = ctx.Json["ShadowSoftness"],
            };
        });

    RegisterGameObjectComponent(kPointLightComponent, [](const ComponentInitContext& ctx)
        {
            PointLightComponent pointLightComponent = PointLightComponent
            {
                .PointLightMemoryPoolIndex = memoryPoolSystem.AllocateObject(kPointLightBuffer)
            };
            ctx.Registry.emplace<PointLightComponent>(ctx.Entity, pointLightComponent);

            PointLight& pointLight = memoryPoolSystem.UpdatePointLight(pointLightComponent.PointLightMemoryPoolIndex);
            pointLight = PointLight
            {
                .LightPosition = vec3(ctx.Position2DOverride.x, ctx.Position2DOverride.y, ctx.Json["LightPosition"][2]),
                .LightColor = vec3(ctx.Json["LightColor"][0], ctx.Json["LightColor"][1], ctx.Json["LightColor"][2]),
                .LightRadius = ctx.Json["LightRadius"],
                .LightIntensity = ctx.Json["LightIntensity"],
                .ShadowStrength = ctx.Json["ShadowStrength"],
                .ShadowBias = ctx.Json["ShadowBias"],
                .ShadowSoftness = ctx.Json["ShadowSoftness"],
            };
        });

    RegisterGameObjectComponent(kDebugObjectComponent, [](const ComponentInitContext& ctx)
        {
            ctx.Registry.emplace<DebugObjectComponent>(ctx.Entity);
        });
}