#pragma once
#include "VulkanWindow.h"
#include "GameController.h"

class InputSystem
{
private:
	int PrimaryController = -1;
	MouseState  mouse;
	KeyboardState keyboard;
	GLFWgamepadstate ControllerState[4];

	void GameControllerJoyStickMoved(int axis);
	void GameControllerButtonPressedEvent(int axis);

public:
	InputSystem();
	~InputSystem();

	void Update(const float& deltaTime);

};
extern InputSystem inputSystem;
