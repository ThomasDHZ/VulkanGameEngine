#pragma once
#include "DLL.h"
#include <Platform.h>
#include "BufferSystem.h"
#include "JsonStruct.h"
#include "MemoryPoolSystem.h"


struct Material
{
    VkGuid MaterialGuid                   = VkGuid();
    VkGuid AlbedoTextureId                = VkGuid();
    VkGuid NormalTextureId                = VkGuid();
    VkGuid MroTextureId                   = VkGuid();
    VkGuid ClearCoatTextureId             = VkGuid();
    VkGuid SubSurfaceScatteringTextureId  = VkGuid();
    VkGuid SheenTextureId                 = VkGuid();
    VkGuid AnisotropyTextureId            = VkGuid();
    VkGuid EmissionTextureId              = VkGuid();
    VkGuid TranslucentTextureId           = VkGuid();
    VkGuid TranslucentPropertiesTextureId = VkGuid();
    uint ShadingModel;
    uint FeatureMask;
    vec4  ClearcoatTint;
    float SheenRoughness;
    float SSSWeight;
    float SSSProfile;
    float IOR;
    float AlphaCutOff;

    Material() = default;
    Material(const Material&) = default;
    Material& operator=(const Material&) = default;
};

struct GPUMaterial
{
    uint MaterialGuid                   = UINT32_MAX;
    uint AlbedoTextureId                = UINT32_MAX;
    uint NormalTextureId                = UINT32_MAX;
    uint MroTextureId                   = UINT32_MAX;
    uint ClearCoatTextureId             = UINT32_MAX;
    uint SubSurfaceScatteringTextureId  = UINT32_MAX;
    uint SheenTextureId                 = UINT32_MAX;
    uint AnisotropyTextureId            = UINT32_MAX;
    uint EmissionTextureId              = UINT32_MAX;
    uint TranslucentTextureId           = UINT32_MAX;
    uint TranslucentPropertiesTextureId = UINT32_MAX;
    uint ShadingModel;
    uint FeatureMask;
    uint _pad0;
    uint _pad1;
    vec4  ClearcoatTint;
    float SheenRoughness;
    float SSSWeight;
    float SSSProfile;
    float IOR;
    float AlphaCutOff;

    GPUMaterial() = default;
    GPUMaterial(const GPUMaterial&) = default;
    GPUMaterial& operator=(const GPUMaterial&) = default;
};

class ENGINE_DLL_EXPORT MaterialSystem
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
         void Destroy();
         Vector<Material> GetMaterialList() { return MaterialList; }
}; 
ENGINE_DLL_EXPORT extern  MaterialSystem& materialSystem;
inline MaterialSystem& MaterialSystem::Get()
{
    static MaterialSystem instance;
    return instance;
}