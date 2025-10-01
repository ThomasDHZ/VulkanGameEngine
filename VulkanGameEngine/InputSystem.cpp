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
    int joy = GLFW_JOYSTICK_1;
    for (auto& input : gameObjectSystem.InputComponentList())
    {
        Sprite* sprite = spriteSystem.FindSprite(input.GameObjectId);
        Transform2DComponent& transform = gameObjectSystem.FindTransform2DComponent(input.GameObjectId);

        if (gameObjectSystem.ComponentBehaviorMap[input.GameObjectId].ControllerInput)
        {
            int joy = GLFW_JOYSTICK_1;
            if (glfwJoystickPresent(GLFW_JOYSTICK_1))
            {
                if (glfwJoystickIsGamepad(joy))
                {
                    GLFWgamepadstate state;
                    glfwGetGamepadState(joy, &state);
                    gameObjectSystem.ComponentBehaviorMap[input.GameObjectId].ControllerInput(deltaTime, state, *sprite, transform);
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
        if (gameObjectSystem.ComponentBehaviorMap[input.GameObjectId].ControllerInput)
        {
            gameObjectSystem.ComponentBehaviorMap[input.GameObjectId].KeyBoardInput(deltaTime, keyboard.GetKeyBoardState(), *sprite, transform);
        }
        
     
    }
}