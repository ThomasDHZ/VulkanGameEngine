#include "MaterialSystem.h"
#include "TextureSystem.h"
#include <Material.h>
#include "BufferSystem.h"
#include "vulkanfilesystem.h"

MaterialSystem materialSystem = MaterialSystem();

MaterialSystem::MaterialSystem()
{
}

MaterialSystem::~MaterialSystem()
{
}

void MaterialSystem::Update(const float& deltaTime)
{
    uint x = 0;
    for (auto& materialPair : MaterialMap)
    {
        materialPair.second.ShaderMaterialBufferIndex = x;

        const Material material = materialPair.second;
        const uint AlbedoMapId = material.AlbedoMapId != VkGuid() ? textureSystem.FindTexture(material.AlbedoMapId).textureBufferIndex : 0;
        const uint MetallicRoughnessMapId = material.MetallicRoughnessMapId != VkGuid() ? textureSystem.FindTexture(material.MetallicRoughnessMapId).textureBufferIndex : 0;
        const uint MetallicMapId = material.MetallicMapId != VkGuid() ? textureSystem.FindTexture(material.MetallicMapId).textureBufferIndex : 0;
        const uint RoughnessMapId = material.RoughnessMapId != VkGuid() ? textureSystem.FindTexture(material.RoughnessMapId).textureBufferIndex : 0;
        const uint AmbientOcclusionMapId = material.AmbientOcclusionMapId != VkGuid() ? textureSystem.FindTexture(material.AmbientOcclusionMapId).textureBufferIndex : 0;
        const uint NormalMapId = material.NormalMapId != VkGuid() ? textureSystem.FindTexture(material.NormalMapId).textureBufferIndex : 0;
        const uint DepthMapId = material.DepthMapId != VkGuid() ? textureSystem.FindTexture(material.DepthMapId).textureBufferIndex : 0;
        const uint AlphaMapId = material.AlphaMapId != VkGuid() ? textureSystem.FindTexture(material.AlphaMapId).textureBufferIndex : 0;
        const uint EmissionMapId = material.EmissionMapId != VkGuid() ? textureSystem.FindTexture(material.EmissionMapId).textureBufferIndex : 0;
        const uint HeightMapId = material.HeightMapId != VkGuid() ? textureSystem.FindTexture(material.HeightMapId).textureBufferIndex : 0;
  
        ShaderStruct& shaderStruct = shaderSystem.FindShaderStruct(material.MaterialBufferId);
        memcpy(shaderSystem.SearchShaderStruct(shaderStruct, "AlbedoMap")->Value, &AlbedoMapId, sizeof(uint));
        memcpy(shaderSystem.SearchShaderStruct(shaderStruct, "MetallicRoughnessMap")->Value, &MetallicRoughnessMapId, sizeof(uint));
        memcpy(shaderSystem.SearchShaderStruct(shaderStruct, "MetallicMap")->Value, &MetallicMapId, sizeof(uint));
        memcpy(shaderSystem.SearchShaderStruct(shaderStruct, "RoughnessMap")->Value, &RoughnessMapId, sizeof(uint));
        memcpy(shaderSystem.SearchShaderStruct(shaderStruct, "AmbientOcclusionMap")->Value, &AmbientOcclusionMapId, sizeof(uint));
        memcpy(shaderSystem.SearchShaderStruct(shaderStruct, "NormalMap")->Value, &NormalMapId, sizeof(uint));
        memcpy(shaderSystem.SearchShaderStruct(shaderStruct, "DepthMap")->Value, &DepthMapId, sizeof(uint));
        memcpy(shaderSystem.SearchShaderStruct(shaderStruct, "AlphaMap")->Value, &AlphaMapId, sizeof(uint));
        memcpy(shaderSystem.SearchShaderStruct(shaderStruct, "EmissionMap")->Value, &EmissionMapId, sizeof(uint));
        memcpy(shaderSystem.SearchShaderStruct(shaderStruct, "HeightMap")->Value, &HeightMapId, sizeof(uint));
        shaderSystem.UpdateShaderBuffer(material.MaterialBufferId);
        x++;
    }
}

VkGuid MaterialSystem::LoadMaterial(const String& materialPath)
{
    if (materialPath.empty() ||
        materialPath == "")
    {
        return VkGuid();
    }

    nlohmann::json json = vulkanFileSystem.LoadJsonFile(materialPath);
    VkGuid materialId = VkGuid(json["MaterialId"].get<String>().c_str());

    if (MaterialMapExists(materialId))
    {
        return materialId;
    }

    int bufferIndex = ++bufferSystem.NextBufferId;
    VulkanBuffer& vulkanBuffer = bufferSystem.VulkanBufferMap[bufferIndex];
    shaderSystem.PipelineShaderStructMap[bufferIndex] = shaderSystem.CopyShaderStructProtoType("MaterialProperitiesBuffer");
    MaterialMap[materialId] = Material_CreateMaterial(renderSystem.renderer, bufferIndex, vulkanBuffer, shaderSystem.PipelineShaderStructMap[bufferIndex].ShaderBufferSize, materialPath.c_str());
    return materialId;
}

const bool MaterialSystem::MaterialMapExists(const VkGuid& renderPassId)  const
{
    return MaterialMap.contains(renderPassId);
}

const Material& MaterialSystem::FindMaterial(const RenderPassGuid& guid)
{
    return MaterialMap.at(guid);
}

const Vector<Material>& MaterialSystem::MaterialList()
{
    Vector<Material> materialList;
    for (const auto& material : MaterialMap)
    {
        materialList.emplace_back(material.second);
    }
    return materialList;
}

const Vector<VkDescriptorBufferInfo> MaterialSystem::GetMaterialPropertiesBuffer()
{
    std::vector<VkDescriptorBufferInfo>	materialPropertiesBuffer;
    if (MaterialMap.empty())
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
        for (auto& material : MaterialMap)
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


void MaterialSystem::Destroy(const VkGuid& guid)
{
    Material& material = MaterialMap[guid];

    VulkanBuffer& materialBuffer = bufferSystem.VulkanBufferMap[material.MaterialBufferId];
    //Material_DestroyBuffer(renderSystem.renderer, materialBuffer);
    bufferSystem.VulkanBufferMap.erase(material.MaterialBufferId);
}

void MaterialSystem::DestroyAllMaterials()
{
    for (auto& materialPair : MaterialMap)
    {
        Destroy(materialPair.second.materialGuid);
    }
}