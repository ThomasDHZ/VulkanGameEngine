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
    for (uint32 x = 0; x < MaterialPool.ObjectDataPool.size(); ++x)
    {
        if (MaterialPool.IsSlotActive(x))
        {
            Material material = MaterialList[x];

            GPUMaterial gpuMaterial = MaterialPool.Get(x);
            gpuMaterial.AlbedoDataId = material.AlbedoDataId != VkGuid() ? textureSystem.FindTexture(material.AlbedoDataId).textureIndex : SIZE_MAX;
            gpuMaterial.NormalDataId = material.NormalDataId != VkGuid() ? textureSystem.FindTexture(material.NormalDataId).textureIndex : SIZE_MAX;
            gpuMaterial.PackedMRODataId = material.PackedMRODataId != VkGuid() ? textureSystem.FindTexture(material.PackedMRODataId).textureIndex : SIZE_MAX;
            gpuMaterial.PackedSheenSSSDataId = material.PackedSheenSSSDataId != VkGuid() ? textureSystem.FindTexture(material.PackedSheenSSSDataId).textureIndex : SIZE_MAX;
            gpuMaterial.UnusedDataId = material.UnusedDataId != VkGuid() ? textureSystem.FindTexture(material.UnusedDataId).textureIndex : SIZE_MAX;
            gpuMaterial.EmissionDataId = material.EmissionDataId != VkGuid() ? textureSystem.FindTexture(material.EmissionDataId).textureIndex : SIZE_MAX;
            activeMaterials.push_back(gpuMaterial);
        }
    }

    auto bufferInfo = GetMaterialBufferInfo();
    MaterialPool.UpdateMemoryPool(activeMaterials);
    renderSystem.UpdateDescriptorSet(pipeline, bufferInfo, 10);
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