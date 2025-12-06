#pragma once
#include <VulkanRenderer.h>

typedef void (*LogVulkanMessageCallback)(const char* message, int severity);

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT void				VulkanSystem_LogVulkanMessage(const char* message, int severity);
	DLL_EXPORT void				VulkanSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback);
	DLL_EXPORT VkInstance		VulkanSystem_CreateVulkanInstance();
	DLL_EXPORT VkSurfaceKHR		VulkanSystem_CreateVulkanSurface(void* windowHandle, VkInstance instance);
	DLL_EXPORT VkCommandBuffer  VulkanSystem_BeginSingleUseCommand(VkDevice device, VkCommandPool commandPool);
	DLL_EXPORT void				VulkanSystem_EndSingleUseCommand(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);
	DLL_EXPORT void				VulkanSystem_DestroyRenderer();
	DLL_EXPORT void				VulkanSystem_DestroyRenderPass(VkDevice device, VkRenderPass* renderPass);
	DLL_EXPORT void				VulkanSystem_DestroyFrameBuffers(VkDevice device, VkFramebuffer* frameBufferList, uint32 count);
	DLL_EXPORT void				VulkanSystem_DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool);
	DLL_EXPORT void				VulkanSystem_DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, VkCommandBuffer* commandBufferList, uint32 count);
	DLL_EXPORT void				VulkanSystem_DestroyBuffer(VkDevice device, VkBuffer* buffer);
#ifdef __cplusplus
}
#endif

