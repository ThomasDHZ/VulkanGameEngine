#define BUFFER_SYSTEM_IMPLEMENTATION
#include "BufferSystem.h"
#include "MemorySystem.h"
#include "vk_mem_alloc.h"

VulkanBufferSystem& bufferSystem = VulkanBufferSystem::Get();
int NextBufferId = 0;

VulkanBuffer& VulkanBufferSystem::FindVulkanBuffer(int id)
{
    return VulkanBufferMap.at(id);
}

VulkanBuffer VulkanBufferSystem::CreateVulkanBuffer(uint bufferId, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer) 
{
    VkDeviceSize bufferSize = bufferElementSize * bufferElementCount;
    VulkanBuffer vulkanBuffer = {
        .BufferId = bufferId,
        .BufferSize = bufferSize,
        .BufferUsage = usage,
        .BufferProperties = properties,
        .BufferType = bufferTypeEnum,
        .UsingStagingBuffer = usingStagingBuffer,
    };

    VkBufferCreateInfo bufferInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocInfo = {};
    if (usingStagingBuffer) 
    {
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vulkanSystem.vmaAllocator, &bufferInfo, &allocInfo, &vulkanBuffer.StagingBuffer, &vulkanBuffer.StagingBufferMemory, nullptr));

        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vulkanSystem.vmaAllocator, &bufferInfo, &allocInfo, &vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, nullptr));
    }
    else
    {
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
          //  allocInfo.flags |= VMA_ALLOCATION_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        }
        VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vulkanSystem.vmaAllocator, &bufferInfo, &allocInfo, &vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, nullptr));
    }

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfo addrInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = vulkanBuffer.Buffer };
        vulkanBuffer.BufferDeviceAddress = vkGetBufferDeviceAddress(vulkanSystem.Device, &addrInfo);
    }
    return vulkanBuffer;
}

VulkanBuffer VulkanBufferSystem::CreateVulkanBuffer(uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
{
    VulkanBuffer buffer = CreateVulkanBuffer(bufferId, bufferElementSize, bufferElementCount, bufferTypeEnum, usage, properties, usingStagingBuffer);

    if (bufferData) 
    {
        if (usingStagingBuffer) 
        {
            void* mapped = nullptr;
            vmaMapMemory(vulkanSystem.vmaAllocator, buffer.StagingBufferMemory, &mapped);
            memcpy(mapped, bufferData, buffer.BufferSize);
            vmaUnmapMemory(vulkanSystem.vmaAllocator, buffer.StagingBufferMemory);

            VkBufferCopy copy = { .size = buffer.BufferSize };
            VkCommandBuffer cmd = vulkanSystem.BeginSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool);
            vkCmdCopyBuffer(cmd, buffer.StagingBuffer, buffer.Buffer, 1, &copy);
            vulkanSystem.EndSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool, vulkanSystem.GraphicsQueue, cmd);
        }
        else 
        {
            void* mapped;
            vmaMapMemory(vulkanSystem.vmaAllocator, buffer.BufferMemory, &mapped);
            memcpy(mapped, bufferData, buffer.BufferSize);
            vmaUnmapMemory(vulkanSystem.vmaAllocator, buffer.BufferMemory);
        }
    }

    return buffer;
}

void VulkanBufferSystem::ResizeBuffer(VulkanBuffer& vulkanBuffer, VkDeviceSize newSize, void* newData)
{
    if (newSize == 0) 
    {
        VULKAN_THROW_IF_FAIL(VK_ERROR_INITIALIZATION_FAILED);
    }

    VkBufferUsageFlags usage = vulkanBuffer.BufferUsage;
    bool useStaging = vulkanBuffer.UsingStagingBuffer;

    VkBuffer newBuffer = VK_NULL_HANDLE;
    VmaAllocation newAlloc = VK_NULL_HANDLE;
    VkBuffer newStaging = VK_NULL_HANDLE;
    VmaAllocation newStagingAlloc = VK_NULL_HANDLE;

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = newSize,
        .usage = useStaging ? (usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT) : usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vulkanSystem.vmaAllocator, &bufferInfo, &allocInfo, &newBuffer, &newAlloc, nullptr));
    if (useStaging) 
    {
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vulkanSystem.vmaAllocator, &bufferInfo, &allocInfo, &newStaging, &newStagingAlloc, nullptr));
    }

    if (newData == nullptr && vulkanBuffer.Buffer != VK_NULL_HANDLE && vulkanBuffer.BufferSize > 0) 
    {
        VkDeviceSize copySize = std::min(vulkanBuffer.BufferSize, newSize);
        if (useStaging && vulkanBuffer.StagingBuffer) 
        {
            CopyBufferMemory(vulkanBuffer.StagingBuffer, newStaging, copySize);
        }
        else 
        {
            CopyBufferMemory(vulkanBuffer.Buffer, newBuffer, copySize);
        }
    }

    if (newData) 
    {
        if (useStaging && newStaging) 
        {
            void* mapped = nullptr;
            vmaMapMemory(vulkanSystem.vmaAllocator, newStagingAlloc, &mapped);
            memcpy(mapped, newData, newSize);
            vmaUnmapMemory(vulkanSystem.vmaAllocator, newStagingAlloc);
        }
        else 
        {
            void* mapped = nullptr;
            vmaMapMemory(vulkanSystem.vmaAllocator, newAlloc, &mapped);
            memcpy(mapped, newData, newSize);
            vmaUnmapMemory(vulkanSystem.vmaAllocator, newAlloc);
        }
    }

    if (vulkanBuffer.Buffer) 
    {
        vmaDestroyBuffer(vulkanSystem.vmaAllocator, vulkanBuffer.Buffer, vulkanBuffer.BufferMemory);
    }
    if (vulkanBuffer.StagingBuffer) 
    {
        vmaDestroyBuffer(vulkanSystem.vmaAllocator, vulkanBuffer.StagingBuffer, vulkanBuffer.StagingBufferMemory);
    }

    vulkanBuffer.Buffer = newBuffer;
    vulkanBuffer.BufferMemory = newAlloc;
    vulkanBuffer.StagingBuffer = newStaging;
    vulkanBuffer.StagingBufferMemory = newStagingAlloc;
    vulkanBuffer.BufferSize = newSize;
    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) 
    {
        VkBufferDeviceAddressInfo addrInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = newBuffer };
        vulkanBuffer.BufferDeviceAddress = vkGetBufferDeviceAddress(vulkanSystem.Device, &addrInfo);
    }
}

void VulkanBufferSystem::UpdateBufferData(VulkanBuffer& vulkanBuffer, void* data, VkDeviceSize size)
{
    if (size > vulkanBuffer.BufferSize) 
    {
        ResizeBuffer(vulkanBuffer, size, data);
        return;
    }

    if (vulkanBuffer.UsingStagingBuffer && vulkanBuffer.StagingBuffer) 
    {
        void* mapped;
        vmaMapMemory(vulkanSystem.vmaAllocator, vulkanBuffer.StagingBufferMemory, &mapped);
        memcpy(mapped, data, size);
        vmaUnmapMemory(vulkanSystem.vmaAllocator, vulkanBuffer.BufferMemory);
        CopyBufferMemory(vulkanBuffer.StagingBuffer, vulkanBuffer.Buffer, size);
    }
    else 
    {
        void* mapped;
        vmaMapMemory(vulkanSystem.vmaAllocator, vulkanBuffer.BufferMemory, &mapped);
        memcpy(mapped, data, size);
        vmaUnmapMemory(vulkanSystem.vmaAllocator, vulkanBuffer.BufferMemory);
    }
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

void VulkanBufferSystem::CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size)
{
    if (!srcBuffer || !dstBuffer)
    {
        VULKAN_THROW_IF_FAIL(VK_ERROR_UNKNOWN);
    }

    VkBufferCopy copyRegion =
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    VkCommandBuffer commandBuffer = vulkanSystem.BeginSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool);
    vkCmdCopyBuffer(commandBuffer, *srcBuffer, *dstBuffer, 1, &copyRegion);
    vulkanSystem.EndSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool, vulkanSystem.GraphicsQueue, commandBuffer);
}

void VulkanBufferSystem::DestroyBuffer(VulkanBuffer& vulkanBuffer)
{
    if (vulkanBuffer.Buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(vulkanSystem.vmaAllocator, vulkanBuffer.Buffer, vulkanBuffer.BufferMemory);
        vulkanBuffer.Buffer = VK_NULL_HANDLE;
        vulkanBuffer.BufferMemory = VK_NULL_HANDLE;
    }
    if (vulkanBuffer.StagingBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(vulkanSystem.vmaAllocator, vulkanBuffer.StagingBuffer, vulkanBuffer.StagingBufferMemory);
        vulkanBuffer.StagingBuffer = VK_NULL_HANDLE;
        vulkanBuffer.StagingBufferMemory = VK_NULL_HANDLE;
    }
    vulkanBuffer.BufferSize = 0;
}