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
    material.ShaderMaterialBufferIndex = MaterialList.size();
    material.MaterialBufferId = bufferId;
    material.AlbedoMapId = VkGuid(json["AlbedoMapId"].get<std::string>());
    material.SpecularMapId = VkGuid(json["SpecularMapId"].get<std::string>());
    material.MetallicMapId = VkGuid(json["MetallicMapId"].get<std::string>());
    material.RoughnessMapId = VkGuid(json["RoughnessMapId"].get<std::string>());
    material.AmbientOcclusionMapId = VkGuid(json["AmbientOcclusionMapId"].get<std::string>());
    material.NormalMapId = VkGuid(json["NormalMapId"].get<std::string>());
    material.AlphaMapId = VkGuid(json["AlphaMapId"].get<std::string>());
    material.EmissionMapId = VkGuid(json["EmissionMapId"].get<std::string>());
    material.HeightMapId = VkGuid(json["HeightMapId"].get<std::string>());

    material.Albedo = vec3(json["Albedo"][0], json["Albedo"][1], json["Albedo"][2]);
    material.Emission = vec3(json["Emission"][0], json["Emission"][1], json["Emission"][2]);
    material.Specular = json["Specular"];
    material.Metallic = json["Metallic"];
    material.Roughness = json["Roughness"];
    material.AmbientOcclusion = json["AmbientOcclusion"];
    material.Alpha = json["Alpha"];
    material.HeightScale = json["HeightScale"];
    material.Height = json["Height"];
    MaterialList.emplace_back(material);
    return materialId;
}

void MaterialSystem::Update(const float& deltaTime)
{
    for (auto& material: MaterialList)
    {
        const uint AlbedoMapId = material.AlbedoMapId != VkGuid() ? textureSystem.FindTexture(material.AlbedoMapId).textureIndex : SIZE_MAX;
        const uint SpecularMapId = material.SpecularMapId != VkGuid() ? textureSystem.FindTexture(material.SpecularMapId).textureIndex : SIZE_MAX;
        const uint MetallicMapId = material.MetallicMapId != VkGuid() ? textureSystem.FindTexture(material.MetallicMapId).textureIndex : SIZE_MAX;
        const uint RoughnessMapId = material.RoughnessMapId != VkGuid() ? textureSystem.FindTexture(material.RoughnessMapId).textureIndex : SIZE_MAX;
        const uint AmbientOcclusionMapId = material.AmbientOcclusionMapId != VkGuid() ? textureSystem.FindTexture(material.AmbientOcclusionMapId).textureIndex : SIZE_MAX;
        const uint NormalMapId = material.NormalMapId != VkGuid() ? textureSystem.FindTexture(material.NormalMapId).textureIndex : SIZE_MAX;
        const uint AlphaMapId = material.AlphaMapId != VkGuid() ? textureSystem.FindTexture(material.AlphaMapId).textureIndex : SIZE_MAX;
        const uint EmissionMapId = material.EmissionMapId != VkGuid() ? textureSystem.FindTexture(material.EmissionMapId).textureIndex : SIZE_MAX;
        const uint HeightMapId = material.HeightMapId != VkGuid() ? textureSystem.FindTexture(material.HeightMapId).textureIndex : SIZE_MAX;

        ShaderStructDLL& shaderStruct = shaderSystem.FindShaderStruct(material.MaterialBufferId);
        shaderSystem.UpdateShaderStructValue<vec3>(shaderStruct, "Albedo", material.Albedo);
        shaderSystem.UpdateShaderStructValue<vec3>(shaderStruct, "Emission", material.Emission);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "Metallic", material.Metallic);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "Roughness", material.Roughness);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "AmbientOcclusion", material.AmbientOcclusion);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "Alpha", material.Alpha);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "HeightScale", material.HeightScale);
        shaderSystem.UpdateShaderStructValue<float>(shaderStruct, "Height", material.Height);

        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AlbedoMap", AlbedoMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "SpecularMap", SpecularMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "MetallicMap", MetallicMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "RoughnessMap", RoughnessMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AmbientOcclusionMap", AmbientOcclusionMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "NormalMap", NormalMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AlphaMap", AlphaMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "EmissionMap", EmissionMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "HeightMap", HeightMapId);
        shaderSystem.UpdateShaderBuffer(shaderStruct, material.MaterialBufferId);
    }
}

const bool MaterialSystem::MaterialMapExists(const MaterialGuid& materialGuid) const
{
    auto it = std::find_if(MaterialList.begin(), MaterialList.end(),
        [&materialGuid](const Material& material)
        {
            return material.materialGuid == materialGuid;
        });
    return it != MaterialList.end();
}

Material& MaterialSystem::FindMaterial(const MaterialGuid& materialGuid)
{
    auto it = std::find_if(MaterialList.begin(), MaterialList.end(),
        [&materialGuid](const Material& material)
        {
            return material.materialGuid == materialGuid;
        });
    return *it;
}

const Vector<VkDescriptorBufferInfo> MaterialSystem::GetMaterialPropertiesBuffer()
{
    Vector<VkDescriptorBufferInfo> materialPropertiesBuffer;
    if (MaterialList.empty())
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
        for (auto& material : MaterialList)
        {
            VkDescriptorBufferInfo meshBufferInfo =
            {
                .buffer = bufferSystem.FindVulkanBuffer(material.MaterialBufferId).Buffer,
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
    Material& material = FindMaterial(materialGuid);
    VulkanBuffer& materialBuffer = bufferSystem.VulkanBufferMap[material.MaterialBufferId];
    //Material_DestroyBuffer(renderSystem.renderer, materialBuffer);
    bufferSystem.VulkanBufferMap.erase(material.MaterialBufferId);
}

void MaterialSystem::DestroyAllMaterials()
{
    for (auto& materialPair : MaterialList)
    {
        Destroy(materialPair.materialGuid);
    }
}