#include "MemoryPoolSystem.h"
#include "MeshSystem.h"
#include "MaterialSystem.h"
#include "LightSystem.h"
#include "RenderSystem.h"

MemoryPoolSystem& memoryPoolSystem = MemoryPoolSystem::Get();

void MemoryPoolSystem::StartUp()
{
    std::cout << "MemoryPoolSystem::StartUp() started\n";

    // Initialize all sub-pools first
    for (int x = 0; x < static_cast<int>(MemoryPoolTypes::kEndofPool); x++)
    {
        MemoryPoolTypes type = (MemoryPoolTypes)x;
        switch (x)
        {
        case MemoryPoolTypes::kMeshBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Count = MeshInitialCapacity,
                .Size = sizeof(MeshPropertiesStruct),
                .IsActive = Vector<byte>(MeshInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;

        case MemoryPoolTypes::kMaterialBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Count = MaterialInitialCapacity,
                .Size = sizeof(GPUMaterial),
                .IsActive = Vector<byte>(MaterialInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;

        case MemoryPoolTypes::kDirectionalLightBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Count = DirectionalLightInitialCapacity,
                .Size = sizeof(DirectionalLight),
                .IsActive = Vector<byte>(DirectionalLightInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;

        case MemoryPoolTypes::kPointLightBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Count = PointLightInitialCapacity,
                .Size = sizeof(PointLight),
                .IsActive = Vector<byte>(PointLightInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;

        case MemoryPoolTypes::kTexture2DMetadataBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Count = Texture2DInitialCapacity,
                .Size = sizeof(TextureMetadataHeader),
                .IsActive = Vector<byte>(Texture2DInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;

        case MemoryPoolTypes::kTexture3DMetadataBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Count = Texture3DInitialCapacity,
                .Size = sizeof(TextureMetadataHeader),
                .IsActive = Vector<byte>(Texture3DInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;

        case MemoryPoolTypes::kTextureCubeMapMetadataBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Count = TextureCubeMapInitialCapacity,
                .Size = sizeof(TextureMetadataHeader),
                .IsActive = Vector<byte>(TextureCubeMapInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;

        case MemoryPoolTypes::kSpriteInstanceBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Count = SpriteInstanceInitialCapacity,
                .Size = sizeof(SpriteInstance),
                .IsActive = Vector<byte>(SpriteInstanceInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;
        }
    }

    // Now update headers and calculate total size
    UpdateMemoryPoolHeader(MemoryPoolTypes::kMeshBuffer, MeshInitialCapacity);

    // Create the big GPU data buffer
    size_t totalGpuBufferSize = sizeof(MemoryPoolBufferHeader) + GpuDataBufferMemoryPoolSize;

    std::cout << "Creating GPU data buffer with size: " << totalGpuBufferSize << " bytes\n";

    GpuDataBufferIndex = bufferSystem.CreateDynamicBuffer(nullptr, totalGpuBufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VulkanBuffer& buffer = bufferSystem.FindVulkanBuffer(GpuDataBufferIndex);
    MappedBufferPtr = buffer.BufferData;

    // Initialize scene data buffer
    SceneDataBuffer sceneData = {};
    SceneDataBufferIndex = bufferSystem.CreateDynamicBuffer(&sceneData, sizeof(SceneDataBuffer),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VulkanBuffer& sceneDataBuffer = bufferSystem.FindVulkanBuffer(SceneDataBufferIndex);
    SceneDataPtr = sceneDataBuffer.BufferData;

    // Flush initial data
    vmaFlushAllocation(bufferSystem.vmaAllocator, buffer.Allocation, 0, totalGpuBufferSize);
    vmaFlushAllocation(bufferSystem.vmaAllocator, sceneDataBuffer.Allocation, 0, sizeof(SceneDataBuffer));

    CreateGlobalBindlessDescriptorSet();

    std::cout << "MemoryPoolSystem::StartUp() completed successfully\n";
}

void MemoryPoolSystem::ResizeMemoryPool(MemoryPoolTypes memoryPoolToUpdate, uint32 resizeCount)
{
    void* oldMappedPtr = MappedBufferPtr;
    uint32 oldBufferId = GpuDataBufferIndex;
    auto oldSubHeaders = MemorySubPoolHeader;

    UpdateMemoryPoolHeader(memoryPoolToUpdate, resizeCount);
    size_t newTotalSize = sizeof(MemoryPoolBufferHeader) + GpuDataBufferMemoryPoolSize;
    uint32 newBufferId = bufferSystem.CreateDynamicBuffer(nullptr, newTotalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VulkanBuffer& newBuf = bufferSystem.FindVulkanBuffer(newBufferId);
    MappedBufferPtr = newBuf.BufferData;

    std::memset(static_cast<byte*>(MappedBufferPtr), 0xFF, newTotalSize);
    memcpy(MappedBufferPtr, &GpuDataMemoryPoolHeader, sizeof(MemoryPoolBufferHeader));
    for (auto& [type, sub] : MemorySubPoolHeader)
    {
        const auto& oldSub = oldSubHeaders[type];
        size_t bytesToCopy = oldSub.ActiveCount * oldSub.Size;

        if (bytesToCopy > 0)
        {
            byte* dst = static_cast<byte*>(MappedBufferPtr) + sub.Offset;
            byte* src = static_cast<byte*>(oldMappedPtr) + oldSub.Offset;
            memcpy(dst, src, bytesToCopy);
        }
    }
    vmaFlushAllocation(bufferSystem.vmaAllocator, newBuf.Allocation, 0, newTotalSize);
    if (oldBufferId != UINT32_MAX)
    {
        bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(oldBufferId));
    }

    GpuDataBufferIndex = newBufferId;
    IsDescriptorSetDirty = true;
    IsHeaderDirty = true;
}

void MemoryPoolSystem::CreateGlobalBindlessDescriptorSet()
{
    VkDescriptorBufferInfo  sceneDataBuffer = VkDescriptorBufferInfo
    {
        .buffer = bufferSystem.FindVulkanBuffer(memoryPoolSystem.SceneDataBufferIndex).Buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };

    VkDescriptorBufferInfo  bindlessDataBuffer = VkDescriptorBufferInfo
    {
        .buffer = bufferSystem.FindVulkanBuffer(memoryPoolSystem.GpuDataBufferIndex).Buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };

    Vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 512},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 512},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Texture2DInitialCapacity + Texture3DInitialCapacity + TextureCubeMapInitialCapacity + 1024},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 64}
    };

    Vector<VkDescriptorSetLayoutBinding> bindings =
    {
        { SceneDataDescriptorBinding   , VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1,                             VK_SHADER_STAGE_ALL},
        { BindlessDataDescriptorBinding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1,                             VK_SHADER_STAGE_ALL},
        { CubeMapDescriptorBinding     , VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, TextureCubeMapInitialCapacity, VK_SHADER_STAGE_ALL},
        { Texture2DBinding             , VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Texture2DInitialCapacity,      VK_SHADER_STAGE_ALL},
        { Texture3DBinding             , VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Texture3DInitialCapacity,      VK_SHADER_STAGE_ALL},
    };

    Vector<VkDescriptorBindingFlags> flags =
    {
        VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT },
        VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT },
        VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT },
        VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT },
        VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT }
    };

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = static_cast<uint32>(flags.size()),
        .pBindingFlags = flags.data()
    };

    VkDescriptorPoolCreateInfo poolInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 64,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorPool(vulkanSystem.Device, &poolInfo, nullptr, &memoryPoolSystem.GlobalBindlessPool));

    VkDescriptorSetLayoutCreateInfo layoutInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &flagsInfo,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorSetLayout(vulkanSystem.Device, &layoutInfo, nullptr, &memoryPoolSystem.GlobalBindlessDescriptorSetLayout));

    VkDescriptorSetAllocateInfo allocInfo = 
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = memoryPoolSystem.GlobalBindlessPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &memoryPoolSystem.GlobalBindlessDescriptorSetLayout
    };
    VULKAN_THROW_IF_FAIL(vkAllocateDescriptorSets(vulkanSystem.Device, &allocInfo, &memoryPoolSystem.GlobalBindlessDescriptorSet));

    Vector<VkWriteDescriptorSet> writeDescriptorSetList = Vector<VkWriteDescriptorSet>
    {
        VkWriteDescriptorSet
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = memoryPoolSystem.GlobalBindlessDescriptorSet,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &sceneDataBuffer,
            },

        VkWriteDescriptorSet
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = memoryPoolSystem.GlobalBindlessDescriptorSet,
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .pBufferInfo = &bindlessDataBuffer,
            }
    };
    vkUpdateDescriptorSets(vulkanSystem.Device, static_cast<uint32>(writeDescriptorSetList.size()), writeDescriptorSetList.data(), 0, nullptr);
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
    if (IsSceneBufferDirty)
    {
        vmaFlushAllocation(bufferSystem.vmaAllocator, bufferSystem.FindVulkanBuffer(SceneDataBufferIndex).Allocation, 0, sizeof(SceneDataBuffer));
        IsSceneBufferDirty = false;
    }

    if (!MappedBufferPtr)
    {
        return;
    }

    VulkanBuffer& buffer = bufferSystem.FindVulkanBuffer(GpuDataBufferIndex);
    for (auto& [type, sub] : MemorySubPoolHeader)
    {
        if (sub.IsDirty)
        {
            size_t start = sizeof(MemoryPoolBufferHeader) + sub.Offset;
            size_t len = sub.ActiveCount * sub.Size;
            if (len > 0)
            {
                vmaFlushAllocation(bufferSystem.vmaAllocator, buffer.Allocation, start, len);
            }
            sub.IsDirty = false;
        }
    }

    if (IsHeaderDirty)
    {
        memcpy(MappedBufferPtr, &GpuDataMemoryPoolHeader, sizeof(MemoryPoolBufferHeader));
        vmaFlushAllocation(bufferSystem.vmaAllocator, buffer.Allocation, 0, sizeof(MemoryPoolBufferHeader));
        IsHeaderDirty = false;
    }

    if (IsDescriptorSetDirty)
    {
        UpdateDataBufferDescriptorSet(GpuDataBufferIndex, BindlessDataDescriptorBinding);
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
        .TextureCubeMapSize = MemorySubPoolHeader[kTextureCubeMapMetadataBuffer].Size,
        .SpriteInstanceOffset = MemorySubPoolHeader[kSpriteInstanceBuffer].Offset,
        .SpriteInstanceCount = MemorySubPoolHeader[kSpriteInstanceBuffer].Count,
        .SpriteInstanceSize = MemorySubPoolHeader[kSpriteInstanceBuffer].Size
    };
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

SpriteInstance& MemoryPoolSystem::UpdateSpriteInstance(uint32 index)
{
    MemoryPoolSubBufferHeader& spriteInstanceSubPool = MemorySubPoolHeader[kSpriteInstanceBuffer];
    if (index >= spriteInstanceSubPool.Count) throw std::out_of_range("Sprite Instance index out of range: " + std::to_string(index) + " >= " + std::to_string(spriteInstanceSubPool.Count));
    if (index >= spriteInstanceSubPool.IsActive.size() || !spriteInstanceSubPool.IsActive[index]) throw std::runtime_error("Sprite Instance slot inactive at index " + std::to_string(index));

    uint32 offset = spriteInstanceSubPool.Offset + (index * sizeof(SpriteInstance));
    spriteInstanceSubPool.IsDirty = true;
    return *reinterpret_cast<SpriteInstance*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

SceneDataBuffer& MemoryPoolSystem::UpdateSceneDataBuffer()
{
    IsSceneBufferDirty = true;
    return *reinterpret_cast<SceneDataBuffer*>(SceneDataPtr);
}

Vector<SpriteInstance*>	MemoryPoolSystem::GetActiveSpriteInstancePointers()
{
    const auto& sub = MemorySubPoolHeader[kSpriteInstanceBuffer];
    if (sub.ActiveCount == 0 || !MappedBufferPtr)
    {
        return {};
    }

    Vector<SpriteInstance*> pointers;
    pointers.reserve(sub.ActiveCount);
    const byte* base = static_cast<const byte*>(MappedBufferPtr) + sub.Offset;
    for (uint32 x = 0; x < sub.Count; ++x)
    {
        if (sub.IsActive[x])
        {
            SpriteInstance* ptr = reinterpret_cast<SpriteInstance*>(const_cast<byte*>(base) + x * sub.Size);
            pointers.push_back(ptr);
        }
    }
    MemorySubPoolHeader[kSpriteInstanceBuffer].IsDirty = true;
    return pointers;
}

Vector<MeshPropertiesStruct> MemoryPoolSystem::MeshBufferList()
{
    const auto& sub = MemorySubPoolHeader[kMeshBuffer];
    if (sub.ActiveCount == 0 || !MappedBufferPtr)
    {
        return {};
    }

    Vector<MeshPropertiesStruct> result(sub.ActiveCount);
    const byte* src = static_cast<const byte*>(MappedBufferPtr) + sub.Offset;
    std::memcpy(result.data(), src, sub.ActiveCount * sizeof(MeshPropertiesStruct));
    return result;
}

Vector<GPUMaterial> MemoryPoolSystem::MaterialBufferList()
{
    const auto& sub = MemorySubPoolHeader[kMaterialBuffer];
    if (sub.ActiveCount == 0 || !MappedBufferPtr)
    {
        return {};
    }

    Vector<GPUMaterial> result(sub.ActiveCount);
    const byte* src = static_cast<const byte*>(MappedBufferPtr) + sub.Offset;
    std::memcpy(result.data(), src, sub.ActiveCount * sizeof(GPUMaterial));
    return result;
}

Vector<DirectionalLight> MemoryPoolSystem::DirectionalLightBufferList()
{
    const auto& sub = MemorySubPoolHeader[kDirectionalLightBuffer];
    if (sub.ActiveCount == 0 || !MappedBufferPtr)
    {
        return {};
    }

    Vector<DirectionalLight> result(sub.ActiveCount);
    const byte* src = static_cast<const byte*>(MappedBufferPtr) + sub.Offset;
    std::memcpy(result.data(), src, sub.ActiveCount * sizeof(DirectionalLight));
    return result;
}

Vector<PointLight> MemoryPoolSystem::PointLightBufferList()
{
    const auto& sub = MemorySubPoolHeader[kPointLightBuffer];
    if (sub.ActiveCount == 0 || !MappedBufferPtr)
    {
        return {};
    }

    Vector<PointLight> result(sub.ActiveCount);
    const byte* src = static_cast<const byte*>(MappedBufferPtr) + sub.Offset;
    std::memcpy(result.data(), src, sub.ActiveCount * sizeof(PointLight));
    return result;
}

Vector<SpriteInstance> MemoryPoolSystem::SpriteInstanceBufferList()
{
    const auto& sub = MemorySubPoolHeader[kSpriteInstanceBuffer];
    if (sub.ActiveCount == 0 || !MappedBufferPtr)
    {
        return {};
    }

    Vector<SpriteInstance> result(sub.ActiveCount);
    const byte* src = static_cast<const byte*>(MappedBufferPtr) + sub.Offset;
    std::memcpy(result.data(), src, sub.ActiveCount * sizeof(SpriteInstance));
    return result;
}

void MemoryPoolSystem::FreeObject(MemoryPoolTypes memoryPoolToUpdate, uint32 index)
{
    MemoryPoolSubBufferHeader& sub = MemorySubPoolHeader[memoryPoolToUpdate];
    if (index >= sub.Count || !sub.IsActive[index])
    {
        return;
    }

    sub.IsActive[index] = 0x00;
    sub.FreeIndices.push_back(index);
    sub.IsDirty = true;
    while (sub.ActiveCount > 0 && sub.IsActive[sub.ActiveCount - 1] == 0)
    {
        sub.ActiveCount--;
    }
}

void MemoryPoolSystem::UpdateTextureDescriptorSet(Texture& texture, uint binding)
{
    VkDescriptorImageInfo textureUpdate = VkDescriptorImageInfo
    {
        .sampler = texture.textureSampler,
        .imageView = texture.textureViewList.front(),
        .imageLayout = texture.colorChannels == ColorChannelUsed::ChannelR ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet descriptorUpdate = VkWriteDescriptorSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = memoryPoolSystem.GlobalBindlessDescriptorSet,
        .dstBinding = binding,
        .dstArrayElement = static_cast<uint32>(texture.bindlessTextureIndex),
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &textureUpdate,
    };
    vkUpdateDescriptorSets(vulkanSystem.Device, 1, &descriptorUpdate, 0, nullptr);
}

void MemoryPoolSystem::UpdateDataBufferDescriptorSet(uint32 vulkanBufferIndex, uint binding)
{
    VkDescriptorBufferInfo bufferUpdate = VkDescriptorBufferInfo
    {
        .buffer = bufferSystem.FindVulkanBuffer(vulkanBufferIndex).Buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };

    VkWriteDescriptorSet descriptorUpdate = VkWriteDescriptorSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = memoryPoolSystem.GlobalBindlessDescriptorSet,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &bufferUpdate,
    };
    vkUpdateDescriptorSets(vulkanSystem.Device, 1, &descriptorUpdate, 0, nullptr);
}

const MemoryPoolSubBufferHeader MemoryPoolSystem::MemoryPoolSubBufferInfo(MemoryPoolTypes memoryPoolType)
{
    return MemorySubPoolHeader[memoryPoolType];
}

const Vector<VkDescriptorBufferInfo> MemoryPoolSystem::GetSceneDataBufferDescriptor() const
{
    return Vector<VkDescriptorBufferInfo>
    {
        VkDescriptorBufferInfo
        {
            .buffer = bufferSystem.FindVulkanBuffer(SceneDataBufferIndex).Buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE
        }
    };
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

const Vector<VkDescriptorImageInfo> MemoryPoolSystem::GetSubPassInputTextureDescriptor(VkGuid& renderPassId) const
{
    Vector<VkDescriptorImageInfo> descriptorSetInfoList;
    Vector<Texture> inputTextureList = textureSystem.FindRenderedTextureList(renderPassId);
    for (auto& texture : inputTextureList)
    {
        descriptorSetInfoList.emplace_back(VkDescriptorImageInfo
            {
                .sampler = texture.textureSampler,
                .imageView = texture.textureViewList.front(),
                .imageLayout = texture.textureImageLayout
            });
    }
    return descriptorSetInfoList;
}