#include "MaterialSystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include "ShaderSystem.h"

MaterialSystem& materialSystem = MaterialSystem::Get();

VkGuid MaterialSystem::LoadMaterial(const String& materialPath)
{
    if (materialPath.empty() || materialPath.c_str() == nullptr)
        return VkGuid::Empty();

    nlohmann::json json = fileSystem.LoadJsonFile(materialPath.c_str());
    VkGuid materialId = VkGuid(json["MaterialId"].get<std::string>());

    if (MaterialMapExists(materialId))
    {
        return materialId;
    }

    ShaderStructDLL shaderStruct = shaderSystem.CopyShaderStructProtoType("MaterialProperitiesBuffer");
    uint32 bufferId = bufferSystem.VMACreateDynamicBuffer(&shaderStruct, shaderStruct.ShaderBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    shaderSystem.PipelineShaderStructMap[bufferId] = shaderStruct;

    Material material;
    material.materialGuid = materialId;
    material.ShaderMaterialBufferIndex = materialSystem.MaterialMap.size();
    material.MaterialBufferId = bufferId;
    material.AlbedoMapId = VkGuid(json["AlbedoMapId"].get<std::string>());
    material.MetallicRoughnessMapId = VkGuid(json["MetallicRoughnessMapId"].get<std::string>());
    material.MetallicMapId = VkGuid(json["MetallicMapId"].get<std::string>());
    material.RoughnessMapId = VkGuid(json["RoughnessMapId"].get<std::string>());
    material.AmbientOcclusionMapId = VkGuid(json["AmbientOcclusionMapId"].get<std::string>());
    material.NormalMapId = VkGuid(json["NormalMapId"].get<std::string>());
    material.AlphaMapId = VkGuid(json["AlphaMapId"].get<std::string>());
    material.EmissionMapId = VkGuid(json["EmissionMapId"].get<std::string>());
    material.HeightMapId = VkGuid(json["HeightMapId"].get<std::string>());
    material.Albedo = vec3(json["Albedo"][0], json["Albedo"][1], json["Albedo"][2]);
    material.Emission = vec3(json["Emission"][0], json["Emission"][1], json["Emission"][2]);
    material.Metallic = json["Metallic"];
    material.Roughness = json["Roughness"];
    material.AmbientOcclusion = json["AmbientOcclusion"];
    material.Alpha = json["Alpha"];
    material.HeightScale = json["HeightScale"];
    materialSystem.MaterialMap.emplace(material.materialGuid, std::move(material));
    return materialId;
}

void MaterialSystem::Update(const float& deltaTime)
{
    uint x = 0;
    for (auto& materialPair : materialSystem.MaterialMap)
    {
        materialPair.second.ShaderMaterialBufferIndex = x;

        const Material material = materialPair.second;
        const uint AlbedoMapId = material.AlbedoMapId != VkGuid() ? textureSystem.FindTexture(material.AlbedoMapId).textureBufferIndex : MAXUINT32;
        const uint MetallicRoughnessMapId = material.MetallicRoughnessMapId != VkGuid() ? textureSystem.FindTexture(material.MetallicRoughnessMapId).textureBufferIndex : MAXUINT32;
        const uint MetallicMapId = material.MetallicMapId != VkGuid() ? textureSystem.FindTexture(material.MetallicMapId).textureBufferIndex : MAXUINT32;
        const uint RoughnessMapId = material.RoughnessMapId != VkGuid() ? textureSystem.FindTexture(material.RoughnessMapId).textureBufferIndex : MAXUINT32;
        const uint AmbientOcclusionMapId = material.AmbientOcclusionMapId != VkGuid() ? textureSystem.FindTexture(material.AmbientOcclusionMapId).textureBufferIndex : MAXUINT32;
        const uint NormalMapId = material.NormalMapId != VkGuid() ? textureSystem.FindTexture(material.NormalMapId).textureBufferIndex : MAXUINT32;
        const uint AlphaMapId = material.AlphaMapId != VkGuid() ? textureSystem.FindTexture(material.AlphaMapId).textureBufferIndex : MAXUINT32;
        const uint EmissionMapId = material.EmissionMapId != VkGuid() ? textureSystem.FindTexture(material.EmissionMapId).textureBufferIndex : MAXUINT32;
        const uint HeightMapId = material.HeightMapId != VkGuid() ? textureSystem.FindTexture(material.HeightMapId).textureBufferIndex : MAXUINT32;

        ShaderStructDLL shaderStruct = shaderSystem.FindShaderStruct(material.MaterialBufferId);
        shaderSystem.UpdateShaderStructValue<vec3>(shaderStruct, "Albedo", material.Albedo);
        shaderSystem.UpdateShaderStructValue<vec3>(shaderStruct, "Emission", material.Emission);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "Metallic", material.Metallic);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "Roughness", material.Roughness);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "AmbientOcclusion", material.AmbientOcclusion);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "Alpha", material.Alpha);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "HeightScale", material.HeightScale);

        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AlbedoMap", AlbedoMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "MetallicRoughnessMap", MetallicRoughnessMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "MetallicMap", MetallicMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "RoughnessMap", RoughnessMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AmbientOcclusionMap", AmbientOcclusionMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "NormalMap", NormalMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AlphaMap", AlphaMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "EmissionMap", EmissionMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "HeightMap", HeightMapId);
        shaderSystem.UpdateShaderBuffer(material.MaterialBufferId);
        x++;
    }
}

const bool MaterialSystem::MaterialMapExists(const MaterialGuid& materialGuid) const
{
    return materialSystem.MaterialMap.contains(materialGuid);
}

const Material& MaterialSystem::FindMaterial(const MaterialGuid& materialGuid)
{
    return materialSystem.MaterialMap.at(materialGuid);
}

const Vector<Material>& MaterialSystem::MaterialList()
{
    Vector<Material> materialList;
    for (const auto& material : materialSystem.MaterialMap)
    {
        materialList.emplace_back(material.second);
    }
    return materialList;
}

const Vector<VkDescriptorBufferInfo> MaterialSystem::GetMaterialPropertiesBuffer()
{
    Vector<VkDescriptorBufferInfo> materialPropertiesBuffer;
    if (materialSystem.MaterialMap.empty())
    {
        materialPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
            {
                .buffer = VK_NULL_HANDLE,
                .offset = 0,
                .range = VK_WHOLE_SIZE
            });
    }
    else
    {
        for (auto& material : materialSystem.MaterialMap)
        {
            VkDescriptorBufferInfo meshBufferInfo =
            {
                .buffer = bufferSystem.FindVulkanBuffer(material.second.MaterialBufferId).Buffer,
                .offset = 0,
                .range = VK_WHOLE_SIZE
            };
            materialPropertiesBuffer.emplace_back(meshBufferInfo);
        }
    }
    return materialPropertiesBuffer;
}

void MaterialSystem::Destroy(const MaterialGuid& materialGuid)
{
    Material& material = materialSystem.MaterialMap[materialGuid];
    VulkanBuffer& materialBuffer = bufferSystem.VulkanBufferMap[material.MaterialBufferId];
    //Material_DestroyBuffer(renderSystem.renderer, materialBuffer);
    bufferSystem.VulkanBufferMap.erase(material.MaterialBufferId);
}

void MaterialSystem::DestroyAllMaterials()
{
    for (auto& materialPair : materialSystem.MaterialMap)
    {
        Destroy(materialPair.second.materialGuid);
    }
}