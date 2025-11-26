#pragma once
#include "Platform.h"
#include "BufferSystem.h"

struct Material
{
    int VectorMapKey;
    VkGuid materialGuid = VkGuid();
    uint ShaderMaterialBufferIndex;
    int MaterialBufferId;
    VkGuid AlbedoMapId = VkGuid();
    VkGuid MetallicRoughnessMapId = VkGuid();
    VkGuid MetallicMapId = VkGuid();
    VkGuid RoughnessMapId = VkGuid();
    VkGuid AmbientOcclusionMapId = VkGuid();
    VkGuid NormalMapId = VkGuid();
    VkGuid DepthMapId = VkGuid();
    VkGuid AlphaMapId = VkGuid();
    VkGuid EmissionMapId = VkGuid();
    VkGuid HeightMapId = VkGuid();
    vec3 Albedo = vec3(0.0f, 0.35f, 0.45f);
    vec3 Emission = vec3(0.0f);
    float Metallic = 0.0f;
    float Roughness = 0.0f;
    float AmbientOcclusion = 1.0f;
    float Alpha = 1.0f;

    Material() = default;
    Material(const Material&) = default;
    Material& operator=(const Material&) = default;

  /*  Material()
    { 
    }

    Material(const Material& material)
    {
        VectorMapKey = material.VectorMapKey;
        materialGuid = material.materialGuid;
        ShaderMaterialBufferIndex = material.ShaderMaterialBufferIndex;
        MaterialBufferId = material.MaterialBufferId;
        AlbedoMapId = material.AlbedoMapId;
        MetallicRoughnessMapId = material.MetallicRoughnessMapId;
        MetallicMapId = material.MetallicMapId;
        RoughnessMapId = material.RoughnessMapId;
        AmbientOcclusionMapId = material.AmbientOcclusionMapId;
        NormalMapId = material.NormalMapId;
        DepthMapId = material.DepthMapId;
        AlphaMapId = material.AlphaMapId;
        EmissionMapId = material.EmissionMapId;
        HeightMapId = material.HeightMapId;
        Albedo = material.Albedo;
        Emission = material.Emission;
        Metallic = material.Metallic;
        Roughness = material.Roughness;
        AmbientOcclusion = material.AmbientOcclusion;
        Alpha = material.Alpha;
    }

    Material& operator=(const Material& material)
    {
		VectorMapKey = material.VectorMapKey;
		materialGuid = material.materialGuid;
		ShaderMaterialBufferIndex = material.ShaderMaterialBufferIndex;
		MaterialBufferId = material.MaterialBufferId;
		AlbedoMapId = material.AlbedoMapId;
		MetallicRoughnessMapId = material.MetallicRoughnessMapId;
		MetallicMapId = material.MetallicMapId;
		RoughnessMapId = material.RoughnessMapId;
		AmbientOcclusionMapId = material.AmbientOcclusionMapId;
		NormalMapId = material.NormalMapId;
		DepthMapId = material.DepthMapId;
		AlphaMapId = material.AlphaMapId;
		EmissionMapId = material.EmissionMapId;
		HeightMapId = material.HeightMapId;
		Albedo = material.Albedo;
		Emission = material.Emission;
		Metallic = material.Metallic;
		Roughness = material.Roughness;
		AmbientOcclusion = material.AmbientOcclusion;
		Alpha = material.Alpha;
    }*/
};

class MaterialSystem
{
private:
    UnorderedMap<MaterialGuid, Material> MaterialMap;
    void Material_DestroyBuffer(VulkanBuffer& materialBuffer);
public:
    MaterialSystem();
    ~MaterialSystem();
    DLL_EXPORT void Update(const float& deltaTime);
    DLL_EXPORT VkGuid LoadMaterial(const String& materialPath);
    DLL_EXPORT const bool MaterialMapExists(const MaterialGuid& materialGuid) const;
    DLL_EXPORT const Material& FindMaterial(const MaterialGuid& materialGuid);
    DLL_EXPORT const Vector<Material>& MaterialList();
    DLL_EXPORT const Vector<VkDescriptorBufferInfo> GetMaterialPropertiesBuffer();
    DLL_EXPORT void Destroy(const MaterialGuid& materialGuid);
    DLL_EXPORT void DestroyAllMaterials();
};

extern DLL_EXPORT MaterialSystem materialSystem;

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT VkGuid MaterialSystem_CreateMaterial(const char* materialPath);
    DLL_EXPORT void MaterialSystem_Update(const float& deltaTime);
    DLL_EXPORT const bool MaterialSystem_MaterialMapExists(const MaterialGuid& materialGuid);
    DLL_EXPORT const Material& MaterialSystem_FindMaterial(const MaterialGuid& materialGuid);
    DLL_EXPORT void MaterialSystem_Destroy(const MaterialGuid& materialGuid);
    DLL_EXPORT void MaterialSystem_DestroyAllMaterials();
#ifdef __cplusplus
}
#endif