#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

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

layout(location = 1) out vec4 outAlbedo;        //R8G8B8A8_SRGB
layout(location = 2) out vec4 outNormalData;    //R16G16B16A16_UNORM 
layout(location = 3) out vec4 outMRO;           //R16G16B16A16_UNORM
layout(location = 4) out vec4 outCoat;           //R16G16B16A16_UNORM
layout(location = 5) out vec4 outFeatureA;      //R16G16B16A16_UNORM
layout(location = 6) out vec4 outFeatureB;      //R16G16B16A16_UNORM
//layout(location = 7) out vec4 outFeatureC;      //R16G16B16A16_UNORM
layout(location = 7) out vec4 outEmission;      //R16G16B16A16_SFLOAT

layout(binding = 0) buffer BindlessBuffer
{
    uint64_t MaterialOffset;
    mat.   MaterialCount;
    mat.   MaterialSize;
    uint64_t Texture2DOffset;
    mat.   Texture2DCount;
    mat.   Texture2DSize;
    //uint64_t Texture3DOffset;
    //mat.   Texture3DCount;
    //mat.   Texture3DSize;
    //uint64_t TextureCubeMapOffset;
    //mat.   TextureCubeMapCount;
    //mat.   TextureCubeMapSize;

    uint Data[];    // flattened uint buffer
} bindlessBuffer;

layout(binding = 1) uniform sampler2D TextureMap[];   

vec2 OctahedronEncode(vec3 normal)
{
    vec2 f = normal.xy / (abs(normal.x) + abs(normal.y) + abs(normal.z));
    return (normal.z < 0.0) ? (1.0 - abs(f.yx)) * sign(f) : f;
}

ImportMaterial GetImportMaterial()
{
    // Material starts at byte offset MaterialOffset → uint index = MaterialOffset / 4
    uint offset = 0;

    ImportMaterial mat;

    mat.Albedo.r                        = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Albedo.g                        = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Albedo.b                        = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    mat.CoatCoatColor.r                 = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.CoatCoatColor.g                 = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.CoatCoatColor.b                 = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    mat.SheenColor.r                    = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.SheenColor.g                    = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.SheenColor.b                    = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    mat.SSSColor.r                      = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.SSSColor.g                      = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.SSSColor.b                      = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    mat.AttenuationColor.r              = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.AttenuationColor.g              = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.AttenuationColor.b              = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    mat.Emission.r                      = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Emission.g                      = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Emission.b                      = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    mat.Metallic                        = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Roughness                       = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.AmbientOcclusion                = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Specular                        = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.IOR                             = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.SelfShadow                      = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.CoatWeight                      = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.CoatRoughness                   = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.CoatDarkening                   = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.SheenWeight                     = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.SheenRoughness                  = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.SSSWeight                       = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Thickness                       = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.SSSProfile                      = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.TransmissionWeight              = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.AttenuationDistance             = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Anisotropy                      = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.AnisotropyRotation              = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.ThinFilmWeight                  = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.ThinFilmThickness               = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    mat.AlbedoMap                       = bindlessBuffer.Data[offset++];
    mat.MetallicMap                     = bindlessBuffer.Data[offset++];
    mat.RoughnessMap                    = bindlessBuffer.Data[offset++];
    mat.ThicknessMap                    = bindlessBuffer.Data[offset++];
    mat.SSSColorMap                     = bindlessBuffer.Data[offset++];
    mat.SheenMap                        = bindlessBuffer.Data[offset++];
    mat.ClearCoatMap                    = bindlessBuffer.Data[offset++];
    mat.AnisotropyMap                   = bindlessBuffer.Data[offset++];
    mat.AmbientOcclusionMap             = bindlessBuffer.Data[offset++];
    mat.NormalMap                       = bindlessBuffer.Data[offset++];
    mat.AlphaMap                        = bindlessBuffer.Data[offset++];
    mat.EmissionMap                     = bindlessBuffer.Data[offset++];
    mat.HeightMap                       = bindlessBuffer.Data[offset++];
    mat.ShadingModel                    = bindlessBuffer.Data[offset++];
    mat.FeatureMask                     = bindlessBuffer.Data[offset++];

    return mat;
}

void main()
{
    ImportMaterial material = GetImportMaterial();
    vec4 albedo =            (material.AlbedoMap != 0xFFFFFFFFu)                    ? textureLod(TextureMap[nonuniformEXT(material.AlbedoMap)], UV, 0.0)                        : vec4(material.Albedo, 1.0);
    vec3 normalMapRaw =      (material.NormalMap != 0xFFFFFFFFu)                    ? textureLod(TextureMap[nonuniformEXT(material.NormalMap)], UV, 0.0).rgb                    : vec3(0.5, 0.5, 1.0);
    vec3 clearcoatTint =     (material.ClearCoatMap != 0xFFFFFFFFu)                 ? textureLod(TextureMap[nonuniformEXT(material.ClearCoatMap)], UV, 0.0).r                   : material.ClearcoatTint;
    vec3 sheenColor =        (material.SheenMap != 0xFFFFFFFFu)                     ? textureLod(TextureMap[nonuniformEXT(material.SheenMap)], UV, 0.0).rgb                     : material.SheenColor;
    vec3 sssColor =          (material.SubSurfaceScatteringColorMap != 0xFFFFFFFFu) ? textureLod(TextureMap[nonuniformEXT(material.SubSurfaceScatteringColorMap)], UV, 0.0).rgb : material.SubSurfaceScatteringColor;
    vec3 AttenuationColor =  (material.AttenuationColorMap != 0xFFFFFFFFu)          ? textureLod(TextureMap[nonuniformEXT(material.AttenuationColorMap)], UV, 0.0).rgb          : material.AttenuationColor;
    vec3 emission =          (material.EmissionMap != 0xFFFFFFFFu)                  ? textureLod(TextureMap[nonuniformEXT(material.EmissionMap)], UV, 0.0).rgb                  : material.Emission;  
    
    float metallic =         (material.MetallicMap != 0xFFFFFFFFu)                  ? textureLod(TextureMap[nonuniformEXT(material.MetallicMap)], UV, 0.0).r                    : material.Metallic;
    float roughness =        (material.RoughnessMap != 0xFFFFFFFFu)                 ? textureLod(TextureMap[nonuniformEXT(material.RoughnessMap)], UV, 0.0).r                   : material.Roughness;
    float ambientOcclusion = (material.AmbientOcclusionMap != 0xFFFFFFFFu)          ? textureLod(TextureMap[nonuniformEXT(material.AmbientOcclusionMap)], UV, 0.0).r            : material.AmbientOcclusion;
   
    float thickness =        (material.ThicknessMap != 0xFFFFFFFFu)                 ? textureLod(TextureMap[nonuniformEXT(material.ThicknessMap)], UV, 0.0).r                   : material.Thickness;
    float height =           (material.HeightMap != 0xFFFFFFFFu)                    ? textureLod(TextureMap[nonuniformEXT(material.HeightMap)], UV, 0.0).r                      : material.Height;

    vec3 tangentNormal = normalMapRaw * 2.0 - 1.0;
    tangentNormal = normalize(tangentNormal);
    vec2 encodedNormal = OctahedronEncode(tangentNormal);

    if (CoatWeight > kFeatureEps || CoatMap != ~0u)    mask |= FEAT_COAT;
    if (SheenWeight > kFeatureEps || SheenMap != ~0u)  mask |= FEAT_SHEEN;
    if (SSSWeight > kFeatureEps || SSSColorMap != ~0u) mask |= FEAT_SSS;
    if (TransmissionWeight > kFeatureEps)              mask |= FEAT_TRANSMISSION;
    if (Anisotropy > kFeatureEps)                      mask |= FEAT_ANISO;
    if (ThinFilmWeight > kFeatureEps)                  mask |= FEAT_FILM;

    outAlbedo     = vec4(albedo.rgb, albedo.a);
    outNormalData = vec4(encodedNormal * 0.5 + 0.5, material.NormalStrength, height);
    outMRO        = vec4(metallic, roughness, ambientOcclusion, CoatRoughness);
    outCoat       = vec4(ClearcoatTint, CoatWeight);
    if ((mask & FEAT_SSS) != 0u)               outFeatureA   = vec4(sssColor, CoatDarkening);
    else if ((mask & FEAT_TRANSMISSION) != 0u) outFeatureA   = vec4(material.TransmissionWeight, material.AttenuationDistance, material.AttenuationDistance, CoatDarkening);
    else                                       outFeatureA   = vec4(0.0, 0.0f, 0.0f, CoatDarkening);
    outFeatureB = vec4(material.Anisotropy, material.AnisotropyRotation, 0.0f, 1.0f);
    //outFeatureC = vec4(material.ThinFilmWeight, material.ThinFilmThickness, 0.0f, 1.0f);  
  
    outEmission   = vec4(emission, material.EmissionIntensity);
}
