#include "MaterialSystem.h"
#include "TextureSystem.h"
#include <Material.h>
#include "BufferSystem.h"

MaterialSystem materialSystem = MaterialSystem();

MaterialSystem::MaterialSystem()
{
}

MaterialSystem::~MaterialSystem()
{
}

VkGuid MaterialSystem::LoadMaterial(const String& materialPath)
{
    if (materialPath.empty() ||
        materialPath == "")
    {
        return VkGuid();
    }

    nlohmann::json json = Json::ReadJson(materialPath);
    VkGuid materialId = VkGuid(json["MaterialId"].get<String>().c_str());

    if (MaterialMapExists(materialId))
    {
        return materialId;
    }

    int bufferIndex = ++bufferSystem.NextBufferId;
    MaterialMap[materialId] = Material_CreateMaterial(renderSystem.renderer, bufferIndex, bufferSystem.VulkanBufferMap[bufferIndex], materialPath.c_str());
    return materialId;
}

bool MaterialSystem::MaterialMapExists(const VkGuid& renderPassId)
{
    auto it = MaterialMap.find(renderPassId);
    if (it != MaterialMap.end())
    {
        return true;
    }
    return false;
}

const Material& MaterialSystem::FindMaterial(const RenderPassGuid& guid)
{
    auto it = MaterialMap.find(guid);
    if (it != MaterialMap.end())
    {
        return it->second;
    }
    throw std::out_of_range("Material not found for given GUID");
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

void MaterialSystem::Update(const float& deltaTime)
{
    uint x = 0;
    for (auto& materialValue : MaterialMap)
    {
        materialValue.second.ShaderMaterialBufferIndex = x;

        const Material material = materialValue.second;
        MaterialProperitiesBuffer materialBufferProperties = MaterialProperitiesBuffer
        {
            .AlbedoMapId = material.AlbedoMapId != VkGuid() ? textureSystem.FindTexture(material.AlbedoMapId).textureBufferIndex : 0,
            .MetallicRoughnessMapId = material.MetallicRoughnessMapId != VkGuid() ? textureSystem.FindTexture(material.MetallicRoughnessMapId).textureBufferIndex : 0,
            .MetallicMapId = material.MetallicMapId != VkGuid() ? textureSystem.FindTexture(material.MetallicMapId).textureBufferIndex : 0,
            .RoughnessMapId = material.RoughnessMapId != VkGuid() ? textureSystem.FindTexture(material.RoughnessMapId).textureBufferIndex : 0,
            .AmbientOcclusionMapId = material.AmbientOcclusionMapId != VkGuid() ? textureSystem.FindTexture(material.AmbientOcclusionMapId).textureBufferIndex : 0,
            .NormalMapId = material.NormalMapId != VkGuid() ? textureSystem.FindTexture(material.NormalMapId).textureBufferIndex : 0,
            .DepthMapId = material.DepthMapId != VkGuid() ? textureSystem.FindTexture(material.DepthMapId).textureBufferIndex : 0,
            .AlphaMapId = material.AlphaMapId != VkGuid() ? textureSystem.FindTexture(material.AlphaMapId).textureBufferIndex : 0,
            .EmissionMapId = material.EmissionMapId != VkGuid() ? textureSystem.FindTexture(material.EmissionMapId).textureBufferIndex : 0,
            .HeightMapId = material.HeightMapId != VkGuid() ? textureSystem.FindTexture(material.HeightMapId).textureBufferIndex : 0
        };

        Material_UpdateBuffer(renderSystem.renderer, bufferSystem.VulkanBufferMap[materialValue.second.MaterialBufferId], materialBufferProperties);
        x++;
    }
}

void MaterialSystem::Destroy(const VkGuid& guid)
{
    Material& material = MaterialMap[guid];

    VulkanBuffer& materialBuffer = bufferSystem.VulkanBufferMap[material.MaterialBufferId];
    Material_DestroyBuffer(renderSystem.renderer, materialBuffer);
    bufferSystem.VulkanBufferMap.erase(material.MaterialBufferId);
}

void MaterialSystem::DestroyAllMaterials()
{
    for (auto& materialPair : MaterialMap)
    {
        Destroy(materialPair.second.materialGuid);
    }
}
