#include "pch.h"
#include "Camera.h"

Camera Camera_CreatePixelPerfectOrthographic(const glm::ivec2& renderResolution, const glm::vec2& worldPosition)
{
	Camera cam{};
	cam.Width = static_cast<float>(renderResolution.x);
	cam.Height = static_cast<float>(renderResolution.y);
	cam.AspectRatio = cam.Width / cam.Height;
	cam.Zoom = 1.0f;
	cam.Position = glm::vec3(worldPosition.x, worldPosition.y, 0.0f);
	cam.ViewScreenSize = glm::vec2(renderResolution);

	cam.ProjectionMatrix = glm::ortho(0.0f, cam.Width, cam.Height, 0.0f, -1000.0f, 1000.0f);
	cam.ViewMatrix = glm::translate(glm::mat4(1.0f), -cam.Position);

	return cam;
}

void Camera_UpdateOrthographicPixelPerfect(Camera& camera)
{
	camera.ProjectionMatrix = glm::ortho(0.0f, camera.Width, camera.Height, 0.0f, -1000.0f, 1000.0f);
	camera.ViewMatrix = glm::translate(glm::mat4(1.0f), -camera.Position);
}

Camera Camera_PerspectiveCamera(const vec2& viewScreenSize, const glm::vec3& position)
{
	return Camera
	{
		.Width = viewScreenSize.x,
		.Height = viewScreenSize.y,
		.Yaw = -90.0f,
		.Pitch = 0.0f,
		.Zoom = 45.0f,
		.MovementSpeed = 0.005f,
		.MouseSensitivity = 0.0001f,
		.Front = glm::vec3(0.0f, 0.0f, -1.0f),
		.Up = glm::vec3(0.0f, 1.0f, 0.0f),
		.WorldUp = glm::vec3(0.0f, 1.0f, 0.0f),
		.Position = position,
	};
}

void Camera_PerspectiveUpdate(Camera& camera)
{
	vec3 front = vec3(0.0f);
	front.x = cos(glm::radians(camera.Yaw)) * cos(glm::radians(camera.Pitch));
	front.y = sin(glm::radians(camera.Pitch));
	front.z = sin(glm::radians(camera.Yaw)) * cos(glm::radians(camera.Pitch));
	camera.Front = glm::normalize(front);

	camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
	camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));

	camera.ViewMatrix = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);

	const auto 	Aspect = camera.Width / camera.Height;
	camera.ProjectionMatrix = glm::perspective(glm::radians(90.0f), camera.Width / camera.Height, 0.1f, 10000.0f);
	camera.ProjectionMatrix[1][1] *= -1;
}
