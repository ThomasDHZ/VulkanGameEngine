#include "MemoryPoolSystem.h"
#include "MeshSystem.h"
#include "MaterialSystem.h"
#include "LightSystem.h"

MemoryPoolSystem& memoryPoolSystem = MemoryPoolSystem::Get();

void MemoryPoolSystem::ResizeMemoryPool()
{
    uint32 newIndex = 0;
    UpdateMemoryPoolHeader();
    const uint32 newBufferSize = sizeof(MemoryPoolBufferHeader2) + GPUMemoryPoolHeader.PointLightOffset + (sizeof(PointLight) * PointLightCount);

    Vector<byte> newObjectDataPool(newBufferSize, 0xFF);
    Vector<byte> newIsActive(newBufferSize, 0x00);
    for (uint32 oldIndex = 0; oldIndex < GPUBufferMemoryPool.size(); ++oldIndex)
    {
        if (IsActive[oldIndex])
        {
            newObjectDataPool[newIndex] = std::move(GPUBufferMemoryPool[oldIndex]);
            newIsActive[newIndex] = 1;
            newIndex++;
        }
    }
    FreeIndices.clear();

    for (uint32 x = newIndex; x < newBufferSize; ++x)
    {
        FreeIndices.push_back(x);
    }

    uint32 newBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, newBufferSize, bufferSystem.FindVulkanBuffer(GPUBufferIndex).BufferUsage);
    bufferSystem.VMAUpdateDynamicBuffer(newBufferId, newObjectDataPool.data(), 0u);
    if (GPUBufferIndex != UINT32_MAX)
    {
        bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(GPUBufferIndex));
    }

    GPUBufferIndex = newBufferId;
    GPUBufferMemoryPool = std::move(newObjectDataPool);
    IsActive = std::move(newIsActive);
    ActiveCount = newIndex;
    IsBufferDirty = true;
}

void MemoryPoolSystem::UpdateMemoryPoolHeader()
{
    if (IsHeaderDirty)
    {
        GPUMemoryPoolHeader.MeshOffset = 0,
        GPUMemoryPoolHeader.MeshCount = MeshCount,
            GPUMemoryPoolHeader.MeshSize = sizeof(MeshPropertiesStruct),
            GPUMemoryPoolHeader.MaterialOffset = GPUMemoryPoolHeader.MeshOffset + GPUMemoryPoolHeader.MeshSize,
            GPUMemoryPoolHeader.MaterialCount = MaterialCount,
            GPUMemoryPoolHeader.MaterialSize = sizeof(GPUMaterial),
            GPUMemoryPoolHeader.DirectionalLightOffset = GPUMemoryPoolHeader.MaterialOffset + GPUMemoryPoolHeader.MaterialSize,
            GPUMemoryPoolHeader.DirectionalLightCount = DirectionalLightCount,
            GPUMemoryPoolHeader.DirectionalLightSize = sizeof(DirectionalLight),
            GPUMemoryPoolHeader.PointLightOffset = GPUMemoryPoolHeader.DirectionalLightOffset + GPUMemoryPoolHeader.DirectionalLightSize,
            GPUMemoryPoolHeader.PointLightCount = PointLightCount,
            GPUMemoryPoolHeader.PointLightSize = sizeof(PointLight);
        IsHeaderDirty = false;
    }
}

void MemoryPoolSystem::StartUp()
{
    MemorySubPoolHeader[kMeshBuffer] = MemoryPoolSubBufferHeader
    {
       .ActiveCount = UINT32_MAX,
       .Offset = sizeof(MeshPropertiesStruct),
       .Count = UINT32_MAX,
       .Size = UINT32_MAX,
       .IsActive,
       .FreeIndices
    }
}

const Vector<VkDescriptorBufferInfo> MemoryPoolSystem::GetMemoryPoolBufferInfo() const
{
    return Vector<VkDescriptorBufferInfo>
    {
        VkDescriptorBufferInfo
        {
            .buffer = bufferSystem.FindVulkanBuffer(GPUBufferIndex).Buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE
        }
    };
}