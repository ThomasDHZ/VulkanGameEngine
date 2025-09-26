#include "VulkanWindow.h"
#include "GLFWWindow.h"
#include <stdio.h>
#include <nlohmann/json.hpp>
#include <ImPlot/implot.h>
#include "SystemClock.h"
#include <iostream>
#include "FrameTimer.h"
#include "GameSystem.h"
#include "MaterialSystem.h"
#include "EngineConfigSystem.h"

int joystick_id = -1;
int main(int argc, char** argv)
{
    GLFWgamepadstate state = GLFWgamepadstate();
    SystemClock systemClock = SystemClock();
    FrameTimer deltaTime = FrameTimer();
    vulkanWindow = Window_CreateWindow(WindowType::GLFW, "Game", configSystem.WindowResolution.x, configSystem.WindowResolution.y);
    gameSystem.StartUp(WindowType::GLFW, vulkanWindow->WindowHandle);


    while (!vulkanWindow->WindowShouldClose(vulkanWindow))
    {
        int a = glfwJoystickPresent(vulkanWindow->jid);
        if (glfwJoystickPresent(vulkanWindow->jid))
        {
            int a = 234;
        }
        const float frameTime = deltaTime.GetFrameTime();

        vulkanWindow->PollEventHandler(vulkanWindow);
        vulkanWindow->SwapBuffer(vulkanWindow);

        if (joystick_id != -1 && glfwGetGamepadState(joystick_id, &state)) {
            if (state.buttons[GLFW_GAMEPAD_BUTTON_CROSS]) {
                printf("Cross button pressed\n");
            }
            float leftX = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
            if (leftX != 0.0f) {
                printf("Left Stick X: %f\n", leftX);
            }
        }
        else {
            printf("Failed to get gamepad state\n");
        }

        gameSystem.Update(frameTime);
        gameSystem.DebugUpdate(frameTime);
        gameSystem.Draw(frameTime);
        deltaTime.EndFrameTime();
    }
    vkDeviceWaitIdle(renderSystem.renderer.Device);
    gameSystem.Destroy();
    vulkanWindow->DestroyWindow(vulkanWindow);
    return 0;
}