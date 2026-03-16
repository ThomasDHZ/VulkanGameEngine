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
    auto view = levelSystem.EntityRegistry.view<GameObjectComponentLinker, Sprite, Transform2DComponent, InputComponent, CameraFollowComponent>();
    for (auto [entity, gameObjectLinker, sprite, transform, input, cameraFollow] : view.each())
    {
        int joy = GLFW_JOYSTICK_1;
        const GameObject& gameObject = gameObjectSystem.GameObjectList[gameObjectLinker.GameObjectId];
        if (glfwJoystickPresent(GLFW_JOYSTICK_1))
        {
            if (glfwJoystickIsGamepad(joy))
            {
                GLFWgamepadstate controllerState;
                glfwGetGamepadState(joy, &controllerState);
                if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] &&
                    controllerState.buttons[GLFW_GAMEPAD_BUTTON_SQUARE])
                {
                    sprite.FlipSprite.x = 1;
                    transform.GameObjectPosition.x -= 200.0f * deltaTime;
                    if (levelSystem.EntityRegistry.all_of<const CameraFollowComponent>(entity)) cameraSystem.CameraList[cameraSystem.ActiveCameraIndex].Position.x -= 200.0f * deltaTime;
                //    spriteSystem.SetSpriteAnimation(&sprite, MegaManAnimationEnum::kShootWalk);
                }
                else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT])
                {
                    sprite.FlipSprite.x = 1;
                    transform.GameObjectPosition.x -= 200.0f * deltaTime;
                    if (levelSystem.EntityRegistry.all_of<const CameraFollowComponent>(entity)) cameraSystem.CameraList[cameraSystem.ActiveCameraIndex].Position.x -= 200.0f * deltaTime;
                   // spriteSystem.SetSpriteAnimation(&sprite, MegaManAnimationEnum::kWalking);
                }
                else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] &&
                    controllerState.buttons[GLFW_GAMEPAD_BUTTON_SQUARE])
                {
                    sprite.FlipSprite.x = 0;
                    transform.GameObjectPosition.x += 200.0f * deltaTime;
                    if (levelSystem.EntityRegistry.all_of<const CameraFollowComponent>(entity)) cameraSystem.CameraList[cameraSystem.ActiveCameraIndex].Position.x += 200.0f * deltaTime;
                 //   spriteSystem.SetSpriteAnimation(&sprite, MegaManAnimationEnum::kShootWalk);
                }
                else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT])
                {
                    sprite.FlipSprite.x = 0;
                    transform.GameObjectPosition.x += 200.0f * deltaTime;
                    if (levelSystem.EntityRegistry.all_of<const CameraFollowComponent>(entity)) cameraSystem.CameraList[cameraSystem.ActiveCameraIndex].Position.x += 200.0f * deltaTime;
                   // spriteSystem.SetSpriteAnimation(&sprite, MegaManAnimationEnum::kWalking);
                }
                else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] &&
                    controllerState.buttons[GLFW_GAMEPAD_BUTTON_CROSS])
                {
                    sprite.FlipSprite.x == 1 ? transform.GameObjectPosition.x += 200.0f * deltaTime : transform.GameObjectPosition.x -= 200.0f * deltaTime;
                    if (levelSystem.EntityRegistry.all_of<const CameraFollowComponent>(entity)) cameraSystem.CameraList[cameraSystem.ActiveCameraIndex].Position.x -= 200.0f * deltaTime;
                  //  spriteSystem.SetSpriteAnimation(&sprite, MegaManAnimationEnum::kSlide);
                }
                else if (controllerState.buttons[GLFW_GAMEPAD_BUTTON_SQUARE])
                {
                    /* MegaManObject* objectData = static_cast<MegaManObject*>(gameObject.GameObjectData);
                     spriteSystem.SetSpriteAnimation(&sprite, MegaManAnimationEnum::kShoot);
                     MegaManShot_CreateObject("Shot", VkGuid("623e5b6b-b1f8-4e69-8dca-237069a373e2"), transform.GameObjectPosition + objectData->ShotPostionOffset, gameObject.GameObjectId);*/
                }
                else
                {
                    spriteSystem.SetSpriteAnimation(&sprite, MegaManAnimationEnum::kStanding);
                }
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