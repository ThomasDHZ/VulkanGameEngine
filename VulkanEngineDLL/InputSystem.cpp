#include "InputSystem.h"
#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "LevelSystem.h"
#include <Keyboard.h>
#include <Mouse.h>

InputSystem& inputSystem = InputSystem::Get();

void InputSystem::Update(const float& deltaTime)
{
#ifndef PLATFORM_ANDROID
    int joy = GLFW_JOYSTICK_1;
    if (glfwJoystickIsGamepad(joy))
    {
        GLFWgamepadstate state;
        if (glfwGetGamepadState(joy, &state))
        {
            keyboard.KeyPressed[KEY_A] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] ? KS_PRESSED : KS_RELEASED;
            keyboard.KeyPressed[KEY_D] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] ? KS_PRESSED : KS_RELEASED;
            keyboard.KeyPressed[KEY_W] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] ? KS_PRESSED : KS_RELEASED;
            keyboard.KeyPressed[KEY_S] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] ? KS_PRESSED : KS_RELEASED;
            keyboard.KeyPressed[KEY_E] = state.buttons[GLFW_GAMEPAD_BUTTON_SQUARE] ? KS_PRESSED : KS_RELEASED;
        }
    }

    GLFWgamepadstate state;
    if (glfwGetGamepadState(joy, &state))
    {
        keyboard.KeyPressed[KEY_A] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] ? KS_PRESSED : KS_RELEASED;
        keyboard.KeyPressed[KEY_D] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] ? KS_PRESSED : KS_RELEASED;
        keyboard.KeyPressed[KEY_W] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] ? KS_PRESSED : KS_RELEASED;
        keyboard.KeyPressed[KEY_S] = state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] ? KS_PRESSED : KS_RELEASED;
        keyboard.KeyPressed[KEY_E] = state.buttons[GLFW_GAMEPAD_BUTTON_SQUARE] ? KS_PRESSED : KS_RELEASED;
    }
    if (state.buttons[GLFW_GAMEPAD_BUTTON_SQUARE] == KS_PRESSED)
    {
        int a = 34;
    }

    auto view = gameObjectSystem.EntityRegistry.view<InputComponent>();
    for (auto [entity, input] : view.each())
    {
        std::copy(keyboard.KeyPressed, keyboard.KeyPressed + MAXKEYBOARDKEY, input.KeyPressed);

        input.MouseX = mouse.X;
        input.MouseY = mouse.Y;
        input.MouseDeltaX = mouse.X - mouse.XLast;
        input.MouseDeltaY = mouse.Y - mouse.YLast;
        std::copy(mouse.MouseButtonState, mouse.MouseButtonState + MAXMOUSEKEY, input.MouseButtons);

        GLFWgamepadstate gamePadState = gameController.GetGamePadState();
        std::copy(gamePadState.buttons, gamePadState.buttons + input.maxButtonCount, input.buttonsList);
        std::copy(gamePadState.axes,    gamePadState.axes + input.maxAxesCount, input.axesList);
        input.DeltaTime = deltaTime;
    }
    std::copy(std::begin(keyboard.KeyPressed), std::end(keyboard.KeyPressed), std::begin(m_previousKeys));
    mouse.XLast = mouse.X;
    mouse.YLast = mouse.Y;
#endif
}

bool InputSystem::IsKeyDown(int key) const
{
    return keyboard.KeyPressed[key] == KS_PRESSED || keyboard.KeyPressed[key] == KS_HELD;
}

bool InputSystem::IsKeyPressed(int key) const
{
    return keyboard.KeyPressed[key] == KS_PRESSED && m_previousKeys[key] != KS_PRESSED;
}

bool InputSystem::IsKeyReleased(int key) const
{
    return keyboard.KeyPressed[key] == KS_RELEASED && m_previousKeys[key] != KS_RELEASED;
}
