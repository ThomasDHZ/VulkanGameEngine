#pragma once
#include "DLL.h"
#include "Typedef.h"
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "TextureSystem.h"
#include "VulkanShaderSystem.h"


struct Material
{
	int VectorMapKey;
	VkGuid materialGuid;
	uint ShaderMaterialBufferIndex;
	int MaterialBufferId;

	VkGuid AlbedoMapId;
	VkGuid MetallicRoughnessMapId;
	VkGuid MetallicMapId;
	VkGuid RoughnessMapId;
	VkGuid AmbientOcclusionMapId;
	VkGuid NormalMapId;
	VkGuid DepthMapId;
	VkGuid AlphaMapId;
	VkGuid EmissionMapId;
	VkGuid HeightMapId;

	vec3 Albedo = vec3(0.0f, 0.35f, 0.45f);
	vec3 Emission = vec3(0.0f);
	float Metallic = 0.0f;
	float Roughness = 0.0f;
	float AmbientOcclusion = 1.0f;
	float Alpha = 1.0f;
};

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT VkGuid Material_CreateMaterial(const char* materialPath);
	DLL_EXPORT void Material_DestroyBuffer(VulkanBuffer& materialBuffer);
#ifdef __cplusplus
}
#endif

DLL_EXPORT void Material_Update(const float& deltaTime);
DLL_EXPORT const bool Material_MaterialMapExists(const VkGuid& renderPassId);
DLL_EXPORT const Material& Material_FindMaterial(const RenderPassGuid& guid);
DLL_EXPORT const Vector<Material>& Material_MaterialList();
DLL_EXPORT const Vector<VkDescriptorBufferInfo> Material_GetMaterialPropertiesBuffer();
DLL_EXPORT void Material_Destroy(const VkGuid& guid);
DLL_EXPORT void Material_DestroyAllMaterials();

class MaterialSystem
{
	private:
	public:
		UnorderedMap<RenderPassGuid, Material> MaterialMap;
		MaterialSystem() { }
		~MaterialSystem() { }

		void Update(const float& deltaTime) { Material_Update(deltaTime); }
		VkGuid LoadMaterial(const String& materialPath) { return Material_CreateMaterial(materialPath.c_str()); }
		const bool MaterialMapExists(const VkGuid& renderPassId)  const { return Material_MaterialMapExists(renderPassId); }
		const Material& FindMaterial(const RenderPassGuid& guid) { return Material_FindMaterial(guid); }
		const Vector<Material>& MaterialList() { return Material_MaterialList(); }
		const Vector<VkDescriptorBufferInfo> GetMaterialPropertiesBuffer() { return Material_GetMaterialPropertiesBuffer(); }
		void Destroy(const VkGuid& guid) { Material_Destroy(guid); }
		void DestroyAllMaterials() { Material_DestroyAllMaterials(); }
};
DLL_EXPORT MaterialSystem materialSystem;