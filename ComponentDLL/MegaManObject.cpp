#include "pch.h"
#include "MegaManObject.h"

void MegaMan_Behaviors(ComponentBehavior& componentBehavior)
{
    componentBehavior.KeyBoardInput = MegaMan_KeyBoardInput;
    componentBehavior.ControllerInput = MegaMan_ControllerInput;
    componentBehavior.Movement = nullptr;
    componentBehavior.Destroy = nullptr;
}

void MegaMan_KeyBoardInput(const float& deltaTime, const KeyState* keyBoardStateArray, Sprite& sprite, Transform2DComponent& transform)
{
    Span<KeyState> keyBoardState(const_cast<KeyState*>(keyBoardStateArray), MAXKEYBOARDKEY);
    if ((keyBoardState[KEY_A] == KS_PRESSED || keyBoardState[KEY_A] == KS_HELD) &&
         keyBoardState[KEY_E] == KS_PRESSED)
    {
        sprite.FlipSprite.x = 0;
        transform.GameObjectPosition.x -= 200.0f * deltaTime;
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kShootWalk);
    }
    else if (keyBoardState[KEY_A] == KS_PRESSED ||
             keyBoardState[KEY_A] == KS_HELD)
    {
        sprite.FlipSprite.x = 0;
        transform.GameObjectPosition.x -= 200.0f * deltaTime;
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kWalking);
    }
    else if ((keyBoardState[KEY_D] == KS_PRESSED || keyBoardState[KEY_D] == KS_HELD) &&
              keyBoardState[KEY_E] == KS_PRESSED)
    {
        sprite.FlipSprite.x = 1;
        transform.GameObjectPosition.x += 200.0f * deltaTime;
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kShootWalk);
    }
    else if (keyBoardState[KEY_D] == KS_PRESSED ||
             keyBoardState[KEY_D] == KS_HELD)
    {
        sprite.FlipSprite.x = 1;
        transform.GameObjectPosition.x += 200.0f * deltaTime;
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kWalking);
    }
    else if ((keyBoardState[KEY_S] == KS_PRESSED || keyBoardState[KEY_S] == KS_HELD) &&
             (keyBoardState[KEY_SPACE] == KS_PRESSED || keyBoardState[KEY_D] == KEY_SPACE))
    {
        sprite.FlipSprite.x == 1 ? transform.GameObjectPosition.x += 200.0f * deltaTime : transform.GameObjectPosition.x -= 200.0f * deltaTime;
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kSlide);
    }
    else if (keyBoardState[KEY_E] == KS_PRESSED ||
             keyBoardState[KEY_E] == KS_HELD)
    {
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kShoot);
    }
    else
    {
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kStanding);
    }
}

void MegaMan_ControllerInput(const float& deltaTime, const GLFWgamepadstate& controllerState, Sprite& sprite, Transform2DComponent& transform)
{
    if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] &&
        controllerState.buttons[GLFW_GAMEPAD_BUTTON_SQUARE])
    {
        sprite.FlipSprite.x = 0;
        transform.GameObjectPosition.x -= 200.0f * deltaTime;
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kShootWalk);
    }
    else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT])
    {
        sprite.FlipSprite.x = 0;
        transform.GameObjectPosition.x -= 200.0f * deltaTime;
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kWalking);
    }
    else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] &&
             controllerState.buttons[GLFW_GAMEPAD_BUTTON_SQUARE])
    {
        sprite.FlipSprite.x = 1;
        transform.GameObjectPosition.x += 200.0f * deltaTime;
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kShootWalk);
    }
    else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT])
    {
        sprite.FlipSprite.x = 1;
        transform.GameObjectPosition.x += 200.0f * deltaTime;
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kWalking);
    }
    else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] &&
             controllerState.buttons[GLFW_GAMEPAD_BUTTON_CROSS])
    {
        sprite.FlipSprite.x == 1 ? transform.GameObjectPosition.x += 200.0f * deltaTime : transform.GameObjectPosition.x -= 200.0f * deltaTime;
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kSlide);
    }
    else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_SQUARE])
    {
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kShoot);
    }
    else
    {
        Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kStanding);
    }
}
