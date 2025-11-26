#pragma once
#include "Platform.h"
#include "InputEnum.h"

struct GraphicsRenderer
{
	VkInstance         Instance = VK_NULL_HANDLE;
	VkDevice           Device = VK_NULL_HANDLE;
	VkPhysicalDevice   PhysicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR       Surface = VK_NULL_HANDLE;
	VkCommandPool      CommandPool = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;

	VkFence* InFlightFences = VK_NULL_HANDLE;
	VkSemaphore* AcquireImageSemaphores = VK_NULL_HANDLE;
	VkSemaphore* PresentImageSemaphores = VK_NULL_HANDLE;
	VkImage* SwapChainImages = VK_NULL_HANDLE;
	VkImageView* SwapChainImageViews = VK_NULL_HANDLE;
	VkSwapchainKHR     Swapchain = VK_NULL_HANDLE;
	VkExtent2D         SwapChainResolution;

	size_t			   SwapChainImageCount = UINT64_MAX;
	uint32			   ImageIndex = UINT32_MAX;
	uint32			   CommandIndex = UINT32_MAX;
	uint32			   GraphicsFamily = UINT32_MAX;
	uint32			   PresentFamily = UINT32_MAX;

	VkQueue			   GraphicsQueue = VK_NULL_HANDLE;
	VkQueue			   PresentQueue = VK_NULL_HANDLE;
	VkFormat           Format;
	VkColorSpaceKHR    ColorSpace;
	VkPresentModeKHR   PresentMode;

	bool               RebuildRendererFlag;
};
extern DLL_EXPORT GraphicsRenderer renderer;


static const int MAX_FRAMES_IN_FLIGHT = 3;
typedef void (*LogVulkanMessageCallback)(const char* message, int severity);

#ifdef __cplusplus
extern "C" {
#endif
	 DLL_EXPORT void			Renderer_CreateLogMessageCallback(LogVulkanMessageCallback callback);
	 DLL_EXPORT VkInstance		Renderer_CreateVulkanInstance();
	 DLL_EXPORT VkSurfaceKHR	Renderer_CreateVulkanSurface(void* windowHandle, VkInstance instance);
	 DLL_EXPORT VkCommandBuffer Renderer_BeginSingleUseCommand(VkDevice device, VkCommandPool commandPool);
	 DLL_EXPORT VkResult		Renderer_EndSingleUseCommand(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);
	 DLL_EXPORT void			Renderer_DestroyRenderer(GraphicsRenderer& renderer);
	 DLL_EXPORT void			Renderer_DestroyRenderPass(VkDevice device, VkRenderPass* renderPass);
	 DLL_EXPORT void			Renderer_DestroyFrameBuffers(VkDevice device, VkFramebuffer* frameBufferList, uint32 count);
	 DLL_EXPORT void			Renderer_DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool);
	 DLL_EXPORT void			Renderer_DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, VkCommandBuffer* commandBufferList, uint32 count);
	 DLL_EXPORT void			Renderer_DestroyBuffer(VkDevice device, VkBuffer* buffer);
#ifdef __cplusplus
}
#endif

	GraphicsRenderer		    Renderer_RendererSetUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface);
	GraphicsRenderer		    Renderer_RebuildSwapChain(void* windowHandle, GraphicsRenderer& renderer);
	VkBool32 VKAPI_CALL		    Renderer_DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* CallBackData, void* pUserData);
	Vector<const char*>		    Renderer_GetRequiredInstanceExtensions();
	Vector<const char*>		    Renderer_GetRequiredDeviceExtensions(VkPhysicalDevice physicalDevice);
	Vector<VkSurfaceFormatKHR>  Renderer_GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	Vector<VkPresentModeKHR>    Renderer_GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	VkPhysicalDeviceFeatures    Renderer_GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice);
	Vector<VkPhysicalDevice>    Renderer_GetPhysicalDeviceList(VkInstance& instance);
	VkSurfaceCapabilitiesKHR    Renderer_GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	Vector<VkSurfaceFormatKHR>  Renderer_GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	Vector<VkPresentModeKHR>    Renderer_GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	VkResult					Renderer_GetDeviceQueue(VkDevice device, uint32 graphicsFamily, uint32 presentFamily, VkQueue & graphicsQueue, VkQueue & presentQueue);
	VkResult					Renderer_GetQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32 & graphicsFamily, uint32 & presentFamily);
	uint32						Renderer_GetMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkSurfaceFormatKHR			Renderer_FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>&availableFormats);
	VkPresentModeKHR			Renderer_FindSwapPresentMode(Vector<VkPresentModeKHR>&availablePresentModes);
	VkExtent2D					Renderer_SetUpSwapChainExtent(void* windowHandle, VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	VkDevice					Renderer_SetUpDevice(VkPhysicalDevice physicalDevice, uint32 graphicsFamily, uint32 presentFamily);
	VkPhysicalDevice			Renderer_SetUpPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, uint32 & graphicsFamily, uint32 & presentFamily);
	void						Renderer_SetUpSwapChain(GraphicsRenderer& renderer);
	VkImage*					Renderer_SetUpSwapChainImages(VkDevice device, VkSwapchainKHR swapChain, uint32 swapChainImageCount);
	VkImageView*				Renderer_SetUpSwapChainImageViews(VkDevice device, VkImage* swapChainImageList, size_t swapChainImageCount, VkSurfaceFormatKHR swapChainImageFormat);
	VkCommandPool				Renderer_SetUpCommandPool(VkDevice device, uint32 graphicsFamily);
	VkResult					Renderer_SetUpSemaphores(VkDevice device, VkFence* inFlightFences, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, int maxFramesInFlight);
	VkResult					Renderer_SetUpSwapChain(void* windowHandle, GraphicsRenderer& renderer);
	void						Renderer_LogVulkanMessage(const char* message, int severity);
	
	void						Renderer_DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT, const VkAllocationCallbacks* pAllocator);
	void						Renderer_DestroyFences(VkDevice device, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, VkFence* fences, size_t semaphoreCount);
	void						Renderer_DestroyCommandPool(VkDevice device, VkCommandPool* commandPool);
	void						Renderer_DestroyDevice(VkDevice device);
	void						Renderer_DestroySurface(VkInstance instance, VkSurfaceKHR* surface);
	void						Renderer_DestroyDebugger(VkInstance* instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT);
	void						Renderer_DestroyInstance(VkInstance* instance);
	void						Renderer_DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout);
	void						Renderer_FreeDeviceMemory(VkDevice device, VkDeviceMemory* memory);
	void						Renderer_DestroySwapChainImageView(VkDevice device, VkSurfaceKHR surface, VkImageView* pSwapChainImageViewList, uint32 count);
	void						Renderer_DestroySwapChain(VkDevice device, VkSwapchainKHR* swapChain);
	void						Renderer_DestroyImageView(VkDevice device, VkImageView* imageView);
	void						Renderer_DestroyImage(VkDevice device, VkImage* image);
	void						Renderer_DestroySampler(VkDevice device, VkSampler* sampler);
	void						Renderer_DestroyPipeline(VkDevice device, VkPipeline* pipeline);
	void						Renderer_DestroyPipelineLayout(VkDevice device, VkPipelineLayout* pipelineLayout);
	void						Renderer_DestroyPipelineCache(VkDevice device, VkPipelineCache* pipelineCache);
