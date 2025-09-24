#pragma once
#include "VulkanWindow.h"

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT void GameEngine_GLFW_GameControllerJoyStickMoved(int controllerId, int axis);
	DLL_EXPORT void GameEngine_GLFW_GameControllerButtonPressedEvent(int controllerId, int button);

	void GameEngine_GLFW_GameControllerConnectCallBack(int controllerId, int event);
	void GameEngine_GLFW_GameControllerStartUp(int controllerId);
#ifdef __cplusplus
}
#endif