#include "CameraSystemDLL.h"

Camera* CameraSystem_UpdateActiveCamera()
{
	return &cameraSystem.CameraList[cameraSystem.ActiveCameraIndex];
}