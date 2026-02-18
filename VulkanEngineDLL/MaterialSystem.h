#pragma once
#include "Platform.h"
#include "BufferSystem.h"

struct Material
{
    int MaterialBufferId;
    VkGuid materialGuid = VkGuid();
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
    UnorderedMap<VkGuid, uint32>                     GuidToPoolIndex;

public:
    DLL_EXPORT VkGuid LoadMaterial(const String& materialPath);
    DLL_EXPORT void Update(const float& deltaTime);
    DLL_EXPORT const bool MaterialExists(const MaterialGuid& materialGuid) const;
    DLL_EXPORT Material FindMaterial(const MaterialGuid& materialGuid);
    DLL_EXPORT uint FindMaterialPoolIndex(const MaterialGuid& materialGuid);
    DLL_EXPORT const Vector<VkDescriptorBufferInfo> GetMaterialPropertiesBuffer();
    DLL_EXPORT void Destroy(const MaterialGuid& materialGuid);
    DLL_EXPORT void DestroyAllMaterials();

    DLL_EXPORT Vector<Material> GetMaterialList() { return MaterialList; }
};
extern DLL_EXPORT MaterialSystem& materialSystem;
inline MaterialSystem& MaterialSystem::Get()
{
    static MaterialSystem instance;
    return instance;
}