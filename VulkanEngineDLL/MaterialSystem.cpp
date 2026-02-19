#include "MaterialSystem.h"
#include "FileSystem.h"
#include "RenderSystem.h"
#include "BufferSystem.h"
#include "from_json.h"

MaterialSystem& materialSystem = MaterialSystem::Get();

void MaterialSystem::StartUp()
{
    constexpr size_t InitialCapacity = 65536;
    MaterialBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr,
        sizeof(MaterialBufferHeader) + InitialCapacity * sizeof(GPUMaterial),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
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

    GPUMaterial gpuMaterial;
    gpuMaterial.AlbedoDataId = material.AlbedoDataId != VkGuid() ? textureSystem.FindTexture(material.AlbedoDataId).textureIndex : UINT32_MAX;
    gpuMaterial.NormalDataId = material.NormalDataId != VkGuid() ? textureSystem.FindTexture(material.NormalDataId).textureIndex : UINT32_MAX;
    gpuMaterial.PackedMRODataId = material.PackedMRODataId != VkGuid() ? textureSystem.FindTexture(material.PackedMRODataId).textureIndex : UINT32_MAX;
    gpuMaterial.PackedSheenSSSDataId = material.PackedSheenSSSDataId != VkGuid() ? textureSystem.FindTexture(material.PackedSheenSSSDataId).textureIndex : UINT32_MAX;
    gpuMaterial.UnusedDataId = material.UnusedDataId != VkGuid() ? textureSystem.FindTexture(material.UnusedDataId).textureIndex : UINT32_MAX;
    gpuMaterial.EmissionDataId = material.EmissionDataId != VkGuid() ? textureSystem.FindTexture(material.EmissionDataId).textureIndex : UINT32_MAX;

    uint32_t index = static_cast<uint32_t>(MaterialPool.size());
    GuidToPoolIndex[materialGuid] = index;
    MaterialPool.emplace_back(gpuMaterial);

    return materialGuid;
}

void MaterialSystem::Update(const float& deltaTime, Vector<VulkanPipeline>& pipelineList)
{
    uint32_t count = static_cast<uint32_t>(MaterialPool.size());
    if (count == 0) return;

    MaterialBufferHeader header{};
    header.MaterialCount = count;
    header.MaterialOffset = sizeof(MaterialBufferHeader);
    header.MaterialSize = sizeof(GPUMaterial);

    Vector<uint8_t> uploadData;
    uploadData.resize(sizeof(MaterialBufferHeader) + count * sizeof(GPUMaterial));
    memcpy(uploadData.data(), &header, sizeof(MaterialBufferHeader));
    if (count > 0)
    {
        memcpy(uploadData.data() + sizeof(MaterialBufferHeader),
            MaterialPool.data(),
            count * sizeof(GPUMaterial));
    }

    bool bufferRecreated = false;

    if (MaterialBufferId == UINT32_MAX)
    {
        MaterialBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr,
            uploadData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        bufferRecreated = true;
    }
    else if (bufferSystem.FindVulkanBuffer(MaterialBufferId).BufferSize < uploadData.size())
    {
        bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(MaterialBufferId));
        MaterialBufferId = bufferSystem.VMACreateDynamicBuffer(nullptr,
            uploadData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
        bufferRecreated = true;
    }

    bufferSystem.VMAUpdateDynamicBuffer(MaterialBufferId, uploadData.data(), uploadData.size());

    auto bufferInfo = GetMaterialBufferInfo();

    // Always update (safe and cheap), but especially after recreation
    for (auto& pipeline : pipelineList)
    {
        renderSystem.UpdateDescriptorSet(pipeline, bufferInfo, 10);
    }

    // Optional: log or assert if recreated, to confirm it happens rarely
    // if (bufferRecreated) { /* log "Material buffer resized and rebound" */ }
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
        return invalid;  // or throw
    }
    return MaterialList[it->second];
}

uint MaterialSystem::FindMaterialPoolIndex(const MaterialGuid& materialGuid)
{
    auto it = GuidToPoolIndex.find(materialGuid);
    return it != GuidToPoolIndex.end() ? it->second : UINT32_MAX;
}

const Vector<VkDescriptorBufferInfo> MaterialSystem::GetMaterialBufferInfo() const
{
    Vector<VkDescriptorBufferInfo> infos;
    infos.emplace_back(VkDescriptorBufferInfo{
        .buffer = bufferSystem.FindVulkanBuffer(MaterialBufferId).Buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE
        });
    return infos;
}

void MaterialSystem::Destroy(const MaterialGuid& materialGuid)
{
    auto it = GuidToPoolIndex.find(materialGuid);
    if (it == GuidToPoolIndex.end()) return;

    uint32_t index = it->second;
    GuidToPoolIndex.erase(it);

    if (index < MaterialList.size())
    {
        MaterialList.erase(MaterialList.begin() + index);
        MaterialPool.erase(MaterialPool.begin() + index);
    }

    // Fix indices after deletion
    for (auto& pair : GuidToPoolIndex)
    {
        if (pair.second > index)
            pair.second--;
    }
}

void MaterialSystem::DestroyAllMaterials()
{
    MaterialList.clear();
    MaterialPool.clear();
    GuidToPoolIndex.clear();

    if (MaterialBufferId != UINT32_MAX)
    {
        bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(MaterialBufferId));
        MaterialBufferId = UINT32_MAX;
    }
}