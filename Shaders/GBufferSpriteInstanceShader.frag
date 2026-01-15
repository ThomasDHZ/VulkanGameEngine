#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"

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

layout(constant_id = 0)   const uint DescriptorBindingType7   = MeshPropertiesDescriptor;
layout(constant_id = 1)   const uint DescriptorBindingType8   = MaterialDescriptor;
layout(constant_id = 2)  const uint DescriptorBindingType11  = TextureDescriptor;
layout(constant_id = 3)  const uint DescriptorBindingType12  = SkyBoxDescriptor;
layout(constant_id = 4)  const uint DescriptorBindingType13  = IrradianceCubeMapDescriptor;
layout(constant_id = 5)  const uint DescriptorBindingType14  = PrefilterDescriptor;

layout(binding = 7)  buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 8)  buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];
layout(binding = 11) uniform sampler2D TextureMap[];
layout(binding = 12) uniform samplerCube CubeMap;
layout(binding = 13) uniform samplerCube IrradianceMap;
layout(binding = 14) uniform samplerCube PrefilterMap;

layout(push_constant) uniform SceneDataBuffer
{
	int	 MeshBufferIndex;
	mat4 Projection;
	mat4 View;
	vec3 CameraPosition;
}sceneData;

mat3 GetTBN() 
{
    vec3 T = normalize(vec3(1.0f, 0.0f, 0.0f));
    vec3 B = normalize(vec3(0.0f, 1.0f, 0.0f));
    vec3 N = normalize(cross(T, B));
    return mat3(T, B, N);
}

vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS, uint heightIdx) 
{
    float heightScale = .015f;
    if (heightScale < 0.001f || heightIdx == 0xFFFFFFFFu) return uv;

    ivec2 texSize = textureSize(TextureMap[heightIdx], 0);
    float numLayers = mix(48.0f, 32.0f, abs(viewDirTS.z));
    vec2 P = viewDirTS.xy * heightScale;
    vec2 deltaUV = P / numLayers;

    vec2 currentUV = uv;
    float layerDepth = 1.0f / numLayers;
    float currentLayerDepth = 0.0f;
    float currentHeight = textureLod(TextureMap[heightIdx], currentUV, 0.0f).r;

    for (int x = 0; x < 48; ++x) {
        if (currentLayerDepth > currentHeight) break;
        currentUV -= deltaUV;
        currentHeight = textureLod(TextureMap[heightIdx], currentUV, 0.0f).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevUV = currentUV + deltaUV;
    float afterDepth = currentHeight - currentLayerDepth;
    float beforeDepth = textureLod(TextureMap[heightIdx], prevUV, 0.0f).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth + beforeDepth + 1e-5f);

    return clamp(mix(currentUV, prevUV, weight), 0.005f, 0.995f);
}

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

    mat3 TBN = GetTBN();                   
    mat3 worldToTangent = transpose(TBN);  
    vec3 viewDirWS = normalize(sceneData.CameraPosition - PS_Position);
    vec3 viewDirTS = normalize(worldToTangent * viewDirWS);
    vec2 finalUV = ParallaxOcclusionMapping(UV, viewDirTS, material.HeightMap);

    vec3 albedo = (material.AlbedoMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlbedoMap], finalUV, 0.0f).rgb : material.Albedo;
    vec3 normalTS = (material.NormalMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.NormalMap], finalUV, 0.0f).xyz * 2.0f - 1.0f : vec3(0,0,1);
    float metallic = (material.MetallicMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.MetallicMap], finalUV, 0.0f).r : material.Metallic;
    float roughness = (material.RoughnessMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.RoughnessMap], finalUV, 0.0f).r : material.Roughness;
    float ao = (material.AmbientOcclusionMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AmbientOcclusionMap], finalUV, 0.0f).r : material.AmbientOcclusion;
    vec3 emission = (material.EmissionMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.EmissionMap], finalUV, 0.0f).rgb : material.Emission;
    float height = (material.HeightMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.HeightMap], finalUV, 0.0f).r : 0.0;
	float alphaMap =			(material.AlphaMap			  != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlphaMap], finalUV, 0.0f).r				 : material.Alpha;

    if (alphaMap.r == 0.0) discard;
    vec3 normalWS = normalize(TBN * normalTS);

    PositionDataMap = vec4(PS_Position, 1.0f);
    AlbedoMap = vec4(albedo, 1.0f);
    NormalMap = vec4(normalWS * 0.5f + 0.5f, 1.0f);
    MatRoughAOHeightMap = vec4(metallic, roughness, ao, height);
    EmissionMap = vec4(emission, 1.0f);
}