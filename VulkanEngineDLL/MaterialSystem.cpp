#include "MaterialSystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"

MaterialSystem materialSystem = MaterialSystem();

VkGuid Material_CreateMaterial(const GraphicsRenderer& renderer, const char* materialPath)
{
    if (materialPath == nullptr)
    {
        return VkGuid();
    }

    nlohmann::json json = File_LoadJsonFile(materialPath);
    VkGuid materialId = VkGuid(json["MaterialId"].get<String>().c_str());
    if (Material_MaterialMapExists(materialId))
    {
        return materialId;
    }

    int bufferIndex = ++NextBufferId;
    shaderSystem.PipelineShaderStructMap[bufferIndex] = Shader_CopyShaderStructProtoType("MaterialProperitiesBuffer");
    bufferSystem.VulkanBufferMap[bufferIndex] = VulkanBuffer_CreateVulkanBuffer(renderer, bufferIndex, shaderSystem.PipelineShaderStructMap[bufferIndex].ShaderBufferSize, 1, BufferTypeEnum::BufferType_MaterialProperitiesBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                                                                                                                                                                                                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                                                                                                                                                                                                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                                                                                                                                                                                                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                                                                                                                                                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                                                                                                                                                                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                                                                                                                                                                                                    VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, false);
    materialSystem.MaterialMap[materialId] = Material
    {
        .materialGuid = VkGuid(json["MaterialId"].get<String>().c_str()),
        .ShaderMaterialBufferIndex = 0,
        .MaterialBufferId = bufferIndex,
        .AlbedoMapId = json["AlbedoMapId"].get<std::string>().empty() ? VkGuid() : json["AlbedoMapId"].get<std::string>().c_str(),
        .MetallicRoughnessMapId = json["MetallicRoughnessMapId"].get<std::string>().empty() ? VkGuid() : json["MetallicRoughnessMapId"].get<std::string>().c_str(),
        .MetallicMapId = json["MetallicMapId"].get<std::string>().empty() ? VkGuid() : json["MetallicMapId"].get<std::string>().c_str(),
        .RoughnessMapId = json["RoughnessMapId"].get<std::string>().empty() ? VkGuid() : json["RoughnessMapId"].get<std::string>().c_str(),
        .AmbientOcclusionMapId = json["AmbientOcclusionMapId"].get<std::string>().empty() ? VkGuid() : json["AmbientOcclusionMapId"].get<std::string>().c_str(),
        .NormalMapId = json["NormalMapId"].get<std::string>().empty() ? VkGuid() : json["NormalMapId"].get<std::string>().c_str(),
        .DepthMapId = json["DepthMapId"].get<std::string>().empty() ? VkGuid() : json["DepthMapId"].get<std::string>().c_str(),
        .AlphaMapId = json["AlphaMapId"].get<std::string>().empty() ? VkGuid() : json["AlphaMapId"].get<std::string>().c_str(),
        .EmissionMapId = json["EmissionMapId"].get<std::string>().empty() ? VkGuid() : json["EmissionMapId"].get<std::string>().c_str(),
        .HeightMapId = json["HeightMapId"].get<std::string>().empty() ? VkGuid() : json["HeightMapId"].get<std::string>().c_str(),
        .Albedo = vec3(json["Albedo"][0], json["Albedo"][1], json["Albedo"][2]),
        .Emission = vec3(json["Emission"][0], json["Emission"][1], json["Emission"][2]),
        .Metallic = json["Metallic"],
        .Roughness = json["Roughness"],
        .AmbientOcclusion = json["AmbientOcclusion"],
        .Alpha = json["Alpha"],
    };
}

void Material_Update(const GraphicsRenderer& renderer, const float& deltaTime)
{
    uint x = 0;
    for (auto& materialPair : materialSystem.MaterialMap)
    {
        materialPair.second.ShaderMaterialBufferIndex = x;

        const Material material = materialPair.second;
        const uint AlbedoMapId = material.AlbedoMapId != VkGuid() ? Texture_FindTexture(material.AlbedoMapId).textureBufferIndex : 0;
        const uint MetallicRoughnessMapId = material.MetallicRoughnessMapId != VkGuid() ? Texture_FindTexture(material.MetallicRoughnessMapId).textureBufferIndex : 0;
        const uint MetallicMapId = material.MetallicMapId != VkGuid() ? Texture_FindTexture(material.MetallicMapId).textureBufferIndex : 0;
        const uint RoughnessMapId = material.RoughnessMapId != VkGuid() ? Texture_FindTexture(material.RoughnessMapId).textureBufferIndex : 0;
        const uint AmbientOcclusionMapId = material.AmbientOcclusionMapId != VkGuid() ? Texture_FindTexture(material.AmbientOcclusionMapId).textureBufferIndex : 0;
        const uint NormalMapId = material.NormalMapId != VkGuid() ? Texture_FindTexture(material.NormalMapId).textureBufferIndex : 0;
        const uint DepthMapId = material.DepthMapId != VkGuid() ? Texture_FindTexture(material.DepthMapId).textureBufferIndex : 0;
        const uint AlphaMapId = material.AlphaMapId != VkGuid() ? Texture_FindTexture(material.AlphaMapId).textureBufferIndex : 0;
        const uint EmissionMapId = material.EmissionMapId != VkGuid() ? Texture_FindTexture(material.EmissionMapId).textureBufferIndex : 0;
        const uint HeightMapId = material.HeightMapId != VkGuid() ? Texture_FindTexture(material.HeightMapId).textureBufferIndex : 0;

        ShaderStruct& shaderStruct = Shader_FindShaderStruct(material.MaterialBufferId);
        memcpy(Shader_SearchShaderStruct(shaderStruct, "AlbedoMap")->Value, &AlbedoMapId, sizeof(uint));
        memcpy(Shader_SearchShaderStruct(shaderStruct, "MetallicRoughnessMap")->Value, &MetallicRoughnessMapId, sizeof(uint));
        memcpy(Shader_SearchShaderStruct(shaderStruct, "MetallicMap")->Value, &MetallicMapId, sizeof(uint));
        memcpy(Shader_SearchShaderStruct(shaderStruct, "RoughnessMap")->Value, &RoughnessMapId, sizeof(uint));
        memcpy(Shader_SearchShaderStruct(shaderStruct, "AmbientOcclusionMap")->Value, &AmbientOcclusionMapId, sizeof(uint));
        memcpy(Shader_SearchShaderStruct(shaderStruct, "NormalMap")->Value, &NormalMapId, sizeof(uint));
        memcpy(Shader_SearchShaderStruct(shaderStruct, "DepthMap")->Value, &DepthMapId, sizeof(uint));
        memcpy(Shader_SearchShaderStruct(shaderStruct, "AlphaMap")->Value, &AlphaMapId, sizeof(uint));
        memcpy(Shader_SearchShaderStruct(shaderStruct, "EmissionMap")->Value, &EmissionMapId, sizeof(uint));
        memcpy(Shader_SearchShaderStruct(shaderStruct, "HeightMap")->Value, &HeightMapId, sizeof(uint));
        Shader_UpdateShaderBuffer(renderer, material.MaterialBufferId);
        x++;
    }
}

void Material_DestroyBuffer(const GraphicsRenderer& renderer, VulkanBuffer& materialBuffer)
{
    VulkanBuffer_DestroyBuffer(renderer, materialBuffer);
}

const bool Material_MaterialMapExists(const VkGuid& renderPassId)
{
    return materialSystem.MaterialMap.contains(renderPassId);
}

const Material& Material_FindMaterial(const RenderPassGuid& guid)
{
    return materialSystem.MaterialMap.at(guid);
}

const Vector<Material>& Material_MaterialList()
{
    Vector<Material> materialList;
    for (const auto& material : materialSystem.MaterialMap)
    {
        materialList.emplace_back(material.second);
    }
    return materialList;
}

const Vector<VkDescriptorBufferInfo> Material_GetMaterialPropertiesBuffer()
{
    std::vector<VkDescriptorBufferInfo>	materialPropertiesBuffer;
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


void Material_Destroy(const VkGuid& guid)
{
    Material& material = materialSystem.MaterialMap[guid];

    VulkanBuffer& materialBuffer = bufferSystem.VulkanBufferMap[material.MaterialBufferId];
    //Material_DestroyBuffer(renderSystem.renderer, materialBuffer);
    bufferSystem.VulkanBufferMap.erase(material.MaterialBufferId);
}

void Material_DestroyAllMaterials()
{
    for (auto& materialPair : materialSystem.MaterialMap)
    {
        Material_Destroy(materialPair.second.materialGuid);
    }
}