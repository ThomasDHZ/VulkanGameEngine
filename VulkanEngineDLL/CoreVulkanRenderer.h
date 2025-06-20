#pragma once
#include <windows.h>
#include <stdbool.h>

extern "C"
{
	#include "CVulkanRenderer.h"
	#include "VulkanWindow.h"
}
#include "Macro.h"
#include "Typedef.h"
#include "VulkanError.h"
#include "JsonStructs.h"
#include "VkGuid.h"

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

static const char* ValidationLayers[] = { "VK_LAYER_KHRONOS_validation" };

static Vector<VkValidationFeatureEnableEXT> enabledList = { VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
															VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT };

static Vector<VkValidationFeatureDisableEXT> disabledList = { VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT,
																VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT,
																VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT,
																VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT };

struct GPUIncludes
{
	Vector<VkDescriptorBufferInfo> vertexProperties;
	Vector<VkDescriptorBufferInfo> indexProperties;
	Vector<VkDescriptorBufferInfo> transformProperties;
	Vector<VkDescriptorBufferInfo> meshProperties;
	Vector<VkDescriptorBufferInfo> LevelLayermeshProperties;
	Vector<VkDescriptorImageInfo>  texturePropertiesList;
	Vector<VkDescriptorBufferInfo> materialProperties;
};

struct GraphicsRenderer
{
	VkInstance         Instance;
	VkDevice           Device;
	VkPhysicalDevice   PhysicalDevice;
	VkSurfaceKHR       Surface;
	VkCommandPool      CommandPool;
	VkDebugUtilsMessengerEXT DebugMessenger;

	VkFence* InFlightFences;
	VkSemaphore* AcquireImageSemaphores;
	VkSemaphore* PresentImageSemaphores;
	VkImage* SwapChainImages;
	VkImageView* SwapChainImageViews;
	VkExtent2D         SwapChainResolution;
	VkSwapchainKHR     Swapchain;

	size_t			   SwapChainImageCount;
	uint32			   ImageIndex;
	uint32			   CommandIndex;
	uint32			   GraphicsFamily;
	uint32			   PresentFamily;

	VkQueue			   GraphicsQueue;
	VkQueue			   PresentQueue;
	VkFormat           Format;
	VkColorSpaceKHR    ColorSpace;
	VkPresentModeKHR   PresentMode;

	bool               RebuildRendererFlag;
};

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT GraphicsRenderer Renderer_RendererSetUp(void* windowHandle);
	DLL_EXPORT GraphicsRenderer Renderer_RendererSetUp_CS(void* windowHandle);
	DLL_EXPORT void Renderer_DestroyRenderer(GraphicsRenderer& renderer);
#ifdef __cplusplus
}
#endif
	Vector<VkExtensionProperties> Renderer_GetDeviceExtensions(VkPhysicalDevice physicalDevice);
	Vector<VkSurfaceFormatKHR> Renderer_GetSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	Vector<VkPresentModeKHR> Renderer_GetSurfacePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	bool Renderer_GetRayTracingSupport();
	void Renderer_GetRendererFeatures(VkPhysicalDeviceVulkan11Features * physicalDeviceVulkan11Features);
	VkInstance Renderer_CreateVulkanInstance();
	VkDebugUtilsMessengerEXT Renderer_SetupDebugMessenger(VkInstance instance);
	VkPhysicalDeviceFeatures Renderer_GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice);
	Vector<VkPhysicalDevice> Renderer_GetPhysicalDeviceList(VkInstance & instance);
	VkPhysicalDevice Renderer_SetUpPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, uint32 & graphicsFamily, uint32 & presentFamily);
	VkResult Renderer_SetUpSemaphores(VkDevice device, VkFence* inFlightFences, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, size_t swapChainImageCount);
	VkDevice Renderer_SetUpDevice(VkPhysicalDevice physicalDevice, uint32 graphicsFamily, uint32 presentFamily);
	VkCommandPool Renderer_SetUpCommandPool(VkDevice device, uint32 graphicsFamily);
	VkResult Renderer_GetDeviceQueue(VkDevice device, uint32 graphicsFamily, uint32 presentFamily, VkQueue & graphicsQueue, VkQueue & presentQueue);

	VkResult SwapChain_GetQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32 & graphicsFamily, uint32 & presentFamily);
	VkSurfaceCapabilitiesKHR SwapChain_GetSurfaceCapabilities(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	Vector<VkSurfaceFormatKHR> SwapChain_GetPhysicalDeviceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	Vector<VkPresentModeKHR> SwapChain_GetPhysicalDevicePresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	VkSwapchainKHR SwapChain_SetUpSwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32 graphicsFamily, uint32 presentFamily, uint32 width, uint32 height, size_t& swapChainImageCount);
	VkImage* SwapChain_SetUpSwapChainImages(VkDevice device, VkSwapchainKHR swapChain, uint32 swapChainImageCount);

	VkImageView* SwapChain_SetUpSwapChainImageViews(VkDevice device, VkImage* swapChainImageList, size_t swapChainImageCount, VkSurfaceFormatKHR swapChainImageFormat);
	VkSurfaceFormatKHR SwapChain_FindSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>&availableFormats);
	VkPresentModeKHR SwapChain_FindSwapPresentMode(Vector<VkPresentModeKHR>&availablePresentModes);

	VkResult Renderer_SetUpSwapChainCS(GraphicsRenderer& renderState);
	VkResult Renderer_SetUpSwapChain(void* windowHandle, GraphicsRenderer& renderState);

