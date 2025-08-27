#include "Material.h"
#include "json.h"

Material Material_CreateMaterial(const GraphicsRenderer& renderer, int bufferIndex, ShaderStruct& materialBuffer, const char* jsonString)
{
    materialBuffer.ShaderStructBuffer = VulkanBuffer_CreateVulkanBuffer(renderer, bufferIndex, materialBuffer.ShaderBufferSize, 1, BufferTypeEnum::BufferType_MaterialProperitiesBuffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                                                                                                                                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                                                                                                                                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                                                                                                                                                           VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, false);
    nlohmann::json json = Json::ReadJson(jsonString);
    return Material
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

void Material_UpdateBuffer(const GraphicsRenderer& renderer, VulkanBuffer& materialBuffer, MaterialProperitiesBuffer& materialProperties)
{
    VulkanBuffer_UpdateBufferMemory(renderer, materialBuffer, static_cast<void*>(&materialProperties), sizeof(MaterialProperitiesBuffer), 1);
}

void Material_DestroyBuffer(const GraphicsRenderer& renderer, VulkanBuffer& materialBuffer)
{
    VulkanBuffer_DestroyBuffer(renderer, materialBuffer);
}
