#pragma once

#include <Platform.h>
#include "CameraSystem.h"

 Camera Camera_CreatePixelPerfectOrthographic(const glm::ivec2& renderResolution, const glm::vec2& worldPosition = glm::vec2(0.0f));
 void Camera_UpdateOrthographicPixelPerfect(Camera& camera);

 Camera Camera_PerspectiveCamera(const vec2& viewScreenSize, const glm::vec3& position);
 void Camera_PerspectiveUpdate(Camera& camera);
 void  Camera_PerspectiveUpdateMouse();