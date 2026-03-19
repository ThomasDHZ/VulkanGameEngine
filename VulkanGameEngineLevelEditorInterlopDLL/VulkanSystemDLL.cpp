#include "VulkanSystemDLL.h"
#include <RenderSystem.h>

LogVulkanMessageCallback g_logVulkanMessageCallback = nullptr;

void VulkanSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback)
{
	g_logVulkanMessageCallback = callback;
}

void VulkanSystem_LogVulkanMessage(const char* message, int severity)
{
    if (g_logVulkanMessageCallback)
    {
        g_logVulkanMessageCallback(message, severity);
    }
}

 VkInstance VulkanSystem_CreateVulkanInstance()
{
	 return vulkanSystem.CreateVulkanInstance();
}

VkSurfaceKHR VulkanSystem_CreateVulkanSurface(void* windowHandle, VkInstance instance)
{
	return vulkanSystem.CreateVulkanSurface(windowHandle, instance);
}

GraphicsSystem VulkanSystem_RendererSetUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface)
{
	vulkanSystem.RendererSetUp(windowHandle, instance, surface);
	return GraphicsSystem
	{
		.ApiVersion = vulkanSystem.ApiVersion,
		.Instance = vulkanSystem.Instance,
		.Device = vulkanSystem.Device,
		.PhysicalDevice = vulkanSystem.PhysicalDevice,
		.Surface = vulkanSystem.Surface,
		.CommandPool = vulkanSystem.CommandPool,
		.DebugMessenger = vulkanSystem.DebugMessenger,
		.Swapchain = vulkanSystem.Swapchain,
		.GraphicsQueue = vulkanSystem.GraphicsQueue,
		.PresentQueue = vulkanSystem.PresentQueue,
		.InFlightFencesPtr = vulkanSystem.InFlightFences.data(),
		.SwapChainImagesPtr = vulkanSystem.SwapChainImages.data(),
		.CommandBuffersPtr = vulkanSystem.CommandBuffers.data(),
		.SwapChainImageViewsPtr = vulkanSystem.SwapChainImageViews.data(),
		.AcquireImageSemaphoresPtr = vulkanSystem.AcquireImageSemaphores.data(),
		.PresentImageSemaphoresPtr = vulkanSystem.PresentImageSemaphores.data(),
		.InFlightFencesCount = vulkanSystem.InFlightFences.size(),
		.CommandBufferCount = vulkanSystem.CommandBuffers.size(),
		.SwapChainImageViewCount = vulkanSystem.SwapChainImageViews.size(),
		.AcquireImageSemaphoreCount = vulkanSystem.AcquireImageSemaphores.size(),
		.PresentImageSemaphoreCount = vulkanSystem.PresentImageSemaphores.size(),
		.SwapChainImageCount = vulkanSystem.SwapChainImageCount,
		.ImageIndex = vulkanSystem.ImageIndex,
		.CommandIndex = vulkanSystem.CommandIndex,
		.GraphicsFamily = vulkanSystem.GraphicsFamily,
		.PresentFamily = vulkanSystem.PresentFamily,
		.MaxFramesInFlight = vulkanSystem.MaxFramesInFlight,
		.SwapChainResolution = vulkanSystem.SwapChainResolution,
		.Format = vulkanSystem.Format,
		.ColorSpace = vulkanSystem.ColorSpace,
		.PresentMode = vulkanSystem.PresentMode,
		.MaxSampleCount = vulkanSystem.MaxSampleCount
	};
}

 VkCommandBuffer VulkanSystem_BeginSingleUseCommand()
{
	 return vulkanSystem.BeginSingleUseCommand();
}

 void VulkanSystem_EndSingleUseCommand(VkCommandBuffer commandBuffer)
{
	 return vulkanSystem.EndSingleUseCommand(commandBuffer);
}

 VkPresentModeKHR* VulkanSystem_GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, size_t* outCount)
{
	 Vector<VkPresentModeKHR> presentModeList = vulkanSystem.GetSurfacePresentModes(physicalDevice, surface);
	 *outCount = presentModeList.size();
	 return presentModeList.data();
}

 VkSurfaceCapabilitiesKHR VulkanSystem_GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	 return vulkanSystem.GetSurfaceCapabilities(physicalDevice, surface);
}

 VkPhysicalDeviceProperties VulkanSystem_GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice)
{
	 return vulkanSystem.GetPhysicalDeviceProperties(physicalDevice);
}

 VkPhysicalDeviceFeatures VulkanSystem_GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice)
{
	 return vulkanSystem.GetPhysicalDeviceFeatures(physicalDevice);
}

 VkPhysicalDeviceFeatures2 VulkanSystem_GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice)
{
	 return vulkanSystem.GetPhysicalDeviceFeatures2(physicalDevice);
}

 VkPhysicalDevice* VulkanSystem_GetPhysicalDeviceList(VkInstance instance, size_t* outCount)
{
	 Vector<VkPhysicalDevice> presentModeList = vulkanSystem.GetPhysicalDeviceList(instance);
	 *outCount = presentModeList.size();
	 return presentModeList.data();
}

 VkSurfaceFormatKHR* VulkanSystem_GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, size_t* outCount)
{
	 Vector<VkSurfaceFormatKHR> physicalDeviceList = vulkanSystem.GetPhysicalDeviceFormats(physicalDevice, surface);
	 *outCount = physicalDeviceList.size();
	 return physicalDeviceList.data();
}

 VkPresentModeKHR* VulkanSystem_GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, size_t* outCount)
{
	 Vector<VkPresentModeKHR> physicalDevicePresentList = vulkanSystem.GetPhysicalDevicePresentModes(physicalDevice, surface);
	 *outCount = physicalDevicePresentList.size();
	 return physicalDevicePresentList.data();
}


 void VulkanSystem_DestroyRenderer()
{
	 return vulkanSystem.DestroyRenderer();
}
