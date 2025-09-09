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
        if (vulkanWindow->keyboard.KeyPressed[KEY_A] == KS_PRESSED ||
            vulkanWindow->keyboard.KeyPressed[KEY_A] == KS_HELD)
        {
            transform.GameObjectPosition.x -= 200.0f * deltaTime;
            spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kWalking);
            sprite->FlipSprite.x = 0;
        }
        else if (vulkanWindow->keyboard.KeyPressed[KEY_D] == KS_PRESSED ||
            vulkanWindow->keyboard.KeyPressed[KEY_D] == KS_HELD)
        {
            transform.GameObjectPosition.x += 200.0f * deltaTime;
            spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kWalking);
            sprite->FlipSprite.x = 1;
        }
        else
        {
            spriteSystem.SetSpriteAnimation(sprite, Sprite::SpriteAnimationEnum::kStanding);
        }
    }
}