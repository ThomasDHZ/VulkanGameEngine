#include "pch.h"
#include "BufferSystemDLL.h"

VulkanBuffer VulkanBuffer_CreateVulkanBuffer(uint bufferId, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
{
    return bufferSystem.CreateVulkanBuffer(bufferId, bufferElementSize, bufferElementCount, bufferTypeEnum, usage, properties, usingStagingBuffer);
}

VulkanBuffer VulkanBuffer_CreateVulkanBuffer2(uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
{
    return bufferSystem.CreateVulkanBuffer(bufferId, bufferData, bufferElementSize, bufferElementCount, bufferTypeEnum, usage, properties, usingStagingBuffer);
}

void VulkanBuffer_UpdateBufferMemory(VulkanBuffer& vulkanBuffer, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount)
{
    bufferSystem.UpdateBufferMemory(vulkanBuffer, bufferData, bufferElementSize, bufferElementCount);
}

void VulkanBuffer_CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size)
{
    bufferSystem.CopyBuffer(srcBuffer, dstBuffer, size);
}

void VulkanBuffer_DestroyBuffer(VulkanBuffer& vulkanBuffer)
{
    bufferSystem.DestroyBuffer(vulkanBuffer);
}
