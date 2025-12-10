#include "BufferSystem.h"
#include "VulkanSystem.h"

VulkanBufferSystem& bufferSystem = VulkanBufferSystem::Get();
int NextBufferId = 0;

// Helper: map once and keep mapped if possible
static void TryPersistentMap(VulkanBuffer& vulkanBuffer, VmaAllocation allocation)
{
    if (allocation == VK_NULL_HANDLE) return;

    VmaAllocationInfo allocInfo;
    vmaGetAllocationInfo(vulkanSystem.vmaAllocator, allocation, &allocInfo);

    if (allocInfo.pMappedData != nullptr)
    {
        vulkanBuffer.MappedData = allocInfo.pMappedData;
        vulkanBuffer.IsPersistentlyMapped = true;
    }
}

VulkanBuffer VulkanBufferSystem::CreateVulkanBuffer(uint bufferId, void* bufferData, VkDeviceSize bufferElementSize, uint bufferElementCount, BufferTypeEnum bufferTypeEnum, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool usingStagingBuffer)
{
    VkDeviceSize bufferSize = bufferElementSize * bufferElementCount;

    VulkanBuffer vulkanBuffer = {
        .BufferId = bufferId,
        .BufferSize = bufferSize,
        .BufferUsage = usage,
        .BufferProperties = properties,
        .BufferType = bufferTypeEnum,
        .UsingStagingBuffer = usingStagingBuffer
    };

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocCreateInfo = {};
    if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    }
    else
    {
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    }

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        int a = 34;
        //allocCreateInfo.flags |= VMA_ALLOCATION_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    }

    if (usingStagingBuffer)
    {
        VmaAllocationCreateInfo stagingAllocInfo = {};
        stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vulkanSystem.vmaAllocator, &bufferInfo, &stagingAllocInfo, &vulkanBuffer.StagingBuffer, &vulkanBuffer.StagingAllocation, nullptr));
        TryPersistentMap(vulkanBuffer, vulkanBuffer.StagingAllocation);

        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vulkanSystem.vmaAllocator, &bufferInfo, &allocCreateInfo, &vulkanBuffer.Buffer, &vulkanBuffer.Allocation, nullptr));
    }
    else
    {
        VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vulkanSystem.vmaAllocator, &bufferInfo, &allocCreateInfo, &vulkanBuffer.Buffer, &vulkanBuffer.Allocation, nullptr));
        TryPersistentMap(vulkanBuffer, vulkanBuffer.Allocation);
    }

    if (bufferData && bufferSize > 0)
    {
        UpdateBufferMemory(vulkanBuffer, bufferData, bufferSize);
    }

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        VkBufferDeviceAddressInfo addrInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = vulkanBuffer.Buffer };
        vulkanBuffer.BufferDeviceAddress = vkGetBufferDeviceAddress(vulkanSystem.Device, &addrInfo);
    }

    return vulkanBuffer;
}

void VulkanBufferSystem::UpdateBufferMemory(VulkanBuffer& vulkanBuffer, void* bufferData, VkDeviceSize bufferSize)
{
    if (bufferSize == 0 || bufferData == nullptr) return;

    VmaAllocation targetAlloc = vulkanBuffer.UsingStagingBuffer ? vulkanBuffer.StagingAllocation : vulkanBuffer.Allocation;
    VkBuffer targetBuffer = vulkanBuffer.UsingStagingBuffer ? vulkanBuffer.StagingBuffer : vulkanBuffer.Buffer;

    if (vulkanBuffer.IsPersistentlyMapped && vulkanBuffer.MappedData)
    {
        memcpy(vulkanBuffer.MappedData, bufferData, static_cast<size_t>(bufferSize));
    }
    else
    {
        void* data;
        VULKAN_THROW_IF_FAIL(vmaMapMemory(vulkanSystem.vmaAllocator, targetAlloc, &data));
        memcpy(data, bufferData, static_cast<size_t>(bufferSize));
        vmaUnmapMemory(vulkanSystem.vmaAllocator, targetAlloc);
    }

    if (vulkanBuffer.UsingStagingBuffer)
    {
        VkBufferCopy copyRegion = { .size = bufferSize };
        VkCommandBuffer cmd = vulkanSystem.BeginSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool);
        vkCmdCopyBuffer(cmd, vulkanBuffer.StagingBuffer, vulkanBuffer.Buffer, 1, &copyRegion);
        vulkanSystem.EndSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool, vulkanSystem.GraphicsQueue, cmd);
    }
}

void VulkanBufferSystem::ResizeBuffer(VulkanBuffer& vulkanBuffer, VkDeviceSize newSize, void* newData)
{
    if (newSize == 0)
    {
        DestroyBuffer(vulkanBuffer);
        return;
    }

    if (newSize == vulkanBuffer.BufferSize)
    {
        if (newData) UpdateBufferMemory(vulkanBuffer, newData, newSize);
        return;
    }

    VkBuffer oldBuffer = vulkanBuffer.Buffer;
    VmaAllocation oldAllocation = vulkanBuffer.Allocation;
    VkBuffer oldStaging = vulkanBuffer.StagingBuffer;
    VmaAllocation oldStagingAlloc = vulkanBuffer.StagingAllocation;

    bool wasStaging = vulkanBuffer.UsingStagingBuffer;
    bool wasMapped = vulkanBuffer.IsPersistentlyMapped;

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = newSize,
        .usage = vulkanBuffer.BufferUsage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocInfo = {};
    if (vulkanBuffer.BufferProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    }
    else
    {
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    }
    if (vulkanBuffer.BufferUsage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
       // allocInfo.flags |= VMA_ALLOCATION_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    }

    VkBuffer newBuffer = VK_NULL_HANDLE;
    VmaAllocation newAllocation = VK_NULL_HANDLE;
    VkBuffer newStaging = VK_NULL_HANDLE;
    VmaAllocation newStagingAlloc = VK_NULL_HANDLE;

    if (wasStaging)
    {
        VmaAllocationCreateInfo stagingInfo = {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST
        };
        VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vulkanSystem.vmaAllocator, &bufferInfo, &stagingInfo, &newStaging, &newStagingAlloc, nullptr));
        TryPersistentMap(vulkanBuffer, newStagingAlloc);

        bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vulkanSystem.vmaAllocator, &bufferInfo, &allocInfo, &newBuffer, &newAllocation, nullptr));
    }
    else
    {
        VULKAN_THROW_IF_FAIL(vmaCreateBuffer(vulkanSystem.vmaAllocator, &bufferInfo, &allocInfo, &newBuffer, &newAllocation, nullptr));
        TryPersistentMap(vulkanBuffer, newAllocation);
    }

    if (oldBuffer != VK_NULL_HANDLE && vulkanBuffer.BufferSize > 0 && newSize >= vulkanBuffer.BufferSize)
    {
        VkBufferCopy copy = { .size = vulkanBuffer.BufferSize };
        VkCommandBuffer cmd = vulkanSystem.BeginSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool);
        vkCmdCopyBuffer(cmd, wasStaging ? oldStaging : oldBuffer, wasStaging ? newStaging : newBuffer, 1, &copy);
        vulkanSystem.EndSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool, vulkanSystem.GraphicsQueue, cmd);
    }

    if (newData)
    {
        vulkanBuffer.Buffer = newBuffer;
        vulkanBuffer.Allocation = newAllocation;
        vulkanBuffer.StagingBuffer = newStaging;
        vulkanBuffer.StagingAllocation = newStagingAlloc;
        vulkanBuffer.BufferSize = newSize;
        UpdateBufferMemory(vulkanBuffer, newData, newSize);
    }
    else
    {
        vulkanBuffer.Buffer = newBuffer;
        vulkanBuffer.Allocation = newAllocation;
        vulkanBuffer.StagingBuffer = newStaging;
        vulkanBuffer.StagingAllocation = newStagingAlloc;
        vulkanBuffer.BufferSize = newSize;
        vulkanBuffer.MappedData = nullptr;
        vulkanBuffer.IsPersistentlyMapped = false;
        TryPersistentMap(vulkanBuffer, wasStaging ? newStagingAlloc : newAllocation);
    }

    if (oldBuffer) vmaDestroyBuffer(vulkanSystem.vmaAllocator, oldBuffer, oldAllocation);
    if (oldStaging) vmaDestroyBuffer(vulkanSystem.vmaAllocator, oldStaging, oldStagingAlloc);
}

void VulkanBufferSystem::CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size)
{
    if (!srcBuffer || !dstBuffer || size == 0) return;

    VkBufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };

    VkCommandBuffer cmd = vulkanSystem.BeginSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool);
    vkCmdCopyBuffer(cmd, *srcBuffer, *dstBuffer, 1, &copyRegion);
    vulkanSystem.EndSingleUseCommand(vulkanSystem.Device, vulkanSystem.CommandPool, vulkanSystem.GraphicsQueue, cmd);
}

VulkanBuffer& VulkanBufferSystem::FindVulkanBuffer(int id)
{
    return VulkanBufferMap.at(id);
}

void VulkanBufferSystem::DestroyBuffer(VulkanBuffer& vulkanBuffer)
{
    if (vulkanBuffer.Buffer != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(vulkanSystem.vmaAllocator, vulkanBuffer.Buffer, vulkanBuffer.Allocation);
        vulkanBuffer.Buffer = VK_NULL_HANDLE;
        vulkanBuffer.Allocation = VK_NULL_HANDLE;
    }
    if (vulkanBuffer.StagingBuffer != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(vulkanSystem.vmaAllocator, vulkanBuffer.StagingBuffer, vulkanBuffer.StagingAllocation);
        vulkanBuffer.StagingBuffer = VK_NULL_HANDLE;
        vulkanBuffer.StagingAllocation = VK_NULL_HANDLE;
    }

    vulkanBuffer.MappedData = nullptr;
    vulkanBuffer.IsPersistentlyMapped = false;
    vulkanBuffer.BufferSize = 0;
}