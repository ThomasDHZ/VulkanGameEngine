#include "MaterialSystem.h"
#include "FileSystem.h"
#include "RenderSystem.h"
#include "BufferSystem.h"
#include "MemoryPoolSystem.h"
#include "from_json.h"

MaterialSystem& materialSystem = MaterialSystem::Get();

void MaterialSystem::StartUp()
{
    //constexpr size_t InitialCapacity = 65536;
    //MaterialPool.CreateMemoryPool(InitialCapacity, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

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

    uint32 poolIndex = memoryPoolSystem.AllocateObject(kMaterialBuffer);

    GPUMaterial& gpuMaterial = memoryPoolSystem.UpdateMaterial(poolIndex);
    gpuMaterial.AlbedoDataId = material.AlbedoDataId != VkGuid() ? textureSystem.FindTexture(material.AlbedoDataId).textureIndex : UINT32_MAX;
    gpuMaterial.NormalDataId = material.NormalDataId != VkGuid() ? textureSystem.FindTexture(material.NormalDataId).textureIndex : UINT32_MAX;
    gpuMaterial.PackedMRODataId = material.PackedMRODataId != VkGuid() ? textureSystem.FindTexture(material.PackedMRODataId).textureIndex : UINT32_MAX;
    gpuMaterial.PackedSheenSSSDataId = material.PackedSheenSSSDataId != VkGuid() ? textureSystem.FindTexture(material.PackedSheenSSSDataId).textureIndex : UINT32_MAX;
    gpuMaterial.UnusedDataId = material.UnusedDataId != VkGuid() ? textureSystem.FindTexture(material.UnusedDataId).textureIndex : UINT32_MAX;
    gpuMaterial.EmissionDataId = material.EmissionDataId != VkGuid() ? textureSystem.FindTexture(material.EmissionDataId).textureIndex : UINT32_MAX;
    GuidToPoolIndex[materialGuid] = poolIndex;

    return materialGuid;
}

void MaterialSystem::Update(const float& deltaTime, Vector<VulkanPipeline>& pipelineList)
{
    //MaterialPool.UpdateMemoryPool(10, pipelineList);
}

const bool MaterialSystem::MaterialExists(const MaterialGuid& materialGuid) const
{
    auto it = GuidToPoolIndex.find(materialGuid);
    return it != GuidToPoolIndex.end();
}

Material& MaterialSystem::FindMaterial(const MaterialGuid& materialGuid)
{
    auto it = GuidToPoolIndex.find(materialGuid);
    if (it == GuidToPoolIndex.end())
    {
        static Material invalid{};
        return invalid;
    }
    return MaterialList[it->second];
}

uint MaterialSystem::FindMaterialPoolIndex(const MaterialGuid& materialGuid)
{
    auto it = GuidToPoolIndex.find(materialGuid);
    return it != GuidToPoolIndex.end() ? it->second : UINT32_MAX;
}

void MaterialSystem::Destroy(const MaterialGuid& materialGuid)
{
    //auto it = GuidToPoolIndex.find(materialGuid);
    //if (it == GuidToPoolIndex.end()) return;

    //uint32_t index = it->second;
    //GuidToPoolIndex.erase(it);

    //if (index < MaterialList.size())
    //{
    //    MaterialList.erase(MaterialList.begin() + index);
    //    MaterialPool.erase(MaterialPool.begin() + index);
    //}

    //// Fix indices after deletion
    //for (auto& pair : GuidToPoolIndex)
    //{
    //    if (pair.second > index)
    //        pair.second--;
    //}
}

void MaterialSystem::DestroyAllMaterials()
{
    //MaterialList.clear();
    //MaterialPool.clear();
    //GuidToPoolIndex.clear();

    //if (MaterialBufferId != UINT32_MAX)
    //{
    //    bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(MaterialBufferId));
    //    MaterialBufferId = UINT32_MAX;
    //}
}