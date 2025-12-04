#include "GameController.h"

#ifndef __ANDROID__
GameController gameController = GameController();

GameController::GameController()
{
}

GameController::~GameController()
{
}

bool GameController::ButtonPressed(int controllerId, int button)
{
    if (glfwJoystickPresent(controllerId) &&
        glfwJoystickIsGamepad(controllerId))
    {
        return GamePadState[controllerId].buttons[button];
    }
    return false;
}

vec2 GameController::LeftJoyStickMoved(int controllerId)
{
    vec2 joyStickMovement = vec2(0.0f);
    if (glfwJoystickPresent(controllerId) &&
        glfwJoystickIsGamepad(controllerId))
    {
        glfwGetGamepadState(controllerId, &GamePadState[controllerId]);        
        joyStickMovement.x = GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_LEFT_X] > Sensitivity || GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_LEFT_X] < -Sensitivity ? GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_LEFT_X] : 0;
        joyStickMovement.y = GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_LEFT_Y] > Sensitivity || GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_LEFT_Y] < -Sensitivity ? GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_LEFT_Y] : 0;
    }
    return joyStickMovement;
}

vec2 GameController::RightJoyStickMoved(int controllerId)
{
    vec2 joyStickMovement = vec2(0.0f);
    if (glfwJoystickPresent(controllerId) &&
        glfwJoystickIsGamepad(controllerId))
    {
        glfwGetGamepadState(controllerId, &GamePadState[controllerId]);
        joyStickMovement.x = GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_RIGHT_X] > Sensitivity || GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_RIGHT_X] ? GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_RIGHT_X] : 0;
        joyStickMovement.y = GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] > Sensitivity || GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] ? GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] : 0;
    }
    return joyStickMovement;
}

vec2 GameController::R2L2Pressed(int controllerId)
{
    vec2 joyStickMovement = vec2(0.0f);
    if (glfwJoystickPresent(controllerId) &&
        glfwJoystickIsGamepad(controllerId))
    {
        glfwGetGamepadState(controllerId, &GamePadState[controllerId]);
        joyStickMovement.x = GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] > Sensitivity ? GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] : 0;
        joyStickMovement.y = GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] > Sensitivity ? GamePadState[controllerId].axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] : 0;
    }
    return joyStickMovement;
}
#endif