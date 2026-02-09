#pragma once
#include <DLL.h>
#include <Platform.h>
#include <JsonStruct.h>
#include <RenderSystem.h>
#include <VulkanSystem.h>
#include <TextureSystem.h>
#include <MaterialSystem.h>
#include "TextureBakerSystem.h"

struct ImportMaterial
{
    VkGuid materialGuid = VkGuid();
    VkGuid RenderPassGuid = VkGuid();
    size_t ShaderMaterialBufferIndex = SIZE_MAX;
    int MaterialBufferId = -1;
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

    Texture AlbedoMap;
    Texture MetallicMap;
    Texture RoughnessMap;
    Texture ThicknessMap;
    Texture SubSurfaceScatteringMap;
    Texture SheenMap;
    Texture ClearCoatMap;
    Texture AmbientOcclusionMap;
    Texture NormalMap;
    Texture AlphaMap;
    Texture EmissionMap;
    Texture HeightMap;
};

class MaterialBakerSystem
{
public:
    static MaterialBakerSystem& Get();

private:
    MaterialBakerSystem() = default;
    ~MaterialBakerSystem() = default;
    MaterialBakerSystem(const MaterialBakerSystem&) = delete;
    MaterialBakerSystem& operator=(const MaterialBakerSystem&) = delete;
    MaterialBakerSystem(MaterialBakerSystem&&) = delete;
    MaterialBakerSystem& operator=(MaterialBakerSystem&&) = delete;

    VkSampler cachedAlbedoSampler = VK_NULL_HANDLE;
    VkSampler cachedNormalSampler = VK_NULL_HANDLE;
    VkSampler cachedPackedORMSampler = VK_NULL_HANDLE;
    VkSampler cachedParallaxSampler = VK_NULL_HANDLE;
    VkSampler cachedAlphaSampler = VK_NULL_HANDLE;
    Texture dummyTextureScalar;   
    Texture dummyTextureColor;      
    Texture dummyTextureNormal;   

    void      InitDummyAndSamplers();

public:
    ShaderStructDLL shaderStruct;
    ImportMaterial material;
    VulkanRenderPass vulkanRenderPass;
    VulkanPipeline vulkanRenderPipeline;
    Vector<VkDescriptorImageInfo> textureBindingList;

    DLL_EXPORT void TransitionImageLayout(Texture& texture, VkImageLayout newLayout, uint32 baseMipLevel = 0, uint32 levelCount = VK_REMAINING_MIP_LEVELS);
    DLL_EXPORT void CreateTextureImage(Texture& texture, VkImageCreateInfo& imageCreateInfo, Vector<byte>& textureData, uint layerCount);
    DLL_EXPORT void CreateTextureView(Texture& texture, VkImageAspectFlags imageAspectFlags);
    DLL_EXPORT void CleanRenderPass();
    DLL_EXPORT Texture LoadTexture(TextureLoader textureLoader, size_t bindingNumber);
    DLL_EXPORT void LoadMaterial(const String& materialPath);
    DLL_EXPORT void UpdateDescriptorSets();
    DLL_EXPORT VkDescriptorImageInfo GetTextureDescriptorbinding(Texture texture, VkSampler sampler);
    DLL_EXPORT void BuildRenderPass(ivec2 renderPassResolution);
    DLL_EXPORT void Run();
    DLL_EXPORT void Draw(VkCommandBuffer& commandBuffer);
    DLL_EXPORT void CleanInputResources();
};

extern DLL_EXPORT MaterialBakerSystem& materialBakerSystem;
inline MaterialBakerSystem& MaterialBakerSystem::Get()
{
    static MaterialBakerSystem instance;
    return instance;
}