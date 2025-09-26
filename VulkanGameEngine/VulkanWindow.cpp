#include "VulkanWindow.h"
#include "SDLWindow.h"
#include "GLFWWindow.h"
#include "MemorySystem.h"

VulkanWindow* vulkanWindow = nullptr;
VulkanWindow* Window_CreateWindow(WindowType windowType, const char* WindowName, uint32_t width, uint32_t height)
{
    switch (windowType)
    {
    case SDL:
    {
        VulkanWindow* window = memorySystem.AddPtrBuffer<VulkanWindow>(sizeof(VulkanWindow), __FILE__, __LINE__, __func__);
        //if (!window)
        //{
        //    fprintf(stderr, "Failed to allocate memory for SDLWindow.\n");
        //    return NULL;
        //}
        //window->CreateGraphicsWindow = Window_SDL_CreateGraphicsWindow;
        //window->PollEventHandler = Window_SDL_PollEventHandler;
        //window->SwapBuffer = Window_SDL_SwapBuffer;
        //window->GetInstanceExtensions = Window_SDL_GetInstanceExtensions;
        //window->CreateSurface = Window_SDL_CreateSurface;
        //window->GetFrameBufferSize = Window_SDL_GetFrameBufferSize;
        //window->DestroyWindow = Window_SDL_DestroyWindow;
        //window->WindowShouldClose = Window_SDL_WindowShouldClose;
        //window->CreateGraphicsWindow(window, WindowName, width, height);
        return window;
    }
    case GLFW:
    {
        VulkanWindow* window = memorySystem.AddPtrBuffer<VulkanWindow>(sizeof(VulkanWindow), __FILE__, __LINE__, __func__);
        if (!window)
        {
            fprintf(stderr, "Failed to allocate memory for SDLWindow.\n");
            return NULL;
        }
        window->WindowType = GLFW;
        window->CreateGraphicsWindow = Window_GLFW_CreateGraphicsWindow;
        window->PollEventHandler = Window_GLFW_PollEventHandler;
        window->SwapBuffer = Window_GLFW_SwapBuffer;
        window->GetInstanceExtensions = Window_GLFW_GetInstanceExtensions;
        window->CreateSurface = Window_GLFW_CreateSurface;
        window->GetFrameBufferSize = Window_GLFW_GetFrameBufferSize;
        window->DestroyWindow = Window_GLFW_DestroyWindow;
        window->WindowShouldClose = Window_GLFW_WindowShouldClose;
        window->CreateGraphicsWindow(window, WindowName, width, height);
        return window;
    }
    default:
    {
        return NULL;
    }
    }
}