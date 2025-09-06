#include "BufferSystem.h"

int VulkanBufferSystem::NextBufferId = 0;

VulkanBufferSystem bufferSystem = VulkanBufferSystem();

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

void VulkanBufferSystem::DestroyBuffer(const GraphicsRenderer& renderer, int vulkanBufferId)
{
    VulkanBuffer_DestroyBuffer(renderer, VulkanBufferMap[vulkanBufferId]);
}

void VulkanBufferSystem::DestroyAllBuffers()
{
    for (auto& buffer : VulkanBufferMap)
    {
        VulkanBuffer_DestroyBuffer(renderSystem.renderer, buffer.second);
    }
}