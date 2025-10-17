#pragma once
#include "DLL.h"

struct RigidBody
{
	vec2 forces = vec2(0.0f);
	vec2 position = vec2(0.0f);
	vec2 velocity = vec2(0.0f);
	vec2 acceleration = vec2(0.0f);

	float mass = 0.0f;
	float torque = 0.0;
	float inertia = 1.0f;
	float rotation = 0.0f;
	float inverseMass = 0.0f;
	float orientation = 0.0f;
	float inverseInertia = 1.0f;
	float angularVelocity = 0.0f;
	bool StaticBody = false;
};

DLL_EXPORT void RigidBody_CreateRigidBody(vec2 position, float mass, bool staticBody);