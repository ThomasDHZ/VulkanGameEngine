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

int main(int argc, char** argv)
{
    SystemClock systemClock = SystemClock();
    FrameTimer deltaTime = FrameTimer();
    vulkanWindow = Window_CreateWindow(WindowType::GLFW, "Game", 1920, 1080);
    //ImPlot::CreateContext();

    gameSystem.StartUp(WindowType::GLFW, vulkanWindow->WindowHandle);
    while (!vulkanWindow->WindowShouldClose(vulkanWindow))
    {
        const float frameTime = deltaTime.GetFrameTime();

        vulkanWindow->PollEventHandler(vulkanWindow);
        vulkanWindow->SwapBuffer(vulkanWindow);

        gameSystem.Input(frameTime);
        gameSystem.Update(frameTime);
        gameSystem.DebugUpdate(frameTime);
        gameSystem.Draw(frameTime);
        deltaTime.EndFrameTime();
    }

    vkDeviceWaitIdle(renderSystem.renderer.Device);
    //ImPlot::DestroyContext();
    gameSystem.Destroy();
    vulkanWindow->DestroyWindow(vulkanWindow);
    return 0;
}