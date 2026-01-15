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
    vec3  ViewDirection;
    vec3 CameraPosition;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

mat3 TBN = mat3(
    vec3(1.0, 0.0, 0.0),   // Tangent   (along X/UV.x)
    vec3(0.0, 1.0, 0.0),   // Bitangent (along Y/UV.y)
    vec3(0.0, 0.0, 1.0)    // Normal    (+Z)
);

float ComputeParallaxShadow(vec2 uv, vec3 lightDirTS, uint heightIdx, float currentHeightAtPixel)
{
    if (sceneData.HeightScale < 0.001f || heightIdx == 0xFFFFFFFFu)
        return 1.0f;

    vec3 lightTS = normalize(lightDirTS);
    if (lightTS.z <= 0.0f) return 1.0f;

    const float shadowLayers = 32.0f;
    vec2 shiftDir = lightTS.xy * sceneData.HeightScale * 0.8f;
    vec2 deltaUV_light = shiftDir / shadowLayers;

    vec2 shadowUV = uv;
    float shadowDepth = currentHeightAtPixel;
    float occlusion = 0.0f;

    for (int i = 0; i < 32; ++i)
    {
        shadowUV += deltaUV_light;
        float sampledHeight = 1.0f - textureLod(TextureMap[heightIdx], shadowUV, 0.0f).r;
        if (sampledHeight > shadowDepth)
        {
            occlusion = 1.0f - (shadowDepth / sampledHeight);
            break;
        }

        shadowDepth += (1.0f / shadowLayers) * sceneData.HeightScale;
    }

    return 1.0f - occlusion * 0.7f;
}

vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS, uint heightIdx)
{
    if (sceneData.UseHeightMap == 0) return uv;

    const float minLayers = 16.0f;
    const float maxLayers = 96.0f;
    float numLayers = mix(maxLayers, minLayers, abs(viewDirTS.z));

    vec2 shiftDirection = viewDirTS.xy * sceneData.HeightScale * -1.0f;
    shiftDirection = clamp(shiftDirection, vec2(-0.06f), vec2(0.06f));

    vec2 deltaUV = shiftDirection / numLayers;

    vec2 currentUV = uv;
    float currentLayerDepth = 0.0f;
    float currentHeight = 1.0f - textureLod(TextureMap[heightIdx], currentUV, 0.0f).r;

    for (int i = 0; i < 96; ++i) {
        currentUV -= deltaUV;
        currentHeight = 1.0f - textureLod(TextureMap[heightIdx], currentUV, 0.0f).r;
        currentLayerDepth += 1.0f / numLayers;
        if (currentLayerDepth >= currentHeight) break;
    }

    vec2 prevUV = currentUV + deltaUV;
    float afterHeight = currentHeight - currentLayerDepth;
    float beforeHeight = 1.0 - textureLod(TextureMap[heightIdx], prevUV, 0.0).r - (currentLayerDepth - 1.0/numLayers);

    // Sharper mix
    float weight = step(0.5, afterHeight / (afterHeight + beforeHeight + 1e-5));
    vec2 finalUV = mix(currentUV, prevUV, weight);

    return clamp(finalUV, 0.01, 0.99);
}

void main() {
    MaterialProperitiesBuffer material = materialBuffer[PS_MaterialID].materialProperties;

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) UV.x = PS_UVOffset.x + PS_UVOffset.z - (UV.x - PS_UVOffset.x);
    if (PS_FlipSprite.y == 1) UV.y = PS_UVOffset.y + PS_UVOffset.w - (UV.y - PS_UVOffset.y);

    vec3  viewDirWS = normalize(sceneData.ViewDirection);
    vec3  viewDirTS = normalize(transpose(TBN) * viewDirWS);
    vec2  finalUV = ParallaxOcclusionMapping(UV, viewDirTS, material.HeightMap);
    float parallaxShader =  ComputeParallaxShadow(UV, )

    vec3 albedo = (material.AlbedoMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlbedoMap], finalUV, 0.0).rgb : material.Albedo;
    vec3 normalTS = (material.NormalMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.NormalMap], finalUV, 0.0).xyz * 2.0 - 1.0 : vec3(0.0, 0.0, 1.0);

    if (PS_FlipSprite.x == 1) normalTS.x = -normalTS.x;

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

    PositionDataMap     = vec4(PS_Position, 1.0);
    AlbedoMap           = vec4(albedo, 1.0);
    NormalMap           = vec4(normalWS * 0.5 + 0.5, 1.0);
    MatRoughAOHeightMap = vec4(metallic, roughness, ao, height);
    EmissionMap         = vec4(emission, 1.0);
}