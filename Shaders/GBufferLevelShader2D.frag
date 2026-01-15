#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 inPS_Position; 
layout(location = 1) in vec2 inPS_UV;    

layout(location = 0) out vec4 PositionDataMap;
layout(location = 1) out vec4 AlbedoMap;
layout(location = 2) out vec4 NormalMap;
layout(location = 3) out vec4 MatRoughAOHeightMap;
layout(location = 4) out vec4 EmissionMap;

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"

layout(constant_id = 0)   const uint DescriptorBindingType0   = SubpassInputDescriptor;
layout(constant_id = 1)   const uint DescriptorBindingType1   = SubpassInputDescriptor;
layout(constant_id = 2)   const uint DescriptorBindingType2   = SubpassInputDescriptor;
layout(constant_id = 3)   const uint DescriptorBindingType3   = SubpassInputDescriptor;
layout(constant_id = 4)   const uint DescriptorBindingType4   = SubpassInputDescriptor;
layout(constant_id = 5)   const uint DescriptorBindingType5   = SubpassInputDescriptor;
layout(constant_id = 6)   const uint DescriptorBindingType6   = SubpassInputDescriptor;
layout(constant_id = 7)   const uint DescriptorBindingType7   = MeshPropertiesDescriptor;
layout(constant_id = 8)   const uint DescriptorBindingType8   = MaterialDescriptor;
layout(constant_id = 9)   const uint DescriptorBindingType9   = DirectionalLightDescriptor;
layout(constant_id = 10)  const uint DescriptorBindingType10  = PointLightDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = TextureDescriptor;
layout(constant_id = 12)  const uint DescriptorBindingType12  = SkyBoxDescriptor;
layout(constant_id = 13)  const uint DescriptorBindingType13  = IrradianceCubeMapDescriptor;
layout(constant_id = 14)  const uint DescriptorBindingType14  = PrefilterDescriptor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionInput;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput albedoInput;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput normalInput;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput matRoughInput;
layout(input_attachment_index = 4, binding = 4) uniform subpassInput emissionInput;
layout(input_attachment_index = 5, binding = 5) uniform subpassInput depthInput;
layout(input_attachment_index = 6, binding = 6) uniform subpassInput skyBoxInput;
layout(binding = 7)  buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 8)  buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];
layout(binding = 9)  buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 10)  buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
layout(binding = 11) uniform sampler2D TextureMap[];
layout(binding = 12) uniform samplerCube CubeMap;
layout(binding = 13) uniform samplerCube IrradianceMap;
layout(binding = 14) uniform samplerCube PrefilterMap;

layout(push_constant) uniform SceneDataBuffer {
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

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