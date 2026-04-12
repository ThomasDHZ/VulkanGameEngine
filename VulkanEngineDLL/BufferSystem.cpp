#define VMA_DEBUG_LOG_LEVEL 0   // Reduce spam in RenderDoc
#define BUFFER_SYSTEM_IMPLEMENTATION
#include "BufferSystem.h"
#include "MemorySystem.h"
#include "VulkanSystem.h"
#include <vk_mem_alloc.h>
#include <iostream>

VulkanBufferSystem& bufferSystem = VulkanBufferSystem::Get();
int NextBufferId = 0;

VulkanBuffer& VulkanBufferSystem::FindVulkanBuffer(int id)
{
    auto it = VulkanBufferMap.find(id);
    if (it == VulkanBufferMap.end())
    {
        std::cerr << "Error: Buffer ID " << id << " not found!" << std::endl;
        static VulkanBuffer empty{};
        return empty;
    }
    return it->second;
}

const Vector<VulkanBuffer>& VulkanBufferSystem::VulkanBufferList()
{
    // This was returning a reference to a local - fixed
    static Vector<VulkanBuffer> vulkanBufferList;
    vulkanBufferList.clear();
    for (const auto& pair : VulkanBufferMap)
    {
        vulkanBufferList.emplace_back(pair.second);
    }
    return vulkanBufferList;
}

void VulkanBufferSystem::DestroyAllBuffers()
{
    for (auto& pair : VulkanBufferMap)
    {
        DestroyBuffer(pair.second);
    }
    VulkanBufferMap.clear();
}

uint32 VulkanBufferSystem::CreateStaticVulkanBuffer(const void* srcData, VkDeviceSize size,
    VkBufferUsageFlags shaderUsageFlags, VkDeviceSize offset)
{
    if (vmaAllocator == VK_NULL_HANDLE)
    {
        std::cerr << "[VMA] Allocator is null! Cannot create static buffer." << std::endl;
        return 0;
    }
    if (size == 0) return 0;

    uint32 bufferId = ++NextBufferId;

    // Main destination buffer (device local)
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | shaderUsageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocInfo = {
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    };

    VkBuffer dstBuffer = VK_NULL_HANDLE;
    VmaAllocation dstAllocation = VK_NULL_HANDLE;

    VkResult result = vmaCreateBuffer(vmaAllocator, &bufferInfo, &allocInfo, &dstBuffer, &dstAllocation, nullptr);
    if (result != VK_SUCCESS)
    {
        std::cerr << "[VMA] Failed to create static buffer. Error: " << result << std::endl;
        return 0;
    }

    if (srcData == nullptr)
    {
        VulkanBufferMap[bufferId] = {
            .BufferId = bufferId,
            .Buffer = dstBuffer,
            .BufferSize = size,
            .Allocation = dstAllocation,
            .UsingStagingBuffer = false
        };
        return bufferId;
    }

    // Staging buffer (host visible)
    VkBufferCreateInfo stagingInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo stagingAllocInfo = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo stagingAllocOut = {};

    result = vmaCreateBuffer(vmaAllocator, &stagingInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, &stagingAllocOut);
    if (result != VK_SUCCESS)
    {
        std::cerr << "[VMA] Failed to create staging buffer. Error: " << result << std::endl;
        vmaDestroyBuffer(vmaAllocator, dstBuffer, dstAllocation);
        return 0;
    }

    void* mappedData = stagingAllocOut.pMappedData;
    bool needUnmap = false;

    if (mappedData == nullptr)
    {
        result = vmaMapMemory(vmaAllocator, stagingAllocation, &mappedData);
        if (result != VK_SUCCESS)
        {
            std::cerr << "[VMA] Failed to map staging buffer." << std::endl;
            vmaDestroyBuffer(vmaAllocator, stagingBuffer, stagingAllocation);
            vmaDestroyBuffer(vmaAllocator, dstBuffer, dstAllocation);
            return 0;
        }
        needUnmap = true;
    }

    memcpy(static_cast<char*>(mappedData) + offset, srcData, size - offset);
    vmaFlushAllocation(vmaAllocator, stagingAllocation, offset, size - offset);

    if (needUnmap)
        vmaUnmapMemory(vmaAllocator, stagingAllocation);

    // Copy from staging → final buffer
    CopyBuffer(&stagingBuffer, &dstBuffer, size - offset, shaderUsageFlags, offset);

    // Cleanup staging buffer
    vmaDestroyBuffer(vmaAllocator, stagingBuffer, stagingAllocation);

    VulkanBufferMap[bufferId] = {
        .BufferId = bufferId,
        .Buffer = dstBuffer,
        .BufferSize = size,
        .Allocation = dstAllocation,
        .UsingStagingBuffer = true
    };

    return bufferId;
}

uint32 VulkanBufferSystem::CreateDynamicBuffer(const void* srcData, VkDeviceSize size, VkBufferUsageFlags usageFlags)
{
    if (vmaAllocator == VK_NULL_HANDLE)
    {
        std::cerr << "[VMA] Allocator is null! Cannot create dynamic buffer." << std::endl;
        return 0;
    }
    if (size == 0) return 0;

    uint32 bufferId = ++NextBufferId;

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocInfo = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
        .preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo allocOut = {};

    VkResult result = vmaCreateBuffer(vmaAllocator, &bufferInfo, &allocInfo, &buffer, &allocation, &allocOut);
    if (result != VK_SUCCESS)
    {
        std::cerr << "[VMA] Failed to create dynamic buffer. Error: " << result << std::endl;
        return 0;
    }

    void* mappedData = allocOut.pMappedData;
    bool needUnmap = false;

    if (mappedData == nullptr)
    {
        result = vmaMapMemory(vmaAllocator, allocation, &mappedData);
        if (result != VK_SUCCESS)
        {
            std::cerr << "[VMA] Failed to map dynamic buffer." << std::endl;
            vmaDestroyBuffer(vmaAllocator, buffer, allocation);
            return 0;
        }
        needUnmap = true;
    }

    if (srcData)
    {
        memcpy(mappedData, srcData, size);
        vmaFlushAllocation(vmaAllocator, allocation, 0, size);
    }

    VulkanBufferMap[bufferId] = {
        .BufferId = bufferId,
        .Buffer = buffer,
        .BufferSize = size,
        .Allocation = allocation,
        .BufferData = mappedData,
        .UsingStagingBuffer = false,
        .IsPersistentlyMapped = true
    };

    return bufferId;
}

void VulkanBufferSystem::UpdateDynamicBuffer(uint32 bufferId, const void* data, VkDeviceSize size, VkDeviceSize offset)
{
    auto it = VulkanBufferMap.find(bufferId);
    if (it == VulkanBufferMap.end() || !it->second.IsPersistentlyMapped || !it->second.BufferData)
    {
        std::cerr << "[VMA] UpdateDynamicBuffer failed: invalid buffer or not mapped." << std::endl;
        return;
    }

    VulkanBuffer& buffer = it->second;
    memcpy(static_cast<char*>(buffer.BufferData) + offset, data, size);
    vmaFlushAllocation(vmaAllocator, buffer.Allocation, offset, size);
}

void VulkanBufferSystem::CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size,
    VkBufferUsageFlags usageFlags, VkDeviceSize offset)
{
    VkCommandBuffer cmd = vulkanSystem.BeginSingleUseCommand();

    VkBufferCopy copyRegion = {
        .srcOffset = offset,
        .dstOffset = offset,
        .size = size
    };

    vkCmdCopyBuffer(cmd, *srcBuffer, *dstBuffer, 1, &copyRegion);

    // Simple barrier - good enough for most cases
    VkBufferMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT |
                         VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = *dstBuffer,
        .offset = offset,
        .size = size
    };

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        0, 0, nullptr, 1, &barrier, 0, nullptr);

    vulkanSystem.EndSingleUseCommand(cmd);
}

void VulkanBufferSystem::DestroyBuffer(VulkanBuffer& vulkanBuffer)
{
    if (vulkanBuffer.Buffer != VK_NULL_HANDLE)
    {
        if (vulkanBuffer.Allocation != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(vmaAllocator, vulkanBuffer.Buffer, vulkanBuffer.Allocation);
        }
        else
        {
            vkDestroyBuffer(vulkanSystem.Device, vulkanBuffer.Buffer, nullptr);
        }
        vulkanBuffer.Buffer = VK_NULL_HANDLE;
        vulkanBuffer.Allocation = VK_NULL_HANDLE;
    }

    if (vulkanBuffer.StagingBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(vulkanSystem.Device, vulkanBuffer.StagingBuffer, nullptr);
        vulkanBuffer.StagingBuffer = VK_NULL_HANDLE;
    }

    vulkanBuffer.BufferData = nullptr;
    vulkanBuffer.BufferSize = 0;
}
