#pragma once
#include "pch.h"
#include "RigidBody.h"

struct BoxCollider2D
{
	uint rigidBodyId = UINT32_MAX;
	vec2 Size = vec2(0.0f, 0.0f);
	vec2 HalfSize = vec2(0.0f, 0.0f);
};

struct PhysicsArchive
{
	Vector<RigidBody> RidgidBodyList;
	Vector<BoxCollider2D> BoxColliderList;
};
DLL_EXPORT extern PhysicsArchive physicsArchive;

DLL_EXPORT BoxCollider2D BoxCollider2D_CreateBoxCollider2D(vec2& min, vec2& max);
DLL_EXPORT vec2 BoxCollider2D_GetBoxCollider2DMin(BoxCollider2D& boxCollider, RigidBody& rigidBody);
DLL_EXPORT vec2 BoxCollider2D_GetBoxCollider2DMax(BoxCollider2D& boxCollider, RigidBody& rigidBody);
DLL_EXPORT Vector<vec2> BoxCollider2D_GetBoxColliderVertices(BoxCollider2D& boxCollider);