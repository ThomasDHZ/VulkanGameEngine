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

void GameEngineWindow::CreateGraphicsWindow(GameEngineWindow* self, const char* WindowName, uint32_t width, uint32_t height)
{
    FrameBufferResized = false;
    Width = width;
    Height = height;
    ShouldClose = false;
 
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    self->WindowHandle = (void*)glfwCreateWindow(width, height, WindowName, NULL, NULL);

    glfwMakeContextCurrent((GLFWwindow*)self->WindowHandle);
    glfwSetWindowUserPointer((GLFWwindow*)self->WindowHandle, self);
    glfwSetErrorCallback(GameEngineWindow::ErrorCallBack);
    glfwSetFramebufferSizeCallback((GLFWwindow*)self->WindowHandle, GameEngineWindow::FrameBufferResizeCallBack);
    glfwSetCursorPosCallback((GLFWwindow*)self->WindowHandle, Mouse::MouseMoveEvent);
    glfwSetMouseButtonCallback((GLFWwindow*)self->WindowHandle, Mouse::MouseButtonPressedEvent);
    glfwSetScrollCallback((GLFWwindow*)self->WindowHandle, Mouse::MouseWheelEvent);
    glfwSetKeyCallback((GLFWwindow*)self->WindowHandle, Keyboard::KeyboardKeyPressed);
    glfwSetJoystickCallback(GameEngineWindow::ControllerConnectCallBack);
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
    glfwCreateWindowSurface(*instance, handle, NULL, surface);
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

void GameEngineWindow::FrameBufferResizeCallBack(GLFWwindow* self, int width, int height)
{
    //GLFWwindow* app = glfwGetWindowUserPointer(vulkanWindow->WindowHandle);
    //if (app)
    //{
    //	vulkanWindow->FrameBufferResized = true;
    //	glfwGetFramebufferSize(vulkanWindow->WindowHandle, &width, &height);

    //	while (width == 0 || height == 0)
    //	{
    //		glfwGetFramebufferSize(vulkanWindow->WindowHandle, &width, &height);
    //		glfwWaitEvents();
    //	}

    //	vulkanWindow->Width = width;
    //	vulkanWindow->Height = height;
    //}
}