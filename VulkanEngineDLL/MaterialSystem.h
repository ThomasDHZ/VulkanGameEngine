#pragma once
#include "Platform.h"
#include "BufferSystem.h"

struct Material
{
    VkGuid materialGuid = VkGuid();
    VkGuid RenderPassGuid = VkGuid();
    size_t ShaderMaterialBufferIndex = SIZE_MAX;
    int MaterialBufferId;

    vec3 Albedo = vec3(0.8f, 0.8f, 0.8f);
    vec3 SheenColor = vec3(0.9f, 0.9f, 0.95f);
    vec3 SubSurfaceScatteringColor = vec3(1.0f, 0.5f, 0.4f);
    vec3 Emission = vec3(0.0f, 0.0f, 0.0f);
    float ClearcoatTint = 1.0f;
    float Metallic = 0.0f;
    float Roughness = 0.6f;
    float AmbientOcclusion = 1.0f;
    float ClearcoatStrength = 0.2f;
    float ClearcoatRoughness = 0.05f;
    float SheenIntensity = 0.4f;
    float Thickness = 0.5f;
    float NormalStrength = 1.0f;
    float HeightScale = 0.05f;
    float Height = 0.0f;
    float Alpha = 1.0f;

    VkGuid AlbedoMapId = VkGuid();
    VkGuid MetallicMapId = VkGuid();
    VkGuid RoughnessMapId = VkGuid();
    VkGuid ThicknessMapId = VkGuid();
    VkGuid SubSurfaceScatteringMapId = VkGuid();
    VkGuid SheenMapId = VkGuid();
    VkGuid ClearCoatMapId = VkGuid();
    VkGuid AmbientOcclusionMapId = VkGuid();
    VkGuid NormalMapId = VkGuid();
    VkGuid AlphaMapId = VkGuid();
    VkGuid EmissionMapId = VkGuid();
    VkGuid HeightMapId = VkGuid();

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
    void Material_DestroyBuffer(VulkanBuffer& materialBuffer);

public:
    DLL_EXPORT VkGuid LoadMaterial(const String& materialPath);
    DLL_EXPORT void Update(const float& deltaTime);
    DLL_EXPORT const bool MaterialMapExists(const MaterialGuid& materialGuid) const;
    DLL_EXPORT Material& FindMaterial(const MaterialGuid& materialGuid);
    DLL_EXPORT const Vector<VkDescriptorBufferInfo> GetMaterialPropertiesBuffer();
    DLL_EXPORT void Destroy(const MaterialGuid& materialGuid);
    DLL_EXPORT void DestroyAllMaterials();
};
extern DLL_EXPORT MaterialSystem& materialSystem;
inline MaterialSystem& MaterialSystem::Get()
{
    static MaterialSystem instance;
    return instance;
}