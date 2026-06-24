#include "VulkanWindow.h"
#include <iostream>

VulkanWindow& vulkanWindow = VulkanWindow::Get();

bool VulkanWindow::Create(const char* title, uint32 width, uint32 height)
{
    if (m_window) return true;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        return false;
    }

    m_width = width;
    m_height = height;

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
    glfwSetErrorCallback(ErrorCallback);

    return true;
}

void VulkanWindow::CreateSurface(VkInstance& instance, VkSurfaceKHR& surface)
{
    GLFWwindow* handle = (GLFWwindow*)m_window;
    VkResult result = glfwCreateWindowSurface(instance, handle, nullptr, &surface);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan surface! Error code: %d\n", result);
        return;
    }
}

void VulkanWindow::PollEvents()
{
    glfwPollEvents();
}

bool VulkanWindow::ShouldClose() const
{
    return glfwWindowShouldClose(m_window);
}

void VulkanWindow::Close()
{
    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

void* VulkanWindow::GetHandle() const
{
    return m_window;
}

ivec2 VulkanWindow::GetSize() const
{
    return { m_width, m_height };
}

ivec2 VulkanWindow::GetFramebufferSize() const
{
    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);
    return { w, h };
}

void VulkanWindow::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    VulkanWindow* self = (VulkanWindow*)glfwGetWindowUserPointer(window);
    if (self)
    {
        self->m_framebufferResized = true;
        self->m_width = width;
        self->m_height = height;
    }
}

void VulkanWindow::ErrorCallback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}