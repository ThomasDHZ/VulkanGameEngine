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
    VulkanBuffer vulkanBuffer = {
        .BufferId = bufferId,
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
        vulkanBuffer.BufferDeviceAddress = vkGetBufferDeviceAddress(renderer.Device, &addrInfo);
    }

    memorySystem.DeletePtr(bufferData);
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

    if (vulkanBuffer.UsingStagingBuffer)
    {
        CreateStagingBuffer(&vulkanBuffer.StagingBuffer, &vulkanBuffer.Buffer, &vulkanBuffer.StagingBufferMemory, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferUsage, vulkanBuffer.BufferProperties);
    }
    else
    {
        CreateBuffer(&vulkanBuffer.Buffer, &vulkanBuffer.BufferMemory, bufferData, bufferSize, vulkanBuffer.BufferProperties, vulkanBuffer.BufferUsage);
    }
    return vulkanBuffer;
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
    VULKAN_THROW_IF_FAIL(vkMapMemory(renderer.Device, bufferMemory, 0, bufferSize, 0, &mappedData));
    memcpy(mappedData, dataToCopy, static_cast<size_t>(bufferSize));
    vkUnmapMemory(renderer.Device, bufferMemory);
}

void VulkanBufferSystem::CopyBufferMemory(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
{
    VkBufferCopy copyRegion =
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = bufferSize
    };

    VkCommandBuffer commandBuffer = Renderer_BeginSingleUseCommand(renderer.Device, renderer.CommandPool);
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    Renderer_EndSingleUseCommand(renderer.Device, renderer.CommandPool, renderer.GraphicsQueue, commandBuffer);
}

void VulkanBufferSystem::AllocateMemory(VkBuffer* bufferData, VkDeviceMemory* bufferMemory, VkMemoryPropertyFlags properties, VkBufferUsageFlags usage)
{
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(renderer.Device, *bufferData, &memRequirements);

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
        .memoryTypeIndex = Renderer_GetMemoryType(renderer.PhysicalDevice, memRequirements.memoryTypeBits, properties)
    };

    VULKAN_THROW_IF_FAIL(vkAllocateMemory(renderer.Device, &allocInfo, nullptr, bufferMemory));
}

void* VulkanBufferSystem::MapBufferMemory(VkDeviceMemory bufferMemory, VkDeviceSize bufferSize, bool* isMapped)
{
    if (*isMapped)
    {
        return nullptr;
    }

    void* mappedData = nullptr;
    VULKAN_THROW_IF_FAIL(vkMapMemory(renderer.Device, bufferMemory, 0, bufferSize, 0, &mappedData));
    *isMapped = true;
    return mappedData;
}

void VulkanBufferSystem::UnmapBufferMemory(VkDeviceMemory bufferMemory, bool* isMapped)
{
    if (*isMapped)
    {
        vkUnmapMemory(renderer.Device, bufferMemory);
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

    VULKAN_THROW_IF_FAIL(vkCreateBuffer(renderer.Device, &bufferInfo, nullptr, buffer));
    vkGetBufferMemoryRequirements(renderer.Device, *buffer, &memReqs);
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = Renderer_GetMemoryType(renderer.PhysicalDevice, memReqs.memoryTypeBits, properties);

    VULKAN_THROW_IF_FAIL(vkAllocateMemory(renderer.Device, &allocInfo, nullptr, bufferMemory));
    VULKAN_THROW_IF_FAIL(vkBindBufferMemory(renderer.Device, *buffer, *bufferMemory, 0));
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

    VkCommandBuffer commandBuffer = Renderer_BeginSingleUseCommand(renderer.Device, renderer.CommandPool);
    vkCmdCopyBuffer(commandBuffer, *srcBuffer, *dstBuffer, 1, &copyRegion);
    Renderer_EndSingleUseCommand(renderer.Device, renderer.CommandPool, renderer.GraphicsQueue, commandBuffer);
}

void VulkanBufferSystem::UpdateBufferSize(VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* bufferData, VkDeviceSize oldBufferSize, VkDeviceSize newBufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags propertyFlags)
{
    if (newBufferSize < oldBufferSize)
    {
      /*  RENDERER_ERROR("%s", (std::string("Buffer size can't be less than the old buffer size. OldSize: ")
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

    VULKAN_THROW_IF_FAIL(vkCreateBuffer(renderer.Device, &bufferCreateInfo, nullptr, &newBuffer));
    AllocateMemory(&newBuffer, &newBufferMemory, propertyFlags, bufferUsageFlags);

    VkResult result = vkBindBufferMemory(renderer.Device, newBuffer, newBufferMemory, 0);
    if (result != VK_SUCCESS)
    {
        Renderer_FreeDeviceMemory(renderer.Device, &newBufferMemory);
        vkDestroyBuffer(renderer.Device, newBuffer, nullptr);
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
        vkDestroyBuffer(renderer.Device, *buffer, nullptr);
    }
    if (*bufferMemory != VK_NULL_HANDLE)
    {
        Renderer_FreeDeviceMemory(renderer.Device, bufferMemory);
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

void VulkanBuffer_CopyBuffer(VkBuffer* srcBuffer, VkBuffer* dstBuffer, VkDeviceSize size)
{
    bufferSystem.CopyBuffer(srcBuffer, dstBuffer, size);
}

void VulkanBuffer_DestroyBuffer(VulkanBuffer& vulkanBuffer)
{
    bufferSystem.DestroyBuffer(vulkanBuffer);
}
