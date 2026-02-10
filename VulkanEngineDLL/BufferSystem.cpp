#define VMA_DEBUG_LOG_LEVEL 4
#define VMA_LEAK_LOG_LEVEL 4
#define VMA_LEAK_LOG_FORMAT(...) printf(__VA_ARGS__)
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

    VmaAllocationInfo stagingAllocOut = {};
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingAllocation = VK_NULL_HANDLE;
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

    CopyBuffer(&stagingBuffer, &dstBuffer, size - offset, shaderUsageFlags, offset);
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

void VulkanBufferSystem::CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size, VkBufferUsageFlags usageFlags, VkDeviceSize offset)
{
    VkCommandBuffer cmd = vulkanSystem.BeginSingleUseCommand();
    VkBufferCopy copyRegion =
    {
        .srcOffset = offset,
        .dstOffset = offset,
        .size = size
    };
    vkCmdCopyBuffer(cmd, *srcBuffer, *dstBuffer, 1, &copyRegion);

    VkPipelineStageFlags dstStageMask = 0;
    VkAccessFlags dstAccessMask = 0;

    if (usageFlags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) 
    {
        dstStageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        dstAccessMask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    }
    if (usageFlags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
        dstStageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        dstAccessMask |= VK_ACCESS_INDEX_READ_BIT;
    }
    if (usageFlags & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) {
        dstStageMask |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
        dstAccessMask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    }
    if (usageFlags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) 
    {
        dstStageMask |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        dstAccessMask |= VK_ACCESS_UNIFORM_READ_BIT;
    }
    if (usageFlags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) 
    {
        dstStageMask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dstAccessMask |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    }
    if (dstStageMask == 0) 
    {
        std::cout << "Unoptimised buffer transfer" << std::endl;
        dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT;
    }

    VkBufferMemoryBarrier barrier =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = dstAccessMask,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = *dstBuffer,
        .offset = offset,
        .size = size
    };
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, dstStageMask, 0, 0, nullptr, 1, &barrier, 0, nullptr);
    vulkanSystem.EndSingleUseCommand(cmd);
}

void VulkanBufferSystem::DestroyBuffer(VulkanBuffer& vulkanBuffer)
{
    if (!vulkanBuffer.Buffer &&
        !vulkanBuffer.StagingBuffer)
    {
        VULKAN_THROW_IF_FAIL(VK_ERROR_INITIALIZATION_FAILED);
    }

    if (vulkanBuffer.Allocation != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(bufferSystem.vmaAllocator, vulkanBuffer.Buffer, vulkanBuffer.Allocation);
        vulkanBuffer.Allocation = VK_NULL_HANDLE;
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