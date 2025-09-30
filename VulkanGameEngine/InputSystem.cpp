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
        if (gameObjectSystem.ComponentBehaviorMap[input.GameObjectId].Input)
        {
            gameObjectSystem.ComponentBehaviorMap[input.GameObjectId].Input(input.GameObjectId, deltaTime);
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