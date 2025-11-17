#pragma once
#include <InputEnum.h>
#include "VulkanWindow.h"
#include <GLFW/glfw3.h>

class Keyboard
{
private:
	KeyState KeyPressed[MAXKEYBOARDKEY];

public:
	Keyboard();
	~Keyboard();
	 static void KeyboardKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods);
	 const KeyState* GetKeyBoardState() const { return KeyPressed; }
};
extern Keyboard keyboard;

