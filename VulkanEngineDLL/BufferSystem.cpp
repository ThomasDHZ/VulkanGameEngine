#include "BufferSystem.h"

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

void VulkanBufferSystem::DestroyBuffer(const GraphicsRenderer& renderer, int vulkanBufferId)
{
    VulkanBuffer_DestroyBuffer(renderer, VulkanBufferMap[vulkanBufferId]);
}

void VulkanBufferSystem::DestroyAllBuffers(const GraphicsRenderer& renderer)
{
    for (auto& buffer : VulkanBufferMap)
    {
        VulkanBuffer_DestroyBuffer(renderer, buffer.second);
    }
}