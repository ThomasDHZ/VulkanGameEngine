extern "C"
{
#include "VulkanWindow.h"
#include "GLFWWindow.h"
}
#include <stdio.h>
#include <nlohmann/json.hpp>
#include <ImPlot/implot.h>
#include "SystemClock.h"
#include <iostream>
#include "FrameTimer.h"
#include "GameSystem.h"
#include "MaterialSystem.h"
#include "EngineConfigSystem.h"

int main(int argc, char** argv)
{
    SystemClock systemClock = SystemClock();
    FrameTimer deltaTime = FrameTimer();
    VulkanWindow_CreateGraphicsWindow(WindowTypeEnum::GLFW, "Game", configSystem.WindowResolution.x, configSystem.WindowResolution.y);

    gameSystem.StartUp(WindowTypeEnum::GLFW, vulkanWindow.WindowHandle);
    while (!VulkanWindow_WindowShouldClose(vulkanWindow.WindowHandle))
    {
        const float frameTime = deltaTime.GetFrameTime();

        VulkanWindow_PollEventHandler();
        VulkanWindow_SwapBuffer();

     /*   for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++) {
            if (glfwJoystickPresent(i) && glfwJoystickIsGamepad(i)) {
                joystick_id = i;
                printf("Controller found: %s\n", glfwGetGamepadName(i));
                break;
            }
        }*/

        gameSystem.Update(frameTime);
        gameSystem.DebugUpdate(frameTime);
        gameSystem.Draw(frameTime);
        deltaTime.EndFrameTime();
    }
    vkDeviceWaitIdle(renderSystem.renderer.Device);
    gameSystem.Destroy();
    VulkanWindow_DestroyWindow(vulkanWindow.WindowHandle);
    return 0;
}