#include "InputSystem.h"
#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "LevelSystem.h"

InputSystem inputSystem = InputSystem();
InputSystem::InputSystem()
{
}

InputSystem::~InputSystem()
{
}

void InputSystem::Update(const float& deltaTime)
{
    for (auto& input : gameObjectSystem.InputComponentList())
    {
        Sprite* sprite = spriteSystem.FindSprite(input.GameObjectId);
        Transform2DComponent& transform = gameObjectSystem.FindTransform2DComponent(input.GameObjectId);

        int joy = GLFW_JOYSTICK_1;
        if (glfwJoystickPresent(GLFW_JOYSTICK_1)) 
        {
            if (glfwJoystickIsGamepad(joy))
            {
                GLFWgamepadstate state;
                glfwGetGamepadState(joy, &state);
                if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] &&
                    state.buttons[GLFW_GAMEPAD_BUTTON_SQUARE])
                {
                    sprite->FlipSprite.x = 0;
                    transform.GameObjectPosition.x -= 200.0f * deltaTime;
                    spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kShootWalk);
                }
                else if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT])
                {
                    sprite->FlipSprite.x = 0;
                    transform.GameObjectPosition.x -= 200.0f * deltaTime;
                    spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kWalking);
                }
                else if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] &&
                         state.buttons[GLFW_GAMEPAD_BUTTON_SQUARE])
                {
                    sprite->FlipSprite.x = 1;
                    transform.GameObjectPosition.x += 200.0f * deltaTime;
                    spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kShootWalk);
                }
                else if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT])
                {
                    sprite->FlipSprite.x = 1;
                    transform.GameObjectPosition.x += 200.0f * deltaTime;
                    spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kWalking);
                }
                else if (state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] &&
                         state.buttons[GLFW_GAMEPAD_BUTTON_CROSS])
                {
                    sprite->FlipSprite.x == 1 ? transform.GameObjectPosition.x += 200.0f * deltaTime : transform.GameObjectPosition.x -= 200.0f * deltaTime;
                    spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kSlide);
                }
                else if (state.buttons[GLFW_GAMEPAD_BUTTON_SQUARE])
                {
                    spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kShoot);

                    if (a)
                    {
                        Vector<ComponentTypeEnum> ab = Vector<ComponentTypeEnum>
                        {
                            kSpriteComponent,
                            kTransform2DComponent,
                        };
                        gameObjectSystem.CreateGameObject("asdfa", ab, VkGuid("129950e8-683a-4379-96df-36dc07b810d5"), transform.GameObjectPosition);
                    }
                }
                else
                {
                    spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kStanding);
                }
            }
            else {
                std::cout << "Not mapped as gamepad. Raw axes/buttons available." << std::endl;
   
                int count;
                const float* axes = glfwGetJoystickAxes(joy, &count);
                const unsigned char* buttons = glfwGetJoystickButtons(joy, &count);
                if (count > 0) {
                    std::cout << "Raw axes count: " << count << std::endl;
                }
            }
        }
        
      /*  if ((vulkanWindow->keyboard.KeyPressed[KEY_A] == KS_PRESSED || vulkanWindow->keyboard.KeyPressed[KEY_A] == KS_HELD) &&
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
        }*/
    }
}