#pragma once
#include "pch.h"
#include "RigidBody.h"

struct AABB
{
	uint RigidBodyId = UINT32_MAX;
	vec2 Min = vec2(0.0f);
	vec2 Max = vec2(0.0f);
	vec2 HalfSize = vec2(0.0f);
	vec2 Center = vec2(0.0f);
	vec2 Size = vec2(0.0f);
};

DLL_EXPORT AABB AABB_CreateAABBCollider(vec2& min, vec2& max);
DLL_EXPORT vec2 AABB_GetAABBMin(AABB& aabb, RigidBody& rigidBody);
DLL_EXPORT vec2 AABB_GetAABBMax(AABB& aabb, RigidBody& rigidBody);
