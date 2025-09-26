//#pragma once
//#include <SDL3/SDL.h>
//#include <SDL3/SDL_vulkan.h>
//#include <SDL3/SDL_events.h>
//#include <glfw/include/GLFW/glfw3.h>
//#include <vulkan/vulkan_core.h>
//#include <stdio.h>
//#include <stdbool.h>
//#include "VulkanWindow.h"
//
//#ifdef __cplusplus
//extern "C" {
//#endif
//    typedef struct GLFWWindow
//    {
//        VulkanWindow* base;
//        GLFWwindow* glfwWindowHandle;
//    } GLFWWindow;
//
//    DLL_EXPORT void Window_GLFW_CreateGraphicsWindow(struct VulkanWindow* self, const char* WindowName, uint32_t width, uint32_t height);
//    DLL_EXPORT void Window_GLFW_PollEventHandler(struct VulkanWindow* self);
//    DLL_EXPORT  void Window_GLFW_SwapBuffer(struct VulkanWindow* self);
//    DLL_EXPORT void Window_GLFW_CreateSurface(void* windowHandle, VkInstance* instance, VkSurfaceKHR* surface);
//    DLL_EXPORT  void Window_GLFW_GetFrameBufferSize(void* windowHandle, int* width, int* height);
//    DLL_EXPORT  const char** Window_GLFW_GetInstanceExtensions(struct VulkanWindow* self, uint32_t* outExtensionCount, bool enableValidationLayers);
//    DLL_EXPORT  void Window_GLFW_DestroyWindow(struct VulkanWindow* self);
//    DLL_EXPORT  bool Window_GLFW_WindowShouldClose(struct VulkanWindow* self);
//
//    void Window_GLFW_FrameBufferResizeCallBack(GLFWwindow* window, int width, int height);
//#ifdef __cplusplus
//}
//#endif