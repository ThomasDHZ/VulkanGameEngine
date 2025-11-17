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

    void CreateGraphicsWindow(GameEngineWindow* self, const char* WindowName, uint32_t width, uint32_t height);
    void PollEventHandler(GameEngineWindow* self);
    void SwapBuffer(GameEngineWindow* self);
    void CreateSurface(void* windowHandle, VkInstance* instance, VkSurfaceKHR* surface);
    void GetFrameBufferSize(void* windowHandle, int* width, int* height);
    void DestroyWindow(GameEngineWindow* self);
    bool WindowShouldClose(GameEngineWindow* self);
    const char** GetInstanceExtensions(struct GameEngineWindow* self, uint32_t* outExtensionCount, bool enableValidationLayers);

    static void ErrorCallBack(int error, const char* description);
    static void ControllerConnectCallBack(int jid, int event);
    static void FrameBufferResizeCallBack(GLFWwindow* window, int width, int height);
};
extern GameEngineWindow* vulkanWindow;