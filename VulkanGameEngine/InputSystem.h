#pragma once
#include <MegaManObject.h>

#include "VulkanWindow.h"
class InputSystem
{
private:

	bool a = true;
public:
	InputSystem();
	~InputSystem();

	void Update(const float& deltaTime);
};
extern InputSystem inputSystem;
