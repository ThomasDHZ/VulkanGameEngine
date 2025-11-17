#include "Mouse.h"

Mouse mouse = Mouse();

Mouse::Mouse()
{

}

Mouse::~Mouse()
{
}

void Mouse::MouseMoveEvent(GLFWwindow* window, double x, double y)
{

	mouse.XOffset = mouse.XOffset - static_cast<float>(x);
	mouse.YOffset = static_cast<float>(y) - mouse.XOffset;
	mouse.X = static_cast<float>(x);
	mouse.Y = static_cast<float>(y);
}

void Mouse::MouseButtonPressedEvent(GLFWwindow* window, int button, int action, int mods)
{
	mouse.MouseButtonState[button] = action;
}

void Mouse::MouseWheelEvent(GLFWwindow* window, double xpos, double ypos)
{
	mouse.WheelOffset += (int)ypos;
}
