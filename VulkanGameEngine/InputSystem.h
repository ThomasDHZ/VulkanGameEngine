#pragma once
#include "VulkanWindow.h"
class InputSystem
{
private:


public:
	InputSystem();
	~InputSystem();

	void Update(const float& deltaTime);
};
extern InputSystem inputSystem;
