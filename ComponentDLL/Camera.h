#pragma once
#include "pch.h"

struct Camera
{
	float Width;
	float Height;
	float AspectRatio;
	float Yaw;
	float Pitch;
	float Zoom;
	float ZNear = 0.1f;
	float ZFar = 10000.0f;

	float MovementSpeed;
	float MouseSensitivity;

	vec3 Front;
	vec3 Up;
	vec3 Right;
	vec3 WorldUp;

	vec3 Position;
	vec2 ViewScreenSize;
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
};

DLL_EXPORT Camera Camera_OrthographicCamera2D(const vec2& viewScreenSize);
DLL_EXPORT Camera Camera_OrthographicCamera2D(const vec2& viewScreenSize, const vec2& position);
DLL_EXPORT void Camera_OrthographicUpdate(Camera& camera);

DLL_EXPORT Camera Camera_PerspectiveCamera(const vec2& viewScreenSize, const glm::vec3& position);
DLL_EXPORT void Camera_PerspectiveUpdate(Camera& camera);
DLL_EXPORT void  Camera_PerspectiveUpdateMouse();