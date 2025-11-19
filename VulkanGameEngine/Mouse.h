#pragma once
#include "InputEnum.h"
#include "VulkanWindow.h"
#include <GLFW/glfw3.h>

class Mouse
{
private:

public:
	float X;
	float Y;
	float XOffset;
	float YOffset;
	int WheelOffset;
	bool MouseButtonState[MAXMOUSEKEY];

	Mouse();
	~Mouse();
	static void MouseMoveEvent(GLFWwindow* window, double Xoffset, double Yoffset);
	static void MouseButtonPressedEvent(GLFWwindow* window, int button, int action, int mods);
	static void MouseWheelEvent(GLFWwindow* window, double xpos, double ypos);
};
extern Mouse mouse;

