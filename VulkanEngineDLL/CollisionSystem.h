#pragma once
#include "Platform.h"
#include "GameObjectSystem.h"
#include "ComponentSystem.h"
#include <entt/entt.hpp>

struct CollisionEvent
{
    entt::entity EntityA = entt::null;
    entt::entity EntityB = entt::null;

    vec2 CollisionPoint = vec2(0.0f);
    bool IsTrigger = false;
};

class CollisionSystem
{
public:
    static CollisionSystem& Get();

private:
    CollisionSystem() = default;
    ~CollisionSystem() = default;
    CollisionSystem(const CollisionSystem&) = delete;
    CollisionSystem& operator=(const CollisionSystem&) = delete;
    CollisionSystem(CollisionSystem&&) = delete;
    CollisionSystem& operator=(CollisionSystem&&) = delete;

    std::unordered_map<entt::entity, Collider2DComponent> ListenerMap;
    void HandleCollision(const CollisionEvent& event);
    bool AABBIntersect(const Transform2DComponent& t1, const Collider2DComponent& c1, const Transform2DComponent& t2, const Collider2DComponent& c2);

public:

    DLL_EXPORT void Update();
    DLL_EXPORT void AddListener(entt::entity entity, Collider2DComponent listener);
    DLL_EXPORT bool FindListener(entt::entity entity);
    DLL_EXPORT void RemoveListener(entt::entity entity);
};
extern DLL_EXPORT CollisionSystem& collisionSystem;
inline CollisionSystem& CollisionSystem::Get()
{
    static CollisionSystem instance;
    return instance;
}