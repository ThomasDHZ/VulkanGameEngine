#include "RegisterGameObjectComponents.h"

void RegisterGameObjectComponents()
{
    gameObjectComponentRegistry.RegisterGameObjectComponent(kCameraFollowComponent, [](const ComponentInitContext& ctx)
        {
            ctx.Registry.emplace<CameraFollowComponent>(ctx.Entity);
        });

    gameObjectComponentRegistry.RegisterGameObjectComponent(kDebugObjectComponent, [](const ComponentInitContext& ctx)
        {
            ctx.Registry.emplace<DebugObjectComponent>(ctx.Entity);
        });

    gameObjectComponentRegistry.RegisterDefaultGameObjectComponents();
}

