#pragma once
#include "pch.h"

struct Camera
{
	float Width;
	float Height;
	float AspectRatio;
	float Zoom;

	vec3 Position;
	vec2 ViewScreenSize;
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
};

DLL_EXPORT Camera Camera_OrthographicCamera2D(const vec2& viewScreenSize);
DLL_EXPORT Camera Camera_OrthographicCamera2D(const vec2& viewScreenSize, const vec2& position);
DLL_EXPORT void Camera_Update(Camera& camera);