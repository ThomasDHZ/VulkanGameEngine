#include "MaterialMemoryPoolSystem.h"

MaterialMemoryPoolSystem& materialMemoryPoolSystem = MaterialMemoryPoolSystem::Get();

void MaterialMemoryPoolSystem::StartUp()
{
    for (int x = 0; x < static_cast<int>(MaterialBakerMemoryPoolTypes::BakerEndofPool); x++)
    {
        MaterialBakerMemoryPoolTypes type = (MaterialBakerMemoryPoolTypes)x;
        switch (x)
        {
        case MaterialBakerMemoryPoolTypes::BakerMaterialBuffer:
        {
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader
            {
                .ActiveCount = 0,
                .Count = BakerMaterialCapacity,
                .Size = sizeof(ImportMaterialShader),
                .IsActive = Vector<byte>(BakerMaterialCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;
        }
        case MaterialBakerMemoryPoolTypes::BakerTexture2DMetadataBuffer:
        {
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader
            {
                .ActiveCount = 0,
                .Count = BakerTexture2DCapacity,
                .Size = sizeof(TextureMetadataHeader),
                .IsActive = Vector<byte>(BakerTexture2DCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;
        }
      /*  case MaterialBakerMemoryPoolTypes::BakerTexture3DMetadataBuffer:
        {
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader
            {
                .ActiveCount = 0,
                .Count = BakerTexture3DCapacity,
                .Size = sizeof(TextureMetadataHeader),
                .IsActive = Vector<byte>(BakerTexture3DCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;
        }
        case MaterialBakerMemoryPoolTypes::BakerTextureCubeMapMetadataBuffer:
        {
            MemorySubPoolHeader[type] = MemoryPoolSubBufferHeader
            {
                .ActiveCount = 0,
                .Count = BakerTextureCubeMapCapacity,
                .Size = sizeof(TextureMetadataHeader),
                .IsActive = Vector<byte>(BakerTextureCubeMapCapacity, 0x00),
                .FreeIndices = Vector<uint32>(),
                .IsDirty = true
            };
            break;
        }*/
        }
    }
    UpdateMemoryPoolHeader(BakerMaterialBuffer, BakerMaterialCapacity);

    Vector<byte> GpuDataBufferMemoryPool2 = Vector<byte>(sizeof(MaterialBakerBufferHeader) + MaterialMemoryPoolSize, 0xFF);
    memcpy(GpuDataBufferMemoryPool2.data(), &MaterialPoolHeader, sizeof(MaterialBakerBufferHeader));
    MaterialBakerBufferId = bufferSystem.VMACreateDynamicBuffer(GpuDataBufferMemoryPool2.data(), GpuDataBufferMemoryPool2.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VulkanBuffer& buffer = bufferSystem.FindVulkanBuffer(MaterialBakerBufferId);
    MaterialBufferPtr = buffer.BufferData;
    vmaFlushAllocation(bufferSystem.vmaAllocator, buffer.Allocation, 0, GpuDataBufferMemoryPool2.size());

    CreateMaterialBakerBindlessDescriptorSet();
}

void MaterialMemoryPoolSystem::ResizeMemoryPool(MaterialBakerMemoryPoolTypes memoryPoolToUpdate, uint32 resizeCount)
{
    void*  oldMappedPtr = MaterialBufferPtr;
    uint32 oldBufferId = MaterialBakerBufferId;
    auto   oldSubHeaders = MemorySubPoolHeader;

    UpdateMemoryPoolHeader(memoryPoolToUpdate, resizeCount);
    size_t newTotalSize = sizeof(MaterialBakerBufferHeader) + MaterialMemoryPoolSize;
    uint32 newBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, newTotalSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VulkanBuffer& newBuf = bufferSystem.FindVulkanBuffer(newBufferId);
    MaterialBufferPtr = newBuf.BufferData;

    std::memcpy(MaterialBufferPtr, &MaterialPoolHeader, sizeof(MaterialBakerBufferHeader));
    for (const auto& [type, sub] : MemorySubPoolHeader)
    {
        const auto& oldSub = oldSubHeaders[type];
        size_t bytesToCopy = oldSub.ActiveCount * oldSub.Size;
        if (bytesToCopy > 0)
        {
            std::byte* dst = static_cast<std::byte*>(MaterialBufferPtr) + sub.Offset;
            const std::byte* src = static_cast<const std::byte*>(oldMappedPtr) + oldSub.Offset;
            std::memcpy(dst, src, bytesToCopy);
        }
    }

    vmaFlushAllocation(bufferSystem.vmaAllocator, newBuf.Allocation, 0, newTotalSize);
    if (oldBufferId != UINT32_MAX)
    {
       //vkQueueWaitIdle(vulkanSystem.GraphicsQueue);  
       //bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(oldBufferId));
    }

    MaterialBakerBufferId = newBufferId;
    IsDescriptorSetDirty = true;
    IsHeaderDirty = true;
}


void MaterialMemoryPoolSystem::UpdateMemoryPoolHeader(MaterialBakerMemoryPoolTypes memoryPoolTypeToUpdate, uint32 newPoolSize)
{
    for (int x = memoryPoolTypeToUpdate; x < static_cast<int>(MaterialBakerMemoryPoolTypes::BakerEndofPool); x++)
    {
        const MaterialBakerMemoryPoolTypes memoryPoolType = (MaterialBakerMemoryPoolTypes)x;
        const MaterialBakerMemoryPoolTypes lastMemoryPoolType = (x == 0) ? MaterialBakerMemoryPoolTypes::BakerEndofPool : (MaterialBakerMemoryPoolTypes)(x - 1);
        const MemoryPoolSubBufferHeader oldMemoryPoolSubHeader = MemorySubPoolHeader[memoryPoolType];
        MemorySubPoolHeader[memoryPoolType] = MemoryPoolSubBufferHeader
        {
           .ActiveCount = MemorySubPoolHeader[memoryPoolType].ActiveCount,
           .Offset = lastMemoryPoolType == MaterialBakerMemoryPoolTypes::BakerEndofPool ? sizeof(MaterialBakerBufferHeader) : MemorySubPoolHeader[lastMemoryPoolType].Offset + (MemorySubPoolHeader[lastMemoryPoolType].Count * MemorySubPoolHeader[lastMemoryPoolType].Size),
           .Count = memoryPoolType == memoryPoolTypeToUpdate ? newPoolSize : MemorySubPoolHeader[memoryPoolType].Count,
           .Size = MemorySubPoolHeader[memoryPoolType].Size,
           .IsActive = memoryPoolType == memoryPoolTypeToUpdate ? Vector<byte>(newPoolSize, 0x00) : MemorySubPoolHeader[memoryPoolType].IsActive,
           .FreeIndices = MemorySubPoolHeader[memoryPoolType].FreeIndices,
           .IsDirty = true
        };
        if (memoryPoolType == (MaterialBakerMemoryPoolTypes)x) memcpy(MemorySubPoolHeader[memoryPoolType].IsActive.data(), oldMemoryPoolSubHeader.IsActive.data(), oldMemoryPoolSubHeader.ActiveCount);
    }
    MemoryPoolSubBufferHeader lastHeader = MemorySubPoolHeader[(MaterialBakerMemoryPoolTypes)((MaterialBakerMemoryPoolTypes)MaterialBakerMemoryPoolTypes::BakerEndofPool - 1)];
    MaterialMemoryPoolSize = lastHeader.Offset + (lastHeader.Size * lastHeader.Count);

    MaterialPoolHeader = MaterialBakerBufferHeader
    {
        .MaterialOffset = MemorySubPoolHeader[BakerMaterialBuffer].Offset,
        .MaterialCount = MemorySubPoolHeader[BakerMaterialBuffer].Count,
        .MaterialSize = MemorySubPoolHeader[BakerMaterialBuffer].Size,
        .Texture2DOffset = MemorySubPoolHeader[BakerTexture2DMetadataBuffer].Offset,
        .Texture2DCount = MemorySubPoolHeader[BakerTexture2DMetadataBuffer].Count,
        .Texture2DSize = MemorySubPoolHeader[BakerTexture2DMetadataBuffer].Size,
        //.Texture3DOffset = MemorySubPoolHeader[BakerTexture3DMetadataBuffer].Offset,
        //.Texture3DCount = MemorySubPoolHeader[BakerTexture3DMetadataBuffer].Count,
        //.Texture3DSize = MemorySubPoolHeader[BakerTexture3DMetadataBuffer].Size,
        //.TextureCubeMapOffset = MemorySubPoolHeader[BakerTextureCubeMapMetadataBuffer].Offset,
        //.TextureCubeMapCount = MemorySubPoolHeader[BakerTextureCubeMapMetadataBuffer].Count,
        //.TextureCubeMapSize = MemorySubPoolHeader[BakerTextureCubeMapMetadataBuffer].Size
    };
}

uint32 MaterialMemoryPoolSystem::AllocateObject(MaterialBakerMemoryPoolTypes memoryPoolToUpdate)
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

void MaterialMemoryPoolSystem::UpdateMemoryPool(Vector<VulkanPipeline>& pipelineList)
{
    if (!MaterialBufferPtr)
    {
        return;
    }

    VulkanBuffer& buffer = bufferSystem.FindVulkanBuffer(MaterialBakerBufferId);
    for (auto& [type, sub] : MemorySubPoolHeader)
    {
        if (sub.IsDirty)
        {
            size_t start = sizeof(MaterialBakerBufferHeader) + sub.Offset;
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
        memcpy(MaterialBufferPtr, &MaterialPoolHeader, sizeof(MaterialBakerBufferHeader));
        vmaFlushAllocation(bufferSystem.vmaAllocator, buffer.Allocation, 0, sizeof(MaterialBakerBufferHeader));
        IsHeaderDirty = false;
    }

    if (IsDescriptorSetDirty)
    {
        UpdateDataBufferDescriptorSet(MaterialBakerBufferId, BakerMaterialDescriptorBinding);
        IsDescriptorSetDirty = false;
    }
}

ImportMaterialShader& MaterialMemoryPoolSystem::UpdateMaterial(uint32 index)
{
    MemoryPoolSubBufferHeader& materialSubPool = MemorySubPoolHeader[BakerMaterialBuffer];
    if (index >= materialSubPool.Count) throw std::out_of_range("Material index out of range: " + std::to_string(index) + " >= " + std::to_string(materialSubPool.Count));
    if (index >= materialSubPool.IsActive.size() || !materialSubPool.IsActive[index]) throw std::runtime_error("Material slot inactive at index " + std::to_string(index));

    uint32 offset = materialSubPool.Offset + (index * sizeof(ImportMaterialShader));
    materialSubPool.IsDirty = true;
    return *reinterpret_cast<ImportMaterialShader*>(static_cast<byte*>(MaterialBufferPtr) + offset);
}

void MaterialMemoryPoolSystem::UpdateTextureDescriptorSet(Texture& texture, uint binding)
{
    if (texture.textureViewList.empty() || texture.textureViewList.front() == VK_NULL_HANDLE) 
    {
        std::cerr << "ERROR: Trying to update descriptor with invalid image view for texture index "
            << texture.bindlessTextureIndex << std::endl;
        return;
    }
    if (texture.textureSampler == VK_NULL_HANDLE) 
    {
        std::cerr << "ERROR: Null sampler for texture index " << texture.bindlessTextureIndex << std::endl;
        return;
    }

    VkDescriptorImageInfo textureUpdate = VkDescriptorImageInfo
    {
        .sampler = texture.textureSampler,
        .imageView = texture.textureViewList.front(),
        .imageLayout = texture.colorChannels == ColorChannelUsed::ChannelR ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkWriteDescriptorSet descriptorUpdate = VkWriteDescriptorSet
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = MaterialBakerBindlessDescriptorSet,
        .dstBinding = binding,
        .dstArrayElement = static_cast<uint32>(texture.bindlessTextureIndex),
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &textureUpdate,
    };
    vkUpdateDescriptorSets(vulkanSystem.Device, 1, &descriptorUpdate, 0, nullptr);
}

void MaterialMemoryPoolSystem::UpdateDataBufferDescriptorSet(uint32 vulkanBufferIndex, uint binding)
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
        .dstSet = MaterialBakerBindlessDescriptorSet,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &bufferUpdate,
    };
    vkUpdateDescriptorSets(vulkanSystem.Device, 1, &descriptorUpdate, 0, nullptr);
}

void MaterialMemoryPoolSystem::FreeObject(MaterialBakerMemoryPoolTypes memoryPoolToUpdate, uint32 index)
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

void MaterialMemoryPoolSystem::CreateMaterialBakerBindlessDescriptorSet()
{
    VkDescriptorBufferInfo materialInfo = 
    {
        .buffer = bufferSystem.FindVulkanBuffer(MaterialBakerBufferId).Buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE
    };

    Vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 512},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 512},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, BakerTexture2DCapacity + 1024},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 64}
    };

    Vector<VkDescriptorSetLayoutBinding> bindings =
    {
        { BakerMaterialDescriptorBinding,   VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,          1, VK_SHADER_STAGE_ALL }, 
        { BakerTexture2DBinding,            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, BakerTexture2DCapacity,      VK_SHADER_STAGE_ALL }, 
        //{ BakerTexture3DBinding,            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, BakerTexture3DCapacity,      VK_SHADER_STAGE_ALL },  
        //{ BakerTextureCubeMapBinding,       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, BakerTextureCubeMapCapacity, VK_SHADER_STAGE_ALL }
    };

    Vector<VkDescriptorBindingFlags> flags =
    {
        VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT },
        VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT },
        //VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT },
        //VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT }
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
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorPool(vulkanSystem.Device, &poolInfo, nullptr, &MaterialBakerBindlessPool));

    VkDescriptorSetLayoutCreateInfo layoutInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &flagsInfo,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorSetLayout(vulkanSystem.Device, &layoutInfo, nullptr, &MaterialBakerBindlessDescriptorSetLayout));

    VkDescriptorSetAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = MaterialBakerBindlessPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &MaterialBakerBindlessDescriptorSetLayout
    };
    VULKAN_THROW_IF_FAIL(vkAllocateDescriptorSets(vulkanSystem.Device, &allocInfo, &MaterialBakerBindlessDescriptorSet));

    VkWriteDescriptorSet materialWrite = 
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = MaterialBakerBindlessDescriptorSet,
        .dstBinding = BakerMaterialDescriptorBinding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &materialInfo
    };

    vkUpdateDescriptorSets(vulkanSystem.Device, 1, &materialWrite, 0, nullptr);
}