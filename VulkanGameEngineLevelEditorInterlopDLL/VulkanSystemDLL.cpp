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

//size_t VulkanSystem_GetInFlightFenceCount()
//{
//	return vulkanSystem.InFlightFences.size();
//}
//
//size_t VulkanSystem_GetInFlightFences(VkFence* pFence, size_t capacity)
//{
//    size_t count = vulkanSystem.InFlightFences.size();
//    if (capacity < count) return MAXSIZE_T; 
//
//    std::copy(vulkanSystem.InFlightFences.begin(), vulkanSystem.InFlightFences.end(), pFence);
//    return count;
//}
//
//size_t VulkanSystem_GetSwapChainImageCount()
//{
//	return vulkanSystem.SwapChainImages.size();
//}
//
//size_t VulkanSystem_GetSwapChainImages(VkImage* pImages, size_t capacity)
//{
//	size_t count = vulkanSystem.SwapChainImages.size();
//	if (capacity < count) return MAXSIZE_T;
//
//	std::copy(vulkanSystem.SwapChainImages.begin(), vulkanSystem.SwapChainImages.end(), pImages);
//	return count;
//}
//
//size_t VulkanSystem_GetCommandBufferCount()
//{
//	return vulkanSystem.CommandBuffers.size();
//}
//
//size_t VulkanSystem_GetCommandBuffers(VkCommandBuffer* pCommandBuffer, size_t capacity)
//{
//	size_t count = vulkanSystem.CommandBuffers.size();
//	if (capacity < count) return MAXSIZE_T;
//
//	std::copy(vulkanSystem.CommandBuffers.begin(), vulkanSystem.CommandBuffers.end(), pCommandBuffer);
//	return count;
//}
//
//size_t VulkanSystem_GetSwapChainImageViewCount()
//{
//	return vulkanSystem.SwapChainImages.size();
//}
//
//size_t VulkanSystem_GetSwapChainImageViews(VkImageView* pSwapChainImages, size_t capacity)
//{
//	size_t count = vulkanSystem.SwapChainImages.size();
//	if (capacity < count) return MAXSIZE_T;
//
//	std::copy(vulkanSystem.SwapChainImages.begin(), vulkanSystem.SwapChainImages.end(), pSwapChainImages);
//	return count;
//}
//
//size_t VulkanSystem_GetAcquireImageSemaphoreCount()
//{
//	return vulkanSystem.AcquireImageSemaphores.size();
//}
//
//size_t VulkanSystem_GetAcquireImageSemaphores(VkSemaphore* pAcquireImageSemaphores, size_t capacity)
//{
//	size_t count = vulkanSystem.AcquireImageSemaphores.size();
//	if (capacity < count) return MAXSIZE_T;
//
//	std::copy(vulkanSystem.AcquireImageSemaphores.begin(), vulkanSystem.AcquireImageSemaphores.end(), pAcquireImageSemaphores);
//	return count;
//}
//
//size_t VulkanSystem_GetPresentImageSemaphoreCount()
//{
//	return vulkanSystem.PresentImageSemaphores.size();
//}
//
//size_t VulkanSystem_GetPresentImageSemaphores(VkSemaphore* pPresentImageSemaphores, size_t capacity)
//{
//	size_t count = vulkanSystem.PresentImageSemaphores.size();
//	if (capacity < count) return MAXSIZE_T;
//
//	std::copy(vulkanSystem.PresentImageSemaphores.begin(), vulkanSystem.PresentImageSemaphores.end(), pPresentImageSemaphores);
//	return count;
//}
//
////
////VkCommandBuffer VulkanSystem_BeginSingleUseCommand(VkDevice device, VkCommandPool commandPool)
////{
////	return vulkanSystem.BeginSingleUseCommand(device, commandPool);
////}
////
////void VulkanSystem_EndSingleUseCommand(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer)
////{
////	return vulkanSystem.EndSingleUseCommand(device, commandPool, graphicsQueue, commandBuffer);
////}
////
////void VulkanSystem_DestroyRenderer()
////{
////	vulkanSystem.DestroyRenderer();
////}
////
////void VulkanSystem_DestroyRenderPass(VkDevice device, VkRenderPass* renderPass)
////{
////	return vulkanSystem.DestroyRenderPass(device, renderPass);
////}
////
////void VulkanSystem_DestroyFrameBuffers(VkDevice device, VkFramebuffer* frameBufferList, uint32 count)
////{
////	return vulkanSystem.DestroyFrameBuffers(device, frameBufferList, count);
////}
////
////void VulkanSystem_DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool)
////{
////	return vulkanSystem.DestroyDescriptorPool(device, descriptorPool);
////}
////
////void VulkanSystem_DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, VkCommandBuffer* commandBufferList, uint32 count)
////{
////	return vulkanSystem.DestroyCommandBuffers(device, commandPool, commandBufferList, count);
////}
////
////void VulkanSystem_DestroyBuffer(VkDevice device, VkBuffer* buffer)
////{
////	return vulkanSystem.DestroyBuffer(device, buffer);
////}
