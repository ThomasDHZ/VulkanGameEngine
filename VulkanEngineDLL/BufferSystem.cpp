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

void VulkanBufferSystem::DestroyAllBuffers()
{
    for (auto& buffer : VulkanBufferMap)
    {
        VulkanBuffer_DestroyBuffer(buffer.second);
    }
}

VulkanBuffer VulkanBufferSystem::CreateVulkanBuffer(uint bufferId, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
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
        result = CreateStagingBuffer(&vulkanBuffer.StagingBuffer, &vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
    }
    else
    {
        result = CreateBuffer(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferProperties, vulkanBuffer.BufferUsage);
    }

    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to create Vulkan buffer");
    }

    memorySystem.RemovePtrBuffer(bufferData);
    return vulkanBuffer;
}

VulkanBuffer VulkanBufferSystem::CreateVulkanBuffer(uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
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
        result = CreateStagingBuffer(&vulkanBuffer.StagingBuffer, &vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
    }
    else
    {
        result = CreateBuffer(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferProperties, vulkanBuffer.BufferUsage);
    }

    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to create Vulkan buffer");
    }

    return vulkanBuffer;
}

void VulkanBufferSystem::UpdateBufferSize(VulkanBuffer& vulkanBuffer, VkDeviceSize newBufferElementSize, uint newBufferElementCount)
{
    VkDeviceSize newBufferSize = newBufferElementSize * newBufferElementCount;
    if (vulkanBuffer.UsingStagingBuffer)
    {
        VULKAN_RESULT(UpdateBufferSize(&vulkanBuffer.StagingBuffer, &vulkanBuffer.StagingBufferMemory, nullptr, vulkanBuffer.BufferSize, newBufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties));
        VULKAN_RESULT(UpdateBufferSize(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, nullptr, vulkanBuffer.BufferSize, newBufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties));
    }
    else
    {
        VULKAN_RESULT(UpdateBufferSize(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, nullptr, vulkanBuffer.BufferSize, newBufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties));
    }
}

void VulkanBufferSystem::UpdateBufferMemory(VulkanBuffer& vulkanBuffer, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount)
{
    VkDeviceSize newBufferSize = bufferElementSize * bufferElementCount;
    if (vulkanBuffer.UsingStagingBuffer)
    {
        if (vulkanBuffer.BufferSize != newBufferSize) {
            UpdateBufferSize(vulkanBuffer, bufferElementSize, bufferElementCount);
        }
        UpdateStagingBufferData(vulkanBuffer.StagingBuffer, vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, newBufferSize);
    }
    else
    {
        if (vulkanBuffer.BufferSize != newBufferSize)
        {
            UpdateBufferSize(vulkanBuffer, bufferElementSize, bufferElementCount);
        }
        if (UpdateBufferMemory(vulkanBuffer.BufferMemory, bufferData, newBufferSize) != VK_SUCCESS)
        {
            RENDERER_ERROR("Failed to update buffer memory.");
        }
    }
}

VkResult VulkanBufferSystem::UpdateBufferMemory(VkDeviceMemory bufferMemory, void* dataToCopy, VkDeviceSize bufferSize)
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

void VulkanBufferSystem::CopyBufferMemory(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
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

VkResult VulkanBufferSystem::AllocateMemory(VkBuffer* bufferData, VkDeviceMemory* bufferMemory, VkMemoryPropertyFlags properties)
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

void* VulkanBufferSystem::MapBufferMemory(VkDeviceMemory bufferMemory, VkDeviceSize bufferSize, bool* isMapped)
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

VkResult VulkanBufferSystem::UnmapBufferMemory(VkDeviceMemory bufferMemory, bool* isMapped)
{
    if (*isMapped)
    {
        vkUnmapMemory(renderer.Device, bufferMemory);
        *isMapped = false;
    }
    return VK_SUCCESS;
}

VkResult VulkanBufferSystem::CreateBuffer(VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VULKAN_RESULT(vkCreateBuffer(renderer.Device, &bufferCreateInfo, nullptr, buffer));
    VULKAN_RESULT(AllocateMemory(buffer, bufferMemory, properties));
    VULKAN_RESULT(vkBindBufferMemory(renderer.Device, *buffer, *bufferMemory, 0));
    return UpdateBufferMemory(*bufferMemory, bufferData, bufferSize);
}

VkResult VulkanBufferSystem::CreateStagingBuffer(VkBuffer* stagingBuffer, VkBuffer* buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags properties)
{
    if (!stagingBuffer || !buffer || !stagingBufferMemory || !bufferMemory)
    {
        RENDERER_ERROR("One or more buffer pointers are nullptr");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VULKAN_RESULT(CreateBuffer(stagingBuffer, stagingBufferMemory, bufferData, bufferSize, properties, bufferUsage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT));

    VkResult result = CreateBuffer(buffer, bufferMemory, bufferData, bufferSize, properties, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    if (result != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to create buffer");
        vkDestroyBuffer(renderer.Device, *stagingBuffer, nullptr);
        Renderer_FreeDeviceMemory(renderer.Device, stagingBufferMemory);
        return result;
    }

    return CopyBuffer(stagingBuffer, buffer, bufferSize);
}

VkResult VulkanBufferSystem::CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size)
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

VkResult VulkanBufferSystem::UpdateBufferSize(VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize oldBufferSize, VkDeviceSize newBufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags propertyFlags)
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

    result = AllocateMemory(&newBuffer, &newBufferMemory, propertyFlags);
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
        result = UpdateBufferMemory(newBufferMemory, bufferData, newBufferSize);
        if (result != VK_SUCCESS) {
            RENDERER_ERROR("Failed to update memory with buffer data");
            vkDestroyBuffer(renderer.Device, newBuffer, nullptr);
            Renderer_FreeDeviceMemory(renderer.Device, &newBufferMemory);
            return result;
        }
    }
    else if (*buffer != VK_NULL_HANDLE && oldBufferSize > 0) {
        result = CopyBuffer(buffer, &newBuffer, oldBufferSize);
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

void VulkanBufferSystem::UpdateBufferData(VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize)
{
    UpdateBufferMemory(*bufferMemory, dataToCopy, bufferSize);
}

void VulkanBufferSystem::UpdateStagingBufferData(VkBuffer stagingBuffer, VkBuffer buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize)
{
    if (UpdateBufferMemory(*stagingBufferMemory, dataToCopy, bufferSize) != VK_SUCCESS)
    {
        RENDERER_ERROR("Failed to update staging buffer memory.");
        return;
    }
    CopyBufferMemory(stagingBuffer, buffer, bufferSize);
}

VkResult VulkanBufferSystem::DestroyBuffer(VulkanBuffer& vulkanBuffer)
{
    if (!vulkanBuffer.Buffer && 
        !vulkanBuffer.StagingBuffer)
    {
        RENDERER_ERROR("Both buffer and stagingBuffer are nullptr");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (vulkanBuffer.Buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(renderer.Device, vulkanBuffer.Buffer, nullptr);
        vulkanBuffer.Buffer = VK_NULL_HANDLE;
    }

    if (vulkanBuffer.StagingBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(renderer.Device, vulkanBuffer.StagingBuffer, nullptr);
        vulkanBuffer.StagingBuffer = VK_NULL_HANDLE;
    }

    if (vulkanBuffer.BufferMemory != VK_NULL_HANDLE)
    {
        Renderer_FreeDeviceMemory(renderer.Device, &vulkanBuffer.BufferMemory);
        vulkanBuffer.BufferMemory = VK_NULL_HANDLE;
    }

    if (vulkanBuffer.StagingBufferMemory != VK_NULL_HANDLE)
    {
        Renderer_FreeDeviceMemory(renderer.Device, &vulkanBuffer.StagingBufferMemory);
        vulkanBuffer.StagingBufferMemory = VK_NULL_HANDLE;
    }

    if (vulkanBuffer.BufferData)
    {
        vulkanBuffer.BufferData = nullptr;
    }
    if (vulkanBuffer.BufferSize)
    {
        vulkanBuffer.BufferSize = 0;
    }
    if (vulkanBuffer.BufferUsage)
    {
        vulkanBuffer.BufferUsage = 0;
    }
    if (vulkanBuffer.BufferProperties)
    {
        vulkanBuffer.BufferProperties = 0;
    }

    return VK_SUCCESS;
}

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

VkResult VulkanBuffer_CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size)
{
    return bufferSystem.CopyBuffer(srcBuffer, dstBuffer, size);
}

VkResult VulkanBuffer_DestroyBuffer(VulkanBuffer& vulkanBuffer)
{
    return bufferSystem.DestroyBuffer(vulkanBuffer);
}
