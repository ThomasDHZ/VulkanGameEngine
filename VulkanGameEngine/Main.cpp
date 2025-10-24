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
#include <RigidBody.h>

int main(int argc, char** argv)
{
    SystemClock systemClock = SystemClock();
    FrameTimer deltaTime = FrameTimer();

    vulkanWindow = new GameEngineWindow();
    vulkanWindow->CreateGraphicsWindow(vulkanWindow, "Game", configSystem.WindowResolution.x, configSystem.WindowResolution.y);

    renderer.Instance = Renderer_CreateVulkanInstance();
    renderer.DebugMessenger = Renderer_SetupDebugMessenger(renderer.Instance);
    glfwCreateWindowSurface(renderer.Instance, (GLFWwindow*)vulkanWindow->WindowHandle, NULL, &renderer.Surface);
    gameSystem.StartUp(vulkanWindow->WindowHandle);
    //imGuiRenderer = ImGui_StartUp(renderer);
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