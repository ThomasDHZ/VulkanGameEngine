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

void GameEngineWindow::CreateGraphicsWindow(GameEngineWindow* self,
    const char* WindowName,
    uint32_t width, uint32_t height)
{
    FrameBufferResized = false;
    Width = width;
    Height = height;
    ShouldClose = false;

    glfwInit();

    // FORCE WSLg to work
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
    glfwRequestWindowAttention((GLFWwindow*)self->WindowHandle);  // flashes taskbar
    glfwFocusWindow((GLFWwindow*)self->WindowHandle);             // brings to front
    glfwSetWindowSize((GLFWwindow*)self->WindowHandle, 1920, 1080); // force full size
    glfwSetWindowAttrib((GLFWwindow*)self->WindowHandle, GLFW_MAXIMIZED, GLFW_TRUE);

    glfwSetWindowUserPointer((GLFWwindow*)self->WindowHandle, self);
    glfwSetFramebufferSizeCallback((GLFWwindow*)self->WindowHandle, FrameBufferResizeCallBack);
    glfwSetCursorPosCallback((GLFWwindow*)self->WindowHandle, Mouse::MouseMoveEvent);
    glfwSetMouseButtonCallback((GLFWwindow*)self->WindowHandle, Mouse::MouseButtonPressedEvent);
    glfwSetScrollCallback((GLFWwindow*)self->WindowHandle, Mouse::MouseWheelEvent);
    glfwSetKeyCallback((GLFWwindow*)self->WindowHandle, Keyboard::KeyboardKeyPressed);
    glfwSetJoystickCallback(ControllerConnectCallBack);
}

void GameEngineWindow::PollEventHandler(GameEngineWindow* self)
{
    glfwPollEvents();
}

void GameEngineWindow::SwapBuffer(GameEngineWindow* self)
{
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
    GLFWwindow* handle = (GLFWwindow*)self->WindowHandle;
    glfwDestroyWindow(handle);
    glfwTerminate();
}

bool GameEngineWindow::WindowShouldClose(GameEngineWindow* self)
{
    GLFWwindow* handle = (GLFWwindow*)self->WindowHandle;
    return  glfwWindowShouldClose(handle);
}

const char** GameEngineWindow::GetInstanceExtensions(GameEngineWindow* self, uint32_t* outExtensionCount, bool enableValidationLayers)
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    size_t totalCount = glfwExtensionCount + (enableValidationLayers ? 1 : 0);

    const char** extensions = memorySystem.AddPtrBuffer<const char*>(totalCount * sizeof(const char*), __FILE__, __LINE__, __func__);
    if (!extensions) {
        fprintf(stderr, "Failed to allocate memory for extensions\n");
        return NULL;
    }

    for (uint32_t x = 0; x < glfwExtensionCount; x++) {
        extensions[x] = glfwExtensions[x];
    }

    if (enableValidationLayers)
    {
        extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    *outExtensionCount = totalCount;
    return extensions;
}


void GameEngineWindow::ControllerConnectCallBack(int jid, int event) {
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