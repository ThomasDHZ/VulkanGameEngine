#include "MaterialSystem.h"
#include "FileSystem.h"
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
    gpuMaterial.AlbedoDataId = FindMaterialPoolIndex(material.AlbedoDataId);
    gpuMaterial.NormalDataId = FindMaterialPoolIndex(material.NormalDataId);
    gpuMaterial.PackedMRODataId = FindMaterialPoolIndex(material.PackedMRODataId);
    gpuMaterial.PackedSheenSSSDataId = FindMaterialPoolIndex(material.PackedSheenSSSDataId);
    gpuMaterial.UnusedDataId = FindMaterialPoolIndex(material.UnusedDataId);
    gpuMaterial.EmissionDataId = FindMaterialPoolIndex(material.EmissionDataId);

    GuidToPoolIndex[materialGuid] = poolIndex;
    MaterialPool.IsDirty = true;

    return materialGuid;
}

void MaterialSystem::Update(const float& deltaTime)
{
    if (!MaterialPool.IsDirty) return;

    Vector<GPUMaterial> activeMaterials;
    activeMaterials.reserve(MaterialPool.ActiveCount);

    for (uint32 x = 0; x < MaterialPool.ObjectDataPool.size(); ++x)
    {
        if (MaterialPool.IsSlotActive(x))
        {
            Material material = MaterialList[x];

            GPUMaterial gpuMaterial = MaterialPool.Get(x);
            gpuMaterial.AlbedoDataId = FindMaterialPoolIndex(material.AlbedoDataId);
            gpuMaterial.NormalDataId = FindMaterialPoolIndex(material.NormalDataId);
            gpuMaterial.PackedMRODataId = FindMaterialPoolIndex(material.PackedMRODataId);
            gpuMaterial.PackedSheenSSSDataId = FindMaterialPoolIndex(material.PackedSheenSSSDataId);
            gpuMaterial.UnusedDataId = FindMaterialPoolIndex(material.UnusedDataId);
            gpuMaterial.EmissionDataId = FindMaterialPoolIndex(material.EmissionDataId);
            activeMaterials.push_back(gpuMaterial);
        }
    }

    VkDeviceSize bufferSize = activeMaterials.size() * sizeof(GPUMaterial);
    if (GiantMaterialBufferId == UINT32_MAX || bufferSystem.FindVulkanBuffer(GiantMaterialBufferId).BufferSize < bufferSize)
    {
        if (GiantMaterialBufferId != UINT32_MAX)
        {
            bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(GiantMaterialBufferId));
        }
        GiantMaterialBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    }
    bufferSystem.VMAUpdateDynamicBuffer(GiantMaterialBufferId, activeMaterials.data(), bufferSize);
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