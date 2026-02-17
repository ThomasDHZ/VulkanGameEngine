#include "MaterialSystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include "ShaderSystem.h"
#include "from_json.h"

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

    ShaderStructDLL shaderStruct = shaderSystem.CopyShaderStructProtoType("PackedMaterial");
    uint32 bufferId = bufferSystem.VMACreateDynamicBuffer(&shaderStruct, shaderStruct.ShaderBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    shaderSystem.PipelineShaderStructMap[bufferId] = shaderStruct;

    Material material;
    material.materialGuid = materialId;
    material.ShaderMaterialBufferIndex = MaterialList.size();
    material.MaterialBufferId = bufferId;
    material.AlbedoDataId =         !json["AlbedoData"].is_null()         ? textureSystem.LoadKTXTexture(json["AlbedoData"].get<TextureLoader>()).textureGuid         : VkGuid();
    material.NormalDataId =         !json["NormalData"].is_null()         ? textureSystem.LoadKTXTexture(json["NormalData"].get<TextureLoader>()).textureGuid         : VkGuid();
    material.PackedMRODataId =      !json["PackedMROData"].is_null()      ? textureSystem.LoadKTXTexture(json["PackedMROData"].get<TextureLoader>()).textureGuid      : VkGuid();
    material.PackedSheenSSSDataId = !json["PackedSheenSSSData"].is_null() ? textureSystem.LoadKTXTexture(json["PackedSheenSSSData"].get<TextureLoader>()).textureGuid : VkGuid();
    material.UnusedDataId =         !json["UnusedData"].is_null()         ? textureSystem.LoadKTXTexture(json["UnusedData"].get<TextureLoader>()).textureGuid         : VkGuid();
    material.EmissionDataId =       !json["EmissionData"].is_null()       ? textureSystem.LoadKTXTexture(json["EmissionData"].get<TextureLoader>()).textureGuid       : VkGuid();
    MaterialList.emplace_back(material);
    return materialId;
}

void MaterialSystem::Update(const float& deltaTime)
{
    for (auto& material : MaterialList)
    {
        const uint albedoDataId =         material.AlbedoDataId != VkGuid()          ? textureSystem.FindTexture(material.AlbedoDataId).textureIndex         : SIZE_MAX;
        const uint normalDataId =         material.NormalDataId != VkGuid()          ? textureSystem.FindTexture(material.NormalDataId).textureIndex         : SIZE_MAX;
        const uint packedMRODataId =      material.PackedMRODataId != VkGuid()       ? textureSystem.FindTexture(material.PackedMRODataId).textureIndex      : SIZE_MAX;
        const uint packedSheenSSSDataId = material.PackedSheenSSSDataId != VkGuid()  ? textureSystem.FindTexture(material.PackedSheenSSSDataId).textureIndex : SIZE_MAX;
        const uint unusedDataId =         material.UnusedDataId != VkGuid()          ? textureSystem.FindTexture(material.UnusedDataId).textureIndex         : SIZE_MAX;
        const uint emissionDataId =       material.EmissionDataId != VkGuid()        ? textureSystem.FindTexture(material.EmissionDataId).textureIndex       : SIZE_MAX;


        ShaderStructDLL& shaderStruct = shaderSystem.FindShaderStruct(material.MaterialBufferId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "AlbedoDataId", albedoDataId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "NormalDataId", normalDataId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "PackedMRODataId", packedMRODataId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "PackedSheenSSSDataId", packedSheenSSSDataId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "UnusedDataId", unusedDataId);
        shaderSystem.UpdateShaderStructValue<uint>(shaderStruct, "EmissionDataId", emissionDataId);
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
    bufferSystem.DestroyBuffer(materialBuffer);
    bufferSystem.VulkanBufferMap.erase(material.MaterialBufferId);
}

void MaterialSystem::DestroyAllMaterials()
{
    for (auto& materialPair : MaterialList)
    {
        Destroy(materialPair.materialGuid);
    }
}