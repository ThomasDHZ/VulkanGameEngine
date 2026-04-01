#pragma once
#include "Platform.h"

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

class CameraSystem
{
public:
    static CameraSystem& Get();

private:
    CameraSystem() = default;
    ~CameraSystem() = default;
    CameraSystem(const CameraSystem&) = delete;
    CameraSystem& operator=(const CameraSystem&) = delete;
    CameraSystem(CameraSystem&&) = delete;
    CameraSystem& operator=(CameraSystem&&) = delete;


public:
	uint ActiveCameraIndex = 0;
    Vector<Camera> CameraList;

    DLL_EXPORT void CreateCamera(const ivec2& renderResolution, const vec2& worldPosition = vec2(0.0f));
	DLL_EXPORT void Update();
	DLL_EXPORT void SetCameraPosition(uint cameraIndex, vec2 worldPosition, float zoom);
};
extern DLL_EXPORT CameraSystem& cameraSystem;
inline CameraSystem& CameraSystem::Get()
{
    static CameraSystem instance;
    return instance;
}
