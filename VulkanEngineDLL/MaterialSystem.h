#pragma once
#include "Platform.h"
#include "BufferSystem.h"
#include "JsonStruct.h"   // assuming this has VkGuid, MaterialGuid, etc.

struct Material
{
    VkGuid MaterialGuid = VkGuid();
    VkGuid AlbedoDataId = VkGuid();
    VkGuid NormalDataId = VkGuid();
    VkGuid PackedMRODataId = VkGuid();
    VkGuid PackedSheenSSSDataId = VkGuid();
    VkGuid UnusedDataId = VkGuid();
    VkGuid EmissionDataId = VkGuid();
    Material() = default;
    Material(const Material&) = default;
    Material& operator=(const Material&) = default;
};

struct GPUMaterial
{
    uint AlbedoDataId = UINT32_MAX;
    uint NormalDataId = UINT32_MAX;
    uint PackedMRODataId = UINT32_MAX;
    uint PackedSheenSSSDataId = UINT32_MAX;
    uint UnusedDataId = UINT32_MAX;
    uint EmissionDataId = UINT32_MAX;
    GPUMaterial() = default;
    GPUMaterial(const GPUMaterial&) = default;
    GPUMaterial& operator=(const GPUMaterial&) = default;
};

struct alignas(4) MaterialBufferHeader
{
    uint MaterialOffset;
    uint MaterialCount;
    uint MaterialSize;
};

class MaterialSystem
{
public:
    static MaterialSystem& Get();

    DLL_EXPORT void StartUp();
    DLL_EXPORT VkGuid LoadMaterial(const String& materialPath);
    DLL_EXPORT void Update(const float& deltaTime, Vector<VulkanPipeline>& pipeline);
    DLL_EXPORT const bool MaterialExists(const MaterialGuid& materialGuid) const;
    DLL_EXPORT Material& FindMaterial(const MaterialGuid& materialGuid);
    DLL_EXPORT uint FindMaterialPoolIndex(const MaterialGuid& materialGuid);
    DLL_EXPORT const Vector<VkDescriptorBufferInfo> GetMaterialBufferInfo() const;
    DLL_EXPORT void Destroy(const MaterialGuid& materialGuid);
    DLL_EXPORT void DestroyAllMaterials();
    DLL_EXPORT Vector<Material> GetMaterialList() { return MaterialList; }

private:
    MaterialSystem() = default;
    ~MaterialSystem() = default;
    MaterialSystem(const MaterialSystem&) = delete;
    MaterialSystem& operator=(const MaterialSystem&) = delete;
    MaterialSystem(MaterialSystem&&) = delete;
    MaterialSystem& operator=(MaterialSystem&&) = delete;

    Vector<Material> MaterialList;
    Vector<GPUMaterial> MaterialPool;          // your makeshift vector pool
    UnorderedMap<VkGuid, uint32> GuidToPoolIndex;
    uint MaterialBufferId = UINT32_MAX;
};

extern DLL_EXPORT MaterialSystem& materialSystem;
inline MaterialSystem& MaterialSystem::Get()
{
    static MaterialSystem instance;
    return instance;
}