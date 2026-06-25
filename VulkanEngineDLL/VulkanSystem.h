#pragma once
#include <Platform.h>
#include <VulkanSystem2.h>
#include <VulkanWindow.h>
#include "InputEnum.h"
#include <vk_mem_alloc.h>
#ifdef __ANDROID__
#include <android/log.h>
#endif
#include <VulkanInstance.h>

//#ifdef __cplusplus
//extern "C" {
//#endif
//	typedef void (*LogVulkanMessageCallback)(const char* message, int severity);
//	DLL_EXPORT void VulkanSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback);
//	DLL_EXPORT void VulkanSystem_LogVulkanMessage(const char* message, int severity);
//#ifdef __cplusplus
//}
//#endif

class VulkanSystem
{
private:
	VulkanSystem() = default;
	~VulkanSystem() = default;
	VulkanSystem(const VulkanSystem&) = delete;
	VulkanSystem& operator=(const VulkanSystem&) = delete;
	VulkanSystem(VulkanSystem&&) = delete;
	VulkanSystem& operator=(VulkanSystem&&) = delete;

	bool m_usingCustomSurface = false;
	VmaAllocator						   SetUpVmaAllocation();
	Vector<const char*>					   GetValidationLayerProperties();

	void								   DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT, const VkAllocationCallbacks* pAllocator);
	void								   DestroyFences(VkDevice device, Vector<VkSemaphore>& acquireImageSemaphores, Vector<VkSemaphore>& presentImageSemaphores, Vector<VkFence>& fences);
	void								   DestroyCommandPool(VkDevice device, VkCommandPool* commandPool);
	void								   DestroyDevice(VkDevice device);
	void								   DestroySurface(VkInstance instance, VkSurfaceKHR* surface);
	void								   DestroyDebugger(VkInstance* instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT);
	void								   DestroyInstance(VkInstance* instance);

public:
	static VulkanSystem& Get();

	VkFormat							   Format = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR						   ColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	VkPresentModeKHR					   PresentMode = VK_PRESENT_MODE_FIFO_KHR;
	bool								   RequestRayTracingSupport = true;
	bool								   RayTracingSupported = false;
	bool								   RebuildRendererFlag = false;

#if defined(__ANDROID__)
	PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress;
#endif

	DLL_EXPORT void						   VulkanSetUp(ivec2 windowResolution, ivec2 renderResolution);
	DLL_EXPORT void						   VulkanSetUp(void* windowHandle, ivec2 windowResolution, ivec2 renderResolution);
	DLL_EXPORT void						   RendererSetUp(const void* windowHandle, ivec2 renderResolution);

	DLL_EXPORT VkCommandBuffer			   BeginSingleUseCommand();
	DLL_EXPORT void						   EndSingleUseCommand(VkCommandBuffer commandBuffer);
	DLL_EXPORT uint32					   GetMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);


	DLL_EXPORT void						   DestroyRenderer();
	DLL_EXPORT void						   DestroySwapChainImageView(VkDevice device, Vector<VkImageView>& pSwapChainImageViewList);
	DLL_EXPORT void						   DestroySwapChain(VkDevice device, VkSwapchainKHR* swapChain);
	DLL_EXPORT void						   DestroyRenderPass(VkDevice device, VkRenderPass* renderPass);
	DLL_EXPORT void						   DestroyFrameBuffers(VkDevice device, Vector<VkFramebuffer>& frameBufferList);
	DLL_EXPORT void						   DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool);
	DLL_EXPORT void						   DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, Vector<VkCommandBuffer>& commandBufferList);
	DLL_EXPORT void						   DestroyBuffer(VkDevice device, VkBuffer* buffer);
	DLL_EXPORT void						   DestroyImageView(VkDevice device, VkImageView* imageView);
	DLL_EXPORT void						   DestroyImage(VkDevice device, VkImage* image);
	DLL_EXPORT void						   DestroySampler(VkDevice device, VkSampler* sampler);
	DLL_EXPORT void						   DestroyPipeline(VkDevice device, VkPipeline* pipeline);
	DLL_EXPORT void						   DestroyPipelineLayout(VkDevice device, VkPipelineLayout* pipelineLayout);
	DLL_EXPORT void						   DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout);
	DLL_EXPORT void						   DestroyPipelineCache(VkDevice device, VkPipelineCache* pipelineCache);
	DLL_EXPORT void						   FreeDeviceMemory(VkDevice device, VkDeviceMemory* memory);
};
extern DLL_EXPORT VulkanSystem& vulkanSystem;
inline VulkanSystem& VulkanSystem::Get()
{
	static VulkanSystem instance;
	return instance;
}

