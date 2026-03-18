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
	DLL_EXPORT void				VulkanSystem_LogVulkanMessage(const char* message, int severity);
	DLL_EXPORT void				VulkanSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback);
	DLL_EXPORT VkInstance		VulkanSystem_CreateVulkanInstance();
	DLL_EXPORT VkSurfaceKHR		VulkanSystem_CreateVulkanSurface(void* windowHandle, VkInstance instance);
	DLL_EXPORT GraphicsSystem   VulkanSystem_RendererSetUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface);
//	DLL_EXPORT size_t			VulkanSystem_GetInFlightFenceCount();
//	DLL_EXPORT size_t			VulkanSystem_GetSwapChainImageCount();
//	DLL_EXPORT size_t			VulkanSystem_GetCommandBufferCount();
//	DLL_EXPORT size_t			VulkanSystem_GetSwapChainImageViewCount();
//	DLL_EXPORT size_t			VulkanSystem_GetAcquireImageSemaphoreCount();
//	DLL_EXPORT size_t			VulkanSystem_GetPresentImageSemaphoreCount();
//	DLL_EXPORT size_t			VulkanSystem_GetInFlightFences(VkFence* pFence, size_t capacity);
//	DLL_EXPORT size_t			VulkanSystem_GetSwapChainImages(VkImage* pImages, size_t capacity);
//	DLL_EXPORT size_t			VulkanSystem_GetCommandBuffers(VkCommandBuffer* pImages, size_t capacity);
//	DLL_EXPORT size_t			VulkanSystem_GetSwapChainImageViews(VkImageView* pImages, size_t capacity);
//	DLL_EXPORT size_t			VulkanSystem_GetAcquireImageSemaphores(VkSemaphore* pImages, size_t capacity);
//	DLL_EXPORT size_t			VulkanSystem_GetPresentImageSemaphores(VkSemaphore* pImages, size_t capacity);
//
////	DLL_EXPORT VkCommandBuffer  VulkanSystem_BeginSingleUseCommand(VkDevice device, VkCommandPool commandPool);
////	DLL_EXPORT void				VulkanSystem_EndSingleUseCommand(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);
////	DLL_EXPORT void				VulkanSystem_DestroyRenderer();
////	DLL_EXPORT void				VulkanSystem_DestroyRenderPass(VkDevice device, VkRenderPass* renderPass);
////	DLL_EXPORT void				VulkanSystem_DestroyFrameBuffers(VkDevice device, VkFramebuffer* frameBufferList, uint32 count);
////	DLL_EXPORT void				VulkanSystem_DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool);
////	DLL_EXPORT void				VulkanSystem_DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, VkCommandBuffer* commandBufferList, uint32 count);
////	DLL_EXPORT void				VulkanSystem_DestroyBuffer(VkDevice device, VkBuffer* buffer);
#ifdef __cplusplus
}
#endif

