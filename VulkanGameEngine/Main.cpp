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
#include "ImGuiRenderer.h"

int main(int argc, char** argv)
{
    SystemClock systemClock = SystemClock();
    FrameTimer deltaTime = FrameTimer();

    Engine_SetRootDirectory("../Assets");
    vulkanWindow = new GameEngineWindow();
    vulkanWindow->CreateGraphicsWindow(vulkanWindow, "Game", configSystem.WindowResolution.x, configSystem.WindowResolution.y);

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkInstance instance = Renderer_CreateVulkanInstance();
    VkDebugUtilsMessengerEXT debugMessenger = Renderer_SetupDebugMessenger(renderer.Instance);
    glfwCreateWindowSurface(instance, (GLFWwindow*)vulkanWindow->WindowHandle, NULL, &surface);
    gameSystem.StartUp(vulkanWindow->WindowHandle, instance, surface, debugMessenger);
    imGuiRenderer = ImGui_StartUp(renderer);
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