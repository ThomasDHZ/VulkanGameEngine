#pragma once
#include "DLL.h"
#include "Typedef.h"
#include "json.h"
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
#include "Vector.h"
#include "Texture.h"
#include "VulkanShader.h"




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

template<>
struct Vector2Traits<Material>
{
	static const VkGuid& GetGuid(const Material& obj) { return obj.materialGuid; }
	static int GetId(const Material& obj) { return obj.MaterialBufferId; }
	static int GetVectorMapKey(const Material& obj) { return obj.VectorMapKey; }
};

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT Material Material_CreateMaterial(const GraphicsRenderer& renderer, int bufferIndex, VulkanBuffer& materialBuffer, ShaderStruct& shaderStruct, const char* jsonString);
	DLL_EXPORT void Material_UpdateBuffer(const GraphicsRenderer& renderer, VulkanBuffer& materialBuffer, ShaderStruct& shaderStruct);
	DLL_EXPORT void Material_DestroyBuffer(const GraphicsRenderer& renderer, ShaderStruct& materialBuffer);
#ifdef __cplusplus
}
#endif