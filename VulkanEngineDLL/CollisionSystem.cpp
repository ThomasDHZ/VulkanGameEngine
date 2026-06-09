#include "CollisionSystem.h"
#include "GameObjectSystem.h"

CollisionSystem& collisionSystem = CollisionSystem::Get();

void CollisionSystem::Update(entt::registry& registry)
{
	auto view = registry.view<Transform2DComponent, Collider2DComponent>();
	for (auto [entityA, transformA, colliderA] : view.each())
	{
		if (!colliderA.Enabled) continue;
		for (auto [entityB, transformB, colliderB] : view.each())
		{
			if (entityB <= entityA) continue;
			if (!colliderB.Enabled) continue;
			if (AABBIntersect(transformA, colliderA, transformB, colliderB))
			{
				HandleCollision(CollisionEvent
					{
						.EntityA = entityA,
						.EntityB = entityB,
					});
			}
		}
	}
}

void CollisionSystem::AddListener(entt::entity entity, Collider2DComponent listener)
{
	ListenerMap[entity] = std::move(listener);
}

bool CollisionSystem::FindListener(entt::entity entity)
{
	auto it = ListenerMap.find(entity);
	return it != ListenerMap.end() && it->second.Enabled;
}

void CollisionSystem::RemoveListener(entt::entity entity)
{
	ListenerMap.erase(entity);
}

void CollisionSystem::HandleCollision(const CollisionEvent& event)
{
	entt::registry& registry = gameObjectSystem.EntityRegistry;
	GameObjectComponentLinker* gameObjectLinkerA = registry.try_get<GameObjectComponentLinker>(event.EntityA);
	if(gameObjectLinkerA);
	{
		GameObject gameObject = gameObjectSystem.FindGameObject(gameObjectLinkerA->GameObjectId);
		const GameObjectBehavior gameObjectBehavior = gameObjectSystem.FindGameObjectBehavior(gameObject.GameObjectType);
		if (gameObjectBehavior.OnCollisionEnter)
		{
			gameObjectBehavior.OnCollisionEnter(gameObject.ObjectPtr, gameObjectLinkerA->GameObjectId, 0);
		};
	}
	
	GameObjectComponentLinker* gameObjectLinkerB = registry.try_get<GameObjectComponentLinker>(event.EntityB);
	if (gameObjectLinkerB);
	{
		GameObject gameObject = gameObjectSystem.FindGameObject(gameObjectLinkerA->GameObjectId);
		const GameObjectBehavior gameObjectBehavior = gameObjectSystem.FindGameObjectBehavior(gameObject.GameObjectType);
		if (gameObjectBehavior.OnCollisionEnter)
		{
			gameObjectBehavior.OnCollisionEnter(gameObject.ObjectPtr, gameObjectLinkerB->GameObjectId, 0);
		};
	};
}

bool CollisionSystem::AABBIntersect(const Transform2DComponent& t1, const Collider2DComponent& c1, const Transform2DComponent& t2, const Collider2DComponent& c2)
{
	int left1 = (int)(t1.GameObjectPosition.x + c1.Offset.x);
	int right1 = left1 + c1.Size.x;
	int top1 = (int)(t1.GameObjectPosition.y + c1.Offset.y);
	int bottom1 = top1 + c1.Size.y;

	int left2 = (int)(t2.GameObjectPosition.x + c2.Offset.x);
	int right2 = left2 + c2.Size.x;
	int top2 = (int)(t2.GameObjectPosition.y + c2.Offset.y);
	int bottom2 = top2 + c2.Size.y;

	return !(right1 <= left2 || left1 >= right2 ||
		bottom1 <= top2 || top1 >= bottom2);
}
