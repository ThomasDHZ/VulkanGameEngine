#pragma once
#include "Platform.h"
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
	DLL_EXPORT VkGuid MaterialSystem_CreateMaterial(const char* materialPath);
	DLL_EXPORT void MaterialSystem_Update(const float& deltaTime);
	DLL_EXPORT const bool MaterialSystem_MaterialMapExists(const MaterialGuid& materialGuid);
	DLL_EXPORT const Material& MaterialSystem_FindMaterial(const MaterialGuid& materialGuid);
	DLL_EXPORT void MaterialSystem_Destroy(const MaterialGuid& materialGuid);
	DLL_EXPORT void MaterialSystem_DestroyAllMaterials();
#ifdef __cplusplus
}
#endif

 const Vector<Material>& Material_MaterialList();
 const Vector<VkDescriptorBufferInfo> Material_GetMaterialPropertiesBuffer();
 void Material_DestroyBuffer(VulkanBuffer& materialBuffer);

class MaterialSystem
{
	private:
	public:
		UnorderedMap<MaterialGuid, Material> MaterialMap;
		MaterialSystem() 
		{ 
		
		}

		~MaterialSystem() 
		{

		}

		void Update(const float& deltaTime) 
		{ 
			MaterialSystem_Update(deltaTime);
		}

		VkGuid LoadMaterial(const String& materialPath) 
		{ 
			return MaterialSystem_CreateMaterial(materialPath.c_str());
		}

		const bool MaterialMapExists(const MaterialGuid& materialGuid) const
		{ 
			return MaterialSystem_MaterialMapExists(materialGuid);
		}

		const Material& FindMaterial(const MaterialGuid& materialGuid)
		{ 
			return MaterialSystem_FindMaterial(materialGuid);
		}

		const Vector<Material>& MaterialList() 
		{ 
			return Material_MaterialList();
		}

		const Vector<VkDescriptorBufferInfo> GetMaterialPropertiesBuffer() 
		{ 
			return Material_GetMaterialPropertiesBuffer();
		}

		void Destroy(const MaterialGuid& materialGuid)
		{ 
			MaterialSystem_Destroy(materialGuid);
		}

		void DestroyAllMaterials() 
		{ 
			MaterialSystem_DestroyAllMaterials();
		}
};
DLL_EXPORT MaterialSystem materialSystem;