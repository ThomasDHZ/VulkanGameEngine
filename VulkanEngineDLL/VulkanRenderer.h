#pragma once
#include <windows.h>
#include <stdbool.h>

#include "DLL.h"
#include "Macro.h"
#include "Typedef.h"
#include "VulkanError.h"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_win32.h>
#include "InputEnum.h"
#include "Typedef.h"

static const char* ValidationLayers[] = { "VK_LAYER_KHRONOS_validation" };

static const char* InstanceExtensionList[] =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

static const char* DeviceExtensionList[] = 
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_MAINTENANCE3_EXTENSION_NAME,
	VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
	VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
	VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
	VK_EXT_ROBUSTNESS_2_EXTENSION_NAME
};

static Vector<VkValidationFeatureEnableEXT> enabledList = 
{
	VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
	VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT
};

static Vector<VkValidationFeatureDisableEXT> disabledList = 
{
	VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT,
	VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
	VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT
};

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
DLL_EXPORT GraphicsRenderer renderer;

extern HWND editorRichTextBoxCallback;
typedef void (*LogVulkanMessageCallback)(const char* message, int severity);

	static const int MAX_FRAMES_IN_FLIGHT = 3;
	typedef void (*RichTextBoxCallback)(const char*);

	typedef struct
	{
		VkRenderPass* pRenderPass;
		const VkAttachmentDescription* pAttachmentList;
		const VkSubpassDescription* pSubpassDescriptionList;
		const VkSubpassDependency* pSubpassDependencyList;
		uint32						AttachmentCount;
		uint32						SubpassCount;
		uint32						DependencyCount;
		uint32						Width;
		uint32						Height;
	}RenderPassCreateInfoStruct;

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT void Debug_SetRichTextBoxHandle(HWND hwnd);
	DLL_EXPORT void SetLogVulkanMessageCallback(LogVulkanMessageCallback callback);
	DLL_EXPORT void LogVulkanMessage(const char* message, int severity);
	DLL_EXPORT VkInstance Renderer_CreateVulkanInstance();
	DLL_EXPORT VkDebugUtilsMessengerEXT Renderer_SetupDebugMessenger(VkInstance instance);
	DLL_EXPORT VkSurfaceKHR Renderer_CreateVulkanSurface(void* windowHandle, VkInstance instance);
	DLL_EXPORT GraphicsRenderer Renderer_RendererSetUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface, VkDebugUtilsMessengerEXT& debugMessenger);
	DLL_EXPORT GraphicsRenderer Renderer_RebuildSwapChain(void* windowHandle, GraphicsRenderer& renderer);
	DLL_EXPORT VkCommandBuffer Renderer_BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
	DLL_EXPORT VkResult Renderer_EndSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);
	DLL_EXPORT void Renderer_DestroyRenderer(GraphicsRenderer& renderer);
	DLL_EXPORT VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	DLL_EXPORT void DestroyDebugUtilsMessengerEXT(VkInstance instance, const VkAllocationCallbacks* pAllocator);
	DLL_EXPORT VkResult Renderer_CreateRenderPass(VkDevice device, RenderPassCreateInfoStruct* renderPassCreateInfo);
	DLL_EXPORT VkResult Renderer_AllocateDescriptorSets(VkDevice device, VkDescriptorSet* descriptorSet, VkDescriptorSetAllocateInfo* descriptorSetAllocateInfo);
	DLL_EXPORT VkResult Renderer_AllocateCommandBuffers(VkDevice device, VkCommandBuffer* commandBuffer, VkCommandBufferAllocateInfo* commandBufferAllocateInfo);
	DLL_EXPORT VkResult Renderer_CreateCommandPool(VkDevice device, VkCommandPool* commandPool, VkCommandPoolCreateInfo* commandPoolInfo);
	DLL_EXPORT VkResult Renderer_SetUpSemaphores(VkDevice device, VkFence* inFlightFences, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, int maxFramesInFlight);
	DLL_EXPORT uint32 Renderer_GetMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	DLL_EXPORT VkCommandBuffer Renderer_BeginSingleUseCommandBuffer(VkDevice device, VkCommandPool commandPool);
	DLL_EXPORT VkResult Renderer_EndSingleUseCommandBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);
	DLL_EXPORT void Renderer_DestroyRenderPass(VkDevice device, VkRenderPass* renderPass);
	DLL_EXPORT void Renderer_DestroyFrameBuffers(VkDevice device, VkFramebuffer* frameBufferList, uint32 count);
	DLL_EXPORT void Renderer_DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool);
	DLL_EXPORT void Renderer_DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, VkCommandBuffer* commandBufferList, uint32 count);
	DLL_EXPORT void Renderer_DestroyBuffer(VkDevice device, VkBuffer* buffer);
#ifdef __cplusplus
}
#endif

	VkResult Renderer_GetWin32Extensions(uint32_t* extensionCount, std::vector<std::string>& enabledExtensions);
	VkBool32 VKAPI_CALL Vulkan_DebugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType, const VkDebugUtilsMessengerCallbackDataEXT* CallBackData, void* pUserData);
	Vector<VkExtensionProperties> Renderer_GetDeviceExtensions(VkPhysicalDevice physicalDevice);
	Vector<VkSurfaceFormatKHR> Renderer_GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	Vector<VkPresentModeKHR> Renderer_GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	bool Renderer_GetRayTracingSupport();
	void Renderer_GetRendererFeatures(VkPhysicalDeviceVulkan11Features* physicalDeviceVulkan11Features);
	VkPhysicalDeviceFeatures Renderer_GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice);
	Vector<VkPhysicalDevice> Renderer_GetPhysicalDeviceList(VkInstance& instance);
	VkPhysicalDevice Renderer_SetUpPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, uint32 & graphicsFamily, uint32 & presentFamily);
	VkDevice Renderer_SetUpDevice(VkPhysicalDevice physicalDevice, uint32 graphicsFamily, uint32 presentFamily);
	VkCommandPool Renderer_SetUpCommandPool(VkDevice device, uint32 graphicsFamily);
	VkResult Renderer_GetDeviceQueue(VkDevice device, uint32 graphicsFamily, uint32 presentFamily, VkQueue & graphicsQueue, VkQueue & presentQueue);
	VkResult SwapChain_GetQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32 & graphicsFamily, uint32 & presentFamily);
	VkSurfaceCapabilitiesKHR SwapChain_GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	Vector<VkSurfaceFormatKHR> SwapChain_GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	Vector<VkPresentModeKHR> SwapChain_GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	void SwapChain_SetUpSwapChain(GraphicsRenderer& renderer);
	VkImage* SwapChain_SetUpSwapChainImages(VkDevice device, VkSwapchainKHR swapChain, uint32 swapChainImageCount);
	VkImageView* SwapChain_SetUpSwapChainImageViews(VkDevice device, VkImage* swapChainImageList, size_t swapChainImageCount, VkSurfaceFormatKHR swapChainImageFormat);
	VkSurfaceFormatKHR SwapChain_FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>&availableFormats);
	VkPresentModeKHR SwapChain_FindSwapPresentMode(Vector<VkPresentModeKHR>&availablePresentModes);
	VkResult Renderer_SetUpSwapChain(void* windowHandle, GraphicsRenderer& renderer);
	void Renderer_DestroyFences(VkDevice device, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, VkFence* fences, size_t semaphoreCount);
	void Renderer_DestroyCommandPool(VkDevice device, VkCommandPool* commandPool);
	void Renderer_DestroyDevice(VkDevice device);
	void Renderer_DestroySurface(VkInstance instance, VkSurfaceKHR* surface);
	void Renderer_DestroyDebugger(VkInstance* instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT);
	void Renderer_DestroyInstance(VkInstance* instance);
	void Renderer_DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout);
	void Renderer_FreeDeviceMemory(VkDevice device, VkDeviceMemory* memory);
	void Renderer_DestroySwapChainImageView(VkDevice device, VkSurfaceKHR surface, VkImageView* pSwapChainImageViewList, uint32 count);
	void Renderer_DestroySwapChain(VkDevice device, VkSwapchainKHR* swapChain);
	void Renderer_DestroyImageView(VkDevice device, VkImageView* imageView);
	void Renderer_DestroyImage(VkDevice device, VkImage* image);
	void Renderer_DestroySampler(VkDevice device, VkSampler* sampler);
	void Renderer_DestroyPipeline(VkDevice device, VkPipeline* pipeline);
	void Renderer_DestroyPipelineLayout(VkDevice device, VkPipelineLayout* pipelineLayout);
	void Renderer_DestroyPipelineCache(VkDevice device, VkPipelineCache* pipelineCache);

