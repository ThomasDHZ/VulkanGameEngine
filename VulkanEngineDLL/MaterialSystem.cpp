#include "MaterialSystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include "ShaderSystem.h"

MaterialSystem materialSystem;

MaterialSystem::MaterialSystem()
{
}

MaterialSystem::~MaterialSystem()
{
}

VkGuid MaterialSystem::LoadMaterial(const String& materialPath)
{
    if (materialPath.empty() || materialPath.c_str() == nullptr)
        return VkGuid::Empty();

    nlohmann::json json = fileSystem.LoadJsonFile(materialPath.c_str());
    VkGuid materialId = VkGuid(json["MaterialId"].get<std::string>());

    if (MaterialSystem_MaterialMapExists(materialId))
        return materialId;

    int bufferIndex = ++NextBufferId;

    shaderSystem.PipelineShaderStructMap[bufferIndex] =
        shaderSystem.CopyShaderStructProtoType("MaterialProperitiesBuffer");

    bufferSystem.VulkanBufferMap[bufferIndex] = bufferSystem.CreateVulkanBuffer(
        bufferIndex,
        shaderSystem.PipelineShaderStructMap[bufferIndex].ShaderBufferSize,
        1,
        BufferTypeEnum::BufferType_MaterialProperitiesBuffer,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        false
    );

    Material material;
    material.materialGuid = materialId;
    material.ShaderMaterialBufferIndex = 0;
    material.MaterialBufferId = bufferIndex;
    material.AlbedoMapId = VkGuid(json["AlbedoMapId"].get<std::string>());
    material.MetallicRoughnessMapId = VkGuid(json["MetallicRoughnessMapId"].get<std::string>());
    material.MetallicMapId = VkGuid(json["MetallicMapId"].get<std::string>());
    material.RoughnessMapId = VkGuid(json["RoughnessMapId"].get<std::string>());
    material.AmbientOcclusionMapId = VkGuid(json["AmbientOcclusionMapId"].get<std::string>());
    material.NormalMapId = VkGuid(json["NormalMapId"].get<std::string>());
    material.DepthMapId = VkGuid(json["DepthMapId"].get<std::string>());
    material.AlphaMapId = VkGuid(json["AlphaMapId"].get<std::string>());
    material.EmissionMapId = VkGuid(json["EmissionMapId"].get<std::string>());
    material.HeightMapId = VkGuid(json["HeightMapId"].get<std::string>());
    material.Albedo = vec3(json["Albedo"][0], json["Albedo"][1], json["Albedo"][2]);
    material.Emission = vec3(json["Emission"][0], json["Emission"][1], json["Emission"][2]);
    material.Metallic = json["Metallic"];
    material.Roughness = json["Roughness"];
    material.AmbientOcclusion = json["AmbientOcclusion"];
    material.Alpha = json["Alpha"];
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
        const uint AlbedoMapId = material.AlbedoMapId != VkGuid() ? TextureSystem_FindTexture(material.AlbedoMapId).textureBufferIndex : 0;
        const uint MetallicRoughnessMapId = material.MetallicRoughnessMapId != VkGuid() ? TextureSystem_FindTexture(material.MetallicRoughnessMapId).textureBufferIndex : 0;
        const uint MetallicMapId = material.MetallicMapId != VkGuid() ? TextureSystem_FindTexture(material.MetallicMapId).textureBufferIndex : 0;
        const uint RoughnessMapId = material.RoughnessMapId != VkGuid() ? TextureSystem_FindTexture(material.RoughnessMapId).textureBufferIndex : 0;
        const uint AmbientOcclusionMapId = material.AmbientOcclusionMapId != VkGuid() ? TextureSystem_FindTexture(material.AmbientOcclusionMapId).textureBufferIndex : 0;
        const uint NormalMapId = material.NormalMapId != VkGuid() ? TextureSystem_FindTexture(material.NormalMapId).textureBufferIndex : 0;
        const uint DepthMapId = material.DepthMapId != VkGuid() ? TextureSystem_FindTexture(material.DepthMapId).textureBufferIndex : 0;
        const uint AlphaMapId = material.AlphaMapId != VkGuid() ? TextureSystem_FindTexture(material.AlphaMapId).textureBufferIndex : 0;
        const uint EmissionMapId = material.EmissionMapId != VkGuid() ? TextureSystem_FindTexture(material.EmissionMapId).textureBufferIndex : 0;
        const uint HeightMapId = material.HeightMapId != VkGuid() ? TextureSystem_FindTexture(material.HeightMapId).textureBufferIndex : 0;

        ShaderStructDLL shaderStruct = shaderSystem.FindShaderStruct(material.MaterialBufferId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AlbedoMap", AlbedoMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "MetallicRoughnessMap", AlbedoMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "MetallicMap", AlbedoMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "RoughnessMap", AlbedoMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AmbientOcclusionMap", AlbedoMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "NormalMap", AlbedoMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "DepthMap", AlbedoMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AlphaMap", AlbedoMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "EmissionMap", AlbedoMapId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "HeightMap", AlbedoMapId);
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
        MaterialSystem_Destroy(materialPair.second.materialGuid);
    }
}

VkGuid MaterialSystem_CreateMaterial(const char* materialPath)
{
    return materialSystem.LoadMaterial(materialPath);
}

void MaterialSystem_Update(const float& deltaTime)
{
    materialSystem.Update(deltaTime);
}

const bool MaterialSystem_MaterialMapExists(const MaterialGuid& materialGuid)
{
    return materialSystem.MaterialMapExists(materialGuid);
}

const Material& MaterialSystem_FindMaterial(const RenderPassGuid& guid)
{
    return materialSystem.FindMaterial(guid);
}

const Vector<Material>& Material_MaterialList()
{
    return materialSystem.MaterialList();
}

void MaterialSystem_Destroy(const MaterialGuid& materialGuid)
{
    materialSystem.Destroy(materialGuid);
}

void MaterialSystem_DestroyAllMaterials()
{
    materialSystem.DestroyAllMaterials();
}