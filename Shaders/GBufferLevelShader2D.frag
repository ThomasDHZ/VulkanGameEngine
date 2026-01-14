#version 460
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
    int   UseHeightMap;
    float HeightScale;
} sceneData;

#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"

layout(binding = 0) buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 1) uniform sampler2D TextureMap[];
layout(binding = 2) buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];

mat3 GetTBN() 
{
    vec3 T = normalize(vec3(1.0f, 0.0f, 0.0f));
    vec3 B = normalize(vec3(0.0f, 1.0f, 0.0f));
    vec3 N = normalize(cross(T, B));
    return mat3(T, B, N);
}

vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS, uint heightIdx) 
{
    if (sceneData.UseHeightMap == 0 || sceneData.HeightScale < 0.001f || heightIdx == 0xFFFFFFFFu) return uv;

    ivec2 texSize = textureSize(TextureMap[heightIdx], 0);
    float numLayers = mix(48.0f, 32.0f, abs(viewDirTS.z));
    vec2 P = viewDirTS.xy * sceneData.HeightScale;
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
    int meshIdx = sceneData.MeshBufferIndex;
    uint matId = meshBuffer[meshIdx].meshProperties.MaterialIndex;
    MaterialProperitiesBuffer material = materialBuffer[matId].materialProperties;

    mat3 TBN = GetTBN();                   
    mat3 worldToTangent = transpose(TBN);  
    vec3 viewDirWS = normalize(sceneData.CameraPosition - inPS_Position);
    vec3 viewDirTS = normalize(worldToTangent * viewDirWS);
    vec2 finalUV = ParallaxOcclusionMapping(inPS_UV, viewDirTS, material.HeightMap);

    vec3  albedo =     (material.AlbedoMap           != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlbedoMap],           finalUV, 0.0f).rgb               : material.Albedo;
    vec3  normalTS =   (material.NormalMap           != 0xFFFFFFFFu) ? textureLod(TextureMap[material.NormalMap],           finalUV, 0.0f).xyz * 2.0f - 1.0f : vec3(0,0,1);
    float metallic =   (material.MetallicMap         != 0xFFFFFFFFu) ? textureLod(TextureMap[material.MetallicMap],         finalUV, 0.0f).r                 : material.Metallic;
    float roughness =  (material.RoughnessMap        != 0xFFFFFFFFu) ? textureLod(TextureMap[material.RoughnessMap],        finalUV, 0.0f).r                 : material.Roughness;
    float ao =         (material.AmbientOcclusionMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AmbientOcclusionMap], finalUV, 0.0f).r                 : material.AmbientOcclusion;
    vec3  emission =   (material.EmissionMap         != 0xFFFFFFFFu) ? textureLod(TextureMap[material.EmissionMap],         finalUV, 0.0f).rgb               : material.Emission;
    float height =     (material.HeightMap           != 0xFFFFFFFFu) ? textureLod(TextureMap[material.HeightMap],           finalUV, 0.0f).r                 : 0.5f;
	float alphaMap =   (material.AlphaMap			 != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlphaMap],            finalUV, 0.0f).r				 : material.Alpha;

    if (alphaMap.r == 0.0) discard;
    vec3 normalWS = normalize(TBN * normalTS);

    PositionDataMap = vec4(inPS_Position, 1.0f);
    AlbedoMap = vec4(albedo, 1.0f);
    NormalMap = vec4(normalWS * 0.5f + 0.5f, 1.0f);
    MatRoughAOHeightMap = vec4(height, roughness, ao, height);
    EmissionMap = vec4(emission, 1.0f);
}