#include "BufferSystem.h"

int VulkanBufferSystem::NextBufferId = 0;

VulkanBufferSystem bufferSystem = VulkanBufferSystem();

VulkanBuffer& VulkanBufferSystem::FindVulkanBuffer(int id)
{
    auto it = VulkanBufferMap.find(id);
    if (it != VulkanBufferMap.end())
    {
        return it->second;
    }
    throw std::out_of_range("VulkanBufferMap not found for given GUID");
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