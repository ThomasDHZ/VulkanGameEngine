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
	DLL_EXPORT VkGuid MaterialSystem_CreateMaterial(const GraphicsRenderer& renderer, const char* materialPath);
	DLL_EXPORT void MaterialSystem_Update(const GraphicsRenderer& renderer, const float& deltaTime);
	DLL_EXPORT const bool MaterialSystem_MaterialMapExists(const VkGuid& renderPassId);
	DLL_EXPORT const Material& MaterialSystem_FindMaterial(const VkGuid& renderPassId);
	DLL_EXPORT void MaterialSystem_Destroy(const VkGuid& guid);
	DLL_EXPORT void MaterialSystem_DestroyAllMaterials();
#ifdef __cplusplus
}
#endif

 const Vector<Material>& Material_MaterialList();
 const Vector<VkDescriptorBufferInfo> Material_GetMaterialPropertiesBuffer();
 void Material_DestroyBuffer(const GraphicsRenderer& renderer, VulkanBuffer& materialBuffer);

class MaterialSystem
{
	private:
	public:
		UnorderedMap<RenderPassGuid, Material> MaterialMap;
		MaterialSystem() 
		{ 
		
		}

		~MaterialSystem() 
		{

		}

		void Update(const float& deltaTime) 
		{ 
			MaterialSystem_Update(renderer, deltaTime);
		}

		VkGuid LoadMaterial(const String& materialPath) 
		{ 
			return MaterialSystem_CreateMaterial(renderer, materialPath.c_str());
		}

		const bool MaterialMapExists(const VkGuid& renderPassId) const 
		{ 
			return MaterialSystem_MaterialMapExists(renderPassId);
		}

		const Material& FindMaterial(const RenderPassGuid& guid)
		{ 
			return MaterialSystem_FindMaterial(guid);
		}

		const Vector<Material>& MaterialList() 
		{ 
			return Material_MaterialList();
		}

		const Vector<VkDescriptorBufferInfo> GetMaterialPropertiesBuffer() 
		{ 
			return Material_GetMaterialPropertiesBuffer();
		}

		void Destroy(const VkGuid& guid) 
		{ 
			MaterialSystem_Destroy(guid);
		}

		void DestroyAllMaterials() 
		{ 
			MaterialSystem_DestroyAllMaterials();
		}
};
DLL_EXPORT MaterialSystem materialSystem;