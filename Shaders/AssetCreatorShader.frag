#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Constants.glsl"
#include "MaterialPropertiesBuffer.glsl"

const uint  NO_MAP      = 0xFFFFFFFFu;
const float kFeatureEps = 1e-3;

layout(location = 0) in vec2 UV;

layout(location = 1) out vec4 outAlbedo;
layout(location = 2) out vec4 outNormalData;
layout(location = 3) out vec4 outMRO;
layout(location = 4) out vec4 outCoat;
layout(location = 5) out vec4 outFeatureA;
layout(location = 6) out vec4 outFeatureB;
layout(location = 7) out vec4 outEmission;
// layout(location = 8) out vec4 outFeatureC;

layout(binding = 0) buffer BindlessBuffer
{
    uint64_t MaterialOffset;
    uint     MaterialCount;
    uint     MaterialSize;
    uint64_t Texture2DOffset;
    uint     Texture2DCount;
    uint     Texture2DSize;
    uint     Data[];
} bindlessBuffer;

layout(binding = 1) uniform sampler2D TextureMap[];

vec2 OctahedronEncode(vec3 normal)
{
    vec2 f = normal.xy / (abs(normal.x) + abs(normal.y) + abs(normal.z));
    return (normal.z < 0.0) ? (1.0 - abs(f.yx)) * sign(f) : f;
}

vec4 SampleOr(uint id, vec4 fallback)
{
    if (id == NO_MAP)
        return fallback;
    return textureLod(TextureMap[nonuniformEXT(id)], UV, 0.0);
}

ImportMaterial GetImportMaterial()
{
    uint offset = uint(bindlessBuffer.MaterialOffset / 4u);
    ImportMaterial m;

    m.Albedo.r = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.Albedo.g = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.Albedo.b = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    m.CoatColor.r = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.CoatColor.g = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.CoatColor.b = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    m.SheenColor.r = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.SheenColor.g = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.SheenColor.b = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    m.SSSColor.r = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.SSSColor.g = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.SSSColor.b = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    m.AttenuationColor.r = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.AttenuationColor.g = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.AttenuationColor.b = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    m.Emission.r = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.Emission.g = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.Emission.b = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    m.Metallic         = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.Roughness        = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.AmbientOcclusion = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.IOR              = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.Alpha            = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.NormalStrength   = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.Height           = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    m.CoatWeight    = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.CoatRoughness = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.CoatDarkening = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    m.SheenWeight    = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.SheenRoughness = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    m.SSSWeight  = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.SSSProfile = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.Thickness  = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    m.TransmissionWeight  = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.AttenuationDistance = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    m.Anisotropy         = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.AnisotropyRotation = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.ThinFilmWeight     = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.ThinFilmThickness  = uintBitsToFloat(bindlessBuffer.Data[offset++]);
    m.EmissionIntensity  = uintBitsToFloat(bindlessBuffer.Data[offset++]);

    m.AlbedoMap                = bindlessBuffer.Data[offset++];
    m.NormalMap                = bindlessBuffer.Data[offset++];
    m.HeightMap                = bindlessBuffer.Data[offset++];
    m.AlphaMap                 = bindlessBuffer.Data[offset++];
    m.MetallicMap              = bindlessBuffer.Data[offset++];
    m.RoughnessMap             = bindlessBuffer.Data[offset++];
    m.AmbientOcclusionMap      = bindlessBuffer.Data[offset++];
    m.EmissionMap              = bindlessBuffer.Data[offset++];
    m.ClearCoatColorMap        = bindlessBuffer.Data[offset++];
    m.ClearCoatPropertiesMap   = bindlessBuffer.Data[offset++];
    m.SheenMap                 = bindlessBuffer.Data[offset++];
    m.SheenPropertiesMap       = bindlessBuffer.Data[offset++];
    m.SSSColorMap              = bindlessBuffer.Data[offset++];
    m.SSSPropertiesMap         = bindlessBuffer.Data[offset++];
    m.AttenuationColorMap      = bindlessBuffer.Data[offset++];
    m.AnisotropyMap            = bindlessBuffer.Data[offset++];

    m.ShadingModel = bindlessBuffer.Data[offset++];
    m.FeatureMask  = bindlessBuffer.Data[offset++];

    return m;
}

ImportMaterial MapToMaterial()
{
    ImportMaterial m   = GetImportMaterial();

    vec4 albedoSamp    = SampleOr(m.AlbedoMap, vec4(m.Albedo, m.Alpha));
    m.Albedo           = albedoSamp.rgb;
    m.Alpha            = SampleOr(m.AlphaMap, vec4(albedoSamp.a)).r;

    vec3 nrm           = SampleOr(m.NormalMap, vec4(0.5f, 0.5f, 1.0f, 1.0f)).rgb;
    m.NormalTS         = normalize(nrm * 2.0f - 1.0f);

    m.Height           = SampleOr(m.HeightMap,           vec4(m.Height)).r;
    m.Metallic         = SampleOr(m.MetallicMap,         vec4(m.Metallic)).r;
    m.Roughness        = SampleOr(m.RoughnessMap,        vec4(m.Roughness)).r;
    m.AmbientOcclusion = SampleOr(m.AmbientOcclusionMap, vec4(m.AmbientOcclusion)).r;

    m.CoatColor        = SampleOr(m.ClearCoatColorMap,   vec4(m.CoatColor, 1.0)).rgb;
    m.SheenColor       = SampleOr(m.SheenMap,            vec4(m.SheenColor, 1.0)).rgb;
    m.SSSColor         = SampleOr(m.SSSColorMap,         vec4(m.SSSColor, 1.0)).rgb;
    m.AttenuationColor = SampleOr(m.AttenuationColorMap, vec4(m.AttenuationColor, 1.0)).rgb;
    m.Emission         = SampleOr(m.EmissionMap,         vec4(m.Emission, 1.0)).rgb;

    vec4 coatProp      = SampleOr(m.ClearCoatPropertiesMap, vec4(m.CoatWeight, m.CoatRoughness, m.CoatDarkening, 0.0f));
    m.CoatWeight       = coatProp.r;
    m.CoatRoughness    = coatProp.g;
    m.CoatDarkening    = coatProp.b;

    vec4 sheenProp     = SampleOr(m.SheenPropertiesMap, vec4(m.SheenWeight, m.SheenRoughness, 0.0f, 0.0f));
    m.SheenWeight      = sheenProp.r;   
    m.SheenRoughness   = sheenProp.g;

    vec4 sssProp       = SampleOr(m.SSSPropertiesMap, vec4(m.SSSWeight, m.SSSProfile, m.Thickness, 0.0f));
    m.SSSWeight        = sssProp.r;
    m.SSSProfile       = sssProp.g;
    m.Thickness        = sssProp.b;

    vec4 anisotropyProperties SampleOr(m.AnisotropyPropertiesMap, vec4(m.Anisotropy, m.AnisotropyRotation, m.ThinFilmWeight, m.ThinFilmThickness));
    m.Anisotropy            = anisotropyProperties.r;
    m.AnisotropyRotation    = anisotropyProperties.g;
    m.ThinFilmWeight        = anisotropyProperties.b;
    m.ThinFilmThickness     = anisotropyProperties.a;

    m.IORNorm          = clamp((m.IOR - 1.0f) / 2.0f, 0.0f, 1.0f);


//    material.Specular;
//    material.TransmissionWeight;
//    material.AttenuationDistance;
//    material.EmissionIntensity;

    return m;
}

void main()
{
    ImportMaterial m = MapToMaterial();

    uint mask = m.FeatureMask;
    if (m.CoatWeight         > kFeatureEps || m.ClearCoatPropertiesMap != NO_MAP) mask |= FEAT_COAT;
    if (m.SheenWeight        > kFeatureEps || m.SheenMap != NO_MAP)               mask |= FEAT_SHEEN;
    if (m.SSSWeight          > kFeatureEps || m.SSSColorMap != NO_MAP)            mask |= FEAT_SSS;
    if (m.TransmissionWeight > kFeatureEps)                                       mask |= FEAT_TRANSMISSION;
    if (m.Anisotropy         > kFeatureEps || m.AnisotropyMap != NO_MAP)          mask |= FEAT_ANISO;
    if (m.ThinFilmWeight     > kFeatureEps)                                       mask |= FEAT_FILM;
    if ((mask & FEAT_SSS) != 0u) mask &= ~FEAT_TRANSMISSION;

    vec2 encN = OctahedronEncode(m.NormalTS);

    outAlbedo     = vec4(m.Albedo, m.Alpha);
    outNormalData = vec4(encN * 0.5f + 0.5f, m.NormalStrength, m.Height);
    outMRO        = vec4(m.Metallic, m.Roughness, m.AmbientOcclusion, m.IORNorm);
    outCoat       = vec4(m.CoatWeight, m.CoatRoughness, m.CoatDarkening, 0.0);

    if ((mask & FEAT_SSS) != 0u) outFeatureA = vec4(m.SSSColor, m.Thickness);
    else if ((mask & FEAT_TRANSMISSION) != 0u) outFeatureA = vec4(m.TransmissionWeight, m.Thickness, m.AttenuationDistance, 0.0f);
    else outFeatureA = vec4(0.0);

    if ((mask & FEAT_SHEEN) != 0u) outFeatureB = vec4(m.SheenColor, m.SheenWeight);
    else outFeatureB = vec4(0.0f);

    outEmission = vec4(m.Emission, m.EmissionIntensity);

    // outFeatureC = vec4(m.Anisotropy, m.AnisotropyRotation, m.ThinFilmWeight, m.ThinFilmThickness);
}