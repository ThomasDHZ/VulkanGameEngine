#pragma once
#include <InputEnum.h>
#include "VulkanWindow.h"

class GameController
{
private:
	static constexpr float Sensitivity = 0.1f;
	GLFWgamepadstate GamePadState[4] = { };

public:	
	GameController();
	~GameController();

	bool ButtonPressed(int controllerId, int button);
	vec2 LeftJoyStickMoved(int controllerId);
	vec2 RightJoyStickMoved(int controllerId);
	vec2 R2L2Pressed(int controllerId);
};
extern GameController gameController;

