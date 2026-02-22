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
    GPUBufferIndex = bufferSystem.VMACreateDynamicBuffer(GPUBufferMemoryPool.data(), GPUBufferMemoryPool.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

void MemoryPoolSystem::ResizeMemoryPool(MemoryPoolTypes memoryPoolToUpdate, uint32 resizeCount)
{
    IsHeaderDirty = true;
    Vector<byte> oldGPUBufferMemoryPool = GPUBufferMemoryPool;
    UnorderedMap<MemoryPoolTypes, MemoryPoolSubBufferHeader> oldMemorySubPoolHeader = MemorySubPoolHeader;
    UpdateMemoryPoolHeader(memoryPoolToUpdate, resizeCount);

    for (auto& copyRange : MemorySubPoolHeader)
    {
        MemoryPoolSubBufferHeader& subHeader = copyRange.second;
        const MemoryPoolSubBufferHeader oldSubHeader = oldMemorySubPoolHeader[copyRange.first];

        subHeader.ActiveCount = oldSubHeader.ActiveCount;
        std::memcpy(subHeader.IsActive.data(), oldSubHeader.IsActive.data(), oldSubHeader.IsActive.size());
    }

    GPUBufferMemoryPool.clear();
    GPUBufferMemoryPool.resize(GPUBufferMemoryPoolSize);
    uint32 newBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, sizeof(MemoryPoolBufferHeader) + GPUBufferMemoryPoolSize, bufferSystem.FindVulkanBuffer(GPUBufferIndex).BufferUsage);
    std::memcpy(static_cast<void*>(&GPUBufferMemoryPool), &GPUMemoryPoolHeader, sizeof(MemoryPoolBufferHeader));
    for (auto& [type, subMemoryPool] : MemorySubPoolHeader)
    {
        auto oldSub = oldMemorySubPoolHeader[type];
        size_t bytesToCopy = oldSub.ActiveCount * oldSub.Size;
        byte* dst = GPUBufferMemoryPool.data() + sizeof(MemoryPoolBufferHeader) + subMemoryPool.Offset;
        byte* src = oldGPUBufferMemoryPool.data() + oldSub.Offset;
        std::memcpy(dst, src, bytesToCopy);
    }

    bufferSystem.VMAUpdateDynamicBuffer(newBufferId, GPUBufferMemoryPool.data(), 0u);
    if (GPUBufferIndex != UINT32_MAX)
    {
        bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(GPUBufferIndex));
    }
    GPUBufferIndex = newBufferId;
    IsBufferDirty = true;
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

    if (subPoolHeader.ActiveCount == subPoolHeader.Size)
    {
        ResizeMemoryPool(memoryPoolToUpdate, subPoolHeader.Size * 1.2f);
    }

    uint32 index = subPoolHeader.ActiveCount++;
    subPoolHeader.IsActive[index] = 0x01;
    subPoolHeader.IsDirty = true;
    return index;
}

void MemoryPoolSystem::UpdateMemoryPool(uint32 descriptorBindingIndex, Vector<VulkanPipeline>& pipelineList)
{
    bool isDescriptorSetDirty = false;
    if (isDescriptorSetDirty)
    {
        if (IsHeaderDirty)
        {
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
            IsHeaderDirty = false;
        }
        bufferSystem.VMAUpdateDynamicBuffer(GPUBufferIndex, GPUBufferMemoryPool.data(), GPUBufferMemoryPool.size());

        Vector<VkDescriptorBufferInfo> bufferInfo = GetMemoryPoolBufferInfo();
        for (auto& pipeline : pipelineList)
        {
            renderSystem.UpdateDescriptorSet(pipeline, bufferInfo, descriptorBindingIndex);
        }
    }
}

void MemoryPoolSystem::UpdateMemoryPoolHeader(MemoryPoolTypes memoryPoolTypeToUpdate, uint32 newPoolSize)
{
    //struct MemoryPoolSubBufferHeader
    //{
    //    uint32					ActiveCount = UINT32_MAX;
    //    size_t					Offset = UINT32_MAX;
    //    uint32					Count = UINT32_MAX;
    //    uint32					Size = UINT32_MAX;
    //    Vector<byte>			IsActive;         // 0 = inactive, 1 = active
    //    Vector<uint32>			FreeIndices;
    //    bool					IsDirty = true;
    //};
    if (IsHeaderDirty)
    {
        for (int x = memoryPoolTypeToUpdate; x < static_cast<int>(MemoryPoolTypes::kEndofPool); x++)
        {
            const MemoryPoolTypes memoryPoolType = (MemoryPoolTypes)x;
            const MemoryPoolTypes lastMemoryPoolType = (x == 0) ? MemoryPoolTypes::kEndofPool : (MemoryPoolTypes)(x - 1);
            const MemoryPoolSubBufferHeader oldMemoryPoolSubHeader = MemorySubPoolHeader[memoryPoolType];
            MemorySubPoolHeader[memoryPoolType] = MemoryPoolSubBufferHeader
            {
               .ActiveCount = 0,
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
        IsHeaderDirty = false;
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
    MemoryPoolSubBufferHeader materialSubPool = MemorySubPoolHeader[kMaterialBuffer];
    if (index >= materialSubPool.Count) throw std::out_of_range("Material index out of range: " + std::to_string(index) + " >= " + std::to_string(materialSubPool.Count));
    if (index >= materialSubPool.IsActive.size() || !materialSubPool.IsActive[index]) throw std::runtime_error("Material slot inactive at index " + std::to_string(index));

    uint32 offset = materialSubPool.Offset + (index * sizeof(GPUMaterial));
    void* materialBuffer = &GPUBufferMemoryPool[offset];
    return *reinterpret_cast<GPUMaterial*>(materialBuffer);
}

DirectionalLight& MemoryPoolSystem::UpdateDirectionalLight(uint32 index)
{
    MemoryPoolSubBufferHeader directionalLightSubPool = MemorySubPoolHeader[kDirectionalLightBuffer];
    if (index >= directionalLightSubPool.Count) throw std::out_of_range("Directional Light index out of range: " + std::to_string(index) + " >= " + std::to_string(directionalLightSubPool.Count));
    if (index >= directionalLightSubPool.IsActive.size() || !directionalLightSubPool.IsActive[index]) throw std::runtime_error("Directional Light slot inactive at index " + std::to_string(index));

    uint32 offset = directionalLightSubPool.Offset + (index * sizeof(DirectionalLight));
    void* directionalLightBuffer = &GPUBufferMemoryPool[offset];
    return *reinterpret_cast<DirectionalLight*>(directionalLightBuffer);
}

PointLight& MemoryPoolSystem::UpdatePointLight(uint32 index)
{
    MemoryPoolSubBufferHeader pointLightSubPool = MemorySubPoolHeader[kPointLightBuffer];
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
    return Vector<MeshPropertiesStruct>();
}

Vector<GPUMaterial> MemoryPoolSystem::MaterialBufferList()
{
    return Vector<GPUMaterial>();
}

Vector<DirectionalLight> MemoryPoolSystem::DirectionalLightBufferList()
{
    return Vector<DirectionalLight>();
}

Vector<PointLight> MemoryPoolSystem::PointLightBufferList()
{
    return Vector<PointLight>();
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