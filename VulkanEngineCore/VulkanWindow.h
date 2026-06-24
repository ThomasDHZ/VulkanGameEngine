#pragma once
#include <Platform.h>
#include "Typedef.h"
#include <GLFW/glfw3.h>

class VulkanWindow
{
private:
    VulkanWindow() = default;
    ~VulkanWindow() = default;
    VulkanWindow(const VulkanWindow&) = delete;
    VulkanWindow& operator=(const VulkanWindow&) = delete;

    bool m_framebufferResized = false;
    uint32 m_width = 0;
    uint32 m_height = 0;

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void ErrorCallback(int error, const char* description);

public:
    static VulkanWindow& Get();

    DLL_EXPORT bool Create(const char* title, uint32 width, uint32 height);
    DLL_EXPORT void CreateSurface(VkInstance& instance, VkSurfaceKHR& surface);
    DLL_EXPORT void PollEvents();
    DLL_EXPORT bool ShouldClose() const;
    DLL_EXPORT void Close();

    DLL_EXPORT void* GetHandle() const;
    DLL_EXPORT ivec2 GetSize() const;
    DLL_EXPORT ivec2 GetFramebufferSize() const;

    GLFWwindow* m_window = nullptr;
};
extern DLL_EXPORT VulkanWindow& vulkanWindow;
inline VulkanWindow& VulkanWindow::Get()
{
    static VulkanWindow instance;
    return instance;
}