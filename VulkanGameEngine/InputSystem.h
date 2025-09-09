#pragma once
#include "VulkanWindow.h"
class InputSystem
{
private:
	MouseState  mouse;
	KeyboardState keyboard;

public:
	InputSystem();
	~InputSystem();

	void Update(const float& deltaTime);
};
extern InputSystem inputSystem;
