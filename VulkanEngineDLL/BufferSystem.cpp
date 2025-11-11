#include "BufferSystem.h"
#include "MemorySystem.h"

VulkanBufferSystem bufferSystem = VulkanBufferSystem();
int NextBufferId = 0;


VulkanBuffer& VulkanBufferSystem::FindVulkanBuffer(int id)
{
    return VulkanBufferMap.at(id);
}

const Vector<VulkanBuffer>& VulkanBufferSystem::VulkanBufferList()
{
    Vector<VulkanBuffer> vulkanBufferList;
    for (const auto& buffer : VulkanBufferMap)
    {
        vulkanBufferList.emplace_back(buffer.second);
    }
    return vulkanBufferList;
}

void VulkanBufferSystem::DestroyBuffer(int vulkanBufferId)
{
    VulkanBuffer_DestroyBuffer(VulkanBufferMap[vulkanBufferId]);
}

void VulkanBufferSystem::DestroyAllBuffers()
{
    for (auto& buffer : VulkanBufferMap)
    {
        VulkanBuffer_DestroyBuffer(buffer.second);
    }
}

VulkanBuffer VulkanBufferSystem::VulkanBuffer_CreateVulkanBuffer(uint bufferId, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
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
        result = Buffer_CreateStagingBuffer(&vulkanBuffer.StagingBuffer, &vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
    }
    else
    {
        result = Buffer_CreateBuffer(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferProperties, vulkanBuffer.BufferUsage);
    }

    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to create Vulkan buffer");
    }

    memorySystem.RemovePtrBuffer(bufferData);
    return vulkanBuffer;
}

VulkanBuffer VulkanBufferSystem::VulkanBuffer_CreateVulkanBuffer2(uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
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
        result = Buffer_CreateStagingBuffer(&vulkanBuffer.StagingBuffer, &vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
    }
    else
    {
        result = Buffer_CreateBuffer(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferProperties, vulkanBuffer.BufferUsage);
    }

    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to create Vulkan buffer");
    }

    return vulkanBuffer;
}

VulkanBuffer VulkanBufferSystem::VulkanBuffer_CreateVulkanBuffer3(VulkanBuffer& vulkanBuffer, uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
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
        result = Buffer_CreateStagingBuffer(&vulkanBuffer.StagingBuffer, &vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
    }
    else
    {
        result = Buffer_CreateBuffer(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferProperties, vulkanBuffer.BufferUsage);
    }

    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to create Vulkan buffer");
    }

    return vulkanBuffer;
}

void VulkanBufferSystem::VulkanBuffer_UpdateBufferSize(VulkanBuffer& vulkanBuffer, VkDeviceSize newBufferElementSize, uint newBufferElementCount)
{
    VkDeviceSize newBufferSize = newBufferElementSize * newBufferElementCount;
    if (vulkanBuffer.UsingStagingBuffer)
    {
        VULKAN_RESULT(Buffer_UpdateBufferSize(&vulkanBuffer.StagingBuffer, &vulkanBuffer.StagingBufferMemory, nullptr, vulkanBuffer.BufferSize, newBufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties));
        VULKAN_RESULT(Buffer_UpdateBufferSize(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, nullptr, vulkanBuffer.BufferSize, newBufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties));
    }
    else
    {
        VULKAN_RESULT(Buffer_UpdateBufferSize(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, nullptr, vulkanBuffer.BufferSize, newBufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties));
    }
}

void VulkanBufferSystem::VulkanBuffer_UpdateBufferMemory(VulkanBuffer& vulkanBuffer, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount)
{
    VkDeviceSize newBufferSize = bufferElementSize * bufferElementCount;
    if (vulkanBuffer.UsingStagingBuffer)
    {
        if (vulkanBuffer.BufferSize != newBufferSize) {
            VulkanBuffer_UpdateBufferSize(vulkanBuffer, bufferElementSize, bufferElementCount);
        }
        Buffer_UpdateStagingBufferData(vulkanBuffer.StagingBuffer, vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, newBufferSize);
    }
    else
    {
        if (vulkanBuffer.BufferSize != newBufferSize)
        {
            VulkanBuffer_UpdateBufferSize(vulkanBuffer, bufferElementSize, bufferElementCount);
        }
        if (Buffer_UpdateBufferMemory(vulkanBuffer.BufferMemory, bufferData, newBufferSize) != VK_SUCCESS)
        {
            RENDERER_ERROR("Failed to update buffer memory.");
        }
    }
}

VkResult VulkanBufferSystem::VulkanBuffer_CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size)
{
    return Buffer_CopyBuffer(srcBuffer, dstBuffer, size);
}


void VulkanBufferSystem::VulkanBuffer_DestroyBuffer(VulkanBuffer& vulkanBuffer) {
    VkResult result = Buffer_DestroyBuffer(&vulkanBuffer.Buffer, &vulkanBuffer.StagingBuffer, &vulkanBuffer.BufferMemory, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferData, &vulkanBuffer.BufferSize, &vulkanBuffer.BufferUsage, &vulkanBuffer.BufferProperties);
    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to destroy buffer");
    }
}

VkResult VulkanBufferSystem::Buffer_UpdateBufferMemory(VkDeviceMemory bufferMemory, void* dataToCopy, VkDeviceSize bufferSize)
{
    if (dataToCopy == nullptr || bufferSize == 0)
    {
        RENDERER_ERROR("Buffer data and size cannot be nullptr");
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    void* mappedData = nullptr;
    VkResult result = vkMapMemory(renderer.Device, bufferMemory, 0, bufferSize, 0, &mappedData);
    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to map buffer memory");
        return result;
    }

    memcpy(mappedData, dataToCopy, static_cast<size_t>(bufferSize));
    vkUnmapMemory(renderer.Device, bufferMemory);
    return VK_SUCCESS;
}

void VulkanBufferSystem::Buffer_CopyBufferMemory(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
{
    VkBufferCopy copyRegion =
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = bufferSize
    };

    VkCommandBuffer commandBuffer = Renderer_BeginSingleUseCommandBuffer(renderer.Device, renderer.CommandPool);
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    Renderer_EndSingleUseCommandBuffer(renderer.Device, renderer.CommandPool, renderer.GraphicsQueue, commandBuffer);
}

VkResult VulkanBufferSystem::Buffer_AllocateMemory(VkBuffer* bufferData, VkDeviceMemory* bufferMemory, VkMemoryPropertyFlags properties)
{
    if (bufferData == nullptr || bufferMemory == nullptr)
    {
        RENDERER_ERROR("Buffer data or buffer memory is nullptr");
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(renderer.Device, *bufferData, &memRequirements);

    VkMemoryAllocateFlagsInfo extendedAllocFlagsInfo =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO
    };

    VkMemoryAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &extendedAllocFlagsInfo,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = Renderer_GetMemoryType(renderer.PhysicalDevice, memRequirements.memoryTypeBits, properties),
    };
    return vkAllocateMemory(renderer.Device, &allocInfo, nullptr, bufferMemory);
}

void* VulkanBufferSystem::Buffer_MapBufferMemory(VkDeviceMemory bufferMemory, VkDeviceSize bufferSize, bool* isMapped)
{
    if (*isMapped)
    {
        RENDERER_ERROR("Buffer already mapped!");
        return nullptr;
    }

    void* mappedData = nullptr;
    VkResult result = vkMapMemory(renderer.Device, bufferMemory, 0, bufferSize, 0, &mappedData);
    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to map buffer memory");
        return nullptr;
    }
    *isMapped = true;
    return mappedData;
}

VkResult VulkanBufferSystem::Buffer_UnmapBufferMemory(VkDeviceMemory bufferMemory, bool* isMapped)
{
    if (*isMapped)
    {
        vkUnmapMemory(renderer.Device, bufferMemory);
        *isMapped = false;
    }
    return VK_SUCCESS;
}

VkResult VulkanBufferSystem::Buffer_CreateBuffer(VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VULKAN_RESULT(vkCreateBuffer(renderer.Device, &bufferCreateInfo, nullptr, buffer));
    VULKAN_RESULT(Buffer_AllocateMemory(buffer, bufferMemory, properties));
    VULKAN_RESULT(vkBindBufferMemory(renderer.Device, *buffer, *bufferMemory, 0));
    return Buffer_UpdateBufferMemory(*bufferMemory, bufferData, bufferSize);
}

VkResult VulkanBufferSystem::Buffer_CreateStagingBuffer(VkBuffer* stagingBuffer, VkBuffer* buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags properties)
{
    if (!stagingBuffer || !buffer || !stagingBufferMemory || !bufferMemory)
    {
        RENDERER_ERROR("One or more buffer pointers are nullptr");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VULKAN_RESULT(Buffer_CreateBuffer(stagingBuffer, stagingBufferMemory, bufferData, bufferSize, properties, bufferUsage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT));

    VkResult result = Buffer_CreateBuffer(buffer, bufferMemory, bufferData, bufferSize, properties, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to create buffer");
        vkDestroyBuffer(renderer.Device, *stagingBuffer, nullptr);
        Renderer_FreeDeviceMemory(renderer.Device, stagingBufferMemory);
        return result;
    }

    return Buffer_CopyBuffer(stagingBuffer, buffer, bufferSize);
}

VkResult VulkanBufferSystem::Buffer_CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size)
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

    VkCommandBuffer commandBuffer = Renderer_BeginSingleUseCommandBuffer(renderer.Device, renderer.CommandPool);
    vkCmdCopyBuffer(commandBuffer, *srcBuffer, *dstBuffer, 1, &copyRegion);
    return Renderer_EndSingleUseCommandBuffer(renderer.Device, renderer.CommandPool, renderer.GraphicsQueue, commandBuffer);
}

VkResult VulkanBufferSystem::Buffer_UpdateBufferSize(VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize oldBufferSize, VkDeviceSize newBufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags propertyFlags)
{
    if (newBufferSize < oldBufferSize)
    {
        RENDERER_ERROR("Buffer size can't be less than the old buffer size. OldSize: " + std::to_string(static_cast<uint>(oldBufferSize)) + " NewSize: " + std::to_string(static_cast<uint>(newBufferSize)));
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

    VkResult result = vkCreateBuffer(renderer.Device, &bufferCreateInfo, nullptr, &newBuffer);
    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to create new buffer");
        return result;
    }

    result = Buffer_AllocateMemory(&newBuffer, &newBufferMemory, propertyFlags);
    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to allocate memory for new buffer");
        vkDestroyBuffer(renderer.Device, newBuffer, nullptr);
        return result;
    }

    result = vkBindBufferMemory(renderer.Device, newBuffer, newBufferMemory, 0);
    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to bind memory to the new buffer");
        Renderer_FreeDeviceMemory(renderer.Device, &newBufferMemory);
        vkDestroyBuffer(renderer.Device, newBuffer, nullptr);
        return result;
    }

    if (bufferData)
    {
        result = Buffer_UpdateBufferMemory(newBufferMemory, bufferData, newBufferSize);
        if (result != VK_SUCCESS) {
            RENDERER_ERROR("Failed to update memory with buffer data");
            vkDestroyBuffer(renderer.Device, newBuffer, nullptr);
            Renderer_FreeDeviceMemory(renderer.Device, &newBufferMemory);
            return result;
        }
    }
    else if (*buffer != VK_NULL_HANDLE && oldBufferSize > 0) {
        result = Buffer_CopyBuffer(buffer, &newBuffer, oldBufferSize);
        if (result != VK_SUCCESS) {
            RENDERER_ERROR("Failed to copy old data to new buffer");
            vkDestroyBuffer(renderer.Device, newBuffer, nullptr);
            Renderer_FreeDeviceMemory(renderer.Device, &newBufferMemory);
            return result;
        }
    }

    if (*buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(renderer.Device, *buffer, nullptr);
    }
    if (*bufferMemory != VK_NULL_HANDLE)
    {
        Renderer_FreeDeviceMemory(renderer.Device, bufferMemory);
    }

    *buffer = newBuffer;
    *bufferMemory = newBufferMemory;

    return VK_SUCCESS;
}

void VulkanBufferSystem::Buffer_UpdateBufferData(VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize)
{
    Buffer_UpdateBufferMemory(*bufferMemory, dataToCopy, bufferSize);
}

void VulkanBufferSystem::Buffer_UpdateStagingBufferData(VkBuffer stagingBuffer, VkBuffer buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize)
{
    if (Buffer_UpdateBufferMemory(*stagingBufferMemory, dataToCopy, bufferSize) != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to update staging buffer memory.");
        return;
    }
    Buffer_CopyBufferMemory(stagingBuffer, buffer, bufferSize);
}

VkResult VulkanBufferSystem::Buffer_DestroyBuffer(VkBuffer* buffer, VkBuffer* stagingBuffer, VkDeviceMemory* bufferMemory, VkDeviceMemory* stagingBufferMemory, void** bufferData, VkDeviceSize* bufferSize, VkBufferUsageFlags* bufferUsage, VkMemoryPropertyFlags* propertyFlags)
{
    if (!buffer && !stagingBuffer)
    {
        RENDERER_ERROR("Both buffer and stagingBuffer are nullptr");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (buffer && *buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(renderer.Device, *buffer, nullptr);
        *buffer = VK_NULL_HANDLE;
    }

    if (stagingBuffer && *stagingBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(renderer.Device, *stagingBuffer, nullptr);
        *stagingBuffer = VK_NULL_HANDLE;
    }

    if (bufferMemory && *bufferMemory != VK_NULL_HANDLE)
    {
        Renderer_FreeDeviceMemory(renderer.Device, bufferMemory);
        *bufferMemory = VK_NULL_HANDLE;
    }

    if (stagingBufferMemory && *stagingBufferMemory != VK_NULL_HANDLE)
    {
        Renderer_FreeDeviceMemory(renderer.Device, stagingBufferMemory);
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

VulkanBuffer VulkanBuffer_CreateVulkanBuffer(uint bufferId, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
{
    return VulkanBuffer_CreateVulkanBuffer(bufferId, bufferElementSize, bufferElementCount, bufferTypeEnum, usage, properties, usingStagingBuffer);
}

VulkanBuffer VulkanBuffer_CreateVulkanBuffer2(uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
{
    return VulkanBuffer_CreateVulkanBuffer2(bufferId, bufferData, bufferElementSize, bufferElementCount, bufferTypeEnum, usage, properties, usingStagingBuffer);
}

VulkanBuffer VulkanBuffer_CreateVulkanBuffer3(VulkanBuffer& vulkanBuffer, uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
{
    return VulkanBuffer_CreateVulkanBuffer3(vulkanBuffer, bufferId, bufferData, bufferElementSize, bufferElementCount, bufferTypeEnum, usage, properties, usingStagingBuffer);
}

void VulkanBuffer_UpdateBufferMemory(VulkanBuffer& vulkanBuffer, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount)
{
    VulkanBuffer_UpdateBufferMemory(vulkanBuffer, bufferData, bufferElementSize, bufferElementCount);
}

VkResult VulkanBuffer_CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size)
{
    return VulkanBuffer_CopyBuffer(srcBuffer, dstBuffer, size);
}

void VulkanBuffer_DestroyBuffer(VulkanBuffer& vulkanBuffer)
{
    return VulkanBuffer_DestroyBuffer(vulkanBuffer);
}
