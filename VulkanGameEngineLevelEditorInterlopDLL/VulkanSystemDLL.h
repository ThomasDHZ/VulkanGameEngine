#pragma once
#include <VulkanSystem.h>

struct GraphicsSystem
{
	uint32								   ApiVersion = VK_API_VERSION_1_1;
	VkInstance							   Instance = VK_NULL_HANDLE;
	VkDevice							   Device = VK_NULL_HANDLE;
	VkPhysicalDevice					   PhysicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR						   Surface = VK_NULL_HANDLE;
	VkCommandPool						   CommandPool = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT			   DebugMessenger = VK_NULL_HANDLE;
	VkSwapchainKHR						   Swapchain = VK_NULL_HANDLE;
	VkQueue								   GraphicsQueue = VK_NULL_HANDLE;
	VkQueue								   PresentQueue = VK_NULL_HANDLE;
	VkFence*						       InFlightFencesPtr = nullptr;
	VkImage*						       SwapChainImagesPtr = nullptr;
	VkCommandBuffer*				       CommandBuffersPtr = nullptr;
	VkImageView*					       SwapChainImageViewsPtr = nullptr;
	VkSemaphore*					       AcquireImageSemaphoresPtr = nullptr;
	VkSemaphore*					       PresentImageSemaphoresPtr = nullptr;
	size_t							       InFlightFencesCount = MAXSIZE_T;
	size_t								   CommandBufferCount = MAXSIZE_T;
	size_t								   SwapChainImageViewCount = MAXSIZE_T;
	size_t								   AcquireImageSemaphoreCount = MAXSIZE_T;
	size_t								   PresentImageSemaphoreCount = MAXSIZE_T;
	size_t								   SwapChainImageCount = UINT32_MAX;
	size_t								   ImageIndex = UINT32_MAX;
	size_t								   CommandIndex = UINT32_MAX;
	size_t								   GraphicsFamily = UINT32_MAX;
	size_t								   PresentFamily = UINT32_MAX;
	size_t								   MaxFramesInFlight = UINT32_MAX;

	VkExtent2D							   SwapChainResolution{};
	VkFormat							   Format = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR						   ColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	VkPresentModeKHR					   PresentMode = VK_PRESENT_MODE_FIFO_KHR;
	VkSampleCountFlagBits				   MaxSampleCount = VK_SAMPLE_COUNT_1_BIT;
};

typedef void (*LogVulkanMessageCallback)(const char* message, int severity);

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT void						   VulkanSystem_LogVulkanMessage(const char* message, int severity);
	DLL_EXPORT void					       VulkanSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback);
	DLL_EXPORT VkInstance				   VulkanSystem_CreateVulkanInstance();
	DLL_EXPORT VkSurfaceKHR				   VulkanSystem_CreateVulkanSurface(void* windowHandle, VkInstance instance);
	DLL_EXPORT GraphicsSystem			   VulkanSystem_RendererSetUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface);
	DLL_EXPORT VkCommandBuffer			   VulkanSystem_BeginSingleUseCommand();
	DLL_EXPORT void						   VulkanSystem_EndSingleUseCommand(VkCommandBuffer commandBuffer);
	DLL_EXPORT VkPresentModeKHR*		   VulkanSystem_GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, size_t* outCount);
	DLL_EXPORT VkSurfaceCapabilitiesKHR    VulkanSystem_GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	DLL_EXPORT VkPhysicalDeviceProperties  VulkanSystem_GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice);
	DLL_EXPORT VkPhysicalDeviceFeatures    VulkanSystem_GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice);
	DLL_EXPORT VkPhysicalDeviceFeatures2   VulkanSystem_GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice);
	DLL_EXPORT VkPhysicalDevice*		   VulkanSystem_GetPhysicalDeviceList(VkInstance instance, size_t* outCount);
	DLL_EXPORT VkSurfaceFormatKHR*		   VulkanSystem_GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, size_t* outCount);
	DLL_EXPORT VkPresentModeKHR*		   VulkanSystem_GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, size_t* outCount);
	DLL_EXPORT void						   VulkanSystem_DestroyRenderer();
#ifdef __cplusplus
}
#endif

