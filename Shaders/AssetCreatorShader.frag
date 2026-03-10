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

layout(location = 0) out vec4 outAlbedo;             //Albedo/Alpha                                                                               - R8G8B8A8_SRGB
layout(location = 1) out vec4 outNormalData;         //Normal/Height/unused                                                                       - R16G16B16A16_UNORM
layout(location = 2) out vec4 outPackedMRO;          //vec4(Metallic/Rough, AO/ClearcoatTint, ClearcoatStrength/ClearcoatRoughness, unused)       - R16G16B16A16_UNORM
layout(location = 3) out vec4 outPackedSheenSSS;     //vec4(sheenColor.r/sheenColor.g, sheenColor.b/sheenIntensity, sss.r/sss.g, sss.b/thickness) - R16G16B16A16_UNORM
layout(location = 4) out vec4 outUnused;             //unused for now                                                                             - R16G16B16A16_UNORM
layout(location = 5) out vec4 outEmission;           //Emission                                                                                   - R16G16B16A16_SFLOAT

layout(binding = 0)  buffer BindlessBuffer 
{ 
    uint64_t MaterialOffset;
    uint     MaterialCount;
    uint     MaterialSize;
    uint64_t Texture2DOffset;
    uint     Texture2DCount;
    uint     Texture2DSize;
//    uint64_t Texture3DOffset;
//    uint     Texture3DCount;
//    uint     Texture3DSize;
//    uint64_t TextureCubeMapOffset;
//    uint     TextureCubeMapCount;
//    uint     TextureCubeMapSize;
    uint     Data[]; 
} bindlessBuffer;
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

ImportMaterial GetImportMaterial()
{
    uint offset = uint(bindlessBuffer.MaterialOffset) / 4u;   // 128 / 4 = 32

    ImportMaterial mat;
    mat.Albedo                        = vec3(uintBitsToFloat(bindlessBuffer.Data[offset++]),
                                             uintBitsToFloat(bindlessBuffer.Data[offset++]),
                                             uintBitsToFloat(bindlessBuffer.Data[offset++]));
    mat.SheenColor                    = vec3(uintBitsToFloat(bindlessBuffer.Data[offset++]),
                                             uintBitsToFloat(bindlessBuffer.Data[offset++]),
                                             uintBitsToFloat(bindlessBuffer.Data[offset++]));
    mat.SubSurfaceScatteringColor     = vec3(uintBitsToFloat(bindlessBuffer.Data[offset++]),
                                             uintBitsToFloat(bindlessBuffer.Data[offset++]),
                                             uintBitsToFloat(bindlessBuffer.Data[offset++]));
    mat.Emission                      = vec3(uintBitsToFloat(bindlessBuffer.Data[offset++]),
                                             uintBitsToFloat(bindlessBuffer.Data[offset++]),
                                             uintBitsToFloat(bindlessBuffer.Data[offset++]));
    mat.ClearcoatTint                 = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Metallic                      = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Roughness                     = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.AmbientOcclusion              = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.ClearcoatStrength             = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.ClearcoatRoughness            = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.SheenIntensity                = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Thickness                     = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Anisotropy                    = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.AnisotropyRotation            = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.NormalStrength                = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.HeightScale                   = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Height                        = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    mat.Alpha                         = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    mat.AlbedoMap                     = bindlessBuffer.Data[offset++];
    mat.MetallicMap                   = bindlessBuffer.Data[offset++];
    mat.RoughnessMap                  = bindlessBuffer.Data[offset++];
    mat.ThicknessMap                  = bindlessBuffer.Data[offset++];
    mat.SubSurfaceScatteringColorMap  = bindlessBuffer.Data[offset++];
    mat.SheenMap                      = bindlessBuffer.Data[offset++];
    mat.ClearCoatMap                  = bindlessBuffer.Data[offset++];
    mat.AnisotropyMap                 = bindlessBuffer.Data[offset++];
    mat.AmbientOcclusionMap           = bindlessBuffer.Data[offset++];
    mat.NormalMap                     = bindlessBuffer.Data[offset++];
    mat.AlphaMap                      = bindlessBuffer.Data[offset++];
    mat.EmissionMap                   = bindlessBuffer.Data[offset++];
    mat.HeightMap                     = bindlessBuffer.Data[offset++];

    return mat;
}

void main()
{
    ImportMaterial material = GetImportMaterial();

    vec3 a = vec3(0.0f);
    for(int x = 0; x < 20; x++)
    {
 a =   textureLod(TextureMap[x], UV, 0.0f).rgb;
    }

    vec4  albedo                    = (material.AlbedoMap                    != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlbedoMap], UV, 0.0f)                        : vec4(material.Albedo, 1.0f);
    float metallic                  = (material.MetallicMap                  != 0xFFFFFFFFu) ? textureLod(TextureMap[material.MetallicMap], UV, 0.0f).r                    : material.Metallic;
    float roughness                 = (material.RoughnessMap                 != 0xFFFFFFFFu) ? textureLod(TextureMap[material.RoughnessMap], UV, 0.0f).r                   : material.Roughness;
    vec3  normalMapRaw              = (material.NormalMap                    != 0xFFFFFFFFu) ? textureLod(TextureMap[material.NormalMap], UV, 0.0f).rgb                    : vec3(0.5f, 0.5f, 1.0f);
    float thickness                 = (material.ThicknessMap                 != 0xFFFFFFFFu) ? textureLod(TextureMap[material.ThicknessMap], UV, 0.0f).r                   : material.Thickness;
    vec3  subSurfaceScatteringColor = (material.SubSurfaceScatteringColorMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.SubSurfaceScatteringColorMap], UV, 0.0f).rgb : material.SubSurfaceScatteringColor;
    vec3  sheenColor                = (material.SheenMap                     != 0xFFFFFFFFu) ? textureLod(TextureMap[material.SheenMap], UV, 0.0f).rgb                     : material.SheenColor;
    float clearcoatTint             = (material.ClearCoatMap                 != 0xFFFFFFFFu) ? textureLod(TextureMap[material.ClearCoatMap], UV, 0.0f).r                   : material.ClearcoatTint;
    float ambientOcclusion          = (material.AmbientOcclusionMap          != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AmbientOcclusionMap], UV, 0.0f).r            : material.AmbientOcclusion;
    vec3  emission                  = (material.EmissionMap                  != 0xFFFFFFFFu) ? textureLod(TextureMap[material.EmissionMap], UV, 0.0f).rgb                  : vec3(1.0f);
    float height                    = (material.HeightMap                    != 0xFFFFFFFFu) ? textureLod(TextureMap[material.HeightMap], UV, 0.0f).r                      : material.Height;

    vec3 tangentNormal = normalMapRaw * 2.0 - 1.0;
    tangentNormal = normalize(tangentNormal);
    vec2 encodedNormal = OctahedronEncode(tangentNormal);

    outAlbedo = vec4(UV, 0.0f, 1.0f);
    outNormalData = vec4(encodedNormal * 0.5 + 0.5, material.NormalStrength, height);
    outPackedMRO = vec4(Pack8bitPair(metallic, roughness), Pack8bitPair(ambientOcclusion, clearcoatTint), Pack8bitPair(material.ClearcoatStrength, material.ClearcoatRoughness), 1.0);
    outPackedSheenSSS = vec4(Pack8bitPair(sheenColor.r, sheenColor.g), Pack8bitPair(sheenColor.b, 1.00f), Pack8bitPair(subSurfaceScatteringColor.r, subSurfaceScatteringColor.g), Pack8bitPair(subSurfaceScatteringColor.b, thickness));
    outUnused = vec4(Pack8bitPair(sheenColor.r, sheenColor.g), 0.0f, 0.0f, 1.0f);
    outEmission = vec4(emission, 1.0f); 
}

