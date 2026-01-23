#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

#include "Constants.glsl"
#include "MaterialPropertiesBuffer.glsl" 
layout(constant_id = 0) const uint DescriptorBindingType0 = MaterialDescriptor;
layout(constant_id = 1) const uint DescriptorBindingType1 = TextureDescriptor;

layout(location = 0) in vec2 UV;    


layout(location = 0) out vec4 outAlbedo;             //Albedo/Alpha                                                                               - R8G8B8A8_SRGB
layout(location = 1) out vec4 outNormalData;         //Normal/NormalStrength/Height                                                               - R16G16B16A16_UNORM
layout(location = 2) out vec4 outPackedMRO;          //vec4(Metallic/Rough, AO/ClearcoatTint, ClearcoatStrength/ClearcoatRoughness, unused)       - R16G16B16A16_UNORM
layout(location = 3) out vec4 outPackedSheenSSS;     //vec4(sheenColor.r/sheenColor.g, sheenColor.b/sheenIntensity, sss.r/sss.g, sss.b/thickness) - R16G16B16A16_UNORM
layout(location = 4) out vec4 outEmission;           //Emission                                                                                   - R8G8B8A8_SRGB

layout(binding = 0) buffer MaterialProperities { MaterialProperitiesBuffer importMaterialProperties; } importMaterialBuffer;
layout(binding = 1) uniform sampler2D TextureMap[];

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

    vec4  albedo                    = (material.AlbedoMap                    != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlbedoMap],                    UV, 0.0f)                   : vec4(material.Albedo, 1.0f);
    float metallic                  = (material.MetallicMap                  != 0xFFFFFFFFu) ? textureLod(TextureMap[material.MetallicMap],                  UV, 0.0f).r                 : material.Metallic;
    float roughness                 = (material.MetallicMap                  != 0xFFFFFFFFu) ? textureLod(TextureMap[material.MetallicMap],                  UV, 0.0f).g                 : material.Roughness;
    float thickness                 = (material.ThicknessMap                 != 0xFFFFFFFFu) ? textureLod(TextureMap[material.ThicknessMap],                 UV, 0.0f).r                 : material.Thickness;
    vec3  subSurfaceScatteringColor = (material.SubSurfaceScatteringColorMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.SubSurfaceScatteringColorMap], UV, 0.0f).rgb               : material.SubSurfaceScatteringColor;
    vec3  sheenColor                = (material.SheenMap                     != 0xFFFFFFFFu) ? textureLod(TextureMap[material.SheenMap],                     UV, 0.0f).rgb               : material.SheenColor;
    float clearcoatTint             = (material.MetallicMap                  != 0xFFFFFFFFu) ? textureLod(TextureMap[material.ClearCoatMap],                 UV, 0.0f).r                 : material.ClearcoatTint;
    float ambientOcclusion          = (material.AmbientOcclusionMap          != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AmbientOcclusionMap],          UV, 0.0f).r                 : material.AmbientOcclusion;
    vec3  normalMap                 = (material.NormalMap                    != 0xFFFFFFFFu) ? textureLod(TextureMap[material.NormalMap],                    UV, 0.0f).xyz               : vec3(0.0f, 0.0f, 1.0f);
    vec3  emission                  = (material.EmissionMap                  != 0xFFFFFFFFu) ? textureLod(TextureMap[material.EmissionMap],                  UV, 0.0f).rgb               : vec3(0.0f);
    float height                    = (material.HeightMap                    != 0xFFFFFFFFu) ? textureLod(TextureMap[material.HeightMap],                    UV, 0.0f).r                 : material.Height;
    //albedo.a                        = (material.AlphaMap                     != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlphaMap],                     UV, 0.0f).a                 : material.Alpha;

    vec2 encodedNormal = OctahedronEncode(normalMap);

    outAlbedo = albedo;
    outNormalData = vec4((encodedNormal * 0.5f) + 0.5f, material.NormalStrength, height);
    outPackedMRO = vec4(Pack8bitPair(metallic, roughness), Pack8bitPair(ambientOcclusion, clearcoatTint), Pack8bitPair(material.ClearcoatStrength, material.ClearcoatRoughness), 1.0);
    outPackedSheenSSS = vec4(Pack8bitPair(sheenColor.r, sheenColor.g), Pack8bitPair(sheenColor.b, material.SheenIntensity), Pack8bitPair(subSurfaceScatteringColor.r, subSurfaceScatteringColor.g), Pack8bitPair(subSurfaceScatteringColor.b, thickness));
    outEmission = vec4(emission, material.ClearcoatRoughness);
}