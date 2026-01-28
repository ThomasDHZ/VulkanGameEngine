#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

#include "Constants.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout(constant_id = 0)  const uint Using16BitPackedDataAttachmentR0   = 0;
layout(constant_id = 1)  const uint Using16BitPackedDataAttachmentG0   = 0;
layout(constant_id = 2)  const uint Using16BitPackedDataAttachmentB0   = 0;
layout(constant_id = 3)  const uint Using16BitPackedDataAttachmentA0   = 0;

layout(constant_id = 4)  const uint Using16BitPackedDataAttachmentR1   = 1;
layout(constant_id = 5)  const uint Using16BitPackedDataAttachmentG1   = 1;
layout(constant_id = 6)  const uint Using16BitPackedDataAttachmentB1   = 0;
layout(constant_id = 7)  const uint Using16BitPackedDataAttachmentA1   = 0;

layout(constant_id = 8)  const uint Using16BitPackedDataAttachmentR2   = 1;
layout(constant_id = 9)  const uint Using16BitPackedDataAttachmentG2   = 1;
layout(constant_id = 10) const uint Using16BitPackedDataAttachmentB2   = 1;
layout(constant_id = 11) const uint Using16BitPackedDataAttachmentA2   = 0;

layout(constant_id = 12) const uint Using16BitPackedDataAttachmentR3   = 1;
layout(constant_id = 13) const uint Using16BitPackedDataAttachmentG3   = 1;
layout(constant_id = 14) const uint Using16BitPackedDataAttachmentB3   = 1;
layout(constant_id = 15) const uint Using16BitPackedDataAttachmentA3   = 0;

layout(constant_id = 16) const uint Using16BitPackedDataAttachmentR4   = 0;
layout(constant_id = 17) const uint Using16BitPackedDataAttachmentG4   = 0;
layout(constant_id = 18) const uint Using16BitPackedDataAttachmentB4   = 0;
layout(constant_id = 19) const uint Using16BitPackedDataAttachmentA4   = 0;

layout(constant_id = 20) const uint DescriptorBindingType0  = MaterialDescriptor;
layout(constant_id = 21) const uint DescriptorBindingType1  = TextureDescriptor;
layout(constant_id = 22) const uint DescriptorBindingType2  = TextureDescriptor;
layout(constant_id = 23) const uint DescriptorBindingType3  = TextureDescriptor;
layout(constant_id = 24) const uint DescriptorBindingType4  = TextureDescriptor;
layout(constant_id = 25) const uint DescriptorBindingType5  = TextureDescriptor;
layout(constant_id = 26) const uint DescriptorBindingType6  = TextureDescriptor;
layout(constant_id = 27) const uint DescriptorBindingType7  = TextureDescriptor;
layout(constant_id = 28) const uint DescriptorBindingType8  = TextureDescriptor;
layout(constant_id = 29) const uint DescriptorBindingType9  = TextureDescriptor;
layout(constant_id = 30) const uint DescriptorBindingType10 = TextureDescriptor;
layout(constant_id = 31) const uint DescriptorBindingType11 = TextureDescriptor;
layout(constant_id = 32) const uint DescriptorBindingType12 = TextureDescriptor;




layout(location = 0) in vec2 UV;    

layout(location = 0) out vec4 outAlbedo;             //Albedo/Alpha                                                                               - R8G8B8A8_SRGB
layout(location = 1) out vec4 outNormalData;         //Normal/Height/unused                                                                       - R16G16B16A16_UNORM
layout(location = 2) out vec4 outPackedMRO;          //vec4(Metallic/Rough, AO/ClearcoatTint, ClearcoatStrength/ClearcoatRoughness, unused)       - R16G16B16A16_UNORM
layout(location = 3) out vec4 outPackedSheenSSS;     //vec4(sheenColor.r/sheenColor.g, sheenColor.b/sheenIntensity, sss.r/sss.g, sss.b/thickness) - R16G16B16A16_UNORM
layout(location = 4) out vec4 outUnused;             //unused for now                                                                             - R16G16B16A16_UNORM
layout(location = 5) out vec4 outEmission;           //Emission                                                                                   - R8G8B8A8_SRGB

layout(binding = 0)  buffer  MaterialProperities { MaterialProperitiesBuffer importMaterialProperties; } importMaterialBuffer;
layout(binding = 1)  uniform sampler2D AlbedoMap;
layout(binding = 2)  uniform sampler2D MetallicMap;
layout(binding = 3)  uniform sampler2D RoughnessMap;
layout(binding = 4)  uniform sampler2D ThicknessMap;
layout(binding = 5)  uniform sampler2D SubSurfaceScatteringMap;
layout(binding = 6)  uniform sampler2D SheenMap;
layout(binding = 7)  uniform sampler2D ClearCoatMap;
layout(binding = 8)  uniform sampler2D AmbientOcclusionMap;
layout(binding = 9)  uniform sampler2D NormalMap;
layout(binding = 10) uniform sampler2D AlphaMap;
layout(binding = 11) uniform sampler2D EmissionMap;
layout(binding = 12) uniform sampler2D HeightMap;

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

void main()
{
    MaterialProperitiesBuffer material = importMaterialBuffer.importMaterialProperties;

    vec4  albedo                    = (material.AlbedoMap                    != 0xFFFFFFFFu) ? textureLod(AlbedoMap,                    UV, 0.0f)                   : vec4(material.Albedo, 1.0f);
    float metallic                  = (material.MetallicMap                  != 0xFFFFFFFFu) ? textureLod(MetallicMap,                  UV, 0.0f).r                 : material.Metallic;
    float roughness                 = (material.MetallicMap                  != 0xFFFFFFFFu) ? textureLod(MetallicMap,                  UV, 0.0f).g                 : material.Roughness;
    vec3  normalMapRaw              = (material.NormalMap                    != 0xFFFFFFFFu) ? textureLod(NormalMap,                    UV, 0.0f).rgb               : vec3(0.5f, 0.5f, 1.0f);
    float thickness                 = (material.ThicknessMap                 != 0xFFFFFFFFu) ? textureLod(ThicknessMap,                 UV, 0.0f).r                 : material.Thickness;
    vec3  subSurfaceScatteringColor = (material.SubSurfaceScatteringColorMap != 0xFFFFFFFFu) ? textureLod(SubSurfaceScatteringMap,      UV, 0.0f).rgb               : material.SubSurfaceScatteringColor;
    vec3  sheenColor                = (material.SheenMap                     != 0xFFFFFFFFu) ? textureLod(SheenMap,                     UV, 0.0f).rgb               : material.SheenColor;
    float clearcoatTint             = (material.ClearCoatMap                 != 0xFFFFFFFFu) ? textureLod(ClearCoatMap,                 UV, 0.0f).r                 : material.ClearcoatTint;
    float ambientOcclusion          = (material.AmbientOcclusionMap          != 0xFFFFFFFFu) ? textureLod(AmbientOcclusionMap,          UV, 0.0f).r                 : material.AmbientOcclusion;
    vec3  emission                  = (material.EmissionMap                  != 0xFFFFFFFFu) ? textureLod(EmissionMap,                  UV, 0.0f).rgb               : vec3(0.0f);
    float height                    = (material.HeightMap                    != 0xFFFFFFFFu) ? textureLod(HeightMap,                    UV, 0.0f).r                 : material.Height;

    vec3 tangentNormal = normalMapRaw * 2.0 - 1.0;
    tangentNormal = normalize(tangentNormal);
    vec2 encodedNormal = OctahedronEncode(tangentNormal);

    outAlbedo = albedo;
    outNormalData = vec4(encodedNormal * 0.5 + 0.5, height, 1.0f);
    outPackedMRO = vec4(metallic, roughness, ambientOcclusion, 1.0);
    outPackedSheenSSS = vec4(0.25f, 0.50f, 0.75f, 1.0f);
    //outPackedMRO = vec4(Pack8bitPair(metallic, roughness), Pack8bitPair(ambientOcclusion, clearcoatTint), Pack8bitPair(material.ClearcoatStrength, material.ClearcoatRoughness), 1.0);
  //  outPackedSheenSSS = vec4(Pack8bitPair(sheenColor.r, sheenColor.g), Pack8bitPair(sheenColor.b, material.SheenIntensity), Pack8bitPair(subSurfaceScatteringColor.r, subSurfaceScatteringColor.g), Pack8bitPair(subSurfaceScatteringColor.b, thickness));

    outUnused = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    outEmission = vec4(emission, material.ClearcoatRoughness);
}