#include "GameController.h"

void GameEngine_GLFW_GameControllerConnectCallBack(int controllerId, int event)
{
	if (event == GLFW_CONNECTED && glfwJoystickIsGamepad(controllerId))
	{
		vulkanWindow->gameControllerState.ControllerId = controllerId;
        GameEngine_GLFW_GameControllerStartUp(controllerId);
	}
	else if (event == GLFW_DISCONNECTED)
	{
		vulkanWindow->gameControllerState.ControllerId = -1;
	}
}

void GameEngine_GLFW_GameControllerStartUp(int controllerId)
{
    if (glfwJoystickIsGamepad(controllerId))
    {
        const char* name = glfwGetJoystickName(controllerId);
    }

    int present = glfwJoystickPresent(controllerId);
}

void GameEngine_GLFW_GameControllerJoyStickMoved(int controllerId, int axis)
{
    if (glfwGetGamepadState(controllerId, &vulkanWindow->gameControllerState.state))
    {
     /*   if (axis == GLFW_GAMEPAD_AXIS_LEFT_TRIGGER ||
            axis == GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER)
        {
            if (vulkanWindow->gameControllerState.state.axes[axis] != -1)
            {
                return 1.0f;
            }
            else
            {
                return 0.0f;
            }
        }
        else
        {
            if (vulkanWindow->gameControllerState.state.axes[axis] >= 0.1 ||
                vulkanWindow->gameControllerState.state.axes[axis] <= -0.1)
            {
                return vulkanWindow->gameControllerState.state.axes[axis];
            }
            else
            {
                return 0.0f;
            }
        }*/
    }
}

void GameEngine_GLFW_GameControllerButtonPressedEvent(int controllerId, int button)
{
    if (glfwGetGamepadState(controllerId, &vulkanWindow->gameControllerState.state))
    {
        if (vulkanWindow->gameControllerState.state.buttons[button] == GLFW_PRESS)
        {
            if (button < MAXMOUSEKEY)
            {
                vulkanWindow->gameControllerState.state.buttons[button] = GLFW_PRESS;
            }
        }
        else if (vulkanWindow->gameControllerState.state.buttons[button] == GLFW_RELEASE)
        {
            if (button < MAXMOUSEKEY)
            {
                vulkanWindow->gameControllerState.state.buttons[button] = GLFW_RELEASE;
            }
        }
    }
}