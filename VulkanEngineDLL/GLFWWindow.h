#pragma once
#include <glfw/include/GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include "VulkanWindow.h"

typedef struct GLFWWindow
{
    VulkanWindow* base;
    GLFWwindow* glfwWindowHandle;
    int jid;
} GLFWWindow;

DLL_EXPORT void Window_GLFW_CreateGraphicsWindow(VulkanWindow* self, const char* WindowName, uint32_t width, uint32_t height);
DLL_EXPORT void Window_GLFW_PollEventHandler( VulkanWindow* self);
DLL_EXPORT void Window_GLFW_SwapBuffer( VulkanWindow* self);
DLL_EXPORT void Window_GLFW_CreateSurface(void* windowHandle, VkInstance* instance, VkSurfaceKHR* surface);
DLL_EXPORT void Window_GLFW_GetFrameBufferSize(void* windowHandle, int* width, int* height);
DLL_EXPORT const char** Window_GLFW_GetInstanceExtensions(VulkanWindow* self, uint32_t* outExtensionCount, bool enableValidationLayers);
DLL_EXPORT void Window_GLFW_DestroyWindow(VulkanWindow* self);
DLL_EXPORT bool Window_GLFW_WindowShouldClose(VulkanWindow* self);

void Window_GLFW_FrameBufferResizeCallBack(GLFWwindow* window, int width, int height);