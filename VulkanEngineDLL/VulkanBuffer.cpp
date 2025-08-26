#include "VulkanBuffer.h"
#include "MemorySystem.h"

VulkanBuffer VulkanBuffer_CreateVulkanBuffer(const GraphicsRenderer& renderer, uint bufferId, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
{
    VkDeviceSize bufferSize = bufferElementSize * bufferElementCount;
    VulkanBuffer vulkanBuffer =
    {
        .BufferId = bufferId,
        .BufferSize = bufferSize,
        .BufferUsage = usage,
        .BufferProperties = properties,
        .BufferType = bufferTypeEnum,
        .UsingStagingBuffer = usingStagingBuffer,
    };

    void* bufferData = memorySystem.AddPtrBuffer<void*>(bufferSize, __FILE__, __LINE__, __func__);
    memset(bufferData, 0, bufferSize);

    VkResult result;
    if (vulkanBuffer.UsingStagingBuffer)
    {
        result = Buffer_CreateStagingBuffer(renderer, &vulkanBuffer.StagingBuffer, &vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
    }
    else
    {
        result = Buffer_CreateBuffer(renderer, &vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferProperties, vulkanBuffer.BufferUsage);
    }

    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to create Vulkan buffer");
    }

    memorySystem.RemovePtrBuffer(bufferData);
    return vulkanBuffer;
}

VulkanBuffer VulkanBuffer_CreateVulkanBuffer2(const GraphicsRenderer& renderer, uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
{
    VkDeviceSize bufferSize = bufferElementSize * bufferElementCount;
    VulkanBuffer vulkanBuffer =
    {
        .BufferId = bufferId,
        .BufferSize = bufferSize,
        .BufferUsage = usage,
        .BufferProperties = properties,
        .BufferType = bufferTypeEnum,
        .UsingStagingBuffer = usingStagingBuffer,
    };

    VkResult result;
    if (vulkanBuffer.UsingStagingBuffer) 
    {
        result = Buffer_CreateStagingBuffer(renderer, &vulkanBuffer.StagingBuffer, &vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
    }
    else 
    {
        result = Buffer_CreateBuffer(renderer, &vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferProperties, vulkanBuffer.BufferUsage);
    }

    if (result != VK_SUCCESS) 
    {
        RENDERER_ERROR("Failed to create Vulkan buffer");
    }

    return vulkanBuffer;
}

VulkanBuffer VulkanBuffer_CreateVulkanBuffer3(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer, uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
{
    VkDeviceSize bufferSize = bufferElementSize * bufferElementCount;
    vulkanBuffer =
    {
        .BufferId = bufferId,
        .BufferSize = bufferSize,
        .BufferUsage = usage,
        .BufferProperties = properties,
        .BufferType = bufferTypeEnum,
        .UsingStagingBuffer = usingStagingBuffer,
    };

    VkResult result;
    if (vulkanBuffer.UsingStagingBuffer)
    {
        result = Buffer_CreateStagingBuffer(renderer, &vulkanBuffer.StagingBuffer, &vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
    }
    else
    {
        result = Buffer_CreateBuffer(renderer, &vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferProperties, vulkanBuffer.BufferUsage);
    }

    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to create Vulkan buffer");
    }

    return vulkanBuffer;
}

void VulkanBuffer_UpdateBufferSize(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer, VkDeviceSize newBufferElementSize, uint newBufferElementCount)
{
    VkDeviceSize newBufferSize = newBufferElementSize * newBufferElementCount;
    if (vulkanBuffer.UsingStagingBuffer)
    {
        VULKAN_RESULT(Buffer_UpdateBufferSize(renderer, &vulkanBuffer.StagingBuffer, &vulkanBuffer.StagingBufferMemory, nullptr, vulkanBuffer.BufferSize, newBufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties));
        VULKAN_RESULT(Buffer_UpdateBufferSize(renderer, &vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, nullptr, vulkanBuffer.BufferSize, newBufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties));
    }
    else 
    {
        VULKAN_RESULT(Buffer_UpdateBufferSize(renderer, &vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, nullptr, vulkanBuffer.BufferSize, newBufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties));
    }
}

void VulkanBuffer_UpdateBufferMemory(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount)
{
    VkDeviceSize newBufferSize = bufferElementSize * bufferElementCount;
    if (vulkanBuffer.UsingStagingBuffer) 
    {
        if (vulkanBuffer.BufferSize != newBufferSize) {
            VulkanBuffer_UpdateBufferSize(renderer, vulkanBuffer, bufferElementSize, bufferElementCount);
        }
        Buffer_UpdateStagingBufferData(renderer, vulkanBuffer.StagingBuffer, vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, newBufferSize);
    }
    else 
    {
        if (vulkanBuffer.BufferSize != newBufferSize) 
        {
            VulkanBuffer_UpdateBufferSize(renderer, vulkanBuffer, bufferElementSize, bufferElementCount);
        }
        if (Buffer_UpdateBufferMemory(renderer, vulkanBuffer.BufferMemory, bufferData, newBufferSize) != VK_SUCCESS) 
        {
            RENDERER_ERROR("Failed to update buffer memory.");
        }
    }
}

VkResult VulkanBuffer_CopyBuffer(const GraphicsRenderer& renderer, VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size) 
{
    return Buffer_CopyBuffer(renderer, srcBuffer, dstBuffer, size);
}


void VulkanBuffer_DestroyBuffer(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer) {
    VkResult result = Buffer_DestroyBuffer(renderer, &vulkanBuffer.Buffer, &vulkanBuffer.StagingBuffer, &vulkanBuffer.BufferMemory, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferData, &vulkanBuffer.BufferSize, &vulkanBuffer.BufferUsage, &vulkanBuffer.BufferProperties);
    if (result != VK_SUCCESS) 
    {
        RENDERER_ERROR("Failed to destroy buffer");
    }
}

VkResult Buffer_UpdateBufferMemory(const GraphicsRenderer& renderState, VkDeviceMemory bufferMemory, void* dataToCopy, VkDeviceSize bufferSize) 
{
    if (dataToCopy == nullptr || bufferSize == 0) 
    {
        RENDERER_ERROR("Buffer data and size cannot be nullptr");
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    void* mappedData = nullptr;
    VkResult result = vkMapMemory(renderState.Device, bufferMemory, 0, bufferSize, 0, &mappedData);
    if (result != VK_SUCCESS) 
    {
        RENDERER_ERROR("Failed to map buffer memory");
        return result;
    }

    memcpy(mappedData, dataToCopy, static_cast<size_t>(bufferSize));
    vkUnmapMemory(renderState.Device, bufferMemory);
    return VK_SUCCESS;
}

void Buffer_CopyBufferMemory(const GraphicsRenderer& renderState, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize) 
{
    VkBufferCopy copyRegion = 
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = bufferSize
    };

    VkCommandBuffer commandBuffer = Renderer_BeginSingleUseCommandBuffer(renderState.Device, renderState.CommandPool);
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    Renderer_EndSingleUseCommandBuffer(renderState.Device, renderState.CommandPool, renderState.GraphicsQueue, commandBuffer);
}

VkResult Buffer_AllocateMemory(const GraphicsRenderer& renderState, VkBuffer* bufferData, VkDeviceMemory* bufferMemory, VkMemoryPropertyFlags properties) 
{
    if (bufferData == nullptr || bufferMemory == nullptr) 
    {
        RENDERER_ERROR("Buffer data or buffer memory is nullptr");
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(renderState.Device, *bufferData, &memRequirements);

    VkMemoryAllocateFlagsInfo extendedAllocFlagsInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO
    };

    VkMemoryAllocateInfo allocInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &extendedAllocFlagsInfo,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = Renderer_GetMemoryType(renderState.PhysicalDevice, memRequirements.memoryTypeBits, properties),
    };

    return vkAllocateMemory(renderState.Device, &allocInfo, nullptr, bufferMemory);
}

void* Buffer_MapBufferMemory(const GraphicsRenderer& renderState, VkDeviceMemory bufferMemory, VkDeviceSize bufferSize, bool* isMapped) 
{
    if (*isMapped) 
    {
        RENDERER_ERROR("Buffer already mapped!");
        return nullptr;
    }

    void* mappedData = nullptr;
    VkResult result = vkMapMemory(renderState.Device, bufferMemory, 0, bufferSize, 0, &mappedData);
    if (result != VK_SUCCESS) 
    {
        RENDERER_ERROR("Failed to map buffer memory");
        return nullptr;
    }
    *isMapped = true;
    return mappedData;
}

VkResult Buffer_UnmapBufferMemory(const GraphicsRenderer& renderState, VkDeviceMemory bufferMemory, bool* isMapped) 
{
    if (*isMapped)
    {
        vkUnmapMemory(renderState.Device, bufferMemory);
        *isMapped = false;
    }
    return VK_SUCCESS;
}

VkResult Buffer_CreateBuffer(const GraphicsRenderer& renderState, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage) 
{
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VULKAN_RESULT(vkCreateBuffer(renderState.Device, &bufferCreateInfo, nullptr, buffer));
    VULKAN_RESULT(Buffer_AllocateMemory(renderState, buffer, bufferMemory, properties));
    VULKAN_RESULT(vkBindBufferMemory(renderState.Device, *buffer, *bufferMemory, 0));
    return Buffer_UpdateBufferMemory(renderState, *bufferMemory, bufferData, bufferSize);
}

VkResult Buffer_CreateStagingBuffer(const GraphicsRenderer& renderState, VkBuffer* stagingBuffer, VkBuffer* buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags properties) 
{
    if (!stagingBuffer || !buffer || !stagingBufferMemory || !bufferMemory) 
    {
        RENDERER_ERROR("One or more buffer pointers are nullptr");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VULKAN_RESULT(Buffer_CreateBuffer(renderState, stagingBuffer, stagingBufferMemory, bufferData, bufferSize, properties, bufferUsage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT));

    VkResult result = Buffer_CreateBuffer(renderState, buffer, bufferMemory, bufferData, bufferSize, properties, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    if (result != VK_SUCCESS) 
    {
        RENDERER_ERROR("Failed to create buffer");
        vkDestroyBuffer(renderState.Device, *stagingBuffer, nullptr);
        Renderer_FreeDeviceMemory(renderState.Device, stagingBufferMemory);
        return result;
    }

    return Buffer_CopyBuffer(renderState, stagingBuffer, buffer, bufferSize);
}

VkResult Buffer_CopyBuffer(const GraphicsRenderer& renderState, VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size) 
{
    if (!srcBuffer || !dstBuffer) {
        RENDERER_ERROR("Source or Destination Buffer is nullptr");
        return VK_ERROR_UNKNOWN;
    }

    VkBufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    VkCommandBuffer commandBuffer = Renderer_BeginSingleUseCommandBuffer(renderState.Device, renderState.CommandPool);
    vkCmdCopyBuffer(commandBuffer, *srcBuffer, *dstBuffer, 1, &copyRegion);
    return Renderer_EndSingleUseCommandBuffer(renderState.Device, renderState.CommandPool, renderState.GraphicsQueue, commandBuffer);
}

VkResult Buffer_UpdateBufferSize(const GraphicsRenderer& renderState, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize oldBufferSize, VkDeviceSize newBufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags propertyFlags) 
{
    if (newBufferSize < oldBufferSize) 
    {
        RENDERER_ERROR("buffer size can't be less than the old buffer size.");
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = newBufferSize,
        .usage = bufferUsageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VkBuffer newBuffer = VK_NULL_HANDLE;
    VkDeviceMemory newBufferMemory = VK_NULL_HANDLE;

    VkResult result = vkCreateBuffer(renderState.Device, &bufferCreateInfo, nullptr, &newBuffer);
    if (result != VK_SUCCESS) 
    {
        RENDERER_ERROR("Failed to create new buffer");
        return result;
    }

    result = Buffer_AllocateMemory(renderState, &newBuffer, &newBufferMemory, propertyFlags);
    if (result != VK_SUCCESS) 
    {
        RENDERER_ERROR("Failed to allocate memory for new buffer");
        vkDestroyBuffer(renderState.Device, newBuffer, nullptr);
        return result;
    }

    result = vkBindBufferMemory(renderState.Device, newBuffer, newBufferMemory, 0);
    if (result != VK_SUCCESS) 
    {
        RENDERER_ERROR("Failed to bind memory to the new buffer");
        Renderer_FreeDeviceMemory(renderState.Device, &newBufferMemory);
        vkDestroyBuffer(renderState.Device, newBuffer, nullptr);
        return result;
    }

    if (bufferData) 
    {
        result = Buffer_UpdateBufferMemory(renderState, newBufferMemory, bufferData, newBufferSize);
        if (result != VK_SUCCESS) {
            RENDERER_ERROR("Failed to update memory with buffer data");
            vkDestroyBuffer(renderState.Device, newBuffer, nullptr);
            Renderer_FreeDeviceMemory(renderState.Device, &newBufferMemory);
            return result;
        }
    }
    else if (*buffer != VK_NULL_HANDLE && oldBufferSize > 0) {
        result = Buffer_CopyBuffer(renderState, buffer, &newBuffer, oldBufferSize);
        if (result != VK_SUCCESS) {
            RENDERER_ERROR("Failed to copy old data to new buffer");
            vkDestroyBuffer(renderState.Device, newBuffer, nullptr);
            Renderer_FreeDeviceMemory(renderState.Device, &newBufferMemory);
            return result;
        }
    }

    if (*buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(renderState.Device, *buffer, nullptr);
    }
    if (*bufferMemory != VK_NULL_HANDLE)
    {
        Renderer_FreeDeviceMemory(renderState.Device, bufferMemory);
    }

    *buffer = newBuffer;
    *bufferMemory = newBufferMemory;

    return VK_SUCCESS;
}

void Buffer_UpdateBufferData(const GraphicsRenderer& renderState, VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize) 
{
    Buffer_UpdateBufferMemory(renderState, *bufferMemory, dataToCopy, bufferSize);
}

void Buffer_UpdateStagingBufferData(const GraphicsRenderer& renderState, VkBuffer stagingBuffer, VkBuffer buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize) 
{
    if (Buffer_UpdateBufferMemory(renderState, *stagingBufferMemory, dataToCopy, bufferSize) != VK_SUCCESS) 
    {
        RENDERER_ERROR("Failed to update staging buffer memory.");
        return;
    }
    Buffer_CopyBufferMemory(renderState, stagingBuffer, buffer, bufferSize);
}

VkResult Buffer_DestroyBuffer(const GraphicsRenderer& renderState, VkBuffer* buffer, VkBuffer* stagingBuffer, VkDeviceMemory* bufferMemory, VkDeviceMemory* stagingBufferMemory, void** bufferData, VkDeviceSize* bufferSize, VkBufferUsageFlags* bufferUsage, VkMemoryPropertyFlags* propertyFlags) 
{
    if (!buffer && !stagingBuffer) 
    {
        RENDERER_ERROR("Both buffer and stagingBuffer are nullptr");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (buffer && *buffer != VK_NULL_HANDLE) 
    {
        vkDestroyBuffer(renderState.Device, *buffer, nullptr);
        *buffer = VK_NULL_HANDLE;
    }

    if (stagingBuffer && *stagingBuffer != VK_NULL_HANDLE) 
    {
        vkDestroyBuffer(renderState.Device, *stagingBuffer, nullptr);
        *stagingBuffer = VK_NULL_HANDLE;
    }

    if (bufferMemory && *bufferMemory != VK_NULL_HANDLE) 
    {
        Renderer_FreeDeviceMemory(renderState.Device, bufferMemory);
        *bufferMemory = VK_NULL_HANDLE;
    }

    if (stagingBufferMemory && *stagingBufferMemory != VK_NULL_HANDLE) 
    {
        Renderer_FreeDeviceMemory(renderState.Device, stagingBufferMemory);
        *stagingBufferMemory = VK_NULL_HANDLE;
    }

    if (bufferData)
    {
        *bufferData = nullptr;
    }
    if (bufferSize)
    {
        *bufferSize = 0;
    }
    if (bufferUsage)
    {
        *bufferUsage = 0;
    }
    if (propertyFlags)
    {
        *propertyFlags = 0;
    }

    return VK_SUCCESS;
}