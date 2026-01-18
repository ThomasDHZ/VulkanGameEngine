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
layout(location = 3) out vec4 MatRoughAOMap;
layout(location = 4) out vec4 ParallaxUVInfoMap;
layout(location = 5) out vec4 EmissionMap;

layout(constant_id = 0)   const uint DescriptorBindingType0   = SubpassInputDescriptor;
layout(constant_id = 1)   const uint DescriptorBindingType1   = SubpassInputDescriptor;
layout(constant_id = 2)   const uint DescriptorBindingType2   = SubpassInputDescriptor;
layout(constant_id = 3)   const uint DescriptorBindingType3   = SubpassInputDescriptor;
layout(constant_id = 4)   const uint DescriptorBindingType4   = SubpassInputDescriptor;
layout(constant_id = 5)   const uint DescriptorBindingType5   = SubpassInputDescriptor;
layout(constant_id = 6)   const uint DescriptorBindingType6   = SubpassInputDescriptor;
layout(constant_id = 7)   const uint DescriptorBindingType7   = SubpassInputDescriptor;
layout(constant_id = 8)   const uint DescriptorBindingType8   = MeshPropertiesDescriptor;
layout(constant_id = 9)   const uint DescriptorBindingType9   = MaterialDescriptor;
layout(constant_id = 10)  const uint DescriptorBindingType10  = DirectionalLightDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = PointLightDescriptor;
layout(constant_id = 12)  const uint DescriptorBindingType12  = TextureDescriptor;
layout(constant_id = 13)  const uint DescriptorBindingType13  = SkyBoxDescriptor;
layout(constant_id = 14)  const uint DescriptorBindingType14  = IrradianceCubeMapDescriptor;
layout(constant_id = 15)  const uint DescriptorBindingType15  = PrefilterDescriptor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionInput;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput albedoInput;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput normalInput;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput matRoughAOInput;
layout(input_attachment_index = 4, binding = 4) uniform subpassInput ParallaxUVInfoInput;
layout(input_attachment_index = 5, binding = 5) uniform subpassInput emissionInput;
layout(input_attachment_index = 6, binding = 6) uniform subpassInput depthInput;
layout(input_attachment_index = 7, binding = 7) uniform subpassInput skyBoxInput;
layout(binding = 8)  buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 9)  buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];
layout(binding = 10)  buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 11)  buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
layout(binding = 12) uniform sampler2D TextureMap[];
layout(binding = 13) uniform samplerCube CubeMap;
layout(binding = 14) uniform samplerCube IrradianceMap;
layout(binding = 15) uniform samplerCube PrefilterMap;

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

    for (int x = 0; x < 96; ++x) 
    {
        currentUV -= deltaUV;
        currentHeight = 1.0f - textureLod(TextureMap[heightIdx], currentUV, 0.0f).r;
        currentLayerDepth += 1.0f / numLayers;
        if (currentLayerDepth >= currentHeight) break;
    }

    vec2 prevUV = currentUV + deltaUV;
    float afterHeight = currentHeight - currentLayerDepth;
    float beforeHeight = 1.0f - textureLod(TextureMap[heightIdx], prevUV, 0.0f).r - (currentLayerDepth - 1.0f/numLayers);

    float weight = step(0.5, afterHeight / (afterHeight + beforeHeight + 1e-5f));
    vec2 finalUV = mix(currentUV, prevUV, weight);

    return clamp(finalUV, 0.01f, 0.99f);
}

void main() {
    MaterialProperitiesBuffer material = materialBuffer[PS_MaterialID].materialProperties;

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) UV.x = PS_UVOffset.x + PS_UVOffset.z - (UV.x - PS_UVOffset.x);
    if (PS_FlipSprite.y == 1) UV.y = PS_UVOffset.y + PS_UVOffset.w - (UV.y - PS_UVOffset.y);

    vec3  viewDirWS = normalize(sceneData.ViewDirection);
    vec3  viewDirTS = normalize(transpose(TBN) * viewDirWS);
    vec2  finalUV = ParallaxOcclusionMapping(UV, viewDirTS, material.HeightMap);

    vec4  albedo     = (material.AlbedoMap           != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlbedoMap],           finalUV, 0.0f)                   : vec4(material.Albedo, 1.0f);
    vec3  normalTS   = (material.NormalMap           != 0xFFFFFFFFu) ? textureLod(TextureMap[material.NormalMap],           finalUV, 0.0f).xyz * 2.0f - 1.0f : vec3(0.0f, 0.0f, 1.0f);
    float metallic   = (material.MetallicMap         != 0xFFFFFFFFu) ? textureLod(TextureMap[material.MetallicMap], finalUV, 0.0f).r :material.Metallic;
    float roughness  = (material.MetallicMap         != 0xFFFFFFFFu) ? textureLod(TextureMap[material.MetallicMap], finalUV, 0.0f).g : material.Roughness;
    float ao         = (material.AmbientOcclusionMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AmbientOcclusionMap], finalUV, 0.0f).r                 : material.AmbientOcclusion;
    vec3  emission   = (material.EmissionMap         != 0xFFFFFFFFu) ? textureLod(TextureMap[material.EmissionMap],         finalUV, 0.0f).rgb               : material.Emission;
    float height     = (material.HeightMap           != 0xFFFFFFFFu) ? textureLod(TextureMap[material.HeightMap],           finalUV, 0.0f).r                 : 0.5f;
    float alpha      = (material.AlphaMap            != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlphaMap],            finalUV, 0.0f).r                 : material.Alpha;

    if (PS_FlipSprite.x == 1) normalTS.x = -normalTS.x;
    if (alpha < 0.01f) discard;

    vec3 normalWS = normalize(TBN * normalTS);
    normalWS.xy *= material.NormalStrength;
    normalWS = normalize(normalWS);

    PositionDataMap     = vec4(PS_Position, 1.0f);
    AlbedoMap           = albedo;
    NormalMap           = vec4(normalWS * 0.5f + 0.5f, 1.0f);
    MatRoughAOMap       = vec4(metallic, roughness, ao, 1.0f);
    ParallaxUVInfoMap   = vec4(finalUV - UV, height, 1.0f);
    EmissionMap         = vec4(emission, 1.0f);
}