#pragma once

#include <Platform.h>
#include "CameraSystem.h"

ENGINE_DLL_EXPORT Camera Camera_CreatePixelPerfectOrthographic(const glm::ivec2& renderResolution, const glm::vec2& worldPosition = glm::vec2(0.0f));
ENGINE_DLL_EXPORT void Camera_UpdateOrthographicPixelPerfect(Camera& camera);

ENGINE_DLL_EXPORT Camera Camera_PerspectiveCamera(const vec2& viewScreenSize, const glm::vec3& position);
ENGINE_DLL_EXPORT void Camera_PerspectiveUpdate(Camera& camera);
ENGINE_DLL_EXPORT void  Camera_PerspectiveUpdateMouse();