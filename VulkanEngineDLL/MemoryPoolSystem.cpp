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
            case MemoryPoolTypes::kTexture2DMetadataBuffer:
            {
                MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader
                {
                    .ActiveCount = 0,
                    .Count = Texture2DInitialCapacity,
                    .Size = sizeof(TextureMetadataHeader),
                    .IsActive = Vector<byte>(Texture2DInitialCapacity, 0x00),
                    .FreeIndices = Vector<uint32>(),
                    .IsDirty = true
                };
                break;
            }
            case MemoryPoolTypes::kTexture3DMetadataBuffer:
            {
                MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader
                {
                    .ActiveCount = 0,
                    .Count = Texture3DInitialCapacity,
                    .Size = sizeof(TextureMetadataHeader),
                    .IsActive = Vector<byte>(Texture3DInitialCapacity, 0x00),
                    .FreeIndices = Vector<uint32>(),
                    .IsDirty = true
                };
                break;
            }
            case MemoryPoolTypes::kTextureCubeMapMetadataBuffer:
            {
                MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader
                {
                    .ActiveCount = 0,
                    .Count = TextureCubeMapInitialCapacity,
                    .Size = sizeof(TextureMetadataHeader),
                    .IsActive = Vector<byte>(TextureCubeMapInitialCapacity, 0x00),
                    .FreeIndices = Vector<uint32>(),
                    .IsDirty = true
                };
                break;
            }
        }
    }
    UpdateMemoryPoolHeader(kMeshBuffer, MeshInitialCapacity);
    
    GpuDataBufferMemoryPool = Vector<byte>(sizeof(MemoryPoolBufferHeader) + GpuDataBufferMemoryPoolSize, 0xFF);
    GpuDataMemoryPoolHeader = MemoryPoolBufferHeader
    {
        .MeshOffset = MemorySubPoolHeader[kMeshBuffer].Offset,
        .MeshCount = MemorySubPoolHeader[kMeshBuffer].Count,
        .MeshSize = MemorySubPoolHeader[kMeshBuffer].Size,
        .MaterialOffset = MemorySubPoolHeader[kMaterialBuffer].Offset,
        .MaterialCount = MemorySubPoolHeader[kMaterialBuffer].Count,
        .MaterialSize = MemorySubPoolHeader[kMaterialBuffer].Size,
        .DirectionalLightOffset = MemorySubPoolHeader[kDirectionalLightBuffer].Offset,
        .DirectionalLightCount = MemorySubPoolHeader[kDirectionalLightBuffer].Count,
        .DirectionalLightSize = MemorySubPoolHeader[kDirectionalLightBuffer].Size,
        .PointLightOffset = MemorySubPoolHeader[kPointLightBuffer].Offset,
        .PointLightCount = MemorySubPoolHeader[kPointLightBuffer].Count,
        .PointLightSize = MemorySubPoolHeader[kPointLightBuffer].Size,
        .Texture2DOffset = MemorySubPoolHeader[kTexture2DMetadataBuffer].Offset,
        .Texture2DCount = MemorySubPoolHeader[kTexture2DMetadataBuffer].Count,
        .Texture2DSize = MemorySubPoolHeader[kTexture2DMetadataBuffer].Size,
        .Texture3DOffset = MemorySubPoolHeader[kTexture3DMetadataBuffer].Offset,
        .Texture3DCount = MemorySubPoolHeader[kTexture3DMetadataBuffer].Count,
        .Texture3DSize = MemorySubPoolHeader[kTexture3DMetadataBuffer].Size,
        .TextureCubeMapOffset = MemorySubPoolHeader[kTextureCubeMapMetadataBuffer].Offset,
        .TextureCubeMapCount = MemorySubPoolHeader[kTextureCubeMapMetadataBuffer].Count,
        .TextureCubeMapSize = MemorySubPoolHeader[kTextureCubeMapMetadataBuffer].Size
    };
    memcpy(GpuDataBufferMemoryPool.data(), &GpuDataMemoryPoolHeader, sizeof(MemoryPoolBufferHeader));
    GpuDataBufferIndex = bufferSystem.VMACreateDynamicBuffer(GpuDataBufferMemoryPool.data(), GpuDataBufferMemoryPool.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    VulkanBuffer& buf = bufferSystem.FindVulkanBuffer(GpuDataBufferIndex);
    MappedBufferPtr = buf.BufferData;

    vmaFlushAllocation(bufferSystem.vmaAllocator, buf.Allocation, 0, GpuDataBufferMemoryPool.size());
}

void MemoryPoolSystem::ResizeMemoryPool(MemoryPoolTypes memoryPoolToUpdate, uint32 resizeCount)
{
    Vector<byte> oldData = std::move(GpuDataBufferMemoryPool);
    auto oldHeaders = MemorySubPoolHeader;

    UpdateMemoryPoolHeader(memoryPoolToUpdate, resizeCount);

    size_t newSize = sizeof(MemoryPoolBufferHeader) + GpuDataBufferMemoryPoolSize;
    GpuDataBufferMemoryPool = Vector<byte>(newSize, 0xFF);

    memcpy(GpuDataBufferMemoryPool.data(), &GpuDataMemoryPoolHeader, sizeof(MemoryPoolBufferHeader));

    for (auto& [type, sub] : MemorySubPoolHeader)
    {
        auto& old = oldHeaders[type];
        size_t bytes = old.ActiveCount * old.Size;
        if (bytes > 0)
        {
            byte* dst = GpuDataBufferMemoryPool.data() + sizeof(MemoryPoolBufferHeader) + sub.Offset;
            byte* src = oldData.data() + sizeof(MemoryPoolBufferHeader) + old.Offset;
            memcpy(dst, src, bytes);
        }
    }

    uint32 newId = bufferSystem.VMACreateDynamicBuffer(GpuDataBufferMemoryPool.data(), newSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    VulkanBuffer& newBuf = bufferSystem.FindVulkanBuffer(newId);
    MappedBufferPtr = newBuf.BufferData;

    vmaFlushAllocation(bufferSystem.vmaAllocator, newBuf.Allocation, 0, newSize);
    if (GpuDataBufferIndex != UINT32_MAX)
    {
        bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(GpuDataBufferIndex));
    }

    GpuDataBufferIndex = newId;
    IsDescriptorSetDirty = true;
    IsHeaderDirty = true;
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

    uint32 index = subPoolHeader.ActiveCount++;
    subPoolHeader.IsActive[index] = 0x01;
    subPoolHeader.IsDirty = true;
    return index;
}

void MemoryPoolSystem::UpdateMemoryPool(Vector<VulkanPipeline>& pipelineList)
{
    if (!MappedBufferPtr) return;

    VulkanBuffer& buf = bufferSystem.FindVulkanBuffer(GpuDataBufferIndex);

    for (auto& [type, sub] : MemorySubPoolHeader)
    {
        if (sub.IsDirty)
        {
            size_t start = sizeof(MemoryPoolBufferHeader) + sub.Offset;
            size_t len = sub.ActiveCount * sub.Size;
            if (len > 0)
                vmaFlushAllocation(bufferSystem.vmaAllocator, buf.Allocation, start, len);
            sub.IsDirty = false;
        }
    }

    if (IsHeaderDirty)
    {
        memcpy(MappedBufferPtr, &GpuDataMemoryPoolHeader, sizeof(MemoryPoolBufferHeader));
        vmaFlushAllocation(bufferSystem.vmaAllocator, buf.Allocation, 0, sizeof(MemoryPoolBufferHeader));
        IsHeaderDirty = false;
    }

    if (IsDescriptorSetDirty)
    {
        auto info = GetBindlessDataBufferDescriptor();
        for (auto& p : pipelineList)
        {
            renderSystem.UpdateDescriptorSet(p, info, BindlessDataDescriptorBinding);
        }
        IsDescriptorSetDirty = false;
    }
}

void MemoryPoolSystem::UpdateMemoryPoolHeader(MemoryPoolTypes memoryPoolTypeToUpdate, uint32 newPoolSize)
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
    GpuDataBufferMemoryPoolSize = lastHeader.Offset + (lastHeader.Size * lastHeader.Count);
}

MeshPropertiesStruct& MemoryPoolSystem::UpdateMesh(uint32 index)
{
    MemoryPoolSubBufferHeader meshSubPool = MemorySubPoolHeader[kMeshBuffer];
    if (index >= meshSubPool.Count) throw std::out_of_range("Mesh index out of range: " + std::to_string(index) + " >= " + std::to_string(meshSubPool.Count));
    if (index >= meshSubPool.IsActive.size() || !meshSubPool.IsActive[index]) throw std::runtime_error("Mesh slot inactive at index " + std::to_string(index));

    uint32 offset = meshSubPool.Offset + (index * sizeof(MeshPropertiesStruct));
    meshSubPool.IsDirty = true;
    return *reinterpret_cast<MeshPropertiesStruct*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

GPUMaterial& MemoryPoolSystem::UpdateMaterial(uint32 index)
{
    MemoryPoolSubBufferHeader& materialSubPool = MemorySubPoolHeader[kMaterialBuffer];
    if (index >= materialSubPool.Count) throw std::out_of_range("Material index out of range: " + std::to_string(index) + " >= " + std::to_string(materialSubPool.Count));
    if (index >= materialSubPool.IsActive.size() || !materialSubPool.IsActive[index]) throw std::runtime_error("Material slot inactive at index " + std::to_string(index));

    uint32 offset = materialSubPool.Offset + (index * sizeof(GPUMaterial));
    materialSubPool.IsDirty = true;
    return *reinterpret_cast<GPUMaterial*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

DirectionalLight& MemoryPoolSystem::UpdateDirectionalLight(uint32 index)
{
    MemoryPoolSubBufferHeader& directionalLightSubPool = MemorySubPoolHeader[kDirectionalLightBuffer];
    if (index >= directionalLightSubPool.Count) throw std::out_of_range("Directional Light index out of range: " + std::to_string(index) + " >= " + std::to_string(directionalLightSubPool.Count));
    if (index >= directionalLightSubPool.IsActive.size() || !directionalLightSubPool.IsActive[index]) throw std::runtime_error("Directional Light slot inactive at index " + std::to_string(index));

    uint32 offset = directionalLightSubPool.Offset + (index * sizeof(DirectionalLight));
    directionalLightSubPool.IsDirty = true;
    return *reinterpret_cast<DirectionalLight*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

PointLight& MemoryPoolSystem::UpdatePointLight(uint32 index)
{
    MemoryPoolSubBufferHeader& pointLightSubPool = MemorySubPoolHeader[kPointLightBuffer];
    if (index >= pointLightSubPool.Count) throw std::out_of_range("Point Light index out of range: " + std::to_string(index) + " >= " + std::to_string(pointLightSubPool.Count));
    if (index >= pointLightSubPool.IsActive.size() || !pointLightSubPool.IsActive[index]) throw std::runtime_error("Point Light slot inactive at index " + std::to_string(index));

    uint32 offset = pointLightSubPool.Offset + (index * sizeof(PointLight));
    pointLightSubPool.IsDirty = true;
    return *reinterpret_cast<PointLight*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

TextureMetadataHeader& MemoryPoolSystem::UpdateTexture2DMetadataHeader(uint32 index)
{
    MemoryPoolSubBufferHeader& sub = MemorySubPoolHeader[kTexture2DMetadataBuffer];
    if (index >= sub.Count || !sub.IsActive[index])
    {
        throw std::runtime_error("Invalid texture metadata index");
    }

    uint32 offset = sub.Offset + (index * sizeof(TextureMetadataHeader));
    return *reinterpret_cast<TextureMetadataHeader*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

TextureMetadataHeader& MemoryPoolSystem::UpdateTexture3DMetadataHeader(uint32 index)
{
    MemoryPoolSubBufferHeader& sub = MemorySubPoolHeader[kTexture3DMetadataBuffer];
    if (index >= sub.Count || !sub.IsActive[index])
    {
        throw std::runtime_error("Invalid texture metadata index");
    }

    uint32 offset = sub.Offset + (index * sizeof(TextureMetadataHeader));
    return *reinterpret_cast<TextureMetadataHeader*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

TextureMetadataHeader& MemoryPoolSystem::UpdateTextureCubeMapMetadataHeader(uint32 index)
{
    MemoryPoolSubBufferHeader& sub = MemorySubPoolHeader[kTextureCubeMapMetadataBuffer];
    if (index >= sub.Count || !sub.IsActive[index])
    {
        throw std::runtime_error("Invalid texture metadata index");
    }

    uint32 offset = sub.Offset + (index * sizeof(TextureMetadataHeader));
    return *reinterpret_cast<TextureMetadataHeader*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

Vector<MeshPropertiesStruct> MemoryPoolSystem::MeshBufferList()
{
    const auto& subBufferData = MemorySubPoolHeader[kMeshBuffer];
    if (subBufferData.Count == 0) return {};

    Vector<MeshPropertiesStruct> bufferData(subBufferData.ActiveCount);
    const byte* src = static_cast<const byte*>(GpuDataBufferMemoryPool.data()) + subBufferData.Offset;
    std::memcpy(bufferData.data(), src, subBufferData.ActiveCount * sizeof(MeshPropertiesStruct));
    return bufferData;
}

Vector<GPUMaterial> MemoryPoolSystem::MaterialBufferList()
{
    const auto& subBufferData = MemorySubPoolHeader[kMaterialBuffer];
    if (subBufferData.Count == 0) return {};

    Vector<GPUMaterial> bufferData(subBufferData.ActiveCount);
    const byte* src = static_cast<const byte*>(GpuDataBufferMemoryPool.data()) + subBufferData.Offset;
    std::memcpy(bufferData.data(), src, subBufferData.ActiveCount * sizeof(GPUMaterial));
    return bufferData;
}

Vector<DirectionalLight> MemoryPoolSystem::DirectionalLightBufferList()
{
    const auto& subBufferData = MemorySubPoolHeader[kDirectionalLightBuffer];
    if (subBufferData.Count == 0) return {};

    Vector<DirectionalLight> bufferData(subBufferData.ActiveCount);
    const byte* src = static_cast<const byte*>(GpuDataBufferMemoryPool.data()) + subBufferData.Offset;
    std::memcpy(bufferData.data(), src, subBufferData.ActiveCount * sizeof(DirectionalLight));
    return bufferData;
}

Vector<PointLight> MemoryPoolSystem::PointLightBufferList()
{
    const auto& subBufferData = MemorySubPoolHeader[kPointLightBuffer];
    if (subBufferData.Count == 0) return {};

    Vector<PointLight> bufferData(subBufferData.ActiveCount);
    const byte* src = static_cast<const byte*>(GpuDataBufferMemoryPool.data()) + subBufferData.Offset;
    std::memcpy(bufferData.data(), src, subBufferData.ActiveCount * sizeof(PointLight));
    return bufferData;
}

const MemoryPoolSubBufferHeader MemoryPoolSystem::MemoryPoolSubBufferInfo(MemoryPoolTypes memoryPoolType)
{
    return MemorySubPoolHeader[memoryPoolType];
}

const Vector<VkDescriptorBufferInfo> MemoryPoolSystem::GetBindlessDataBufferDescriptor() const
{
    return Vector<VkDescriptorBufferInfo>
    {
        VkDescriptorBufferInfo
        {
            .buffer = bufferSystem.FindVulkanBuffer(GpuDataBufferIndex).Buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE
        }
    };
}
