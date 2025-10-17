#include "pch.h"
#include "AABB.h"

 AABB AABB_CreateAABBCollider(vec2& min, vec2& max)
{
	 vec2 size = max - min;
	 return AABB
	 {
		 .Center = min + (vec2(size) * 0.5f),
		 .Size = size
	 };
}

 vec2 AABB_GetAABBMin(AABB& aabb, RigidBody& rigidBody)
 {
	 return rigidBody.position - aabb.HalfSize;
 }

 vec2 AABB_GetAABBMax(AABB& aabb, RigidBody& rigidBody)
 {
	 return rigidBody.position + aabb.HalfSize;
 }
