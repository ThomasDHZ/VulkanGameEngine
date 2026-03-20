#pragma once
#include <BufferSystem.h>

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT uint32                      BufferSystem_CreateStaticVulkanBuffer(const void* srcData, VkDeviceSize size, VkBufferUsageFlags shaderUsageFlags, VkDeviceSize offset = 0);
    DLL_EXPORT uint32                      BufferSystem_CreateDynamicBuffer(const void* srcData, VkDeviceSize size, VkBufferUsageFlags usageFlags);
    DLL_EXPORT void                        BufferSystem_UpdateDynamicBuffer(uint32 bufferId, const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    DLL_EXPORT void                        BufferSystem_CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size, VkBufferUsageFlags shaderUsageFlags, VkDeviceSize offset = 0);
    DLL_EXPORT void                        BufferSystem_DestroyBuffer(VulkanBuffer& vulkanBuffer);
    DLL_EXPORT void                        BufferSystem_DestroyAllBuffers();
#ifdef __cplusplus
}
#endif
