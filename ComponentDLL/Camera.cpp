#include "pch.h"
#include "Camera.h"

Camera Camera_OrthographicCamera2D(const vec2& viewScreenSize)
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

void Camera_Update(Camera& camera, ShaderPushConstantDLL sceneDataBuffer)
{
	mat4 view = mat4(1.0f);
	camera.ProjectionMatrix = glm::ortho(0.0f, camera.Width, camera.Height, 0.0f);
	memcpy(shaderSystem.FindShaderPushConstantStructVariable(sceneDataBuffer, "Projection").Value, &camera.ProjectionMatrix, sizeof(camera.ProjectionMatrix));
	memcpy(shaderSystem.FindShaderPushConstantStructVariable(sceneDataBuffer, "View").Value, &view, sizeof(view));
	memcpy(shaderSystem.FindShaderPushConstantStructVariable(sceneDataBuffer, "CameraPosition").Value, &camera.Position, sizeof(camera.Position));
}
