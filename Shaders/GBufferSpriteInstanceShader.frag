#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 0;
layout(constant_id = 1) const uint DescriptorBindingType1 = 1;
layout(constant_id = 2) const uint DescriptorBindingType2 = 2;

layout (location = 0) in vec3  PS_Position;
layout (location = 1) in vec2  PS_UV;
layout (location = 2) in vec2  PS_SpriteSize;
layout (location = 3) in flat ivec2 PS_FlipSprite;
layout (location = 4) in vec4  PS_Color;
layout (location = 5) in flat uint  PS_MaterialID;
layout (location = 6) in flat vec4  PS_UVOffset;

layout(location = 0) out vec4 PositionDataMap;
layout(location = 1) out vec4 AlbedoMap;
layout(location = 2) out vec4 NormalMap;
layout(location = 3) out vec4 MatRoughAOHeightMap;
layout(location = 4) out vec4 EmissionMap;

layout(push_constant) uniform SceneDataBuffer
{
	int	 MeshBufferIndex;
	mat4 Projection;
	mat4 View;
	vec3 CameraPosition;
}sceneData;

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
	MaterialProperitiesBuffer material = materialBuffer[PS_MaterialID].materialProperties;

	vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) 
	{
		UV.x = PS_UVOffset.x + PS_UVOffset.z - (UV.x - PS_UVOffset.x);
    }
    if (PS_FlipSprite.y == 1) 
	{
		UV.y = PS_UVOffset.y + PS_UVOffset.w - (UV.y - PS_UVOffset.y);
    }

    vec4  albedoMap =			(material.AlbedoMap			  != 0xFFFFFFFFu) ? texture(TextureMap[material.AlbedoMap], PS_UV).rgba			 : vec4(material.Albedo, 1.0f);
    vec3  normalMap =			(material.NormalMap			  != 0xFFFFFFFFu) ? texture(TextureMap[material.NormalMap], PS_UV).rgb			 : vec3(0.0, 0.0, 1.0);
	float specularMap =			(material.SpecularMap		  != 0xFFFFFFFFu) ? texture(TextureMap[material.SpecularMap], PS_UV).r			 : material.Specular;
    float metallicMap =			(material.MetallicMap		  != 0xFFFFFFFFu) ? texture(TextureMap[material.MetallicMap], PS_UV).r			 : material.Metallic;
    float roughnessMap =		(material.RoughnessMap		  != 0xFFFFFFFFu) ? texture(TextureMap[material.RoughnessMap], PS_UV).r			 : material.Roughness;
    float ambientOcclusionMap = (material.AmbientOcclusionMap != 0xFFFFFFFFu) ? texture(TextureMap[material.AmbientOcclusionMap], PS_UV).r	 : material.AmbientOcclusion;
	float heightMap =			(material.HeightMap			  != 0xFFFFFFFFu) ? texture(TextureMap[material.HeightMap], PS_UV).r			 : material.Height;
    vec3  emissionMap =			(material.EmissionMap		  != 0xFFFFFFFFu) ? texture(TextureMap[material.EmissionMap], PS_UV).rgb		 : material.Emission;
	float alphaMap =			(material.AlphaMap			  != 0xFFFFFFFFu) ? texture(TextureMap[material.AlphaMap], PS_UV).r				 : material.Alpha;

    if (albedoMap.a == 0.0)
	{
        discard;
	}

	PositionDataMap = vec4(PS_Position, 1.0);
	AlbedoMap = vec4(albedoMap.rgb, 1.0f);
	NormalMap = vec4(normalMap, 1.0f);
	MatRoughAOHeightMap = vec4(metallicMap, roughnessMap, ambientOcclusionMap, heightMap);
	EmissionMap = vec4(emissionMap, specularMap);
}