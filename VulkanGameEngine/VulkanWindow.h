#pragma once
#include <InputEnum.h>
#include <vulkan/vulkan_core.h>
#include <stdbool.h>
#include "Macro.h"
#include <glfw/include/GLFW/glfw3.h>

typedef struct VulkanWindow
{
    WindowType WindowType;
    void* WindowHandle;
    uint32_t    Width;
    uint32_t    Height;
    bool        FrameBufferResized;
    bool        ShouldClose;
    MouseState  mouse;
    KeyboardState keyboard;
    int jid;

    void (*CreateGraphicsWindow)(struct VulkanWindow* self, const char* WindowName, uint32_t width, uint32_t height);
    void (*PollEventHandler)(struct VulkanWindow* self);
    void (*SwapBuffer)(struct VulkanWindow* self);
    const char** (*GetInstanceExtensions)(struct VulkanWindow* self, uint32_t* outExtensionCount, bool enableValidationLayers);
    void (*CreateSurface)(void* windowHandle, VkInstance* instance, VkSurfaceKHR* surface);
    void (*GetFrameBufferSize)(void* windowHandle, int* width, int* height);
    void (*DestroyWindow)(struct VulkanWindow* self);
    bool (*WindowShouldClose)(struct VulkanWindow* self);
} VulkanWindow;
extern VulkanWindow* vulkanWindow;
VulkanWindow* Window_CreateWindow(WindowType windowType, const char* WindowName, uint32_t width, uint32_t height);