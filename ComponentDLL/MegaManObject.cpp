#include "pch.h"
#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "MegaManObject.h"

void MegaMan_Behaviors(GameObjectBehavior& componentBehavior)
{
    componentBehavior.KeyBoardInput = MegaMan_KeyBoardInput;
    componentBehavior.ControllerInput = MegaMan_ControllerInput;
    componentBehavior.Update = MegaMan_Update;
    componentBehavior.Destroy = nullptr;
}

void MegaMan_Update(uint gameObjectId, const float& deltaTime)
{
    GameObject& gameObject = GameObjectSystem_FindGameObject(gameObjectId);
    if (gameObject.GameObjectType == GameObjectTypeEnum::kGameObjectMegaMan)
    {
        static_cast<MegaManObject*>(gameObject.GameObjectData)->CurrentShotTime += deltaTime;
    }
}

void MegaMan_KeyBoardInput(uint gameObjectId, const float& deltaTime, const KeyState* keyBoardStateArray)
{
    //Span<KeyState> keyBoardState(const_cast<KeyState*>(keyBoardStateArray), MAXKEYBOARDKEY);
    //if ((keyBoardState[KEY_A] == KS_PRESSED || keyBoardState[KEY_A] == KS_HELD) &&
    //     keyBoardState[KEY_E] == KS_PRESSED)
    //{
    //    sprite.FlipSprite.x = 0;
    //    transform.GameObjectPosition.x -= 200.0f * deltaTime;
    //    Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kShootWalk);
    //}
    //else if (keyBoardState[KEY_A] == KS_PRESSED ||
    //         keyBoardState[KEY_A] == KS_HELD)
    //{
    //    sprite.FlipSprite.x = 0;
    //    transform.GameObjectPosition.x -= 200.0f * deltaTime;
    //    Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kWalking);
    //}
    //else if ((keyBoardState[KEY_D] == KS_PRESSED || keyBoardState[KEY_D] == KS_HELD) &&
    //          keyBoardState[KEY_E] == KS_PRESSED)
    //{
    //    sprite.FlipSprite.x = 1;
    //    transform.GameObjectPosition.x += 200.0f * deltaTime;
    //    Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kShootWalk);
    //}
    //else if (keyBoardState[KEY_D] == KS_PRESSED ||
    //         keyBoardState[KEY_D] == KS_HELD)
    //{
    //    sprite.FlipSprite.x = 1;
    //    transform.GameObjectPosition.x += 200.0f * deltaTime;
    //    Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kWalking);
    //}
    //else if ((keyBoardState[KEY_S] == KS_PRESSED || keyBoardState[KEY_S] == KS_HELD) &&
    //         (keyBoardState[KEY_SPACE] == KS_PRESSED || keyBoardState[KEY_D] == KEY_SPACE))
    //{
    //    sprite.FlipSprite.x == 1 ? transform.GameObjectPosition.x += 200.0f * deltaTime : transform.GameObjectPosition.x -= 200.0f * deltaTime;
    //    Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kSlide);
    //}
    //else if (keyBoardState[KEY_E] == KS_PRESSED ||
    //         keyBoardState[KEY_E] == KS_HELD)
    //{
    //    Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kShoot);
    //}
    //else
    //{
    //    Sprite_SetSpriteAnimation(sprite, MegaManAnimationEnum::kStanding);
    //}
}

void MegaMan_ControllerInput(uint gameObjectId, const float& deltaTime, const GLFWgamepadstate& controllerState)
{
    Sprite* sprite = SpriteSystem_FindSprite(gameObjectId);
    const GameObject& gameObject = GameObjectSystem_FindGameObject(gameObjectId);
    Transform2DComponent transform = GameObjectSystem_FindTransform2DComponent(gameObjectId);
    if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] &&
        controllerState.buttons[GLFW_GAMEPAD_BUTTON_SQUARE])
    {
        sprite->FlipSprite.x = 0;
        transform.GameObjectPosition.x -= 200.0f * deltaTime;
        SpriteSystem_SetSpriteAnimation(sprite, MegaManAnimationEnum::kShootWalk);
    }
    else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT])
    {
        sprite->FlipSprite.x = 0;
        transform.GameObjectPosition.x -= 200.0f * deltaTime;
        SpriteSystem_SetSpriteAnimation(sprite, MegaManAnimationEnum::kWalking);
    }
    else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] &&
             controllerState.buttons[GLFW_GAMEPAD_BUTTON_SQUARE])
    {
        sprite->FlipSprite.x = 1;
        transform.GameObjectPosition.x += 200.0f * deltaTime;
        SpriteSystem_SetSpriteAnimation(sprite, MegaManAnimationEnum::kShootWalk);
    }
    else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT])
    {
        sprite->FlipSprite.x = 1;
        transform.GameObjectPosition.x += 200.0f * deltaTime;
        SpriteSystem_SetSpriteAnimation(sprite, MegaManAnimationEnum::kWalking);
    }
    else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] &&
             controllerState.buttons[GLFW_GAMEPAD_BUTTON_CROSS])
    {
        sprite->FlipSprite.x == 1 ? transform.GameObjectPosition.x += 200.0f * deltaTime : transform.GameObjectPosition.x -= 200.0f * deltaTime;
        SpriteSystem_SetSpriteAnimation(sprite, MegaManAnimationEnum::kSlide);
    }
    else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_SQUARE])
    {
        MegaManObject* objectData = static_cast<MegaManObject*>(gameObject.GameObjectData);
        SpriteSystem_SetSpriteAnimation(sprite, MegaManAnimationEnum::kShoot);
        MegaManShot_CreateObject("Shot", VkGuid("623e5b6b-b1f8-4e69-8dca-237069a373e2"), transform.GameObjectPosition + objectData->ShotPostionOffset, gameObject.GameObjectId);
    }
    else
    {
        SpriteSystem_SetSpriteAnimation(sprite, MegaManAnimationEnum::kStanding);
    }
}
