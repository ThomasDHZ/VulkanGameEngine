#include "InputSystem.h"
#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "LevelSystem.h"
#include "Keyboard.h"

InputSystem inputSystem = InputSystem();
InputSystem::InputSystem()
{
}

InputSystem::~InputSystem()
{
}

void InputSystem::Update(const float& deltaTime)
{
#ifndef PLATFORM_ANDROID
    int joy = GLFW_JOYSTICK_1;
    for (auto& input : gameObjectSystem.InputComponentList)
    {
        const GameObject& gameObject = gameObjectSystem.GameObjectList[input.GameObjectId];
        if (gameObjectSystem.ComponentBehaviorMap[gameObject.GameObjectType].ControllerInput)
        {
            int joy = GLFW_JOYSTICK_1;
            if (glfwJoystickPresent(GLFW_JOYSTICK_1))
            {
                if (glfwJoystickIsGamepad(joy))
                {
                    GLFWgamepadstate state;
                    glfwGetGamepadState(joy, &state);
                    gameObjectSystem.ComponentBehaviorMap[gameObject.GameObjectType].ControllerInput(input.GameObjectId, deltaTime, state);
                }
            }
            else 
            {
                std::cout << "Not mapped as gamepad. Raw axes/buttons available." << std::endl;

                int count;
                const float* axes = glfwGetJoystickAxes(joy, &count);
                const unsigned char* buttons = glfwGetJoystickButtons(joy, &count);
                if (count > 0) {
                    std::cout << "Raw axes count: " << count << std::endl;
                }
            }
        }
        if (gameObjectSystem.ComponentBehaviorMap[gameObject.GameObjectType].ControllerInput)
        {
            gameObjectSystem.ComponentBehaviorMap[gameObject.GameObjectType].KeyBoardInput(input.GameObjectId, deltaTime, keyboard.GetKeyBoardState());
        }
    }
#endif
}