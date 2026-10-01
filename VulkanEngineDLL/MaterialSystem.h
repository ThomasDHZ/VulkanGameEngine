#pragma once
#include "DLL.h"
#include <Platform.h>
#include "BufferSystem.h"
#include "JsonStruct.h"
#include "MemoryPoolSystem.h"

enum MaterialPropertiesEnum : uint
{
    kMaterialFeature_None                 = 0,
    kMaterialFeature_ClearCoat            = 1u << 0,
    kMaterialFeature_Sheen                = 1u << 1,
    kMaterialFeature_SubSurfaceScattering = 1u << 2,
    kMaterialFeature_Transmission         = 1u << 3,
    kMaterialFeature_Anisotropy           = 1u << 4,
    kMaterialFeature_FILM                 = 1u << 5,
    kMaterialFeature_TWO_SIDED            = 1u << 6,
    kMaterialFeature_ClearCoatNormal      = 1u << 7,
    kMaterialFeature_AlbedoOnly           = 1u << 8,
    kMaterialFeature_UsingAlpha           = 1u << 9,
};

struct Material
{
    VkGuid MaterialGuid                            = VkGuid();
    VkGuid AlbedoTextureId                         = VkGuid();
    VkGuid NormalTextureId                         = VkGuid();
    VkGuid MroTextureId                            = VkGuid();
    VkGuid ClearCoatTextureId                      = VkGuid();
    VkGuid SubSurfaceScatteringTextureId           = VkGuid();
    VkGuid SubSurfaceScatteringPropertiesTextureId = VkGuid();
    VkGuid SheenTextureId                          = VkGuid();
    VkGuid AnisotropyTextureId                     = VkGuid();
    VkGuid EmissionTextureId                       = VkGuid();
    VkGuid TranslucentTextureId                    = VkGuid();
    VkGuid TranslucentPropertiesTextureId          = VkGuid();
    uint ShadingModel;
    MaterialPropertiesEnum FeatureMask;
    vec4  ClearcoatTint;
    float IOR;
    float AlphaCutOff;

    Material() = default;
    Material(const Material&) = default;
    Material& operator=(const Material&) = default;
};

struct GPUMaterial
{
    uint AlbedoTextureId = UINT32_MAX;
    uint NormalTextureId = UINT32_MAX;
    uint MroTextureId = UINT32_MAX;
    uint ClearCoatTextureId = UINT32_MAX;
    uint SubSurfaceScatteringTextureId = UINT32_MAX;
    uint SubSurfaceScatteringPropertiesTextureId = UINT32_MAX;
    uint SheenTextureId = UINT32_MAX;
    uint AnisotropyTextureId = UINT32_MAX;
    uint EmissionTextureId = UINT32_MAX;
    uint TranslucentTextureId = UINT32_MAX;
    uint TranslucentPropertiesTextureId = UINT32_MAX;
    uint ShadingModel = 0;
    MaterialPropertiesEnum FeatureMask = kMaterialFeature_None;
    vec3 ClearcoatTint = vec3(1.0f);
    float IOR = 1.45f;
    float AlphaCutOff = 0.1f;
};
static_assert(sizeof(GPUMaterial) == 18 * 4);

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