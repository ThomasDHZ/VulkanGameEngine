#pragma once
#include "pch.h"
#include <CameraSystem.h>

DLL_EXPORT Camera Camera_CreatePixelPerfectOrthographic(const glm::ivec2& renderResolution, const glm::vec2& worldPosition = glm::vec2(0.0f));
DLL_EXPORT void Camera_UpdateOrthographicPixelPerfect(Camera& camera);

DLL_EXPORT Camera Camera_PerspectiveCamera(const vec2& viewScreenSize, const glm::vec3& position);
DLL_EXPORT void Camera_PerspectiveUpdate(Camera& camera);
DLL_EXPORT void  Camera_PerspectiveUpdateMouse();