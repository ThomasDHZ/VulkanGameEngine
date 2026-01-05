#include "InputSystem.h"
#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "LevelSystem.h"
#include "Keyboard.h"
#include "Mouse.h"

InputSystem& inputSystem = InputSystem::Get();

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

    float xoffset = 0.0f;
    float yoffset = 0.0f;
    if (mouse.MouseButtonState[1])
    {
         xoffset = mouse.X - xoffset;
         yoffset = yoffset - mouse.Y;

        mouse.XLast = mouse.X;
        mouse.YLast = mouse.Y;
    }

        xoffset *= levelSystem.PerspectiveCamera->MouseSensitivity;
        yoffset *= levelSystem.PerspectiveCamera->MouseSensitivity;

        levelSystem.PerspectiveCamera->Yaw += xoffset;
        levelSystem.PerspectiveCamera->Pitch += yoffset;


        if (levelSystem.PerspectiveCamera->Pitch > 89.9999f)
        {
            levelSystem.PerspectiveCamera->Pitch = 89.9999f;
        }
        if (levelSystem.PerspectiveCamera->Pitch < -89.9999f)
        {
            levelSystem.PerspectiveCamera->Pitch = -89.9999f;
        }

#endif
}