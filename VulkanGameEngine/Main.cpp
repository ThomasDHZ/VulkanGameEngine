#include "VulkanWindow.h"
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
    SystemClock systemClock = SystemClock();
    FrameTimer deltaTime = FrameTimer();

    vulkanWindow = new GameEngineWindow();
    vulkanWindow->CreateGraphicsWindow(vulkanWindow, "Game", configSystem.WindowResolution.x, configSystem.WindowResolution.y);
    gameSystem.StartUp(WindowType::GLFW, vulkanWindow->WindowHandle);
    while (!vulkanWindow->WindowShouldClose(vulkanWindow))
    {
        const float frameTime = deltaTime.GetFrameTime();

        vulkanWindow->PollEventHandler(vulkanWindow);
        vulkanWindow->SwapBuffer(vulkanWindow);

        gameSystem.Update(frameTime);
        gameSystem.DebugUpdate(frameTime);
        gameSystem.Draw(frameTime);
        deltaTime.EndFrameTime();
    }
    vkDeviceWaitIdle(renderer.Device);
    gameSystem.Destroy();
    vulkanWindow->DestroyWindow(vulkanWindow);
    return 0;
}