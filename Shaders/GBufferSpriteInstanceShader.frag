#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout (location = 0) in vec3  WorldPos;
layout (location = 1) in vec2  PS_UV;
layout (location = 2) in vec2  PS_SpriteSize;
layout (location = 3) in flat ivec2 PS_FlipSprite;
layout (location = 4) in vec4  PS_Color;
layout (location = 5) in flat uint  PS_MaterialID;
layout (location = 6) in flat vec4  PS_UVOffset;

layout(location = 0) out vec4 outPosition;           //Position                                                                                   - R16G16B16A16_SFLOAT
layout(location = 1) out vec4 outAlbedo;             //Albedo/Alpha                                                                               - R8G8B8A8_UNORM
layout(location = 2) out vec4 outNormalData;         //Normal/NormalStrength                                                                      - R16G16B16A16_UNORM
layout(location = 3) out vec4 outPackedMRO;          //vec4(Metallic/Rough, AO/ClearcoatTint, ClearcoatStrength/ClearcoatRoughness, unused)       - R16G16B16A16_UNORM
layout(location = 4) out vec4 outPackedSheenSSS;     //vec4(sheenColor.r/sheenColor.g, sheenColor.b/sheenIntensity, sss.r/sss.g, sss.b/thickness) - R16G16B16A16_UNORM
layout(location = 5) out vec4 TempMap;               //vec4(                                                                                    ) - R16G16B16A16_UNORM
layout(location = 6) out vec4 outParallaxInfo;       //ParallaxUV/Height                                                                          - R16G16B16A16_UNORM
layout(location = 7) out vec4 outEmission;           //Emission                                                                                   - R8G8B8A8_UNORM

layout(constant_id = 0)   const uint DescriptorBindingType0   = SubpassInputDescriptor;
layout(constant_id = 1)   const uint DescriptorBindingType1   = SubpassInputDescriptor;
layout(constant_id = 2)   const uint DescriptorBindingType2   = SubpassInputDescriptor;
layout(constant_id = 3)   const uint DescriptorBindingType3   = SubpassInputDescriptor;
layout(constant_id = 4)   const uint DescriptorBindingType4   = SubpassInputDescriptor;
layout(constant_id = 5)   const uint DescriptorBindingType5   = SubpassInputDescriptor;
layout(constant_id = 6)   const uint DescriptorBindingType6   = SubpassInputDescriptor;
layout(constant_id = 7)   const uint DescriptorBindingType7   = SubpassInputDescriptor;
layout(constant_id = 8)   const uint DescriptorBindingType8   = SubpassInputDescriptor;
layout(constant_id = 9)   const uint DescriptorBindingType9   = SubpassInputDescriptor;
layout(constant_id = 10)  const uint DescriptorBindingType10   = MeshPropertiesDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = MaterialDescriptor;
layout(constant_id = 12)  const uint DescriptorBindingType12  = DirectionalLightDescriptor;
layout(constant_id = 13)  const uint DescriptorBindingType13  = PointLightDescriptor;
layout(constant_id = 14)  const uint DescriptorBindingType14  = TextureDescriptor;
layout(constant_id = 15)  const uint DescriptorBindingType15  = SkyBoxDescriptor;
layout(constant_id = 16)  const uint DescriptorBindingType16  = IrradianceCubeMapDescriptor;
layout(constant_id = 17)  const uint DescriptorBindingType17  = PrefilterDescriptor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionInput;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput albedoInput;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput normalInput;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput packedMROInput;
layout(input_attachment_index = 4, binding = 4) uniform subpassInput packedSheenSSSInput;
layout(input_attachment_index = 5, binding = 5) uniform subpassInput tempInput;
layout(input_attachment_index = 6, binding = 6) uniform subpassInput parallaxUVInfoInput;
layout(input_attachment_index = 7, binding = 7) uniform subpassInput emissionInput;
layout(input_attachment_index = 8, binding = 8) uniform subpassInput depthInput;
layout(input_attachment_index = 9, binding = 9) uniform subpassInput skyBoxInput;
layout(binding = 10)  buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 11)  buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];
layout(binding = 12)  buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 13)  buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
layout(binding = 14) uniform sampler2D TextureMap[];
layout(binding = 15) uniform samplerCube CubeMap;
layout(binding = 16) uniform samplerCube IrradianceMap;
layout(binding = 17) uniform samplerCube PrefilterMap;

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

vec2 OctahedronEncode(vec3 normal) 
{
    vec2 f = normal.xy / (abs(normal.x) + abs(normal.y) + abs(normal.z));
    return (normal.z < 0.0) ? (1.0 - abs(f.yx)) * sign(f) : f;
}

float Pack8bitPair(float high, float low) {
    uint u_high = uint(high * 255.0 + 0.5) & 0xFFu;
    uint u_low  = uint(low  * 255.0 + 0.5) & 0xFFu;
    uint combined = (u_high << 8) | u_low;  // high in MSBs, low in LSBs
    return float(combined) / 65535.0;
}

void main() {
    MaterialProperitiesBuffer material = materialBuffer[PS_MaterialID].materialProperties;

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) UV.x = PS_UVOffset.x + PS_UVOffset.z - (UV.x - PS_UVOffset.x);
    if (PS_FlipSprite.y == 1) UV.y = PS_UVOffset.y + PS_UVOffset.w - (UV.y - PS_UVOffset.y);

    vec3  viewDirWS = normalize(sceneData.ViewDirection);
    vec3  viewDirTS = normalize(transpose(TBN) * viewDirWS);
    vec2  finalUV = ParallaxOcclusionMapping(UV, viewDirTS, material.HeightMap);

    vec4  albedo                    = (material.AlbedoMap                    != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlbedoMap],                    finalUV, 0.0f)                 : vec4(material.Albedo, material.Alpha);
    float metallic                  = (material.MetallicMap                  != 0xFFFFFFFFu) ? textureLod(TextureMap[material.MetallicMap],                  finalUV, 0.0f).r               : material.Metallic;
    float roughness                 = (material.MetallicMap                  != 0xFFFFFFFFu) ? textureLod(TextureMap[material.MetallicMap],                  finalUV, 0.0f).g               : material.Roughness;
    float thickness                 = (material.ThicknessMap                 != 0xFFFFFFFFu) ? textureLod(TextureMap[material.ThicknessMap],                 finalUV, 0.0f).r               : material.Thickness;
    vec3  subSurfaceScatteringColor = (material.SubSurfaceScatteringColorMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.SubSurfaceScatteringColorMap], finalUV, 0.0f).rgb             : material.SubSurfaceScatteringColor;
    vec3  sheenColor                = (material.SheenMap                     != 0xFFFFFFFFu) ? textureLod(TextureMap[material.SheenMap],                     finalUV, 0.0f).rgb             : material.SheenColor;
    float clearcoatTint             = (material.MetallicMap                  != 0xFFFFFFFFu) ? textureLod(TextureMap[material.ClearCoatMap],                 finalUV, 0.0f).r               : material.ClearcoatTint;
    float ambientOcclusion          = (material.AmbientOcclusionMap          != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AmbientOcclusionMap],          finalUV, 0.0f).r               : material.AmbientOcclusion;
    vec3  normalMap                 = (material.NormalMap                    != 0xFFFFFFFFu) ? textureLod(TextureMap[material.NormalMap],                    finalUV, 0.0f).xyz * 2.0 - 1.0 : vec3(0.0f, 0.0f, 1.0f) * 2.0 - 1.0;
    float alpha                     = (material.AlphaMap                     != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlphaMap],                     finalUV, 0.0f).r               : material.Alpha;
    vec3  emission                  = (material.EmissionMap                  != 0xFFFFFFFFu) ? textureLod(TextureMap[material.EmissionMap],                  finalUV, 0.0f).rgb             : vec3(0.0f);
    float height                    = (material.HeightMap                    != 0xFFFFFFFFu) ? textureLod(TextureMap[material.HeightMap],                    finalUV, 0.0).r                : material.Height;

    if (alpha < 0.1) discard; 

    vec3 normalWS = normalize(TBN * normalMap);
    normalWS.xy *= material.NormalStrength;
    normalWS = normalize(normalWS);
    vec2 encodedNormal = OctahedronEncode(normalWS);

    outPosition = vec4(WorldPos, 1.0);
    outAlbedo = albedo;
    outNormalData = vec4((encodedNormal * 0.5f) + 0.5f, 0.0f, 1.0f);
    outPackedMRO = vec4(Pack8bitPair(metallic, roughness), Pack8bitPair(ambientOcclusion, clearcoatTint), Pack8bitPair(material.ClearcoatStrength, material.ClearcoatRoughness), 1.0);
    outPackedSheenSSS = vec4(Pack8bitPair(sheenColor.r, sheenColor.g), Pack8bitPair(sheenColor.b, material.SheenIntensity), Pack8bitPair(subSurfaceScatteringColor.r, subSurfaceScatteringColor.g), Pack8bitPair(subSurfaceScatteringColor.b, thickness));
    outParallaxInfo = vec4(finalUV - UV, height, 0.0);
    outEmission = vec4(emission, material.ClearcoatRoughness);
}