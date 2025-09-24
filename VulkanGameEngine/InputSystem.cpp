#include "InputSystem.h"
#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "LevelSystem.h"
//GLFWgamepadstate GameController::ControllerState;
InputSystem inputSystem = InputSystem();
InputSystem::InputSystem()
{
    PrimaryController = glfwJoystickPresent(GLFW_JOYSTICK_1);
}

InputSystem::~InputSystem()
{
}

void InputSystem::GameControllerJoyStickMoved(int axis)
{
    GameEngine_GLFW_GameControllerJoyStickMoved(PrimaryController, axis);
}

void InputSystem::GameControllerButtonPressedEvent(int button)
{
    GameEngine_GLFW_GameControllerButtonPressedEvent(PrimaryController, button);
}

void InputSystem::Update(const float& deltaTime)
{
    if (glfwGetGamepadState(PrimaryController, &ControllerState[PrimaryController]))
    {
        if (ControllerState[PrimaryController].buttons[GLFW_GAMEPAD_BUTTON_CROSS] == GLFW_PRESS)
        {
            int a = 34;
        }
        else if (ControllerState[PrimaryController].buttons[GLFW_GAMEPAD_BUTTON_SQUARE] == GLFW_PRESS)
        {
            int a = 34;
        }
    }
    for (auto& input : gameObjectSystem.InputComponentList())
    {
        Sprite* sprite = spriteSystem.FindSprite(input.GameObjectId);
        Transform2DComponent& transform = gameObjectSystem.FindTransform2DComponent(input.GameObjectId);

        if ((vulkanWindow->keyboard.KeyPressed[KEY_A] == KS_PRESSED || vulkanWindow->keyboard.KeyPressed[KEY_A] == KS_HELD) &&
             vulkanWindow->keyboard.KeyPressed[KEY_E] == KS_PRESSED)
        {
            sprite->FlipSprite.x = 0;
            transform.GameObjectPosition.x -= 200.0f * deltaTime;
            spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kShootWalk);
        }
        else if (vulkanWindow->keyboard.KeyPressed[KEY_A] == KS_PRESSED ||
                 vulkanWindow->keyboard.KeyPressed[KEY_A] == KS_HELD)
        {
            sprite->FlipSprite.x = 0;
            transform.GameObjectPosition.x -= 200.0f * deltaTime;
            spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kWalking);
        }
        else if ((vulkanWindow->keyboard.KeyPressed[KEY_D] == KS_PRESSED || vulkanWindow->keyboard.KeyPressed[KEY_D] == KS_HELD) &&
                  vulkanWindow->keyboard.KeyPressed[KEY_E] == KS_PRESSED)
        {
            sprite->FlipSprite.x = 1;
            transform.GameObjectPosition.x += 200.0f * deltaTime;
            spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kShootWalk);
        }
        else if (vulkanWindow->keyboard.KeyPressed[KEY_D] == KS_PRESSED ||
                 vulkanWindow->keyboard.KeyPressed[KEY_D] == KS_HELD)
        {
            sprite->FlipSprite.x = 1;
            transform.GameObjectPosition.x += 200.0f * deltaTime;
            spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kWalking);
        }
        else if ((vulkanWindow->keyboard.KeyPressed[KEY_S] == KS_PRESSED || vulkanWindow->keyboard.KeyPressed[KEY_S] == KS_HELD) &&
                 (vulkanWindow->keyboard.KeyPressed[KEY_SPACE] == KS_PRESSED || vulkanWindow->keyboard.KeyPressed[KEY_D] == KEY_SPACE))
        {
            sprite->FlipSprite.x == 1 ? transform.GameObjectPosition.x += 200.0f * deltaTime : transform.GameObjectPosition.x -= 200.0f * deltaTime;
            spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kSlide);
        }
        else if (vulkanWindow->keyboard.KeyPressed[KEY_E] == KS_PRESSED ||
                 vulkanWindow->keyboard.KeyPressed[KEY_E] == KS_HELD)
        {
            spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kShoot);
        }
        else
        {
            spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kStanding);
        }
    }
}