#define BUFFER_SYSTEM_IMPLEMENTATION
#include "BufferSystem.h"
#include "MemorySystem.h"
#include <vk_mem_alloc.h>

VulkanBufferSystem& bufferSystem = VulkanBufferSystem::Get();
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
        DestroyBuffer(buffer.second);
    }
}

uint32 VulkanBufferSystem::VMACreateStaticVulkanBuffer(const void* srcData, VkDeviceSize size, VkBufferUsageFlags shaderUsageFlags, VkDeviceSize offset)
{
    assert(offset < size);

    uint32 bufferId = ++NextBufferId;
    VkBufferCreateInfo bufferInfo =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | shaderUsageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocInfo =
    {
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    };

    VkBuffer dstBuffer = VK_NULL_HANDLE;
    VmaAllocation dstAllocation = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vmaAllocator, &bufferInfo, &allocInfo, &dstBuffer, &dstAllocation, nullptr));

    if (srcData == nullptr)
    {
        VulkanBufferMap[bufferId] = 
        {
            .BufferId = bufferId,
            .Buffer = dstBuffer,
            .BufferSize = size,
            .Allocation = dstAllocation,
            .BufferData = nullptr,
            .UsingStagingBuffer = false,
            .IsPersistentlyMapped = false
        };
        return bufferId;
    }

    VkBufferCreateInfo stagingBufferInfo =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo stagingAllocInfo =
    {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo stagingAllocOut = {};
    VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vmaAllocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, &stagingAllocOut));

    bool needUnmap = false;
    void* mappedData = stagingAllocOut.pMappedData;
    if (mappedData == nullptr)
    {
        VULKAN_THROW_IF_FAIL(vmaMapMemory(vmaAllocator, stagingAllocation, &mappedData));
        needUnmap = true;
    }

    memcpy((byte*)mappedData + offset, srcData, size - offset);
    VULKAN_THROW_IF_FAIL(vmaFlushAllocation(vmaAllocator, stagingAllocation, offset, size - offset));
    if (needUnmap)
    {
        vmaUnmapMemory(vmaAllocator, stagingAllocation);
    }

    CopyBuffer(&stagingBuffer, &dstBuffer, size - offset, offset);
    vmaDestroyBuffer(vmaAllocator, stagingBuffer, stagingAllocation);

    VulkanBufferMap[bufferId] = 
    {
        .BufferId = bufferId,
        .Buffer = dstBuffer,
        .BufferSize = size,
        .Allocation = dstAllocation,
        .BufferData = nullptr,
        .UsingStagingBuffer = true,
        .IsPersistentlyMapped = false
    };

    return bufferId;
}

uint32 VulkanBufferSystem::VMACreateDynamicBuffer(const void* srcData, VkDeviceSize size, VkBufferUsageFlags usageFlags)
{
    uint32 bufferId = ++NextBufferId;
    VkBufferCreateInfo bufferInfo =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocInfo =
    {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo allocOut = {};
    VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vmaAllocator, &bufferInfo, &allocInfo, &buffer, &allocation, &allocOut));

    void* mappedData = allocOut.pMappedData;
    bool needFallback = (mappedData == nullptr);
    if (needFallback)
    {
        VULKAN_THROW_IF_FAIL(vmaMapMemory(vmaAllocator, allocation, &mappedData));
    }

    if (srcData)
    {
        memcpy(mappedData, srcData, size);
        vmaFlushAllocation(vmaAllocator, allocation, 0, size);
    }

    if (needFallback && needFallback)
    {
        vmaUnmapMemory(vmaAllocator, allocation);
    }

    VulkanBufferMap[bufferId] = 
    {
        .BufferId = bufferId,
        .Buffer = buffer,
        .BufferSize = size,
        .Allocation = allocation,
        .BufferData = mappedData,
        .UsingStagingBuffer = false,
        .IsPersistentlyMapped = !needFallback
    };

    return bufferId;
}

void VulkanBufferSystem::VMAUpdateDynamicBuffer(uint32 bufferId, const void* data, VkDeviceSize size, VkDeviceSize offset)
{
    VulkanBuffer& buffer = VulkanBufferMap[bufferId];
    if (buffer.IsPersistentlyMapped)
    {
        memcpy((uint8_t*)buffer.BufferData + offset, data, size);
        vmaFlushAllocation(vmaAllocator, buffer.Allocation, offset, size);
    }
    else
    {
        void* mapped = nullptr;
        vmaMapMemory(vmaAllocator, buffer.Allocation, &mapped);
        memcpy((uint8_t*)mapped + offset, data, size);
        vmaFlushAllocation(vmaAllocator, buffer.Allocation, offset, size);
        vmaUnmapMemory(vmaAllocator, buffer.Allocation);
    }
}

uint VulkanBufferSystem::CreateVulkanBuffer(VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
{
    uint bufferIndex = ++NextBufferId;
    VkDeviceSize bufferSize = bufferElementSize * bufferElementCount;
    VulkanBuffer vulkanBuffer = {
        .BufferId = bufferIndex,
        .BufferSize = bufferSize,
        .BufferUsage = usage,
        .BufferProperties = properties,
        .BufferType = bufferTypeEnum,
        .UsingStagingBuffer = usingStagingBuffer,
    };

    void* bufferData = memorySystem.AddPtrBuffer<void*>(bufferSize, __FILE__, __LINE__, __func__);
    memset(bufferData, 0, bufferSize);

    if (vulkanBuffer.UsingStagingBuffer) 
    {
        CreateStagingBuffer(&vulkanBuffer.StagingBuffer, &vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
    }
    else 
    {
       CreateBuffer(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferProperties, vulkanBuffer.BufferUsage);
    }

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) 
    {
        VkBufferDeviceAddressInfo addrInfo = {};
        addrInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        addrInfo.buffer = vulkanBuffer.Buffer;
#if defined(__ANDROID__)
        vulkanBuffer.BufferDeviceAddress = vulkanSystem.vkGetBufferDeviceAddress(vulkanSystem.Device, &addrInfo);
#else
		vulkanBuffer.BufferDeviceAddress = vkGetBufferDeviceAddress(vulkanSystem.Device, &addrInfo);
#endif
    }
    VulkanBufferMap[bufferIndex] = vulkanBuffer;
    memorySystem.DeletePtr(bufferData);
    return bufferIndex;
}

uint VulkanBufferSystem::CreateVulkanBuffer(void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
{
    uint bufferIndex = ++NextBufferId;
    VkDeviceSize bufferSize = bufferElementSize * bufferElementCount;
    VulkanBuffer vulkanBuffer =
    {
        .BufferId = bufferIndex,
        .BufferSize = bufferSize,
        .BufferUsage = usage,
        .BufferProperties = properties,
        .BufferType = bufferTypeEnum,
        .UsingStagingBuffer = usingStagingBuffer,
    };

    if (vulkanBuffer.UsingStagingBuffer)
    {
        CreateStagingBuffer(&vulkanBuffer.StagingBuffer, &vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
    }
    else
    {
        CreateBuffer(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferProperties, vulkanBuffer.BufferUsage);
    }
    VulkanBufferMap[bufferIndex] = vulkanBuffer;
    return bufferIndex;
}

void VulkanBufferSystem::UpdateBufferSize(VulkanBuffer& vulkanBuffer, VkDeviceSize newBufferElementSize, uint newBufferElementCount)
{
    VkDeviceSize newBufferSize = newBufferElementSize * newBufferElementCount;
    if (vulkanBuffer.UsingStagingBuffer)
    {
        UpdateBufferSize(&vulkanBuffer.StagingBuffer, &vulkanBuffer.StagingBufferMemory, nullptr, vulkanBuffer.BufferSize, newBufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
        UpdateBufferSize(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, nullptr, vulkanBuffer.BufferSize, newBufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
    }
    else
    {
        UpdateBufferSize(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, nullptr, vulkanBuffer.BufferSize, newBufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
    }
}

void VulkanBufferSystem::UpdateBufferMemory(VulkanBuffer& vulkanBuffer, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount)
{
    VkDeviceSize newBufferSize = bufferElementSize * bufferElementCount;
    if (vulkanBuffer.UsingStagingBuffer)
    {
        if (vulkanBuffer.BufferSize != newBufferSize) 
        {
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
        UpdateBufferMemory(vulkanBuffer.BufferMemory, bufferData, newBufferSize);
    }
}

void VulkanBufferSystem::UpdateBufferMemory(VkDeviceMemory bufferMemory, void* dataToCopy, VkDeviceSize bufferSize)
{
    if (dataToCopy == nullptr || bufferSize == 0)
    {
        VULKAN_THROW_IF_FAIL(VK_ERROR_MEMORY_MAP_FAILED);
    }

    void* mappedData = nullptr;
    VULKAN_THROW_IF_FAIL(vkMapMemory(vulkanSystem.Device, bufferMemory, 0, bufferSize, 0, &mappedData));
    memcpy(mappedData, dataToCopy, static_cast<size_t>(bufferSize));
    vkUnmapMemory(vulkanSystem.Device, bufferMemory);
}

void VulkanBufferSystem::CopyBufferMemory(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
{
    VkBufferCopy copyRegion =
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = bufferSize
    };

    VkCommandBuffer commandBuffer = vulkanSystem.BeginSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool);
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vulkanSystem.EndSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool, vulkanSystem.GraphicsQueue, commandBuffer);
}

void VulkanBufferSystem::AllocateMemory(VkBuffer* bufferData, VkDeviceMemory* bufferMemory, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage)
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vulkanSystem.Device, *bufferData, &memRequirements);

    VkMemoryAllocateFlagsInfo allocateFlagsInfo = {};
    allocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    allocateFlagsInfo.flags = 0;
    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        allocateFlagsInfo.flags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    }

    VkMemoryAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &allocateFlagsInfo,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = vulkanSystem.GetMemoryType(vulkanSystem.PhysicalDevice, memRequirements.memoryTypeBits, properties)
    };

    VULKAN_THROW_IF_FAIL(vkAllocateMemory(vulkanSystem.Device, &allocInfo, nullptr, bufferMemory));
}

void* VulkanBufferSystem::MapBufferMemory(VkDeviceMemory bufferMemory, VkDeviceSize bufferSize, bool* isMapped)
{
    if (*isMapped)
    {
        return nullptr;
    }

    void* mappedData = nullptr;
    VULKAN_THROW_IF_FAIL(vkMapMemory(vulkanSystem.Device, bufferMemory, 0, bufferSize, 0, &mappedData));
    *isMapped = true;
    return mappedData;
}

void VulkanBufferSystem::UnmapBufferMemory(VkDeviceMemory bufferMemory, bool* isMapped)
{
    if (*isMapped)
    {
        vkUnmapMemory(vulkanSystem.Device, bufferMemory);
        *isMapped = false;
    }
}

void VulkanBufferSystem::CreateBuffer(VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage)
{
    VkBufferCreateInfo bufferInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VkMemoryAllocateFlagsInfo flagsInfo = {};
    flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) 
    {
        flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        flagsInfo.pNext = nullptr;
    }

    VkMemoryAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) ? &flagsInfo : nullptr,
        .allocationSize = 0,
        .memoryTypeIndex = 0
    };
    VkMemoryRequirements memReqs;

    VULKAN_THROW_IF_FAIL(vkCreateBuffer(vulkanSystem.Device, &bufferInfo, nullptr, buffer));
    vkGetBufferMemoryRequirements(vulkanSystem.Device, *buffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = vulkanSystem.GetMemoryType(vulkanSystem.PhysicalDevice, memReqs.memoryTypeBits, properties);

    VULKAN_THROW_IF_FAIL(vkAllocateMemory(vulkanSystem.Device, &allocInfo, nullptr, bufferMemory));
    VULKAN_THROW_IF_FAIL(vkBindBufferMemory(vulkanSystem.Device, *buffer, *bufferMemory, 0));
    if (bufferData) 
    {
        UpdateBufferMemory(*bufferMemory, bufferData, bufferSize);
    }
}

void VulkanBufferSystem::CreateStagingBuffer(VkBuffer* stagingBuffer, VkBuffer* buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags  bufferUsage, VkMemoryPropertyFlags stagingProperties)
{
    if (!stagingBuffer || !buffer || !stagingBufferMemory || !bufferMemory)
    {
        VULKAN_THROW_IF_FAIL(VK_ERROR_INITIALIZATION_FAILED);
    }

    CreateBuffer(stagingBuffer, stagingBufferMemory, bufferData, bufferSize, stagingProperties, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    CreateBuffer(buffer, bufferMemory, nullptr, bufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    CopyBufferMemory(*stagingBuffer, *buffer, bufferSize);
}

void VulkanBufferSystem::CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size, VkDeviceSize offset)
{
    VkCommandBuffer cmd = vulkanSystem.BeginSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool);
    VkBufferCopy copyRegion =
    {
        .srcOffset = offset,
        .dstOffset = offset,
        .size = size
    };

    vkCmdCopyBuffer(cmd, *srcBuffer, *dstBuffer, 1, &copyRegion);
    VkBufferMemoryBarrier barrier =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = *dstBuffer,
        .offset = offset,
        .size = size
    };

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);
    vulkanSystem.EndSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool, vulkanSystem.GraphicsQueue, cmd);
}

void VulkanBufferSystem::UpdateBufferSize(VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize oldBufferSize, VkDeviceSize newBufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags propertyFlags)
{
    if (newBufferSize < oldBufferSize)
    {
      /*  vulkanSystem.ERROR("%s", (std::string("Buffer size can't be less than the old buffer size. OldSize: ")
            + std::to_string(static_cast<uint32_t>(oldBufferSize))
            + " NewSize: "
            + std::to_string(static_cast<uint32_t>(newBufferSize))).c_str());*/
        VULKAN_THROW_IF_FAIL(VK_ERROR_MEMORY_MAP_FAILED);
    }

    VkBufferCreateInfo bufferCreateInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = newBufferSize,
        .usage = bufferUsageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VkBuffer newBuffer = VK_NULL_HANDLE;
    VkDeviceMemory newBufferMemory = VK_NULL_HANDLE;

    VULKAN_THROW_IF_FAIL(vkCreateBuffer(vulkanSystem.Device, &bufferCreateInfo, nullptr, &newBuffer));
    AllocateMemory(&newBuffer, &newBufferMemory, propertyFlags, bufferUsageFlags);

    VkResult result = vkBindBufferMemory(vulkanSystem.Device, newBuffer, newBufferMemory, 0);
    if (result != VK_SUCCESS)
    {
        vulkanSystem.FreeDeviceMemory(vulkanSystem.Device, &newBufferMemory);
        vkDestroyBuffer(vulkanSystem.Device, newBuffer, nullptr);
        VULKAN_THROW_IF_FAIL(result);
    }

    if (bufferData)
    {
        UpdateBufferMemory(newBufferMemory, bufferData, newBufferSize);
    }
    else if (*buffer != VK_NULL_HANDLE && oldBufferSize > 0) 
    {
        CopyBuffer(buffer, &newBuffer, oldBufferSize);
    }

    if (*buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(vulkanSystem.Device, *buffer, nullptr);
    }
    if (*bufferMemory != VK_NULL_HANDLE)
    {
        vulkanSystem.FreeDeviceMemory(vulkanSystem.Device, bufferMemory);
    }

    *buffer = newBuffer;
    *bufferMemory = newBufferMemory;
}

void VulkanBufferSystem::UpdateBufferData(VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize)
{
    UpdateBufferMemory(*bufferMemory, dataToCopy, bufferSize);
}

void VulkanBufferSystem::UpdateStagingBufferData(VkBuffer stagingBuffer, VkBuffer buffer, VkDeviceMemory* stagingBufferMemory, VkDeviceMemory* bufferMemory, void* dataToCopy, VkDeviceSize bufferSize)
{
    UpdateBufferMemory(*stagingBufferMemory, dataToCopy, bufferSize);
    CopyBufferMemory(stagingBuffer, buffer, bufferSize);
}

void VulkanBufferSystem::DestroyBuffer(VulkanBuffer& vulkanBuffer)
{
    if (!vulkanBuffer.Buffer &&
        !vulkanBuffer.StagingBuffer)
    {
        VULKAN_THROW_IF_FAIL(VK_ERROR_INITIALIZATION_FAILED);
    }

    if (vulkanBuffer.Buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(vulkanSystem.Device, vulkanBuffer.Buffer, nullptr);
        vulkanBuffer.Buffer = VK_NULL_HANDLE;
    }

    if (vulkanBuffer.StagingBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(vulkanSystem.Device, vulkanBuffer.StagingBuffer, nullptr);
        vulkanBuffer.StagingBuffer = VK_NULL_HANDLE;
    }

    if (vulkanBuffer.BufferMemory != VK_NULL_HANDLE)
    {
        vulkanSystem.FreeDeviceMemory(vulkanSystem.Device, &vulkanBuffer.BufferMemory);
        vulkanBuffer.BufferMemory = VK_NULL_HANDLE;
    }

    if (vulkanBuffer.StagingBufferMemory != VK_NULL_HANDLE)
    {
        vulkanSystem.FreeDeviceMemory(vulkanSystem.Device, &vulkanBuffer.StagingBufferMemory);
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
}