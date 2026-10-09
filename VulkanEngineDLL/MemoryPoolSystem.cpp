#include "MemoryPoolSystem.h"
#include "MeshSystem.h"
#include "MaterialSystem.h"
#include "LightSystem.h"
#include "RenderSystem.h"

MemoryPoolSystem& memoryPoolSystem = MemoryPoolSystem::Get();

void MemoryPoolSystem::StartUp()
{
    std::cout << "MemoryPoolSystem::StartUp() started\n";
    for (int x = 0; x < static_cast<int>(MemoryPoolTypes::kEndofPool); x++)
    {
        MemoryPoolTypes type = (MemoryPoolTypes)x;
        switch (x)
        {
        case MemoryPoolTypes::kMeshBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Capacity = MeshInitialCapacity,
                .Size = sizeof(MeshPropertiesStruct),
                .IsSlotActive = Vector<byte>(MeshInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;
        case MemoryPoolTypes::kMaterialBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Capacity = MaterialInitialCapacity,
                .Size = sizeof(GPUMaterial),
                .IsSlotActive = Vector<byte>(MaterialInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;
        case MemoryPoolTypes::kDirectionalLightBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Capacity = DirectionalLightInitialCapacity,
                .Size = sizeof(DirectionalLight),
                .IsSlotActive = Vector<byte>(DirectionalLightInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;
        case MemoryPoolTypes::kPointLightBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Capacity = PointLightInitialCapacity,
                .Size = sizeof(PointLight),
                .IsSlotActive = Vector<byte>(PointLightInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;
        case MemoryPoolTypes::kTexture2DMetadataBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Capacity = Texture2DInitialCapacity,
                .Size = sizeof(TextureMetadataHeader),
                .IsSlotActive = Vector<byte>(Texture2DInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;
        case MemoryPoolTypes::kTexture3DMetadataBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Capacity = Texture3DInitialCapacity,
                .Size = sizeof(TextureMetadataHeader),
                .IsSlotActive = Vector<byte>(Texture3DInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;
        case MemoryPoolTypes::kTextureCubeMapMetadataBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Capacity = TextureCubeMapInitialCapacity,
                .Size = sizeof(TextureMetadataHeader),
                .IsSlotActive = Vector<byte>(TextureCubeMapInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;
        case MemoryPoolTypes::kSpriteInstanceBuffer:
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader{
                .ActiveCount = 0,
                .Capacity = SpriteInstanceInitialCapacity,
                .Size = sizeof(SpriteInstance),
                .IsSlotActive = Vector<byte>(SpriteInstanceInitialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;
        }
    }

    UpdateMemoryPoolHeader(MemoryPoolTypes::kMeshBuffer, MeshInitialCapacity);

    size_t totalGpuBufferSize = GpuDataBufferMemoryPoolSize;
    GpuDataBufferIndex = bufferSystem.CreateDynamicBuffer(nullptr, totalGpuBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VulkanBuffer& buffer = bufferSystem.FindVulkanBuffer(GpuDataBufferIndex);
    MappedBufferPtr = buffer.BufferMappedData();
    memcpy(MappedBufferPtr, &GpuDataMemoryPoolHeader, sizeof(MemoryPoolBufferHeader));

    SceneDataBuffer sceneData = {};
    SceneDataBufferIndex = bufferSystem.CreateDynamicBuffer(&sceneData, sizeof(SceneDataBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VulkanBuffer& sceneDataBuffer = bufferSystem.FindVulkanBuffer(SceneDataBufferIndex);
    SceneDataPtr = sceneDataBuffer.BufferMappedData();

    vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), buffer.BufferAllocation(), 0, totalGpuBufferSize);
    vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), sceneDataBuffer.BufferAllocation(), 0, sizeof(SceneDataBuffer));
    CreateGlobalBindlessDescriptorSet();
}

void MemoryPoolSystem::ResizeMemoryPool(MemoryPoolTypes memoryPoolToUpdate, uint32 resizeCount)
{
    void* oldMappedPtr = MappedBufferPtr;
    uint32 oldBufferId = GpuDataBufferIndex;
    auto oldSubHeaders = MemorySubPoolHeader;

    UpdateMemoryPoolHeader(memoryPoolToUpdate, resizeCount);
    size_t newTotalSize = GpuDataBufferMemoryPoolSize;
    uint32 newBufferId = bufferSystem.CreateDynamicBuffer(nullptr, newTotalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VulkanBuffer& newBuf = bufferSystem.FindVulkanBuffer(newBufferId);
    MappedBufferPtr = newBuf.BufferMappedData();

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
    vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), newBuf.BufferAllocation(), 0, newTotalSize);
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
        .buffer = bufferSystem.FindVulkanBuffer(memoryPoolSystem.SceneDataBufferIndex).Buffer(),
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };

    VkDescriptorBufferInfo  bindlessDataBuffer = VkDescriptorBufferInfo
    {
        .buffer = bufferSystem.FindVulkanBuffer(memoryPoolSystem.GpuDataBufferIndex).Buffer(),
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
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorPool(vulkan.LogicalDevice(), &poolInfo, nullptr, &memoryPoolSystem.GlobalBindlessPool));

    VkDescriptorSetLayoutCreateInfo layoutInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &flagsInfo,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorSetLayout(vulkan.LogicalDevice(), &layoutInfo, nullptr, &memoryPoolSystem.GlobalBindlessDescriptorSetLayout));

    VkDescriptorSetAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = memoryPoolSystem.GlobalBindlessPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &memoryPoolSystem.GlobalBindlessDescriptorSetLayout
    };
    VULKAN_THROW_IF_FAIL(vkAllocateDescriptorSets(vulkan.LogicalDevice(), &allocInfo, &memoryPoolSystem.GlobalBindlessDescriptorSet));

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
    vkUpdateDescriptorSets(vulkan.LogicalDevice(), static_cast<uint32>(writeDescriptorSetList.size()), writeDescriptorSetList.data(), 0, nullptr);
}

void MemoryPoolSystem::SwapSpriteInstanceMemoryPoolElement(uint32 oldSlotIndex, uint32 newSlotIndex)
{
    SwapMemoryPoolElement(MemoryPoolTypes::kSpriteInstanceBuffer, oldSlotIndex, newSlotIndex);

    MemoryPoolSubBufferHeader& memoryPoolSubPool = MemorySubPoolHeader[kSpriteInstanceBuffer];
    auto UpdateFreeIndices = [&memoryPoolSubPool](uint index)
        {
            auto it = std::find(memoryPoolSubPool.FreeIndices.begin(), memoryPoolSubPool.FreeIndices.end(), index);
            if (memoryPoolSubPool.IsSlotActive[index])
            {
                if (it != memoryPoolSubPool.FreeIndices.end()) memoryPoolSubPool.FreeIndices.erase(it);
            }
            else if (it == memoryPoolSubPool.FreeIndices.end()) memoryPoolSubPool.FreeIndices.emplace_back(index);
        };

    SpriteInstance& spriteInstance1 = UpdateSpriteInstance(oldSlotIndex);
    SpriteInstance& spriteInstance2 = UpdateSpriteInstance(newSlotIndex);

    std::swap(spriteInstance1.SpriteId, spriteInstance2.SpriteId);
    std::swap(memoryPoolSubPool.IsSlotActive[oldSlotIndex], memoryPoolSubPool.IsSlotActive[newSlotIndex]);

    UpdateFreeIndices(oldSlotIndex);
    UpdateFreeIndices(newSlotIndex);
}

void MemoryPoolSystem::SortSpriteInstancePool()
{
    auto& pool = MemorySubPoolHeader[kSpriteInstanceBuffer];
    if (!MappedBufferPtr || pool.ActiveCount == 0) return;

    byte* base = static_cast<byte*>(MappedBufferPtr) + pool.Offset;
    auto spriteAt = [&](uint32 slot) -> const SpriteInstance&
        {
            return *reinterpret_cast<const SpriteInstance*>(base + slot * pool.Size);
        };
    auto hasAlpha = [&](uint32 slot)
        {
            const SpriteInstance& sprite = spriteAt(slot);
            const Material& material = materialSystem.FindMaterial(materialSystem.FindMemoryPoolIndexByGuid(sprite.MaterialId));
            return (material.FeatureMask & MaterialPropertiesEnum::kMaterialFeature_UsingAlpha) != 0;
        };

    Vector<uint32> slots;
    slots.reserve(pool.ActiveCount);
    for (uint32 x = 0; x < pool.IsSlotActive.size(); ++x)
    {
        if (pool.IsSlotActive[x]) slots.push_back(x);
    }

    std::stable_sort(slots.begin(), slots.end(), [&](uint32 a, uint32 b)
        {
            return !hasAlpha(a) && hasAlpha(b);
        });

    const uint32 slotSize = static_cast<uint32>(slots.size());
    Vector<byte> packed(slotSize * pool.Size);
    for (uint32 x = 0; x < slotSize; ++x)
    {
        std::memcpy(packed.data() + x * pool.Size, base + slots[x] * pool.Size, pool.Size);
    }
    if (slotSize > 0) std::memcpy(base, packed.data(), slotSize * pool.Size);

    std::fill(pool.IsSlotActive.begin(), pool.IsSlotActive.end(), byte{ 0x00 });
    pool.FreeIndices.clear();
    for (uint32 x = 0; x < slotSize; ++x)
    {
        pool.IsSlotActive[x] = 1;
    }
    for (uint32 x = slotSize; x < pool.Capacity; ++x)
    {
        pool.FreeIndices.push_back(pool.Capacity - 1 - x); 
    }
    pool.ActiveCount = slotSize;
    pool.IsDirty = true;
    RefreshGpuHeaderCounts();
}

uint32 MemoryPoolSystem::FindFirstAlphaSpriteIndex()
{
    auto& pool = MemorySubPoolHeader[kSpriteInstanceBuffer];
    if (!MappedBufferPtr || pool.ActiveCount == 0) return pool.ActiveCount; 

    const byte* base = static_cast<const byte*>(MappedBufferPtr) + pool.Offset;
    const uint32 limit = std::min(pool.ActiveCount, static_cast<uint32>(pool.IsSlotActive.size()));

    for (uint32 x = 0; x < limit; ++x)
    {
        if (!pool.IsSlotActive[x]) continue;

        const SpriteInstance& sprite = *reinterpret_cast<const SpriteInstance*>(base + x * pool.Size);
        const Material& material = materialSystem.FindMaterial( materialSystem.FindMemoryPoolIndexByGuid(sprite.MaterialId));
        if ((material.FeatureMask & MaterialPropertiesEnum::kMaterialFeature_UsingAlpha) != 0) return x;
    }
    return pool.ActiveCount;
}

void MemoryPoolSystem::SwapMemoryPoolElement(MemoryPoolTypes memoryPoolType, uint32 oldSlotIndex, uint32 newSlotIndex)
{
    if (oldSlotIndex == newSlotIndex) return;

    MemoryPoolSubBufferHeader& memoryPoolSubPool = MemorySubPoolHeader[memoryPoolType];
    if (oldSlotIndex >= memoryPoolSubPool.Capacity || newSlotIndex >= memoryPoolSubPool.Capacity) throw std::out_of_range("slot out of range");

    uint32 oldOffset = memoryPoolSubPool.Offset + (oldSlotIndex * memoryPoolSubPool.Size);
    uint32 newOffset = memoryPoolSubPool.Offset + (newSlotIndex * memoryPoolSubPool.Size);

    void* swapCopyPtr = memorySystem.AddPtrBuffer<void*>(memoryPoolSubPool.Size, __FILE__, __LINE__, __func__);
    std::memcpy(swapCopyPtr, static_cast<byte*>(MappedBufferPtr) + newOffset, memoryPoolSubPool.Size);
    std::memcpy(static_cast<byte*>(MappedBufferPtr) + newOffset, static_cast<byte*>(MappedBufferPtr) + oldOffset, memoryPoolSubPool.Size);
    std::memcpy(static_cast<byte*>(MappedBufferPtr) + oldOffset, swapCopyPtr, memoryPoolSubPool.Size);
    memorySystem.DeletePtr(swapCopyPtr);

    memoryPoolSubPool.IsDirty = true;
}

uint32 MemoryPoolSystem::AllocateObject(MemoryPoolTypes memoryPoolToUpdate)
{
    MemoryPoolSubBufferHeader& subPoolHeader = MemorySubPoolHeader[memoryPoolToUpdate];

    if (!subPoolHeader.FreeIndices.empty())
    {
        uint32 index = subPoolHeader.FreeIndices.back();
        subPoolHeader.FreeIndices.pop_back();
        subPoolHeader.IsSlotActive[index] = 1;

        if (index + 1 > subPoolHeader.ActiveCount) subPoolHeader.ActiveCount = index + 1;
        subPoolHeader.IsDirty = true;
        RefreshGpuHeaderCounts();
        return index;
    }

    if (subPoolHeader.ActiveCount == subPoolHeader.Capacity)
        ResizeMemoryPool(memoryPoolToUpdate, subPoolHeader.Capacity * 2);

    uint32 index = subPoolHeader.ActiveCount++;
    subPoolHeader.IsSlotActive[index] = 0x01;
    subPoolHeader.IsDirty = true;
    RefreshGpuHeaderCounts();
    return index;
}

void MemoryPoolSystem::UpdateMemoryPool()
{
    if (IsSceneBufferDirty)
    {
        vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(),
            bufferSystem.FindVulkanBuffer(SceneDataBufferIndex).BufferAllocation(),
            0, sizeof(SceneDataBuffer));
        IsSceneBufferDirty = false;
    }
    if (!MappedBufferPtr) return;

    VulkanBuffer& buffer = bufferSystem.FindVulkanBuffer(GpuDataBufferIndex);
    const VkDeviceSize atom = vulkan.Device().GetPhysicalDeviceProperties(vulkan.PhysicalDevice()).limits.nonCoherentAtomSize;
    const VkDeviceSize allocationSize = GpuDataBufferMemoryPoolSize;

    auto flushRange = [&](VkDeviceSize start, VkDeviceSize len)
        {
            if (len == 0 || start >= allocationSize) return;

            VkDeviceSize end = std::min(start + len, allocationSize);
            VkDeviceSize alignedStart = start & ~(atom - 1);
            VkDeviceSize alignedEnd = (end + atom - 1) & ~(atom - 1);
            alignedEnd = std::min(alignedEnd, (allocationSize + atom - 1) & ~(atom - 1));
            vmaFlushAllocation(bufferSystem.VmaAllocatorHandle(), buffer.BufferAllocation(), alignedStart, alignedEnd - alignedStart);
        };

    for (auto& [type, sub] : MemorySubPoolHeader)
    {
        if (!sub.IsDirty) continue;
        flushRange(sub.Offset, static_cast<VkDeviceSize>(sub.Capacity) * sub.Size);
        sub.IsDirty = false;
    }

    if (IsHeaderDirty)
    {
        memcpy(MappedBufferPtr, &GpuDataMemoryPoolHeader, sizeof(MemoryPoolBufferHeader));
        flushRange(0, sizeof(MemoryPoolBufferHeader));
        IsHeaderDirty = false;
    }

    if (IsDescriptorSetDirty)
    {
        UpdateDataBufferDescriptorSet(GpuDataBufferIndex, BindlessDataDescriptorBinding);
        IsDescriptorSetDirty = false;
    }
}

void MemoryPoolSystem::UpdateTextureDescriptorSet(uint32 textureGpuBufferIndex, VulkanTexture& texture, uint binding)
{
    VkDescriptorImageInfo textureUpdate = {
          .sampler = texture.TextureSampler(),
          .imageView = texture.TextureViews().front(),
          .imageLayout = texture.m_colorChannels == ColorChannelEnum::ChannelR ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet descriptorUpdate = VkWriteDescriptorSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = memoryPoolSystem.GlobalBindlessDescriptorSet,
        .dstBinding = binding,
        .dstArrayElement = textureGpuBufferIndex,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &textureUpdate,
    };
    vkUpdateDescriptorSets(vulkan.LogicalDevice(), 1, &descriptorUpdate, 0, nullptr);
}

void MemoryPoolSystem::UpdateDataBufferDescriptorSet(uint32 vulkanGpuBufferIndex, uint binding)
{
    VkDescriptorBufferInfo bufferUpdate = VkDescriptorBufferInfo
    {
        .buffer = bufferSystem.FindVulkanBuffer(vulkanGpuBufferIndex).Buffer(),
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
    vkUpdateDescriptorSets(vulkan.LogicalDevice(), 1, &descriptorUpdate, 0, nullptr);
}

void MemoryPoolSystem::RefreshGpuHeaderCounts()
{
    GpuDataMemoryPoolHeader.MeshCount             = MemorySubPoolHeader[kMeshBuffer].ActiveCount;
    GpuDataMemoryPoolHeader.MaterialCount         = MemorySubPoolHeader[kMaterialBuffer].ActiveCount;
    GpuDataMemoryPoolHeader.DirectionalLightCount = MemorySubPoolHeader[kDirectionalLightBuffer].ActiveCount;
    GpuDataMemoryPoolHeader.PointLightCount       = MemorySubPoolHeader[kPointLightBuffer].ActiveCount;
    GpuDataMemoryPoolHeader.Texture2DCount        = MemorySubPoolHeader[kTexture2DMetadataBuffer].ActiveCount;
    GpuDataMemoryPoolHeader.Texture3DCount        = MemorySubPoolHeader[kTexture3DMetadataBuffer].ActiveCount;
    GpuDataMemoryPoolHeader.TextureCubeMapCount   = MemorySubPoolHeader[kTextureCubeMapMetadataBuffer].ActiveCount;
    GpuDataMemoryPoolHeader.SpriteInstanceCount   = MemorySubPoolHeader[kSpriteInstanceBuffer].ActiveCount;
    IsHeaderDirty = true;
}

void MemoryPoolSystem::UpdateMemoryPoolHeader(MemoryPoolTypes memoryPoolTypeToUpdate, uint32 newPoolSize)
{
    for (int x = static_cast<int>(memoryPoolTypeToUpdate); x < static_cast<int>(MemoryPoolTypes::kEndofPool); x++)
    {
        const MemoryPoolTypes memoryPoolType = (MemoryPoolTypes)x;
        const MemoryPoolTypes lastMemoryPoolType = (x == 0) ? MemoryPoolTypes::kEndofPool : (MemoryPoolTypes)(x - 1);
        const MemoryPoolSubBufferHeader oldMemoryPoolSubHeader = MemorySubPoolHeader[memoryPoolType];
        MemorySubPoolHeader[memoryPoolType] = MemoryPoolSubBufferHeader
        {
           .ActiveCount = MemorySubPoolHeader[memoryPoolType].ActiveCount,
           .Offset = lastMemoryPoolType == MemoryPoolTypes::kEndofPool ? sizeof(MemoryPoolBufferHeader) : MemorySubPoolHeader[lastMemoryPoolType].Offset + (MemorySubPoolHeader[lastMemoryPoolType].Capacity * MemorySubPoolHeader[lastMemoryPoolType].Size),
           .Capacity = memoryPoolType == memoryPoolTypeToUpdate ? newPoolSize : MemorySubPoolHeader[memoryPoolType].Capacity,
           .Size = MemorySubPoolHeader[memoryPoolType].Size,
           .IsSlotActive = memoryPoolType == memoryPoolTypeToUpdate ? Vector<byte>(newPoolSize, 0x00) : MemorySubPoolHeader[memoryPoolType].IsSlotActive,
           .FreeIndices = MemorySubPoolHeader[memoryPoolType].FreeIndices,
           .IsDirty = true
        };

        const uint32 bytesToCopy = std::min(oldMemoryPoolSubHeader.ActiveCount, static_cast<uint32>(MemorySubPoolHeader[memoryPoolType].IsSlotActive.size()));
        if (bytesToCopy > 0 && !oldMemoryPoolSubHeader.IsSlotActive.empty())
        {
            memcpy(MemorySubPoolHeader[memoryPoolType].IsSlotActive.data(), oldMemoryPoolSubHeader.IsSlotActive.data(), bytesToCopy);
        }
    }

    MemoryPoolSubBufferHeader lastHeader = MemorySubPoolHeader[(MemoryPoolTypes)((int)MemoryPoolTypes::kEndofPool - 1)];
    GpuDataBufferMemoryPoolSize = lastHeader.Offset + (lastHeader.Size * lastHeader.Capacity);
    GpuDataMemoryPoolHeader = MemoryPoolBufferHeader
    {
        .MeshOffset = MemorySubPoolHeader[kMeshBuffer].Offset,
        .MeshCount = MemorySubPoolHeader[kMeshBuffer].ActiveCount,
        .MeshSize = MemorySubPoolHeader[kMeshBuffer].Size,

        .MaterialOffset = MemorySubPoolHeader[kMaterialBuffer].Offset,
        .MaterialCount = MemorySubPoolHeader[kMaterialBuffer].ActiveCount,
        .MaterialSize = MemorySubPoolHeader[kMaterialBuffer].Size,

        .DirectionalLightOffset = MemorySubPoolHeader[kDirectionalLightBuffer].Offset,
        .DirectionalLightCount = MemorySubPoolHeader[kDirectionalLightBuffer].ActiveCount,
        .DirectionalLightSize = MemorySubPoolHeader[kDirectionalLightBuffer].Size,

        .PointLightOffset = MemorySubPoolHeader[kPointLightBuffer].Offset,
        .PointLightCount = MemorySubPoolHeader[kPointLightBuffer].ActiveCount,
        .PointLightSize = MemorySubPoolHeader[kPointLightBuffer].Size,

        .Texture2DOffset = MemorySubPoolHeader[kTexture2DMetadataBuffer].Offset,
        .Texture2DCount = MemorySubPoolHeader[kTexture2DMetadataBuffer].ActiveCount,
        .Texture2DSize = MemorySubPoolHeader[kTexture2DMetadataBuffer].Size,

        .Texture3DOffset = MemorySubPoolHeader[kTexture3DMetadataBuffer].Offset,
        .Texture3DCount = MemorySubPoolHeader[kTexture3DMetadataBuffer].ActiveCount,
        .Texture3DSize = MemorySubPoolHeader[kTexture3DMetadataBuffer].Size,

        .TextureCubeMapOffset = MemorySubPoolHeader[kTextureCubeMapMetadataBuffer].Offset,
        .TextureCubeMapCount = MemorySubPoolHeader[kTextureCubeMapMetadataBuffer].ActiveCount,
        .TextureCubeMapSize = MemorySubPoolHeader[kTextureCubeMapMetadataBuffer].Size,

        .SpriteInstanceOffset = MemorySubPoolHeader[kSpriteInstanceBuffer].Offset,
        .SpriteInstanceCount = MemorySubPoolHeader[kSpriteInstanceBuffer].ActiveCount,
        .SpriteInstanceSize = MemorySubPoolHeader[kSpriteInstanceBuffer].Size
    };
}

MeshPropertiesStruct& MemoryPoolSystem::UpdateMesh(uint32 index)
{
    MemoryPoolSubBufferHeader& meshSubPool = MemorySubPoolHeader[kMeshBuffer];
    if (index >= meshSubPool.Capacity) throw std::out_of_range("Mesh index out of range: " + std::to_string(index) + " >= " + std::to_string(meshSubPool.Capacity));
    if (index >= meshSubPool.IsSlotActive.size() || !meshSubPool.IsSlotActive[index]) throw std::runtime_error("Mesh slot inactive at index " + std::to_string(index));

    uint32 offset = meshSubPool.Offset + (index * sizeof(MeshPropertiesStruct));
    meshSubPool.IsDirty = true;
    auto a = reinterpret_cast<MeshPropertiesStruct*>(static_cast<byte*>(MappedBufferPtr) + offset);
    return *reinterpret_cast<MeshPropertiesStruct*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

GPUMaterial& MemoryPoolSystem::UpdateMaterial(uint32 index)
{
    MemoryPoolSubBufferHeader& materialSubPool = MemorySubPoolHeader[kMaterialBuffer];
    if (index >= materialSubPool.Capacity) throw std::out_of_range("Material index out of range: " + std::to_string(index) + " >= " + std::to_string(materialSubPool.Capacity));
    if (index >= materialSubPool.IsSlotActive.size() || !materialSubPool.IsSlotActive[index]) throw std::runtime_error("Material slot inactive at index " + std::to_string(index));

    uint32 offset = materialSubPool.Offset + (index * sizeof(GPUMaterial));
    materialSubPool.IsDirty = true;
    return *reinterpret_cast<GPUMaterial*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

DirectionalLight& MemoryPoolSystem::UpdateDirectionalLight(uint32 index)
{
    MemoryPoolSubBufferHeader& directionalLightSubPool = MemorySubPoolHeader[kDirectionalLightBuffer];
    if (index >= directionalLightSubPool.Capacity) throw std::out_of_range("Directional Light index out of range: " + std::to_string(index) + " >= " + std::to_string(directionalLightSubPool.Capacity));
    if (index >= directionalLightSubPool.IsSlotActive.size() || !directionalLightSubPool.IsSlotActive[index]) throw std::runtime_error("Directional Light slot inactive at index " + std::to_string(index));

    uint32 offset = directionalLightSubPool.Offset + (index * sizeof(DirectionalLight));
    directionalLightSubPool.IsDirty = true;
    return *reinterpret_cast<DirectionalLight*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

PointLight& MemoryPoolSystem::UpdatePointLight(uint32 index)
{
    MemoryPoolSubBufferHeader& pointLightSubPool = MemorySubPoolHeader[kPointLightBuffer];
    if (index >= pointLightSubPool.Capacity) throw std::out_of_range("Point Light index out of range: " + std::to_string(index) + " >= " + std::to_string(pointLightSubPool.Capacity));
    if (index >= pointLightSubPool.IsSlotActive.size() || !pointLightSubPool.IsSlotActive[index]) throw std::runtime_error("Point Light slot inactive at index " + std::to_string(index));

    uint32 offset = pointLightSubPool.Offset + (index * sizeof(PointLight));
    pointLightSubPool.IsDirty = true;
    return *reinterpret_cast<PointLight*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

TextureMetadataHeader& MemoryPoolSystem::UpdateTexture2DMetadataHeader(uint32 index)
{
    MemoryPoolSubBufferHeader& sub = MemorySubPoolHeader[kTexture2DMetadataBuffer];
    if (index >= sub.Capacity || index >= sub.IsSlotActive.size() || !sub.IsSlotActive[index]) throw std::runtime_error("Invalid texture metadata index");

    uint32 offset = sub.Offset + (index * sizeof(TextureMetadataHeader));
    sub.IsDirty = true;
    return *reinterpret_cast<TextureMetadataHeader*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

TextureMetadataHeader& MemoryPoolSystem::UpdateTexture3DMetadataHeader(uint32 index)
{
    MemoryPoolSubBufferHeader& sub = MemorySubPoolHeader[kTexture3DMetadataBuffer];
    if (index >= sub.Capacity || index >= sub.IsSlotActive.size() || !sub.IsSlotActive[index]) throw std::runtime_error("Invalid texture metadata index");

    uint32 offset = sub.Offset + (index * sizeof(TextureMetadataHeader));
    sub.IsDirty = true;
    return *reinterpret_cast<TextureMetadataHeader*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

TextureMetadataHeader& MemoryPoolSystem::UpdateTextureCubeMapMetadataHeader(uint32 index)
{
    MemoryPoolSubBufferHeader& sub = MemorySubPoolHeader[kTextureCubeMapMetadataBuffer];
    if (index >= sub.Capacity || index >= sub.IsSlotActive.size() || !sub.IsSlotActive[index]) throw std::runtime_error("Invalid texture metadata index");

    uint32 offset = sub.Offset + (index * sizeof(TextureMetadataHeader));
    sub.IsDirty = true;
    return *reinterpret_cast<TextureMetadataHeader*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

SpriteInstance& MemoryPoolSystem::UpdateSpriteInstance(uint32 index)
{
    MemoryPoolSubBufferHeader& spriteInstanceSubPool = MemorySubPoolHeader[kSpriteInstanceBuffer];
    if (index >= spriteInstanceSubPool.Capacity) throw std::out_of_range("Sprite Instance index out of range: " + std::to_string(index) + " >= " + std::to_string(spriteInstanceSubPool.Capacity));
    if (index >= spriteInstanceSubPool.IsSlotActive.size() || !spriteInstanceSubPool.IsSlotActive[index]) throw std::runtime_error("Sprite Instance slot inactive at index " + std::to_string(index));

    uint32 offset = spriteInstanceSubPool.Offset + (index * sizeof(SpriteInstance));
    spriteInstanceSubPool.IsDirty = true;
    return *reinterpret_cast<SpriteInstance*>(static_cast<byte*>(MappedBufferPtr) + offset);
}

SceneDataBuffer& MemoryPoolSystem::UpdateSceneDataBuffer()
{
    IsSceneBufferDirty = true;
    return *reinterpret_cast<SceneDataBuffer*>(SceneDataPtr);
}

uint MemoryPoolSystem::FindDirectionalLightIndex(void* ptr)
{
    MemoryPoolSubBufferHeader& directionalLightSubPool = MemorySubPoolHeader[kDirectionalLightBuffer];
    for (int x = 0; x < directionalLightSubPool.ActiveCount; x++)
    {
        uint32 offset = directionalLightSubPool.Offset + (x * sizeof(DirectionalLight));
        void* directionalLightAddress = reinterpret_cast<void*>(static_cast<byte*>(MappedBufferPtr) + offset);
        if (directionalLightAddress == ptr) return x;
    }
    return UINT32_MAX;
}

uint MemoryPoolSystem::FindPointLightIndex(void* ptr)
{
    MemoryPoolSubBufferHeader& pointLightSubPool = MemorySubPoolHeader[kPointLightBuffer];
    for (int x = 0; x < pointLightSubPool.ActiveCount; x++)
    {
        uint32 offset = pointLightSubPool.Offset + (x * sizeof(PointLight));
        void* pointLightAddress = reinterpret_cast<void*>(static_cast<byte*>(MappedBufferPtr) + offset);
        if (pointLightAddress == ptr) return x;
    }
    return UINT32_MAX;
}

uint32 MemoryPoolSystem::AddToMemoryPool(VulkanTexture& texture)
{
    if (texture.IsCubeMap())
    {
        uint32 gpuTextureIndex = memoryPoolSystem.AllocateObject(kTextureCubeMapMetadataBuffer);
        TextureMetadataHeader& textureMetaDataHeader = memoryPoolSystem.UpdateTextureCubeMapMetadataHeader(gpuTextureIndex);
        textureMetaDataHeader.Width = texture.TextureSize().x;
        textureMetaDataHeader.Height = texture.TextureSize().y;
        textureMetaDataHeader.Depth = texture.TextureSize().z;
        textureMetaDataHeader.MipLevels = texture.MipMapLevels();
        textureMetaDataHeader.LayerCount = texture.TextureArrayLayers();
        textureMetaDataHeader.Format = (uint32)texture.TextureImageLayout();
        textureMetaDataHeader.Type = 1;
        memoryPoolSystem.UpdateTextureDescriptorSet(gpuTextureIndex, texture, memoryPoolSystem.CubeMapDescriptorBinding);
        return gpuTextureIndex;
    }
    else
    {
        uint32 gpuTextureIndex = memoryPoolSystem.AllocateObject(kTexture2DMetadataBuffer);
        TextureMetadataHeader& textureMetaDataHeader = memoryPoolSystem.UpdateTexture2DMetadataHeader(gpuTextureIndex);
        textureMetaDataHeader.Width = texture.TextureSize().x;
        textureMetaDataHeader.Height = texture.TextureSize().y;
        textureMetaDataHeader.Depth = texture.TextureSize().z;
        textureMetaDataHeader.MipLevels = texture.MipMapLevels();
        textureMetaDataHeader.LayerCount = texture.TextureArrayLayers();
        textureMetaDataHeader.Format = (uint32)texture.TextureImageLayout();
        textureMetaDataHeader.Type = 0;
        memoryPoolSystem.UpdateTextureDescriptorSet(gpuTextureIndex, texture, memoryPoolSystem.Texture2DBinding);
        return gpuTextureIndex;
    }
}

Vector<SpriteInstance*> MemoryPoolSystem::GetActiveSpriteInstancePointers()
{
    auto& sub = MemorySubPoolHeader[kSpriteInstanceBuffer];
    if (sub.ActiveCount == 0 || !MappedBufferPtr) return {};

    Vector<SpriteInstance*> pointers;
    pointers.reserve(sub.ActiveCount);
    byte* base = static_cast<byte*>(MappedBufferPtr) + sub.Offset;
    for (uint32 x = 0; x < sub.Capacity; ++x)
    {
        if (sub.IsSlotActive[x])
        {
            pointers.push_back(reinterpret_cast<SpriteInstance*>(base + x * sub.Size));
        }
    }
    sub.IsDirty = true;
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

void MemoryPoolSystem::ResetMemoryPool()
{
    GpuDataBufferMemoryPoolSize = UINT32_MAX;
    MemorySubPoolHeader.clear();
    GpuDataMemoryPoolHeader = MemoryPoolBufferHeader();
    GpuDataBufferMemoryPool.clear();
}

void MemoryPoolSystem::FreeObject(MemoryPoolTypes memoryPoolToUpdate, uint32 index)
{
    MemoryPoolSubBufferHeader& sub = MemorySubPoolHeader[memoryPoolToUpdate];
    if (index >= sub.Capacity || index >= sub.IsSlotActive.size() || !sub.IsSlotActive[index])
        return;

    sub.IsSlotActive[index] = 0x00;
    sub.FreeIndices.push_back(index);
    sub.IsDirty = true;

    while (sub.ActiveCount > 0 && sub.IsSlotActive[sub.ActiveCount - 1] == 0)
    {
        sub.ActiveCount--;
    }

    RefreshGpuHeaderCounts();
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
            .buffer = bufferSystem.FindVulkanBuffer(SceneDataBufferIndex).Buffer(),
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
            .buffer = bufferSystem.FindVulkanBuffer(GpuDataBufferIndex).Buffer(),
            .offset = 0,
            .range = VK_WHOLE_SIZE
        }
    };
}

const Vector<VkDescriptorImageInfo> MemoryPoolSystem::GetSubPassInputTextureDescriptor(VkGuid& renderPassId) const
{
    Vector<VkDescriptorImageInfo> descriptorSetInfoList;
    Vector<Texture> inputTextureList = renderSystem.FindRenderPassAttachmentList(renderPassId);
    for (auto& texture : inputTextureList)
    {
        descriptorSetInfoList.emplace_back(VkDescriptorImageInfo
            {
                .sampler = texture.texture.TextureSampler(),
                .imageView = texture.texture.TextureViews().front(),
                .imageLayout = texture.texture.TextureImageLayout()
            });
    }
    return descriptorSetInfoList;
}

const MemoryPoolLoader MemoryPoolSystem::GetMemoryPoolInfo()
{
    return MemoryPoolLoader
    {
        .GlobalBindlessPool = GlobalBindlessPool,
        .GlobalBindlessDescriptorSet = GlobalBindlessDescriptorSet,
        .GlobalBindlessDescriptorSetLayout = GlobalBindlessDescriptorSetLayout
    };
}
