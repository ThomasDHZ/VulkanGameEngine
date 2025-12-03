#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <MemorySystem.h>
#include "VulkanWindow.h"
#include "GameController.h"
#include "Mouse.h"
#include "Keyboard.h"

GameEngineWindow* vulkanWindow = nullptr;

GameEngineWindow::GameEngineWindow()
{
}

GameEngineWindow::~GameEngineWindow()
{

}

void GameEngineWindow::CreateGraphicsWindow(GameEngineWindow* self, const char* WindowName, uint32 width, uint32 height)
{
    FrameBufferResized = false;
    Width = width;
    Height = height;
    ShouldClose = false;


#ifdef PLATFORM_ANDROID
    self->WindowHandle = platformWindowHandle; 

    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .window = (ANativeWindow*)platformWindowHandle
    };

    VkResult err = vkCreateAndroidSurfaceKHR(self->Instance, &surfaceCreateInfo, nullptr, &self->Surface);
    if (err != VK_SUCCESS)
    {
        __android_log_print(ANDROID_LOG_ERROR, "VulkanEngine", "vkCreateAndroidSurfaceKHR failed: %d", err);
        return;
    }

    __android_log_print(ANDROID_LOG_INFO, "VulkanEngine", "Android surface created successfully");
#else

    glfwInit();
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    self->WindowHandle = glfwCreateWindow(width, height, WindowName, nullptr, nullptr);
    if (!self->WindowHandle) {
        fprintf(stderr, "ERROR: glfwCreateWindow failed!\n");
        const char* err = nullptr;
        glfwGetError(&err);
        fprintf(stderr, "GLFW: %s\n", err ? err : "unknown error");
        return;
    }

    glfwShowWindow((GLFWwindow*)self->WindowHandle);
    glfwRequestWindowAttention((GLFWwindow*)self->WindowHandle); 
    glfwFocusWindow((GLFWwindow*)self->WindowHandle);      
    glfwSetWindowSize((GLFWwindow*)self->WindowHandle, width, height);
    glfwSetWindowAttrib((GLFWwindow*)self->WindowHandle, GLFW_MAXIMIZED, GLFW_TRUE);

    glfwSetWindowUserPointer((GLFWwindow*)self->WindowHandle, self);
    glfwSetFramebufferSizeCallback((GLFWwindow*)self->WindowHandle, FrameBufferResizeCallBack);
    glfwSetCursorPosCallback((GLFWwindow*)self->WindowHandle, Mouse::MouseMoveEvent);
    glfwSetMouseButtonCallback((GLFWwindow*)self->WindowHandle, Mouse::MouseButtonPressedEvent);
    glfwSetScrollCallback((GLFWwindow*)self->WindowHandle, Mouse::MouseWheelEvent);
    glfwSetKeyCallback((GLFWwindow*)self->WindowHandle, Keyboard::KeyboardKeyPressed);
    glfwSetJoystickCallback(ControllerConnectCallBack);
#endif
}

void GameEngineWindow::PollEventHandler(GameEngineWindow* self)
{
#ifndef PLATFORM_ANDROID
    glfwPollEvents();
#endif
}


void GameEngineWindow::CreateSurface(void* windowHandle, VkInstance* instance, VkSurfaceKHR* surface)
{
    GLFWwindow* handle = (GLFWwindow*)windowHandle;
    VkResult result = glfwCreateWindowSurface(*instance, handle, NULL, surface);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan surface! Error code: %d\n", result);
        return;
    }
}

void GameEngineWindow::GetFrameBufferSize(void* windowHandle, int* width, int* height)
{
    GLFWwindow* handle = (GLFWwindow*)windowHandle;
    glfwGetFramebufferSize(handle, &*width, &*height);
}

void GameEngineWindow::DestroyWindow(GameEngineWindow* self)
{
#ifndef PLATFORM_ANDROID
    if (self->WindowHandle) 
    {
        glfwDestroyWindow((GLFWwindow*)self->WindowHandle);
        self->WindowHandle = nullptr;
    }
    glfwTerminate();
#else
    if (self->Surface) 
    {
        vkDestroySurfaceKHR(self->Instance, self->Surface, nullptr);
        self->Surface = VK_NULL_HANDLE;
    }
    self->WindowHandle = nullptr;
#endif
}

bool GameEngineWindow::WindowShouldClose(GameEngineWindow* self)
{
#ifndef PLATFORM_ANDROID
    return glfwWindowShouldClose((GLFWwindow*)self->WindowHandle);
#else
    return true;
#endif
}

void GameEngineWindow::ControllerConnectCallBack(int jid, int event) 
{
    if (event == GLFW_CONNECTED && glfwJoystickIsGamepad(jid)) {
        printf("Controller connected: %s\n", glfwGetGamepadName(jid));
  /*      GLFWWindow* window = ((GLFWWindow*)vulkanWindow);
        window->jid = jid;*/
    }
    else if (event == GLFW_DISCONNECTED) {
        printf("Controller disconnected\n");
  /*      GLFWWindow* window = ((GLFWWindow*)vulkanWindow);
        window->jid = -1;*/
    }
}

void GameEngineWindow::ErrorCallBack(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void GameEngineWindow::FrameBufferResizeCallBack(GLFWwindow* window, int width, int height)
{
    auto* engineWindow = (GameEngineWindow*)glfwGetWindowUserPointer(window);
    if (engineWindow) 
    {
        engineWindow->FrameBufferResized = true;
        engineWindow->Width = width;
        engineWindow->Height = height;
    }
}