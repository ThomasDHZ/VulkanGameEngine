#include "VulkanSystem2.h"
#include "VulkanDebugger.h"
#include "VulkanDevice.h"
#include "VulkanWindow.h"
VulkanSystem2& vulkan = VulkanSystem2::Get();

void VulkanSystem2::VulkanSetUp(ivec2 windowResolution, ivec2 renderResolution)
{
    vulkanWindow.Create("Game", windowResolution.x, windowResolution.y);

    m_usingCustomSurface = false;
    m_windowHandle = vulkanWindow.m_window;
    m_windowResolution = windowResolution;
    RendererSetUp(vulkanWindow.m_window, renderResolution);
}

void VulkanSystem2::VulkanSetUp(void* windowHandle, ivec2 windowResolution, ivec2 renderResolution)
{
    m_usingCustomSurface = true;
    m_windowHandle = windowHandle;
    m_windowResolution = windowResolution;
    RendererSetUp(m_windowHandle, renderResolution);
}

void VulkanSystem2::RendererSetUp(void* windowHandle, ivec2 renderResolution)
{
    m_instance.Initialize();
    m_device.Initialize();
   // m_swapChain.Initialize(renderResolution);
   // m_commandBuffer.Initialize();

#if defined(__ANDROID__)
    vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)nvkGetDeviceProcAddr(Device, "vkGetBufferDeviceAddress");
    if (vkGetBufferDeviceAddress == nullptr) {
        throw std::runtime_error("Failed to load vkGetBufferDeviceAddress function pointer!");
    }
#endif
}

uint32 VulkanSystem2::GetMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32 x = 0; x < memProperties.memoryTypeCount; x++)
    {
        if ((typeFilter & (1 << x)) &&
            (memProperties.memoryTypes[x].propertyFlags & properties) == properties)
        {
            return x;
        }
    }

    return UINT32_MAX;
}

void VulkanSystem2::Shutdown()
{
}


VulkanInstance			  VulkanSystem2::Instance()                   { return m_instance; }
VulkanDebugger			  VulkanSystem2::Debug()                      { return m_debug; }
VulkanDevice			  VulkanSystem2::Device()                     { return m_device; }
bool					  VulkanSystem2::CustomSurface()		const { return m_usingCustomSurface; }
const void*               VulkanSystem2::WindowHandle()			const { return m_windowHandle; }
ivec2					  VulkanSystem2::WindowResolution()		const { return m_windowResolution; }
uint32					  VulkanSystem2::ApiVersion()			const { return m_instance.ApiVersion(); }
VkInstance				  VulkanSystem2::InstanceHandle()		const { return m_instance.InstanceHandle(); }
VkSurfaceKHR			  VulkanSystem2::Surface()				const { return m_instance.Surface(); }
VkPhysicalDevice		  VulkanSystem2::PhysicalDevice()		const { return m_device.PhysicalDevice(); }
VkDevice				  VulkanSystem2::LogicalDevice()		const { return m_device.LogicalDevice(); }
VkQueue                   VulkanSystem2::GraphicsQueue()		const { return m_device.GraphicsQueue(); }
VkQueue                   VulkanSystem2::PresentQueue()			const { return m_device.PresentQueue(); }
VkSampleCountFlagBits     VulkanSystem2::MaxSampleCount()		const { return m_device.MaxSampleCount(); }