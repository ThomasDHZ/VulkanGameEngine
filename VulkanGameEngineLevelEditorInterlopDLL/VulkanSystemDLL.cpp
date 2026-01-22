//#include "VulkanSystemDLL.h"
//
//LogVulkanMessageCallback g_logVulkanMessageCallback = nullptr;
//
//void VulkanSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback)
//{
//    g_logVulkanMessageCallback = callback;
//}
//
//void VulkanSystem_LogVulkanMessage(const char* message, int severity)
//{
//    if (g_logVulkanMessageCallback)
//    {
//        g_logVulkanMessageCallback(message, severity);
//    }
//}
//
// VkInstance VulkanSystem_CreateVulkanInstance()
//{
//	 return vulkanSystem.CreateVulkanInstance();
//}
//
//VkSurfaceKHR VulkanSystem_CreateVulkanSurface(void* windowHandle, VkInstance instance)
//{
//	return vulkanSystem.CreateVulkanSurface(windowHandle, instance);
//}
//
//VkCommandBuffer VulkanSystem_BeginSingleUseCommand(VkDevice device, VkCommandPool commandPool)
//{
//	return vulkanSystem.BeginSingleUseCommand(device, commandPool);
//}
//
//void VulkanSystem_EndSingleUseCommand(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer)
//{
//	return vulkanSystem.EndSingleUseCommand(device, commandPool, graphicsQueue, commandBuffer);
//}
//
//void VulkanSystem_DestroyRenderer()
//{
//	vulkanSystem.DestroyRenderer();
//}
//
//void VulkanSystem_DestroyRenderPass(VkDevice device, VkRenderPass* renderPass)
//{
//	return vulkanSystem.DestroyRenderPass(device, renderPass);
//}
//
//void VulkanSystem_DestroyFrameBuffers(VkDevice device, VkFramebuffer* frameBufferList, uint32 count)
//{
//	return vulkanSystem.DestroyFrameBuffers(device, frameBufferList, count);
//}
//
//void VulkanSystem_DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool)
//{
//	return vulkanSystem.DestroyDescriptorPool(device, descriptorPool);
//}
//
//void VulkanSystem_DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, VkCommandBuffer* commandBufferList, uint32 count)
//{
//	return vulkanSystem.DestroyCommandBuffers(device, commandPool, commandBufferList, count);
//}
//
//void VulkanSystem_DestroyBuffer(VkDevice device, VkBuffer* buffer)
//{
//	return vulkanSystem.DestroyBuffer(device, buffer);
//}
