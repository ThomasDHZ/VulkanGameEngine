#pragma once
#include <DLL.h>
#include "Platform.h"
#include "MemoryPoolSystem.h"

enum MaterialBakerMemoryPoolTypes
{
    BakerMaterialBuffer,
    BakerTexture2DMetadataBuffer,
    //BakerTexture3DMetadataBuffer,
    //BakerTextureCubeMapMetadataBuffer,
    BakerEndofPool
};

struct MaterialBakerBufferHeader
{
    uint64 MaterialOffset = UINT64_MAX;
    uint32 MaterialCount = UINT32_MAX;
    uint32 MaterialSize = UINT32_MAX;
    uint64 Texture2DOffset = UINT64_MAX;
    uint32 Texture2DCount = UINT32_MAX;
    uint32 Texture2DSize = UINT32_MAX;
    uint64 Texture3DOffset = UINT64_MAX;
    uint32 Texture3DCount = UINT32_MAX;
    uint32 Texture3DSize = UINT32_MAX;
    uint64 TextureCubeMapOffset = UINT64_MAX;
    uint32 TextureCubeMapCount = UINT32_MAX;
    uint32 TextureCubeMapSize = UINT32_MAX;
};

struct ImportMaterialShader
{
    vec3  Albedo;
    vec3  SheenColor;
    vec3  SubSurfaceScatteringColor;
    vec3  Emission;
    float ClearcoatTint;
    float Metallic;
    float Roughness;
    float AmbientOcclusion;
    float ClearcoatStrength;
    float ClearcoatRoughness;
    float SheenIntensity;
    float Thickness;
    float Anisotropy;
    float AnisotropyRotation;
    float NormalStrength;
    float HeightScale;
    float Height;
    float Alpha;

    uint AlbedoMap;
    uint MetallicMap;
    uint RoughnessMap;
    uint ThicknessMap;
    uint SubSurfaceScatteringColorMap;
    uint SheenMap;
    uint ClearCoatMap;
    uint AnisotropyMap;
    uint AmbientOcclusionMap;
    uint NormalMap;
    uint AlphaMap;
    uint EmissionMap;
    uint HeightMap;
};

class MaterialMemoryPoolSystem
{
public:
    static MaterialMemoryPoolSystem& Get();

private:
    MaterialMemoryPoolSystem() = default;
    ~MaterialMemoryPoolSystem() = default;
    MaterialMemoryPoolSystem(const MaterialMemoryPoolSystem&) = delete;
    MaterialMemoryPoolSystem& operator=(const MaterialMemoryPoolSystem&) = delete;
    MaterialMemoryPoolSystem(MaterialMemoryPoolSystem&&) = delete;
    MaterialMemoryPoolSystem& operator=(MaterialMemoryPoolSystem&&) = delete;

    void													              UpdateMemoryPoolHeader(MaterialBakerMemoryPoolTypes memoryPoolType, uint32 newPoolSize);
    void													              ResizeMemoryPool(MaterialBakerMemoryPoolTypes memoryPoolToUpdate, uint32 resizeCount);
    void													              CreateMaterialBakerBindlessDescriptorSet();

public:
    static constexpr size_t									              BakerMaterialCapacity = 1;
    static constexpr size_t									              BakerTexture2DCapacity = 20;
    //static constexpr size_t									              BakerTexture3DCapacity = 4;
    //static constexpr size_t									              BakerTextureCubeMapCapacity = 4;

    static constexpr uint									              BakerMaterialDescriptorBinding = 0;
   // static constexpr uint									              BakerTextureCubeMapBinding = 1;
    static constexpr uint									              BakerTexture2DBinding = 1;
    //static constexpr uint									              BakerTexture3DBinding = 3;

    bool													              IsHeaderDirty = true;
    bool													              IsDescriptorSetDirty = true;

    VkDescriptorPool										              MaterialBakerBindlessPool = VK_NULL_HANDLE;
    VkDescriptorSet											              MaterialBakerBindlessDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout									              MaterialBakerBindlessDescriptorSetLayout = VK_NULL_HANDLE;

    uint32                                                                MaterialBakerBufferId = UINT32_MAX;
    size_t													              MaterialMemoryPoolSize = UINT32_MAX;
    UnorderedMap<MaterialBakerMemoryPoolTypes, MemoryPoolSubBufferHeader> MemorySubPoolHeader;
    MaterialBakerBufferHeader									          MaterialPoolHeader;
    void* MaterialBufferPtr = nullptr;
    Vector<byte>											              MaterialBufferMemoryPool;

    DLL_EXPORT void											              StartUp();
    DLL_EXPORT uint32										              AllocateObject(MaterialBakerMemoryPoolTypes memoryPoolToUpdate);
    DLL_EXPORT void											              UpdateMemoryPool(Vector<VulkanPipeline>& pipelineList);
    DLL_EXPORT ImportMaterialShader&                                      UpdateMaterial(uint32 index);
    DLL_EXPORT void											              UpdateTextureDescriptorSet(Texture& texture, uint binding);
    DLL_EXPORT void											              UpdateDataBufferDescriptorSet(uint32 vulkanBufferIndex, uint binding);
    DLL_EXPORT void											              FreeObject(MaterialBakerMemoryPoolTypes memoryPoolToUpdate, uint32 index);

};
extern DLL_EXPORT MaterialMemoryPoolSystem& materialMemoryPoolSystem;
inline MaterialMemoryPoolSystem& MaterialMemoryPoolSystem::Get()
{
    static MaterialMemoryPoolSystem instance;
    return instance;
}
