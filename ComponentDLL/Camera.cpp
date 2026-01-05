#include "pch.h"
#include "Camera.h"

Camera CameraOrthographic_OrthograpCamera_OrthographicCamera2DhicCamera2D(const vec2& viewScreenSize)
{
	return Camera
	{
		.Width = viewScreenSize.x,
		.Height = viewScreenSize.y,
		.AspectRatio = viewScreenSize.x / viewScreenSize.y,
		.Zoom = 1.0f,
		.Position = vec3(0.0f),
		.ViewScreenSize = viewScreenSize,
		.ProjectionMatrix = glm::ortho(0.0f, viewScreenSize.x, viewScreenSize.y, 0.0f),
		.ViewMatrix = mat4(1.0f)
	};
}

Camera Camera_OrthographicCamera2D(const vec2& viewScreenSize, const vec2& position)
{
	return Camera
	{
		.Width = viewScreenSize.x,
		.Height = viewScreenSize.y,
		.AspectRatio = viewScreenSize.x / viewScreenSize.y,
		.Zoom = 1.0f,
		.Position = vec3(position.x, position.y, 0.0f),
		.ViewScreenSize = viewScreenSize,
		.ProjectionMatrix = glm::ortho(0.0f, viewScreenSize.x, viewScreenSize.y, 0.0f),
		.ViewMatrix = mat4(1.0f)
	};
}

void Camera_OrthographicUpdate(Camera& camera)
{
	mat4 view = mat4(1.0f);
	camera.ProjectionMatrix = glm::ortho(0.0f, camera.Width, camera.Height, 0.0f);
	shaderSystem.UpdatePushConstantValue<mat4>("sceneData", "Projection", camera.ProjectionMatrix);
	shaderSystem.UpdatePushConstantValue<mat4>("sceneData", "View", view);
	shaderSystem.UpdatePushConstantValue<vec3>("sceneData", "CameraPosition", camera.Position);
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

	shaderSystem.UpdatePushConstantValue<mat4>("skyBoxViewData", "Projection", camera.ProjectionMatrix);
	shaderSystem.UpdatePushConstantValue<mat4>("skyBoxViewData", "View", camera.ViewMatrix);
	shaderSystem.UpdatePushConstantValue<vec3>("skyBoxViewData", "CameraPosition", camera.Position);
}
