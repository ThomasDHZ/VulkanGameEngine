#pragma once
#include "Platform.h"
#include "BufferSystem.h"

struct Material
{
    VkGuid materialGuid = VkGuid();
    VkGuid RenderPassGuid = VkGuid();
    size_t ShaderMaterialBufferIndex = SIZE_MAX;
    int MaterialBufferId;

    VkGuid AlbedoMapId = VkGuid();
    VkGuid SpecularMapId = VkGuid();
    VkGuid MetallicMapId = VkGuid();
    VkGuid RoughnessMapId = VkGuid();
    VkGuid AmbientOcclusionMapId = VkGuid();
    VkGuid NormalMapId = VkGuid();
    VkGuid AlphaMapId = VkGuid();
    VkGuid EmissionMapId = VkGuid();
    VkGuid HeightMapId = VkGuid();

    vec3  Albedo = vec3(0.8f);
    float Specular = 0.0f;
    float Metallic = 0.0f;
    float Roughness = 0.5f;
    float AmbientOcclusion = 1.0f;
    float Alpha = 1.0f;
    float NormalStrength = 0.5;
    float HeightScale = 0.04f;
    float Height = 0.0f;
    vec3  Emission = vec3(0.0f);

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