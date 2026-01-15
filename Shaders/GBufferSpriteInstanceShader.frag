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
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

mat3 CalculateTBN(vec3 worldPos, vec2 uv) {
    vec3 dp1   = dFdx(worldPos);
    vec3 dp2   = dFdy(worldPos);
    vec2 duv1  = dFdx(uv);
    vec2 duv2  = dFdy(uv);

    vec3 N = vec3(0.0, 0.0, 1.0);

    vec3 T = normalize(dp1 * duv2.t - dp2 * duv1.t);
    vec3 B = -normalize(cross(N, T));

    return mat3(T, B, N);
}

vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS, uint heightIdx) 
{
    if (sceneData.UseHeightMap == 0 || sceneData.HeightScale < 0.001 || heightIdx == 0xFFFFFFFFu)
        return uv;

    const float minLayers = 16.0;
    const float maxLayers = 96.0;
    float numLayers = mix(maxLayers, minLayers, max(0.0, viewDirTS.z));

    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;

    vec2 P = viewDirTS.xy * (sceneData.HeightScale * max(0.1, viewDirTS.z));
    vec2 deltaUV = P / numLayers;

    vec2 currentUV = uv;
    float currentHeight = textureLod(TextureMap[heightIdx], currentUV, 0.0).r;

    for (float i = 0.0; i < numLayers; ++i) {
        if (currentLayerDepth > currentHeight) break;
        currentUV += deltaUV;
        currentHeight = textureLod(TextureMap[heightIdx], currentUV, 0.0).r;
        currentLayerDepth += layerDepth;
    }

    vec2 prevUV = currentUV - deltaUV;
    float afterHeight  = currentHeight - currentLayerDepth;
    float beforeHeight = textureLod(TextureMap[heightIdx], prevUV, 0.0).r - (currentLayerDepth - layerDepth);
    float weight = afterHeight / (afterHeight + beforeHeight + 1e-6);

    vec2 finalUV = mix(currentUV, prevUV, weight);
    return clamp(finalUV, 0.001, 0.999);
}

void main() {
    MaterialProperitiesBuffer material = materialBuffer[PS_MaterialID].materialProperties;

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1)
        UV.x = PS_UVOffset.x + PS_UVOffset.z - (UV.x - PS_UVOffset.x);
    if (PS_FlipSprite.y == 1)
        UV.y = PS_UVOffset.y + PS_UVOffset.w - (UV.y - PS_UVOffset.y);

    mat3 TBN = CalculateTBN(PS_Position, UV);

    vec3 viewDirWS = normalize(sceneData.CameraPosition - PS_Position);
    vec3 viewDirTS = normalize(transpose(TBN) * viewDirWS);

    vec2 finalUV = ParallaxOcclusionMapping(UV, viewDirTS, material.HeightMap);

    vec3 albedo = (material.AlbedoMap != 0xFFFFFFFFu) ?
        textureLod(TextureMap[material.AlbedoMap], finalUV, 0.0).rgb : material.Albedo;

    vec3 normalTS = (material.NormalMap != 0xFFFFFFFFu) ?
        textureLod(TextureMap[material.NormalMap], finalUV, 0.0).xyz * 2.0 - 1.0 :
        vec3(0.0, 0.0, 1.0);

    if (PS_FlipSprite.x == 1)
        normalTS.x = -normalTS.x;

    float metallic   = (material.MetallicMap   != 0xFFFFFFFFu) ? textureLod(TextureMap[material.MetallicMap],   finalUV, 0.0).r : material.Metallic;
    float roughness  = (material.RoughnessMap  != 0xFFFFFFFFu) ? textureLod(TextureMap[material.RoughnessMap],  finalUV, 0.0).r : material.Roughness;
    float ao         = (material.AmbientOcclusionMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AmbientOcclusionMap], finalUV, 0.0).r : material.AmbientOcclusion;
    vec3 emission    = (material.EmissionMap   != 0xFFFFFFFFu) ? textureLod(TextureMap[material.EmissionMap],   finalUV, 0.0).rgb : material.Emission;
    float height     = (material.HeightMap     != 0xFFFFFFFFu) ? textureLod(TextureMap[material.HeightMap],     finalUV, 0.0).r : 0.5;
    float alpha      = (material.AlphaMap      != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlphaMap],      finalUV, 0.0).r : material.Alpha;

    if (alpha < 0.01) discard;

    vec3 normalWS = normalize(TBN * normalTS);
    const float normalStrength = 0.5; 
    normalWS.xy *= normalStrength;
    normalWS = normalize(normalWS);

    // Output
    PositionDataMap     = vec4(PS_Position, 1.0);
    AlbedoMap           = vec4(albedo, 1.0);
    NormalMap           = vec4(normalWS * 0.5 + 0.5, 1.0);
    MatRoughAOHeightMap = vec4(metallic, roughness, ao, height);
    EmissionMap         = vec4(emission, 1.0);
}