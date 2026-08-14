#pragma once

#include <Platform.h>
#include "BufferSystem.h"
#include "JsonStruct.h"
#include "MemoryPoolSystem.h"

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

class MaterialSystem
{
    public:
        static MaterialSystem& Get();
    
private:
        MaterialSystem() = default;
        ~MaterialSystem() = default;
        MaterialSystem(const MaterialSystem&) = delete;
        MaterialSystem& operator=(const MaterialSystem&) = delete;
        MaterialSystem(MaterialSystem&&) = delete;
        MaterialSystem& operator=(MaterialSystem&&) = delete;

        Vector<Material> MaterialList;
        UnorderedMap<VkGuid, uint32> GuidToPoolIndex;

    public:

         VkGuid LoadMaterial(const String& materialPath);
         VkGuid LoadMaterial(const nlohmann::json& json);
         const bool MaterialExists(const MaterialGuid& materialGuid) const;
         Material& FindMaterial(const MaterialGuid& materialGuid);
         uint FindMaterialPoolIndex(const MaterialGuid& materialGuid);
         void Destroy(const MaterialGuid& materialGuid);
         void DestroyAllMaterials();
         Vector<Material> GetMaterialList() { return MaterialList; }
}; 
extern  MaterialSystem& materialSystem;
inline MaterialSystem& MaterialSystem::Get()
{
    static MaterialSystem instance;
    return instance;
}