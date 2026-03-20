#include "BufferSystemDLL.h"

 uint32 BufferSystem_CreateStaticVulkanBuffer(const void* srcData, VkDeviceSize size, VkBufferUsageFlags shaderUsageFlags, VkDeviceSize offset)
{
     return  BufferSystem_CreateStaticVulkanBuffer(srcData, size, shaderUsageFlags, offset);
}

 uint32 BufferSystem_CreateDynamicBuffer(const void* srcData, VkDeviceSize size, VkBufferUsageFlags usageFlags)
{
     return BufferSystem_CreateDynamicBuffer(srcData, size, usageFlags);
}

 void BufferSystem_UpdateDynamicBuffer(uint32 bufferId, const void* data, VkDeviceSize size, VkDeviceSize offset)
{
     return BufferSystem_UpdateDynamicBuffer(bufferId, data, size, offset);
}

 void BufferSystem_CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size, VkBufferUsageFlags shaderUsageFlags, VkDeviceSize offset)
{
     return BufferSystem_CopyBuffer(srcBuffer, dstBuffer, size, shaderUsageFlags, offset);
}

 void BufferSystem_DestroyBuffer(VulkanBuffer& vulkanBuffer)
{
     return BufferSystem_DestroyBuffer(vulkanBuffer);
}

 void BufferSystem_DestroyAllBuffers()
{
     return BufferSystem_DestroyAllBuffers();
}
