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

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    vulkanWindow = memorySystem.AddPtrBuffer<VulkanWindow>(sizeof(VulkanWindow), __FILE__, __LINE__, __func__);
    vulkanWindow->PollEventHandler = Window_GLFW_PollEventHandler;
    vulkanWindow->SwapBuffer = Window_GLFW_SwapBuffer;
    vulkanWindow->GetInstanceExtensions = Window_GLFW_GetInstanceExtensions;
    vulkanWindow->CreateSurface = Window_GLFW_CreateSurface;
    vulkanWindow->GetFrameBufferSize = Window_GLFW_GetFrameBufferSize;
    vulkanWindow->DestroyWindow = Window_GLFW_DestroyWindow;
    vulkanWindow->WindowShouldClose = Window_GLFW_WindowShouldClose;
    vulkanWindow->FrameBufferResized = false;
    vulkanWindow->Width = configSystem.WindowResolution.x;
    vulkanWindow->Height = configSystem.WindowResolution.y;
    vulkanWindow->ShouldClose = false;
    vulkanWindow->mouse.X = 0;
    vulkanWindow->mouse.Y = 0;
    vulkanWindow->mouse.WheelOffset = 0;
    memset(vulkanWindow->mouse.MouseButtonState, 0, sizeof(vulkanWindow->mouse.MouseButtonState));
    memset(vulkanWindow->keyboard.KeyPressed, 0, sizeof(vulkanWindow->keyboard.KeyPressed));
    vulkanWindow->WindowHandle = (void*)glfwCreateWindow(configSystem.WindowResolution.x, configSystem.WindowResolution.y, "WindowName", NULL, NULL);
    int present = glfwJoystickPresent(GLFW_JOYSTICK_1);

    glfwMakeContextCurrent((GLFWwindow*)vulkanWindow->WindowHandle);
    glfwSetWindowUserPointer((GLFWwindow*)vulkanWindow->WindowHandle, vulkanWindow->WindowHandle);
    glfwSetErrorCallback(error_callback);
    glfwSetFramebufferSizeCallback((GLFWwindow*)vulkanWindow->WindowHandle, Window_GLFW_FrameBufferResizeCallBack);
    //glfwSetCursorPosCallback((GLFWWindow*)self->WindowHandle, GameEngine_GLFW_MouseMoveEvent);
    //glfwSetMouseButtonCallback((GLFWWindow*)self->WindowHandle, GameEngine_GLFW_MouseButtonPressedEvent);
    //glfwSetScrollCallback((GLFWWindow*)self->WindowHandle, GameEngine_GLFW_MouseWheelEvent);
    //glfwSetKeyCallback((GLFWWindow*)self->WindowHandle, GameEngine_GLFW_KeyboardKeyPressed);
    glfwSetJoystickCallback(joystick_callback);

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
    vkDeviceWaitIdle(renderSystem.renderer.Device);
    gameSystem.Destroy();
    vulkanWindow->DestroyWindow(vulkanWindow);
    return 0;
}