#include "MaterialSystem.h"
#include "FileSystem.h"
#include "RenderSystem.h"
#include "BufferSystem.h"
#include "ShaderSystem.h"
#include "from_json.h"

MaterialSystem& materialSystem = MaterialSystem::Get();

void MaterialSystem::StartUp()
{
    MaterialPool.CreateMemoryPool(65536, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

VkGuid MaterialSystem::LoadMaterial(const String& materialPath)
{
    if (materialPath.empty()) return VkGuid::Empty();

    nlohmann::json json = fileSystem.LoadJsonFile(materialPath.c_str());
    VkGuid materialGuid = VkGuid(json["MaterialId"].get<std::string>());

    if (MaterialExists(materialGuid))
    {
        return materialGuid;
    }

    Material material;
    material.MaterialGuid = materialGuid;
    material.AlbedoDataId = json.contains("AlbedoData") ? textureSystem.LoadKTXTexture(json["AlbedoData"].get<TextureLoader>()).textureGuid : VkGuid();
    material.NormalDataId = json.contains("NormalData") ? textureSystem.LoadKTXTexture(json["NormalData"].get<TextureLoader>()).textureGuid : VkGuid();
    material.PackedMRODataId = json.contains("PackedMROData") ? textureSystem.LoadKTXTexture(json["PackedMROData"].get<TextureLoader>()).textureGuid : VkGuid();
    material.PackedSheenSSSDataId = json.contains("PackedSheenSSSData") ? textureSystem.LoadKTXTexture(json["PackedSheenSSSData"].get<TextureLoader>()).textureGuid : VkGuid();
    material.UnusedDataId = json.contains("UnusedData") ? textureSystem.LoadKTXTexture(json["UnusedData"].get<TextureLoader>()).textureGuid : VkGuid();
    material.EmissionDataId = json.contains("EmissionData") ? textureSystem.LoadKTXTexture(json["EmissionData"].get<TextureLoader>()).textureGuid : VkGuid();
    MaterialList.emplace_back(material);

    uint32_t poolIndex = MaterialPool.AllocateObject();
    GPUMaterial gpuMaterial = MaterialPool.Get(poolIndex);
    gpuMaterial.AlbedoDataId = material.AlbedoDataId != VkGuid() ? textureSystem.FindTexture(material.AlbedoDataId).textureIndex : SIZE_MAX;
    gpuMaterial.NormalDataId = material.NormalDataId != VkGuid() ? textureSystem.FindTexture(material.NormalDataId).textureIndex : SIZE_MAX;
    gpuMaterial.PackedMRODataId = material.PackedMRODataId != VkGuid() ? textureSystem.FindTexture(material.PackedMRODataId).textureIndex : SIZE_MAX;
    gpuMaterial.PackedSheenSSSDataId = material.PackedSheenSSSDataId != VkGuid() ? textureSystem.FindTexture(material.PackedSheenSSSDataId).textureIndex : SIZE_MAX;
    gpuMaterial.UnusedDataId = material.UnusedDataId != VkGuid() ? textureSystem.FindTexture(material.UnusedDataId).textureIndex : SIZE_MAX;
    gpuMaterial.EmissionDataId = material.EmissionDataId != VkGuid() ? textureSystem.FindTexture(material.EmissionDataId).textureIndex : SIZE_MAX;

    GuidToPoolIndex[materialGuid] = poolIndex;
    MaterialPool.IsDirty = true;

    return materialGuid;
}

void MaterialSystem::Update(const float& deltaTime, VulkanPipeline& pipeline)
{
    if (!MaterialPool.IsDirty) return;

    Vector<GPUMaterial> activeMaterials;
    activeMaterials.reserve(MaterialPool.ActiveCount);

    for (uint32_t x = 0; x < MaterialPool.ObjectDataPool.size(); ++x)
    {
        if (MaterialPool.IsSlotActive(x))
        {
            const Material& cpuMat = MaterialList[x];
            GPUMaterial gpuMat = MaterialPool.Get(x);

            gpuMat.AlbedoDataId = cpuMat.AlbedoDataId != VkGuid() ? textureSystem.FindTexture(cpuMat.AlbedoDataId).textureIndex : ~0u;
            gpuMat.NormalDataId = cpuMat.NormalDataId != VkGuid() ? textureSystem.FindTexture(cpuMat.NormalDataId).textureIndex : ~0u;
            gpuMat.PackedMRODataId = cpuMat.PackedMRODataId != VkGuid() ? textureSystem.FindTexture(cpuMat.PackedMRODataId).textureIndex : ~0u;
            gpuMat.PackedSheenSSSDataId = cpuMat.PackedSheenSSSDataId != VkGuid() ? textureSystem.FindTexture(cpuMat.PackedSheenSSSDataId).textureIndex : ~0u;
            gpuMat.UnusedDataId = cpuMat.UnusedDataId != VkGuid() ? textureSystem.FindTexture(cpuMat.UnusedDataId).textureIndex : ~0u;
            gpuMat.EmissionDataId = cpuMat.EmissionDataId != VkGuid() ? textureSystem.FindTexture(cpuMat.EmissionDataId).textureIndex : ~0u;

            activeMaterials.push_back(gpuMat);
        }
    }

    MaterialBufferHeader header{};
    header.MaterialCount = static_cast<uint32_t>(activeMaterials.size());
    header.MaterialOffset = sizeof(MaterialBufferHeader);
    header.MaterialSize = sizeof(GPUMaterial);

    Vector<uint8_t> uploadData;
    uploadData.resize(sizeof(MaterialBufferHeader) + activeMaterials.size() * sizeof(GPUMaterial));
    memcpy(uploadData.data(), &header, sizeof(MaterialBufferHeader));

    if (!activeMaterials.empty())
    {
        memcpy(uploadData.data() + sizeof(MaterialBufferHeader),
            activeMaterials.data(),
            activeMaterials.size() * sizeof(GPUMaterial));
    }

    if (MaterialPool.BufferId == UINT32_MAX)
    {
        MaterialPool.BufferId = bufferSystem.VMACreateDynamicBuffer(nullptr,
            uploadData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    }
    else if (bufferSystem.FindVulkanBuffer(MaterialPool.BufferId).BufferSize < uploadData.size())
    {
        // resize logic if needed
        bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(MaterialPool.BufferId));
        MaterialPool.BufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, uploadData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    }
    bufferSystem.VMAUpdateDynamicBuffer(MaterialPool.BufferId, uploadData.data(), uploadData.size());

    auto bufferInfo = GetMaterialBufferInfo();
    renderSystem.UpdateDescriptorSet(pipeline, bufferInfo, 10);

    MaterialPool.IsDirty = false;
}

const bool MaterialSystem::MaterialExists(const MaterialGuid& materialGuid) const
{
    auto it = GuidToPoolIndex.find(materialGuid);
    return it != GuidToPoolIndex.end();
}

Material& MaterialSystem::FindMaterial(const MaterialGuid& materialGuid)
{
    return MaterialList[MaterialExists(materialGuid)];
}

const Vector<VkDescriptorBufferInfo> MaterialSystem::GetMaterialBufferInfo() const
{
    return Vector<VkDescriptorBufferInfo>
    {
        VkDescriptorBufferInfo
        {
            .buffer = bufferSystem.FindVulkanBuffer(MaterialPool.BufferId).Buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE
        }
    };
}

uint MaterialSystem::FindMaterialPoolIndex(const MaterialGuid& materialGuid)
{
    return GuidToPoolIndex[materialGuid];
}

void MaterialSystem::Destroy(const MaterialGuid& materialGuid)
{
    auto it = GuidToPoolIndex.find(materialGuid);
    if (it == GuidToPoolIndex.end()) return;

    uint32 poolIndex = it->second;
    MaterialPool.FreeDataSlot(poolIndex);
    GuidToPoolIndex.erase(it);
}

void MaterialSystem::DestroyAllMaterials()
{
    for (auto& materialPair : MaterialList)
    {
        Destroy(materialPair.MaterialGuid);
    }
}