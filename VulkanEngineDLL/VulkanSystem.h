#pragma once
#include "Platform.h"
#include "InputEnum.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif

	class VulkanSystem
	{
		public:
			static VulkanSystem& Get();  

			uint32_t                 ApiVersion = VK_API_VERSION_1_1;
			VkInstance               Instance = VK_NULL_HANDLE;
			VkDevice                 Device = VK_NULL_HANDLE;
			VkPhysicalDevice         PhysicalDevice = VK_NULL_HANDLE;
			VkSurfaceKHR             Surface = VK_NULL_HANDLE;
			VkCommandPool            CommandPool = VK_NULL_HANDLE;
			VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;

			VkFence* InFlightFences = nullptr;
			VkSemaphore* AcquireImageSemaphores = nullptr;
			VkSemaphore* PresentImageSemaphores = nullptr;

			VkImage* SwapChainImages = nullptr;
			VkImageView* SwapChainImageViews = nullptr;
			VkSwapchainKHR Swapchain = VK_NULL_HANDLE;

			VkExtent2D   SwapChainResolution{};
			size_t       SwapChainImageCount = UINT64_MAX;

			uint32_t     ImageIndex = UINT32_MAX;
			uint32_t     CommandIndex = UINT32_MAX;
			uint32_t     GraphicsFamily = UINT32_MAX;
			uint32_t     PresentFamily = UINT32_MAX;

			VkQueue      GraphicsQueue = VK_NULL_HANDLE;
			VkQueue      PresentQueue = VK_NULL_HANDLE;

			VkFormat          Format = VK_FORMAT_UNDEFINED;
			VkColorSpaceKHR   ColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			VkPresentModeKHR  PresentMode = VK_PRESENT_MODE_FIFO_KHR;

			bool RebuildRendererFlag = false;

#if defined(__ANDROID__)
			PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress;
#endif



			DLL_EXPORT VkInstance		  CreateVulkanInstance();
			DLL_EXPORT VkSurfaceKHR	  CreateVulkanSurface(void* windowHandle, VkInstance instance);
			DLL_EXPORT VkCommandBuffer  BeginSingleUseCommand(VkDevice device, VkCommandPool commandPool);
			DLL_EXPORT void			  EndSingleUseCommand(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);
			DLL_EXPORT  uint32			  GetMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
			DLL_EXPORT void			  DestroyRenderer();
			DLL_EXPORT void			  DestroyRenderPass(VkDevice device, VkRenderPass* renderPass);
			 DLL_EXPORT void			  DestroyFrameBuffers(VkDevice device, VkFramebuffer* frameBufferList, uint32 count);
			 DLL_EXPORT void			  DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool);
			 DLL_EXPORT void			  DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, VkCommandBuffer* commandBufferList, uint32 count);
			 DLL_EXPORT void			  DestroyBuffer(VkDevice device, VkBuffer* buffer);
			 DLL_EXPORT void			  FreeDeviceMemory(VkDevice device, VkDeviceMemory* memory);

			 
			uint32						FindMaxApiVersion(VkPhysicalDevice physicalDevice);
			void		    RendererSetUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface);
			void		    RebuildSwapChain(void* windowHandle);
			static VkBool32 VKAPI_CALL	DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* CallBackData, void* pUserData);
			Vector<const char*>		    GetRequiredInstanceExtensions();
			Vector<const char*>		    GetRequiredDeviceExtensions(VkPhysicalDevice physicalDevice);
			Vector<const char*>			GetValidationLayerProperties();
			Vector<VkSurfaceFormatKHR>  GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
			Vector<VkPresentModeKHR>    GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
			VkSurfaceCapabilitiesKHR    GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
			VkPhysicalDeviceProperties  GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice);
			VkPhysicalDeviceFeatures    GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice);
			Vector<VkPhysicalDevice>    GetPhysicalDeviceList(VkInstance& instance);
			Vector<VkSurfaceFormatKHR>  GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
			Vector<VkPresentModeKHR>    GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
			void						GetDeviceQueue(VkDevice device, uint32 graphicsFamily, uint32 presentFamily, VkQueue& graphicsQueue, VkQueue& presentQueue);
			void						GetQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32& graphicsFamily, uint32& presentFamily);
			VkSurfaceFormatKHR			FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats);
			VkPresentModeKHR			FindSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes);
			VkExtent2D					SetUpSwapChainExtent(void* windowHandle, VkSurfaceCapabilitiesKHR& surfaceCapabilities);
			VkDevice					SetUpDevice(VkPhysicalDevice physicalDevice, uint32 graphicsFamily, uint32 presentFamily);
			VkPhysicalDevice			SetUpPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, uint32& graphicsFamily, uint32& presentFamily);
			void						SetUpSwapChain();
			VkImage*					SetUpSwapChainImages(VkDevice device, VkSwapchainKHR swapChain, uint32 swapChainImageCount);
			VkImageView*				SetUpSwapChainImageViews(VkDevice device, VkImage* swapChainImageList, size_t swapChainImageCount, VkSurfaceFormatKHR swapChainImageFormat);
			VkCommandPool				SetUpCommandPool(VkDevice device, uint32 graphicsFamily);
			void						SetUpSemaphores(VkDevice device, VkFence* inFlightFences, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, int maxFramesInFlight);
			void						SetUpSwapChain(void* windowHandle);

			void						DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT, const VkAllocationCallbacks* pAllocator);
			void						DestroyFences(VkDevice device, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, VkFence* fences, size_t semaphoreCount);
			void						DestroyCommandPool(VkDevice device, VkCommandPool* commandPool);
			void						DestroyDevice(VkDevice device);
			void						DestroySurface(VkInstance instance, VkSurfaceKHR* surface);
			void						DestroyDebugger(VkInstance* instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT);
			void						DestroyInstance(VkInstance* instance);
			void						DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout);
			void						DestroySwapChainImageView(VkDevice device, VkSurfaceKHR surface, VkImageView* pSwapChainImageViewList, uint32 count);
			void						DestroySwapChain(VkDevice device, VkSwapchainKHR* swapChain);
			void						DestroyImageView(VkDevice device, VkImageView* imageView);
			void						DestroyImage(VkDevice device, VkImage* image);
			void						DestroySampler(VkDevice device, VkSampler* sampler);
			void						DestroyPipeline(VkDevice device, VkPipeline* pipeline);
			void						DestroyPipelineLayout(VkDevice device, VkPipelineLayout* pipelineLayout);
			void						DestroyPipelineCache(VkDevice device, VkPipelineCache* pipelineCache);
            
private:
	// Prevent anyone from creating/destroying copies
	VulkanSystem() = default;
	~VulkanSystem() = default;
	VulkanSystem(const VulkanSystem&) = delete;
	VulkanSystem& operator=(const VulkanSystem&) = delete;
	VulkanSystem(VulkanSystem&&) = delete;
	VulkanSystem& operator=(VulkanSystem&&) = delete;
};

extern DLL_EXPORT VulkanSystem& vulkanSystem;
inline VulkanSystem& VulkanSystem::Get()
{
	static VulkanSystem instance;
	return instance;
}