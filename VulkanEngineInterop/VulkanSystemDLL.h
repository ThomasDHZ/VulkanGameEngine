#pragma once
#include "DLL.h"
#include <VulkanSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT void							 VulkanSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback);
	DLL_EXPORT void							 VulkanSystem_VulkanSetUp(void* windowHandle, ivec2 windowSize, ivec2 renderSize);
	DLL_EXPORT uint32						 VulkanSystem_GetMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties);
	DLL_EXPORT void							 VulkanSystem_StartFrame();
	DLL_EXPORT void							 VulkanSystem_EndFrame(VkCommandBuffer& commandBuffer);
	//DLL_EXPORT void							 VulkanSystem_Shutdown();
#ifdef __cplusplus
}
#endif