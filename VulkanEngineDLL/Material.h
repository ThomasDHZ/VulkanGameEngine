#pragma once
#include "DLL.h"
#include "Typedef.h"
#include "VulkanRenderer.h"
#include "VulkanBuffer.h"
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

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT Material Material_CreateMaterial(const GraphicsRenderer& renderer, int bufferIndex, VulkanBuffer& materialBuffer, size_t shaderStructBufferSize, const char* jsonString);
	DLL_EXPORT void Material_DestroyBuffer(const GraphicsRenderer& renderer, VulkanBuffer& materialBuffer);
#ifdef __cplusplus
}
#endif