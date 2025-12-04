#pragma once
#include <Platform.h>
#include "Typedef.h"
#include <InputEnum.h>
#include <vulkan/vulkan_core.h>
#include <stdbool.h>
#include <GLFW/glfw3.h>

class GameEngineWindow
{
public:
    WindowType  WindowType;
    void*       WindowHandle;
    uint        Width;
    uint        Height;
    bool        FrameBufferResized;
    bool        ShouldClose;

    GameEngineWindow();
    ~GameEngineWindow();

    void CreateGraphicsWindow(GameEngineWindow* self, const char* WindowName, uint32 width, uint32 height);
    void PollEventHandler(GameEngineWindow* self);
    void CreateSurface(void* windowHandle, VkInstance* instance, VkSurfaceKHR* surface);
    void GetFrameBufferSize(void* windowHandle, int* width, int* height);
    void DestroyWindow(GameEngineWindow* self);
    bool WindowShouldClose(GameEngineWindow* self);

    static void ErrorCallBack(int error, const char* description);
    static void FrameBufferResizeCallBack(GLFWwindow* window, int width, int height);
    
#ifndef PLATFORM_ANDROID
    static void ControllerConnectCallBack(int jid, int event);
#endif
};
extern GameEngineWindow* vulkanWindow;