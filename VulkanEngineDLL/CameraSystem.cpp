#include "CameraSystem.h"

CameraSystem& cameraSystem = CameraSystem::Get();

void CameraSystem::CreateCamera(const ivec2& renderResolution, const vec2& worldPosition)
{
	CameraList.emplace_back(Camera
	{
		.Width = static_cast<float>(renderResolution.x),
		.Height = static_cast<float>(renderResolution.y),
		.AspectRatio = static_cast<float>(renderResolution.x) / static_cast<float>(renderResolution.y),
		.Zoom = 1.0f,
		.Position = vec3(worldPosition.x, worldPosition.y, 0.0f),
		.ViewScreenSize = vec2(renderResolution),
		.ProjectionMatrix = glm::ortho(0.0f, static_cast<float>(renderResolution.x), static_cast<float>(renderResolution.y), 0.0f, -1000.0f, 1000.0f),
		.ViewMatrix = glm::translate(glm::mat4(1.0f), -vec3(worldPosition.x, worldPosition.y, 0.0f))
	});
}

void CameraSystem::Update()
{ 
	CameraList[ActiveCameraIndex].ProjectionMatrix = glm::ortho(0.0f, CameraList[ActiveCameraIndex].Width * CameraList[ActiveCameraIndex].Zoom, CameraList[ActiveCameraIndex].Height * CameraList[ActiveCameraIndex].Zoom, 0.0f, -1000.0f, 1000.0f);
	CameraList[ActiveCameraIndex].ViewMatrix = glm::translate(mat4(1.0f), -CameraList[ActiveCameraIndex].Position);
}
