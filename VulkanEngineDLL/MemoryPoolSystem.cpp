#include "MemoryPoolSystem.h"
#include "MeshSystem.h"
#include "MaterialSystem.h"
#include "LightSystem.h"
#include "RenderSystem.h"

MemoryPoolSystem& memoryPoolSystem = MemoryPoolSystem::Get();

void MemoryPoolSystem::StartUp()
{
    for (int x = 0; x < static_cast<int>(MemoryPoolTypes::kEndofPool); x++)
    {
        MemoryPoolTypes type = (MemoryPoolTypes)x;
        switch (x)
        {
            case MemoryPoolTypes::kMeshBuffer:
            {
                MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader
                {
                    .ActiveCount = 0,
                    .Count = MeshInitialCapacity,
                    .Size = sizeof(MeshPropertiesStruct),
                    .IsActive = Vector<byte>(MeshInitialCapacity, 0x00),
                    .FreeIndices = Vector<uint32>(),
                    .IsDirty = true
                };
                break;
            }
            case MemoryPoolTypes::kMaterialBuffer:
            {
                MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader
                {
                    .ActiveCount = 0,
                    .Count = MaterialInitialCapacity,
                    .Size = sizeof(GPUMaterial),
                    .IsActive = Vector<byte>(MaterialInitialCapacity, 0x00),
                    .FreeIndices = Vector<uint32>(),
                    .IsDirty = true
                };
                break;
            }
            case MemoryPoolTypes::kDirectionalLightBuffer:
            {

                MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader
                {
                    .ActiveCount = 0,
                    .Count = DirectionalLightInitialCapacity,
                    .Size = sizeof(DirectionalLight),
                    .IsActive = Vector<byte>(DirectionalLightInitialCapacity, 0x00),
                    .FreeIndices = Vector<uint32>(),
                    .IsDirty = true
                };
                break;
            }
            case MemoryPoolTypes::kPointLightBuffer:
            {
                MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader
                {
                    .ActiveCount = 0,
                    .Count = PointLightInitialCapacity,
                    .Size = sizeof(PointLight),
                    .IsActive = Vector<byte>(PointLightInitialCapacity, 0x00),
                    .FreeIndices = Vector<uint32>(),
                    .IsDirty = true
                };
                break;
            }
        }
    }
    UpdateMemoryPoolHeader(kMeshBuffer, MeshInitialCapacity);
    
    GPUBufferMemoryPool = Vector<byte>(sizeof(MemoryPoolBufferHeader) + GPUBufferMemoryPoolSize, 0xFF);
    GPUMemoryPoolHeader = MemoryPoolBufferHeader
    {
        .MeshOffset = static_cast<uint32>(MemorySubPoolHeader[kMeshBuffer].Offset),
        .MeshCount = MemorySubPoolHeader[kMeshBuffer].Count,
        .MeshSize = MemorySubPoolHeader[kMeshBuffer].Size,
        .MaterialOffset = static_cast<uint32>(MemorySubPoolHeader[kMaterialBuffer].Offset),
        .MaterialCount = MemorySubPoolHeader[kMaterialBuffer].Count,
        .MaterialSize = MemorySubPoolHeader[kMaterialBuffer].Size,
        .DirectionalLightOffset = static_cast<uint32>(MemorySubPoolHeader[kDirectionalLightBuffer].Offset),
        .DirectionalLightCount = MemorySubPoolHeader[kDirectionalLightBuffer].Count,
        .DirectionalLightSize = MemorySubPoolHeader[kDirectionalLightBuffer].Size,
        .PointLightOffset = static_cast<uint32>(MemorySubPoolHeader[kPointLightBuffer].Offset),
        .PointLightCount = MemorySubPoolHeader[kPointLightBuffer].Count,
        .PointLightSize = MemorySubPoolHeader[kPointLightBuffer].Size
    };
    memcpy(GPUBufferMemoryPool.data(), &GPUMemoryPoolHeader, sizeof(MemoryPoolBufferHeader));
    GPUBufferIndex = bufferSystem.VMACreateDynamicBuffer(GPUBufferMemoryPool.data(), GPUBufferMemoryPool.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

void MemoryPoolSystem::ResizeMemoryPool(MemoryPoolTypes memoryPoolToUpdate, uint32 resizeCount)
{
    IsHeaderDirty = true;
    Vector<byte> oldGPUBufferMemoryPool = GPUBufferMemoryPool;
    UnorderedMap<MemoryPoolTypes, MemoryPoolSubBufferHeader> oldMemorySubPoolHeader = MemorySubPoolHeader;

    UpdateMemoryPoolHeader(memoryPoolToUpdate, resizeCount);
    GPUMemoryPoolHeader = MemoryPoolBufferHeader
    {
        .MeshOffset = static_cast<uint32>(MemorySubPoolHeader[kMeshBuffer].Offset),
        .MeshCount = MemorySubPoolHeader[kMeshBuffer].Count,
        .MeshSize = MemorySubPoolHeader[kMeshBuffer].Size,
        .MaterialOffset = static_cast<uint32>(MemorySubPoolHeader[kMaterialBuffer].Offset),
        .MaterialCount = MemorySubPoolHeader[kMaterialBuffer].Count,
        .MaterialSize = MemorySubPoolHeader[kMaterialBuffer].Size,
        .DirectionalLightOffset = static_cast<uint32>(MemorySubPoolHeader[kDirectionalLightBuffer].Offset),
        .DirectionalLightCount = MemorySubPoolHeader[kDirectionalLightBuffer].Count,
        .DirectionalLightSize = MemorySubPoolHeader[kDirectionalLightBuffer].Size,
        .PointLightOffset = static_cast<uint32>(MemorySubPoolHeader[kPointLightBuffer].Offset),
        .PointLightCount = MemorySubPoolHeader[kPointLightBuffer].Count,
        .PointLightSize = MemorySubPoolHeader[kPointLightBuffer].Size
    };

    GPUBufferMemoryPool = Vector<byte>(sizeof(MemoryPoolBufferHeader) + GPUBufferMemoryPoolSize, 0xFF);
    memcpy(GPUBufferMemoryPool.data(), &GPUMemoryPoolHeader, sizeof(MemoryPoolBufferHeader));
    for (int x = 0; x < static_cast<int>(MemoryPoolTypes::kEndofPool); x++)
    {
        const MemoryPoolTypes memoryPoolType = (MemoryPoolTypes)x;
        size_t srcOffset = x == 0 ? sizeof(MemoryPoolBufferHeader) : oldMemorySubPoolHeader[memoryPoolType].Offset;
        size_t dstOffset = x == 0 ? sizeof(MemoryPoolBufferHeader) : MemorySubPoolHeader[memoryPoolType].Offset;

        size_t size      = oldMemorySubPoolHeader[memoryPoolType].Size * oldMemorySubPoolHeader[memoryPoolType].Count;
        byte* srcAddress = oldGPUBufferMemoryPool.data() + srcOffset;
        byte* dstAddress = GPUBufferMemoryPool.data() +    dstOffset;
        memcpy(dstAddress, srcAddress, size);
    }
    
    uint32 newBufferId = bufferSystem.VMACreateDynamicBuffer(GPUBufferMemoryPool.data(), GPUBufferMemoryPool.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    bufferSystem.VMAUpdateDynamicBuffer(newBufferId, GPUBufferMemoryPool.data(), 0u);
    if (GPUBufferIndex != UINT32_MAX)
    {
        bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(GPUBufferIndex));
    }
    GPUBufferIndex = newBufferId;
    IsBufferDirty = true;
    IsDescriptorSetDirty = true;
}

uint32 MemoryPoolSystem::AllocateObject(MemoryPoolTypes memoryPoolToUpdate)
{
    MemoryPoolSubBufferHeader& subPoolHeader = MemorySubPoolHeader[memoryPoolToUpdate];
    if (!subPoolHeader.FreeIndices.empty())
    {
        uint32 index = subPoolHeader.FreeIndices.back();
        subPoolHeader.FreeIndices.pop_back();

        subPoolHeader.IsActive[index] = 1;
        if (index + 1 > subPoolHeader.ActiveCount)
        {
            subPoolHeader.ActiveCount = index + 1;
        }

        subPoolHeader.IsDirty = true;
        return index;
    }

    if (subPoolHeader.ActiveCount == subPoolHeader.Count)
    {
        ResizeMemoryPool(memoryPoolToUpdate, subPoolHeader.Count * 2);
    }

    uint32 index = subPoolHeader.ActiveCount;
    subPoolHeader.ActiveCount++;
    subPoolHeader.IsActive[index] = 0x01;
    subPoolHeader.IsDirty = true;
    return index;
}

void MemoryPoolSystem::UpdateMemoryPool(uint32 descriptorBindingIndex, Vector<VulkanPipeline>& pipelineList)
{
    if (IsBufferDirty)
    {
        memcpy(GPUBufferMemoryPool.data(), &GPUMemoryPoolHeader, sizeof(MemoryPoolBufferHeader));
        bufferSystem.VMAUpdateDynamicBuffer(GPUBufferIndex, GPUBufferMemoryPool.data(), GPUBufferMemoryPool.size());

        IsBufferDirty = false;
    }

    if (IsDescriptorSetDirty)
    {
        Vector<VkDescriptorBufferInfo> bufferInfo = GetMemoryPoolBufferInfo();
        for (auto& pipeline : pipelineList)
        {
            renderSystem.UpdateDescriptorSet(pipeline, bufferInfo, descriptorBindingIndex);
        }
        IsDescriptorSetDirty = false;
    }
}

void MemoryPoolSystem::UpdateMemoryPoolHeader(MemoryPoolTypes memoryPoolTypeToUpdate, uint32 newPoolSize)
{
    if (IsHeaderDirty)
    {
        for (int x = memoryPoolTypeToUpdate; x < static_cast<int>(MemoryPoolTypes::kEndofPool); x++)
        {
            const MemoryPoolTypes memoryPoolType = (MemoryPoolTypes)x;
            const MemoryPoolTypes lastMemoryPoolType = (x == 0) ? MemoryPoolTypes::kEndofPool : (MemoryPoolTypes)(x - 1);
            const MemoryPoolSubBufferHeader oldMemoryPoolSubHeader = MemorySubPoolHeader[memoryPoolType];
            MemorySubPoolHeader[memoryPoolType] = MemoryPoolSubBufferHeader
            {
               .ActiveCount = MemorySubPoolHeader[memoryPoolType].ActiveCount,
               .Offset = lastMemoryPoolType == MemoryPoolTypes::kEndofPool ? sizeof(MemoryPoolBufferHeader) : MemorySubPoolHeader[lastMemoryPoolType].Offset + (MemorySubPoolHeader[lastMemoryPoolType].Count * MemorySubPoolHeader[lastMemoryPoolType].Size),
               .Count = memoryPoolType == memoryPoolTypeToUpdate ? newPoolSize : MemorySubPoolHeader[memoryPoolType].Count,
               .Size = MemorySubPoolHeader[memoryPoolType].Size,
               .IsActive = memoryPoolType == memoryPoolTypeToUpdate ? Vector<byte>(newPoolSize, 0x00) : MemorySubPoolHeader[memoryPoolType].IsActive,
               .FreeIndices = MemorySubPoolHeader[memoryPoolType].FreeIndices,
               .IsDirty = true
            };
            if (memoryPoolType == (MemoryPoolTypes)x) memcpy(MemorySubPoolHeader[memoryPoolType].IsActive.data(), oldMemoryPoolSubHeader.IsActive.data(), oldMemoryPoolSubHeader.ActiveCount);
        }
        MemoryPoolSubBufferHeader lastHeader = MemorySubPoolHeader[(MemoryPoolTypes)((MemoryPoolTypes)MemoryPoolTypes::kEndofPool - 1)];
        GPUBufferMemoryPoolSize = lastHeader.Offset + (lastHeader.Size * lastHeader.Count);
    }
}

MeshPropertiesStruct& MemoryPoolSystem::UpdateMesh(uint32 index)
{
    MemoryPoolSubBufferHeader meshSubPool = MemorySubPoolHeader[kMeshBuffer];
    if (index >= meshSubPool.Count) throw std::out_of_range("Mesh index out of range: " + std::to_string(index) + " >= " + std::to_string(meshSubPool.Count));
    if (index >= meshSubPool.IsActive.size() || !meshSubPool.IsActive[index]) throw std::runtime_error("Mesh slot inactive at index " + std::to_string(index));

    uint32 offset = meshSubPool.Offset + (index * sizeof(MeshPropertiesStruct));
    void* meshBuffer = &GPUBufferMemoryPool[offset];
    return *reinterpret_cast<MeshPropertiesStruct*>(meshBuffer);
}

GPUMaterial& MemoryPoolSystem::UpdateMaterial(uint32 index)
{
    MemoryPoolSubBufferHeader& materialSubPool = MemorySubPoolHeader[kMaterialBuffer];
    if (index >= materialSubPool.Count) throw std::out_of_range("Material index out of range: " + std::to_string(index) + " >= " + std::to_string(materialSubPool.Count));
    if (index >= materialSubPool.IsActive.size() || !materialSubPool.IsActive[index]) throw std::runtime_error("Material slot inactive at index " + std::to_string(index));

    uint32 offset = materialSubPool.Offset + (index * sizeof(GPUMaterial));
    void* materialBuffer = &GPUBufferMemoryPool[offset];
    return *reinterpret_cast<GPUMaterial*>(materialBuffer);
}

DirectionalLight& MemoryPoolSystem::UpdateDirectionalLight(uint32 index)
{
    MemoryPoolSubBufferHeader& directionalLightSubPool = MemorySubPoolHeader[kDirectionalLightBuffer];
    if (index >= directionalLightSubPool.Count) throw std::out_of_range("Directional Light index out of range: " + std::to_string(index) + " >= " + std::to_string(directionalLightSubPool.Count));
    if (index >= directionalLightSubPool.IsActive.size() || !directionalLightSubPool.IsActive[index]) throw std::runtime_error("Directional Light slot inactive at index " + std::to_string(index));

    uint32 offset = directionalLightSubPool.Offset + (index * sizeof(DirectionalLight));
    void* directionalLightBuffer = &GPUBufferMemoryPool[offset];
    return *reinterpret_cast<DirectionalLight*>(directionalLightBuffer);
}

PointLight& MemoryPoolSystem::UpdatePointLight(uint32 index)
{
    MemoryPoolSubBufferHeader& pointLightSubPool = MemorySubPoolHeader[kPointLightBuffer];
    if (index >= pointLightSubPool.Count) throw std::out_of_range("Point Light index out of range: " + std::to_string(index) + " >= " + std::to_string(pointLightSubPool.Count));
    if (index >= pointLightSubPool.IsActive.size() || !pointLightSubPool.IsActive[index]) throw std::runtime_error("Point Light slot inactive at index " + std::to_string(index));

    uint32 offset = pointLightSubPool.Offset + (index * sizeof(PointLight));
    void* pointLightBuffer = &GPUBufferMemoryPool[offset];
    return *reinterpret_cast<PointLight*>(pointLightBuffer);
}

void MemoryPoolSystem::MarkMeshBufferDirty()
{
    MemorySubPoolHeader[kMeshBuffer].IsDirty = true;
}

void MemoryPoolSystem::MarkMaterialBufferDirty()
{
    MemorySubPoolHeader[kMaterialBuffer].IsDirty = true;
}

void MemoryPoolSystem::MarkDirectionalLightBufferDirty()
{
    MemorySubPoolHeader[kDirectionalLightBuffer].IsDirty = true;
}

void MemoryPoolSystem::MarkPointLightBufferDirty()
{
    MemorySubPoolHeader[kPointLightBuffer].IsDirty = true;
}

Vector<MeshPropertiesStruct> MemoryPoolSystem::MeshBufferList()
{
    const auto& subBufferData = MemorySubPoolHeader[kMeshBuffer];
    if (subBufferData.Count == 0) return {};

    Vector<MeshPropertiesStruct> bufferData(subBufferData.ActiveCount);
    const byte* src = static_cast<const byte*>(GPUBufferMemoryPool.data()) + subBufferData.Offset;
    std::memcpy(bufferData.data(), src, subBufferData.ActiveCount * sizeof(MeshPropertiesStruct));
    return bufferData;
}

Vector<GPUMaterial> MemoryPoolSystem::MaterialBufferList()
{
    const auto& subBufferData = MemorySubPoolHeader[kMaterialBuffer];
    if (subBufferData.Count == 0) return {};

    auto* a = GPUBufferMemoryPool.data() + subBufferData.Offset;
    Vector<GPUMaterial> bufferData(subBufferData.ActiveCount);
    const byte* src = static_cast<const byte*>(GPUBufferMemoryPool.data()) + subBufferData.Offset;
    std::memcpy(bufferData.data(), src, subBufferData.ActiveCount * sizeof(GPUMaterial));
    return bufferData;
}

Vector<DirectionalLight> MemoryPoolSystem::DirectionalLightBufferList()
{
    const auto& subBufferData = MemorySubPoolHeader[kDirectionalLightBuffer];
    if (subBufferData.Count == 0) return {};

    Vector<DirectionalLight> bufferData(subBufferData.ActiveCount);
    const byte* src = static_cast<const byte*>(GPUBufferMemoryPool.data()) + subBufferData.Offset;
    std::memcpy(bufferData.data(), src, subBufferData.ActiveCount * sizeof(DirectionalLight));
    return bufferData;
}

Vector<PointLight> MemoryPoolSystem::PointLightBufferList()
{
    const auto& subBufferData = MemorySubPoolHeader[kPointLightBuffer];
    if (subBufferData.Count == 0) return {};

    Vector<PointLight> bufferData(subBufferData.ActiveCount);
    const byte* src = static_cast<const byte*>(GPUBufferMemoryPool.data()) + subBufferData.Offset;
    std::memcpy(bufferData.data(), src, subBufferData.ActiveCount * sizeof(PointLight));
    return bufferData;
}

const MemoryPoolSubBufferHeader MemoryPoolSystem::MemoryPoolSubBufferInfo(MemoryPoolTypes memoryPoolType)
{
    return MemorySubPoolHeader[memoryPoolType];
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