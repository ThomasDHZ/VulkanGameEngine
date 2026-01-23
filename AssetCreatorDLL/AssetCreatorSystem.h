#pragma once
#include <DLL.h>
#include <Platform.h>
#include <JsonStruct.h>
#include <RenderSystem.h>
#include <VulkanSystem.h>
#include <TextureSystem.h>
#include <MaterialSystem.h>

struct ImportMaterial
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

    ImportMaterial() = default;
    ImportMaterial(const ImportMaterial&) = default;
    ImportMaterial& operator=(const ImportMaterial&) = default;
};

class AssetCreatorSystem
{
public:
    static AssetCreatorSystem& Get();

private:
    AssetCreatorSystem() = default;
    ~AssetCreatorSystem() = default;
    AssetCreatorSystem(const AssetCreatorSystem&) = delete;
    AssetCreatorSystem& operator=(const AssetCreatorSystem&) = delete;
    AssetCreatorSystem(AssetCreatorSystem&&) = delete;
    AssetCreatorSystem& operator=(AssetCreatorSystem&&) = delete;
    
    VkSampler GetAlbedoMapSamplerSettings();
    VkSampler GetNormalMapSamplerSettings();
    VkSampler GetPackedORMMapSamplerSettings();
    VkSampler GetParallaxMapSamplerSettings();
    VkSampler GetAlphaMapSamplerSettings();
    VkSampler GetThicknessMapSamplerSettings();
    VkSampler GetSubSurfaceScatteringMapSamplerSettings();
    VkSampler GetSheenMapSamplerSettings();
    VkSampler GetClearCoatMapSamplerSettings();
    VkSampler GetEmissionMapSamplerSettings();
public:
    ImportMaterial material;
    VulkanRenderPass vulkanRenderPass;
    VulkanPipeline   vulkanRenderPipeline;
    DLL_EXPORT void MaterialUpdate(ImportMaterial& material);
    DLL_EXPORT VkDescriptorImageInfo GetTextureDescriptorbinding(Texture texture, VkSampler sampler);
    DLL_EXPORT void BuildRenderPass(const String& materialPath);
    DLL_EXPORT void Run(String materialPath);
    DLL_EXPORT void Draw();
};
extern DLL_EXPORT AssetCreatorSystem& assetCreatorSystem;
inline AssetCreatorSystem& AssetCreatorSystem::Get()
{
    static AssetCreatorSystem instance;
    return instance;
}

