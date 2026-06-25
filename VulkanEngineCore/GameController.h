#pragma once
#include "InputEnum.h"
#include "VulkanWindow.h"

#ifndef __ANDROID__
class GameController
{
private:
	static constexpr float Sensitivity = 0.1f;
	GLFWgamepadstate GamePadState[4] = { };

public:
	GameController();
	~GameController();

	DLL_EXPORT bool ButtonPressed(int controllerId, int button);
	DLL_EXPORT vec2 LeftJoyStickMoved(int controllerId);
	DLL_EXPORT vec2 RightJoyStickMoved(int controllerId);
	DLL_EXPORT vec2 R2L2Pressed(int controllerId);
};
extern DLL_EXPORT GameController gameController;
#endif
