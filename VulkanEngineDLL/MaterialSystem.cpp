#include "MaterialSystem.h"
#include "FileSystem.h"
#include "RenderSystem.h"
#include "BufferSystem.h"
#include "MemoryPoolSystem.h"
#include "from_json.h"

MaterialSystem& materialSystem = MaterialSystem::Get();

VkGuid MaterialSystem::LoadMaterial(const String& materialPath)
{
    if (materialPath.empty()) return VkGuid::Empty();

    nlohmann::json json = fileSystem.LoadJsonFile(materialPath.c_str());
    return LoadMaterial(json);
}

VkGuid MaterialSystem::LoadMaterial(const nlohmann::json& json)
{
    VkGuid materialGuid = VkGuid(json["MaterialId"].get<std::string>());
    if (MaterialExists(materialGuid))
    {
        return materialGuid;
    }

    Material material;
    material.MaterialGuid                             = materialGuid;
    material.AlbedoTextureId                          = json.contains("AlbedoTexture") ? textureSystem.LoadTexture(json["AlbedoTexture"].get<TextureLoader>()).textureGuid : VkGuid();
    material.NormalTextureId                          = json.contains("NormalTexture") ? textureSystem.LoadTexture(json["NormalTexture"].get<TextureLoader>()).textureGuid : VkGuid();
    material.MroTextureId                             = json.contains("MROTexture") ? textureSystem.LoadTexture(json["MROTexture"].get<TextureLoader>()).textureGuid : VkGuid();
    material.ClearCoatTextureId                       = json.contains("ClearCoatTexture") ? textureSystem.LoadTexture(json["ClearCoatTexture"].get<TextureLoader>()).textureGuid : VkGuid();
    material.SubSurfaceScatteringTextureId            = json.contains("SubSurfaceScatteringTexture") ? textureSystem.LoadTexture(json["SubSurfaceScatteringTexture"].get<TextureLoader>()).textureGuid : VkGuid();
    material.SubSurfaceScatteringPropertiesTextureId  = json.contains("SubSurfaceScatteringPropertiesTexture") ? textureSystem.LoadTexture(json["SubSurfaceScatteringPropertiesTexture"].get<TextureLoader>()).textureGuid : VkGuid();
    material.SheenTextureId                           = json.contains("SheenTexture") ? textureSystem.LoadTexture(json["SheenTexture"].get<TextureLoader>()).textureGuid : VkGuid();
    material.AnisotropyTextureId                      = json.contains("AnisotropyTexture") ? textureSystem.LoadTexture(json["AnisotropyTexture"].get<TextureLoader>()).textureGuid : VkGuid();
    material.EmissionTextureId                        = json.contains("EmissionTexture") ? textureSystem.LoadTexture(json["EmissionTexture"].get<TextureLoader>()).textureGuid : VkGuid();
    material.TranslucentTextureId                     = json.contains("TranslucentTexture") ? textureSystem.LoadTexture(json["TranslucentTexture"].get<TextureLoader>()).textureGuid : VkGuid();
    material.TranslucentPropertiesTextureId           = json.contains("TranslucentPropertiesTexture") ? textureSystem.LoadTexture(json["TranslucentPropertiesTexture"].get<TextureLoader>()).textureGuid : VkGuid();
    material.ShadingModel                             = json.value("ShadingModel", 0u);
    material.FeatureMask                              = json.value("FeatureMask", 0u);
    material.ClearcoatTint                            = vec4(json["ClearcoatTint"].at(0).get<float>(), json["ClearcoatTint"].at(1).get<float>(), json["ClearcoatTint"].at(2).get<float>(), 0.0f);
    material.IOR                                      = json.value("IOR", 1.45f);
    material.AlphaCutOff                              = json.value("AlphaCutoff", json.value("AlphaCutOff", 0.1f));
    MaterialList.emplace_back(material);

    uint32 poolIndex = memoryPoolSystem.AllocateObject(kMaterialBuffer);
    GPUMaterial& gpuMaterial = memoryPoolSystem.UpdateMaterial(poolIndex);
    gpuMaterial.AlbedoTextureId                         = material.AlbedoTextureId != VkGuid() ? textureSystem.FindTexture(material.AlbedoTextureId).gpuTextureBufferIndex : UINT32_MAX;
    gpuMaterial.NormalTextureId                         = material.NormalTextureId != VkGuid() ? textureSystem.FindTexture(material.NormalTextureId).gpuTextureBufferIndex : UINT32_MAX;
    gpuMaterial.MroTextureId                            = material.MroTextureId != VkGuid() ? textureSystem.FindTexture(material.MroTextureId).gpuTextureBufferIndex : UINT32_MAX;
    gpuMaterial.ClearCoatTextureId                      = material.ClearCoatTextureId != VkGuid() ? textureSystem.FindTexture(material.ClearCoatTextureId).gpuTextureBufferIndex : UINT32_MAX;
    gpuMaterial.SubSurfaceScatteringTextureId           = material.SubSurfaceScatteringTextureId != VkGuid() ? textureSystem.FindTexture(material.SubSurfaceScatteringTextureId).gpuTextureBufferIndex : UINT32_MAX;
    gpuMaterial.SubSurfaceScatteringPropertiesTextureId = material.SubSurfaceScatteringPropertiesTextureId != VkGuid() ? textureSystem.FindTexture(material.SubSurfaceScatteringPropertiesTextureId).gpuTextureBufferIndex : UINT32_MAX;
    gpuMaterial.SheenTextureId                          = material.SheenTextureId != VkGuid() ? textureSystem.FindTexture(material.SheenTextureId).gpuTextureBufferIndex : UINT32_MAX;
    gpuMaterial.AnisotropyTextureId                     = material.AnisotropyTextureId != VkGuid() ? textureSystem.FindTexture(material.AnisotropyTextureId).gpuTextureBufferIndex : UINT32_MAX;
    gpuMaterial.EmissionTextureId                       = material.EmissionTextureId != VkGuid() ? textureSystem.FindTexture(material.EmissionTextureId).gpuTextureBufferIndex : UINT32_MAX;
    gpuMaterial.TranslucentTextureId                    = material.TranslucentTextureId != VkGuid() ? textureSystem.FindTexture(material.TranslucentTextureId).gpuTextureBufferIndex : UINT32_MAX;
    gpuMaterial.TranslucentPropertiesTextureId          = material.TranslucentPropertiesTextureId != VkGuid() ? textureSystem.FindTexture(material.TranslucentPropertiesTextureId).gpuTextureBufferIndex : UINT32_MAX;
    gpuMaterial.ShadingModel                            = material.ShadingModel;
    gpuMaterial.FeatureMask                             = material.FeatureMask;
    gpuMaterial.ClearcoatTint                           = material.ClearcoatTint;
    gpuMaterial.IOR                                     = material.IOR;
    gpuMaterial.AlphaCutOff                             = material.AlphaCutOff;
    GuidToPoolIndex[materialGuid] = poolIndex;
    return materialGuid;
}

const bool MaterialSystem::MaterialExists(const MaterialGuid& materialGuid) const
{
    auto it = GuidToPoolIndex.find(materialGuid);
    return it != GuidToPoolIndex.end();
}

Material& MaterialSystem::FindMaterial(const MaterialGuid& materialGuid)
{
    auto it = GuidToPoolIndex.find(materialGuid);
    if (it == GuidToPoolIndex.end())
    {
        std::cerr << "Couldn't find Material Id:" + materialGuid.ToString() << std::endl;
        static Material invalid{};
        return invalid;
    }
    return MaterialList[it->second];
}

uint MaterialSystem::FindMaterialPoolIndex(const MaterialGuid& materialGuid)
{
    auto it = GuidToPoolIndex.find(materialGuid);
    return it != GuidToPoolIndex.end() ? it->second : UINT32_MAX;
}

void MaterialSystem::Destroy(const MaterialGuid& materialGuid)
{
    //auto it = GuidToPoolIndex.find(materialGuid);
    //if (it == GuidToPoolIndex.end()) return;

    //uint32_t index = it->second;
    //GuidToPoolIndex.erase(it);

    //if (index < MaterialList.size())
    //{
    //    MaterialList.erase(MaterialList.begin() + index);
    //    MaterialPool.erase(MaterialPool.begin() + index);
    //}

    //// Fix indices after deletion
    //for (auto& pair : GuidToPoolIndex)
    //{
    //    if (pair.second > index)
    //        pair.second--;
    //}
}

void MaterialSystem::Destroy()
{
    //MaterialList.clear();
    //MaterialPool.clear();
    //GuidToPoolIndex.clear();

    //if (MaterialBufferId != UINT32_MAX)
    //{
    //    bufferSystem.DestroyBuffer(bufferSystem.FindVulkanBuffer(MaterialBufferId));
    //    MaterialBufferId = UINT32_MAX;
    //}
}