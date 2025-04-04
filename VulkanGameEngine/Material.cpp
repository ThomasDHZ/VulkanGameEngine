#include "Material.h"
#include "MemoryManager.h"

uint32 Material::NextMaterialId = 0;

Material::Material()
{
}

Material::Material(const String& materialName)
{
	Name = materialName;
	MaterialID = ++NextMaterialId;
	MaterialBufferIndex = 0;
	MaterialBuffer = VulkanBuffer<MaterialProperitiesBuffer>(MaterialInfo,  VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
																			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
																			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
																			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, false);
}

Material::~Material()
{
}

void Material::UpdateMaterialBufferIndex(uint64 bufferIndex)
{
	MaterialBufferIndex = bufferIndex;
}

void Material::UpdateBuffer()
{
	MaterialInfo.Albedo = Albedo;
	MaterialInfo.Metallic = Metallic;
	MaterialInfo.Roughness = Roughness;
	MaterialInfo.AmbientOcclusion = AmbientOcclusion;
	MaterialInfo.Emission = Emission;
	MaterialInfo.Alpha = Alpha;

	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::Destroy()
{
	AlbedoMap.reset();
	MetallicRoughnessMap.reset();
	MetallicMap.reset();
	RoughnessMap.reset();
	AmbientOcclusionMap.reset();
	NormalMap.reset();
	DepthMap.reset();
	AlphaMap.reset();
	EmissionMap.reset();
	HeightMap.reset();
	MaterialBuffer.DestroyBuffer();
}

void Material::GetMaterialPropertiesBuffer(std::vector<VkDescriptorBufferInfo>& materialBufferList)
{
	UpdateBuffer();

	VkDescriptorBufferInfo meshBufferInfo =
	{
		.buffer = MaterialBuffer.Buffer,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};
	materialBufferList.emplace_back(meshBufferInfo);
}

void Material::SetAlbedo(glm::vec3 color)
{
	Albedo = color;
	MaterialInfo.Albedo = color;
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetMetallic(float value)
{
	Metallic = value;
	MaterialInfo.Metallic = value;
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetRoughness(float value)
{
	Roughness = value;
	MaterialInfo.Roughness = value;
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetAmbientOcclusion(float value)
{
	AmbientOcclusion = value;
	MaterialInfo.AmbientOcclusion = value;
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetEmission(glm::vec3 color)
{
	Emission = color;
	MaterialInfo.Emission = color;
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetAlpha(float value)
{
	Alpha = value;
	MaterialInfo.Alpha = value;
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetAlbedoMap(SharedPtr<Texture> texture)
{
	AlbedoMap = texture;
	MaterialInfo.AlbedoMap = AlbedoMap->GetTextureBufferIndex();
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetMetallicRoughnessMap(SharedPtr<Texture> texture)
{
	MetallicRoughnessMap = texture;
	MaterialInfo.MetallicRoughnessMap = MetallicRoughnessMap->GetTextureBufferIndex();
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetMetallicMap(SharedPtr<Texture> texture)
{
	MetallicMap = texture;
	MaterialInfo.MetallicMap = MetallicMap->GetTextureBufferIndex();
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetRoughnessMap(SharedPtr<Texture> texture)
{
	RoughnessMap = texture;
	MaterialInfo.RoughnessMap = RoughnessMap->GetTextureBufferIndex();
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetAmbientOcclusionMap(SharedPtr<Texture> texture)
{
	AmbientOcclusionMap = texture;
	MaterialInfo.AmbientOcclusionMap = AmbientOcclusionMap->GetTextureBufferIndex();
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetNormalMap(SharedPtr<Texture> texture)
{
	NormalMap = texture;
	MaterialInfo.NormalMap = NormalMap->GetTextureBufferIndex();
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetDepthMap(SharedPtr<Texture> texture)
{
	DepthMap = texture;
	MaterialInfo.DepthMap = DepthMap->GetTextureBufferIndex();
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetAlphaMap(SharedPtr<Texture> texture)
{
	AlphaMap = texture;
	MaterialInfo.AlphaMap = AlphaMap->GetTextureBufferIndex();
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetEmissionMap(SharedPtr<Texture> texture)
{
	EmissionMap = texture;
	MaterialInfo.EmissionMap = EmissionMap->GetTextureBufferIndex();
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

void Material::SetHeightMap(SharedPtr<Texture> texture)
{
	HeightMap = texture;
	MaterialInfo.HeightMap = HeightMap->GetTextureBufferIndex();
	MaterialBuffer.UpdateBufferMemory(MaterialInfo);
}

