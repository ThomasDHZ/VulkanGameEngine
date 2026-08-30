#include "VulkanSystemDLL.h"

void VulkanSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback)
{
    vulkan.Debug().CreateLogMessageCallback(callback);
}

void VulkanSystem_VulkanSetUp(void* windowHandle, ivec2 windowSize, ivec2 renderSize)
{
    vulkan.VulkanSetUp(windowHandle, windowSize, renderSize);
}

uint32 VulkanSystem_GetMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties)
{
    return vulkan.GetMemoryType(physicalDevice, typeFilter, properties);
}

VkCommandBuffer VulkanSystem_StartFrame()
{
    return vulkan.StartFrame();
}

void VulkanSystem_SetCustomFrameBufferSize(ivec2 size)
{
    return vulkan.SetCustomFrameBufferSize(size);
}

void VulkanSystem_EndFrame(VkCommandBuffer& commandBuffer)
{
    vulkan.EndFrame(commandBuffer);
}

//
//void VulkanSystem_Shutdown()
//{
//    vulkan.Destroy();
//}
