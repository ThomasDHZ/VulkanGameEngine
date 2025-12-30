#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 0;
layout(constant_id = 1) const uint DescriptorBindingType1 = 1;
layout(constant_id = 2) const uint DescriptorBindingType2 = 2;

layout(location = 0) in vec3 inPS_Position; 
layout(location = 1) in vec2 inPS_UV;    

layout(location = 0) out vec4 PositionDataMap;
layout(location = 1) out vec4 AlbedoMap;
layout(location = 2) out vec4 NormalMap;
layout(location = 3) out vec4 MatRoughAOHeightMap;
layout(location = 4) out vec4 EmissionMap;


layout(push_constant) uniform SceneDataBuffer {
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
} sceneData;

struct MeshProperitiesBuffer
{
	int	   MaterialIndex;
	mat4   MeshTransform;
};

struct MaterialProperitiesBuffer
{
	vec3 Albedo;
	float Specular;
	float Metallic;
	float Roughness;
	float AmbientOcclusion;
	vec3 Emission;
	float Alpha;
	float HeightScale;
	float Height;

	uint AlbedoMap;
	uint SpecularMap;
	uint MetallicMap;
	uint RoughnessMap;
	uint AmbientOcclusionMap;
	uint NormalMap;
	uint AlphaMap;
	uint EmissionMap;
	uint HeightMap;
};

layout(binding = 0) buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 1) uniform sampler2D TextureMap[];
layout(binding = 2) buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];

void main()
{
	int meshIndex = sceneData.MeshBufferIndex;
    uint materialId = meshBuffer[meshIndex].meshProperties.MaterialIndex;
	MaterialProperitiesBuffer material = materialBuffer[materialId].materialProperties;

    vec4  albedoMap =			(material.AlbedoMap			  != 0xFFFFFFFFu) ? texture(TextureMap[material.AlbedoMap], inPS_UV).rgba		 : vec4(material.Albedo, 1.0f);
    vec3  normalMap =			(material.NormalMap			  != 0xFFFFFFFFu) ? texture(TextureMap[material.NormalMap], inPS_UV).rgb		 : vec3(0.0, 0.0, 1.0);
	float specularMap =			(material.SpecularMap		  != 0xFFFFFFFFu) ? texture(TextureMap[material.SpecularMap], inPS_UV).r		 : material.Specular;
    float metallicMap =			(material.MetallicMap		  != 0xFFFFFFFFu) ? texture(TextureMap[material.MetallicMap], inPS_UV).r		 : material.Metallic;
    float roughnessMap =		(material.RoughnessMap		  != 0xFFFFFFFFu) ? texture(TextureMap[material.RoughnessMap], inPS_UV).r		 : material.Roughness;
    float ambientOcclusionMap = (material.AmbientOcclusionMap != 0xFFFFFFFFu) ? texture(TextureMap[material.AmbientOcclusionMap], inPS_UV).r : material.AmbientOcclusion;
	float heightMap =			(material.HeightMap			  != 0xFFFFFFFFu) ? texture(TextureMap[material.HeightMap], inPS_UV).r			 : material.Height;
    vec3  emissionMap =			(material.EmissionMap		  != 0xFFFFFFFFu) ? texture(TextureMap[material.EmissionMap], inPS_UV).rgb		 : material.Emission;
	float alphaMap =			(material.AlphaMap			  != 0xFFFFFFFFu) ? texture(TextureMap[material.AlphaMap], inPS_UV).r			 : material.Alpha;

    if (albedoMap.a == 0.0)
	{
        discard;
	}

	PositionDataMap = vec4(inPS_Position, 1.0);
	AlbedoMap = vec4(albedoMap.rgb, 1.0f);
	NormalMap = vec4(normalMap, 1.0f);
	MatRoughAOHeightMap = vec4(metallicMap, roughnessMap, ambientOcclusionMap, heightMap);
	EmissionMap = vec4(emissionMap, specularMap);
}