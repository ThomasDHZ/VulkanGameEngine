#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"

layout(std430, binding = 0) buffer SceneDataBuffer
{
    uint HDRMapInputIndex;
    uint EnvironmentMapIndex;
    uint BRDFMapId;
    uint CubeMapId;
    uint IrradianceMapId;
    uint PrefilterMapId;
    uint _padIds0;
    uint _padIds1;

    mat4 OrthoProjection;
    mat4 OrthoView;
    mat4 InverseOrthoProjection;
    mat4 InverseOrthoView;
    mat4 InversePerspectiveProjection;
    mat4 InversePerspectiveView;

    vec3  PerspectiveCameraPosition;
    float Time;
    vec3  PerspectiveViewDirection;
    uint  FrameIndex;
    vec2  InvertResolution;
    vec2  _padEnd;
} sceneDataBuffer;

layout(binding = 1) buffer BindlessBuffer
{
    uint64_t MeshOffset;
    uint MeshCount;
    uint MeshSize;
    uint64_t MaterialOffset;
    uint MaterialCount;
    uint MaterialSize;
    uint64_t DirectionalLightOffset;
    uint DirectionalLightCount;
    uint DirectionalLightSize;
    uint64_t PointLightOffset;
    uint PointLightCount;
    uint PointLightSize;
    uint64_t Texture2DOffset;
    uint Texture2DCount;
    uint Texture2DSize;
    uint64_t Texture3DOffset;
    uint Texture3DCount;
    uint Texture3DSize;
    uint64_t TextureCubeMapOffset;
    uint TextureCubeMapCount;
    uint TextureCubeMapSize;
    uint64_t SpriteInstanceOffset;
    uint SpriteInstanceCount;
    uint SpriteInstanceSize;
    uint Data[];
} bindlessBuffer;

layout(binding = 2) uniform samplerCube CubeMap[];
layout(binding = 3) uniform sampler2D TextureMap[];
layout(binding = 4) uniform sampler3D Texture3DMap[];

layout(location = 0)  in vec3       WorldPos;
layout(location = 1)  in vec2       PS_UV;
layout(location = 2)  in vec2       PS_SpriteSize;
layout(location = 3)  in flat ivec2 PS_FlipSprite;
layout(location = 4)  in vec4       PS_Color;
layout(location = 5)  in flat uint  PS_MaterialId;
layout(location = 6)  in flat vec4  PS_UVOffset;
layout(location = 7)  in flat uint  PS_SpriteId;
layout(location = 8)  in vec3       PS_T;
layout(location = 9)  in vec3       PS_B;
layout(location = 10) in vec3       PS_N;

layout(location = 0) out vec4 outAlphaColor;
layout(location = 1) out vec4 outAlphaBloom;

#include "BindlessHelpers.glsl"

const float IBL_EXPOSURE = 2.5;
const float kFeatureEps  = 1e-3;
const uint FEAT_COAT         = 1u << 0;
const uint FEAT_SHEEN        = 1u << 1;
const uint FEAT_SSS          = 1u << 2;
const uint FEAT_TRANSMISSION = 1u << 3;
const uint FEAT_ANISO        = 1u << 4;
const uint FEAT_FILM         = 1u << 5;
const uint FEAT_TWO_SIDED    = 1u << 6;
const uint FEAT_COAT_NORMAL  = 1u << 7;
const uint FEAT_ALBEDO_ONLY  = 1u << 8;

vec3 OctahedronDecode(vec2 f)
{
    vec3 n;
    n.xy = f.xy;
    n.z  = 1.0 - abs(f.x) - abs(f.y);
    n.xy = (n.z < 0.0) ? (1.0 - abs(n.yx)) * sign(n.xy) : n.xy;
    return normalize(n);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a     = roughness * roughness;
    float a2    = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec2 Unpack8bitPair(float packed)
{
    uint combined = uint(packed * 65535.0 + 0.5);
    return vec2(float((combined >> 8) & 0xFFu) / 255.0,
                float(combined & 0xFFu) / 255.0);
}

void AnisoFrame(vec3 N, float rotation, out vec3 T, out vec3 B)
{
    vec3 up = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    T = normalize(cross(up, N));
    B = cross(N, T);
    float a = rotation * 6.2831853;
    float c = cos(a), s = sin(a);
    vec3 T2 = normalize(c * T + s * B);
    B = cross(N, T2);
    T = T2;
}

float DistributionGGX_Aniso(vec3 N, vec3 T, vec3 B, vec3 H, float rough, float aniso)
{
    float at = max(rough * (1.0 + aniso), 0.001);
    float ab = max(rough * (1.0 - aniso), 0.001);
    float ToH = dot(T, H);
    float BoH = dot(B, H);
    float NoH = max(dot(N, H), 0.0);
    float d = ToH * ToH / (at * at) + BoH * BoH / (ab * ab) + NoH * NoH;
    return 1.0 / max(3.14159265 * at * ab * d * d, 1e-6);
}

float CoatFresnel(vec3 N, vec3 V, float weight)
{
    return weight * fresnelSchlickRoughness(max(dot(N, V), 0.0), vec3(0.04), 0.0).x;
}

Material UnpackMaterial();
vec3 DirectionalLightFunc(vec3 F0, vec3 V, Material material);
vec3 PointLightFunc(vec3 F0, vec3 V, Material material);
vec3 ImageBasedLighting(vec3 F0, vec3 V, vec3 N_ibl, vec3 R, Material material);
vec3 SheenData(Material material, vec3 N, vec3 V);
vec3 SubSurfaceScatteringData(Material material, vec3 N, vec3 L);
vec3 ThinFilm(vec3 spec, Material m, float HdotV);
vec3 ClearCoat(Material m, vec3 N, vec3 V, vec3 L, vec3 H, vec3 radiance, float NdotL);

void main()
{    
    PackedMaterial material = GetMaterial(PS_MaterialId);

    vec2 minUV = PS_UVOffset.xy;
    vec2 maxUV = PS_UVOffset.xy + PS_UVOffset.zw;

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) UV.x = minUV.x + maxUV.x - UV.x;
    if (PS_FlipSprite.y == 1) UV.y = minUV.y + maxUV.y - UV.y;


    float alpha = texture(TextureMap[material.AlbedoTextureId], UV, -0.5).a;

    if (alpha >= material.AlphaCutOff || alpha < 0.01f) discard;

    outAlphaColor = vec4(1.0f, 0.0f, 0.0f, 0.5f);
    outAlphaBloom = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}

Material UnpackMaterial()
{
//    vec4 positionAttachment = subpassLoad(PositionInput);
//    vec4 albedoAttachment   = subpassLoad(AlbedoInput);
//    vec4 normalAttachment   = subpassLoad(NormalDataInput);
//    vec4 mroAttachment      = subpassLoad(MROInput);
//    vec4 featureAAttachment = subpassLoad(FeatureAInput);
//    vec4 featureBAttachment = subpassLoad(FeatureBInput);
//    vec4 featureCAttachment = subpassLoad(FeatureCInput);
//    vec4 emissionAttachment = subpassLoad(EmissionInput);
//    vec4 depthAttachment    = subpassLoad(depthInput);
//
//    vec2 sssW_coatW    = Unpack8bitPair(featureAAttachment.a);
//    vec2 thick_coatR   = Unpack8bitPair(featureBAttachment.a);
//    vec2 aniso         = Unpack8bitPair(featureCAttachment.r);
//    vec2 film          = Unpack8bitPair(featureCAttachment.g);
//    vec2 coatD_prof    = Unpack8bitPair(featureCAttachment.b);
//    vec2 sheenR_sheenW = Unpack8bitPair(featureCAttachment.a);
//
    Material m;
//    m.Depth            = depthAttachment.r;
//    m.Position         = positionAttachment.xyz;
//    m.Albedo           = albedoAttachment.rgb;
//    m.Metallic         = mroAttachment.r;
//    m.Roughness        = mroAttachment.g;
//    m.AmbientOcclusion = mroAttachment.b;
//    m.IOR              = mroAttachment.a * 2.0 + 1.0;
//    m.Normal           = normalize(OctahedronDecode(normalAttachment.xy * 2.0 - 1.0));
//    m.Emission         = emissionAttachment.rgb;
//
//    m.SheenColor     = featureAAttachment.rgb;
//    m.SheenRoughness = sheenR_sheenW.x;
//    m.SheenWeight    = sheenR_sheenW.y;
//    m.SSSColor       = featureBAttachment.rgb;
//    m.SSSWeight      = sssW_coatW.x;
//    m.SSSProfile     = coatD_prof.y;
//    m.Thickness      = thick_coatR.x;
//    m.CoatColor      = vec3(1.0);
//    m.CoatWeight     = sssW_coatW.y;
//    m.CoatRoughness  = thick_coatR.y;
//    m.CoatDarkening  = coatD_prof.x;
//    m.Anisotropy         = aniso.x;
//    m.AnisotropyRotation = aniso.y;
//    m.ThinFilmWeight     = film.x;
//    m.ThinFilmThickness  = film.y;
//    m.FeatureMask = uint(normalAttachment.b * 65535.0 + 0.5);
//    m.SelfShadow  = normalAttachment.a;
//    m.ShadingModel = 0u;
    return m;
}

vec3 DirectionalLightFunc(vec3 F0, vec3 V, Material material)
{
    vec3 Lo = vec3(0.0);
    for (uint x = 0; x < bindlessBuffer.DirectionalLightCount; ++x)
    {
        const DirectionalLightBuffer light = GetDirectionalLight(x);
        if(light.LightActive != 1u) continue;

        vec3 L = normalize(-light.LightDirection);
        vec3 H = normalize(V + L);
        vec3 N = material.Normal;
        vec3 T, B;

        vec3 radiance = light.LightColor * light.LightIntensity * material.SelfShadow;
        float NdotL   = max(dot(N, L), 0.0);

        Lo += SubSurfaceScatteringData(material, N, L) * radiance;
        float back = max(-dot(N, L), 0.0);
        Lo += material.Albedo * material.SSSColor * material.SSSWeight * back * material.Thickness * radiance *exp(-1.0 / max(material.Thickness, 0.05));

        if (NdotL <= 0.0) continue;

        AnisoFrame(N, material.AnisotropyRotation, T, B);
        float NDF = DistributionGGX_Aniso(N, T, B, H, material.Roughness, material.Anisotropy);
        float G   = GeometrySmith(N, V, L, material.Roughness);
        vec3  F   = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, material.Roughness);
        vec3  kD  = (vec3(1.0) - F) * (1.0 - material.Metallic);
        float NdotV = max(dot(N, V), 0.0);
        vec3  spec  = (NDF * G * F) / max(4.0 * NdotV * NdotL, 1e-4);
        spec = ThinFilm(spec, material, max(dot(H, V), 0.0));

        vec3 base = (kD * material.Albedo / PI + spec) * radiance * NdotL;
        if (material.CoatWeight > 0.0)
        {
            float coatF = CoatFresnel(N, V, material.CoatWeight);
            base *= mix(1.0, material.CoatDarkening, material.CoatWeight);
            base *= (1.0 - coatF);
        }
        base += SheenData(material, N, V) * radiance * NdotL;

        Lo += base;
        Lo += ClearCoat(material, N, V, L, H, radiance, NdotL);
    }
    return Lo;
}

vec3 PointLightFunc(vec3 F0, vec3 V, Material material)
{
    vec3 Lo = vec3(0.0);
    for (uint x = 0; x < bindlessBuffer.PointLightCount; ++x)
    {
        const PointLightBuffer light = GetPointLight(x);
        if(light.LightActive != 1u) continue;

        vec3  toLight  = light.LightPosition - material.Position;
        float distance = length(toLight);
        if (distance > light.LightRadius) continue;

        vec3 L = toLight / max(distance, 1e-4);
        vec3 H = normalize(V + L);
        vec3 N = material.Normal;
        vec3 T, B;

        float atten = 1.0 - clamp(distance / light.LightRadius, 0.0, 1.0);
        atten *= atten;
        vec3 radiance = vec3(1.0f) * light.LightIntensity * atten * material.SelfShadow;
        float NdotL   = max(dot(N, L), 0.0);

        Lo += SubSurfaceScatteringData(material, N, L) * radiance;
        float back = max(-dot(N, L), 0.0);
        Lo += material.Albedo * material.SSSColor * material.SSSWeight * back * material.Thickness * radiance *exp(-1.0 / max(material.Thickness, 0.05));

        if (NdotL <= 0.0) continue;

        AnisoFrame(N, material.AnisotropyRotation, T, B);
        float NDF = DistributionGGX_Aniso(N, T, B, H, material.Roughness, material.Anisotropy);
        float G   = GeometrySmith(N, V, L, material.Roughness);
        vec3  F   = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, material.Roughness);
        vec3  kD  = (vec3(1.0) - F) * (1.0 - material.Metallic);
        float NdotV = max(dot(N, V), 0.0);
        vec3  spec  = (NDF * G * F) / max(4.0 * NdotV * NdotL, 1e-4);
        spec = ThinFilm(spec, material, max(dot(H, V), 0.0));

        vec3 base = (kD * material.Albedo / PI + spec) * radiance * NdotL;
        if (material.CoatWeight > 0.0)
        {
            float coatF = CoatFresnel(N, V, material.CoatWeight);
            base *= mix(1.0, material.CoatDarkening, material.CoatWeight);
            base *= (1.0 - coatF);
        }
        base += SheenData(material, N, V) * radiance * NdotL;

        Lo += base;
        Lo += ClearCoat(material, N, V, L, H, radiance, NdotL);
    }
    return Lo;
}

vec3 ImageBasedLighting(vec3 F0, vec3 V, vec3 N_ibl, vec3 R, Material material)
{
    vec3 F  = fresnelSchlickRoughness(max(dot(N_ibl, V), 0.0), F0, material.Roughness);
    vec3 kD = (vec3(1.0) - F) * (1.0 - material.Metallic);

    vec3 irradiance = texture(CubeMap[sceneDataBuffer.IrradianceMapId], N_ibl).rgb;
    vec3 diffuseIBL = material.Albedo * irradiance * IBL_EXPOSURE;

    float maxLod = float(textureQueryLevels(CubeMap[sceneDataBuffer.PrefilterMapId]) - 1);
    float lod    = clamp(material.Roughness * maxLod, 0.0, maxLod);
    vec3  prefiltered = textureLod(CubeMap[sceneDataBuffer.PrefilterMapId], R, lod).rgb;

    vec2 brdf = texture(TextureMap[sceneDataBuffer.BRDFMapId],
                        vec2(max(dot(N_ibl, V), 0.0), material.Roughness)).rg;
    vec3 specularIBL = prefiltered * (F * brdf.x + brdf.y) * IBL_EXPOSURE;
    specularIBL = ThinFilm(specularIBL, material, max(dot(N_ibl, V), 0.0));

    vec3 ambient = (kD * diffuseIBL + specularIBL) * material.AmbientOcclusion;
    if (material.CoatWeight > 0.0)
    {
        float coatF   = CoatFresnel(N_ibl, V, material.CoatWeight);
        float coatLod = clamp(material.CoatRoughness * maxLod, 0.0, maxLod);
        vec3  coatPref = textureLod(CubeMap[sceneDataBuffer.PrefilterMapId], R, coatLod).rgb;
        ambient *= mix(1.0, material.CoatDarkening, material.CoatWeight);
        ambient *= (1.0 - coatF);
        ambient += coatPref * coatF * IBL_EXPOSURE * material.CoatColor;
    }
    ambient += SheenData(material, N_ibl, V) * irradiance * IBL_EXPOSURE * 0.25;
    ambient += material.SSSWeight * material.SSSColor * material.Albedo * irradiance * IBL_EXPOSURE * 0.25;
    return max(ambient, vec3(0.02) * material.Albedo);
}

vec3 SheenData(Material material, vec3 N, vec3 V)
{
    float NdotV = max(dot(N, V), 0.0);
    float rough = max(material.SheenRoughness, 0.04);
    float fresnel = pow(1.0 - NdotV, 2.0 + 3.0 * rough);
    return material.SheenColor * material.SheenWeight * fresnel * (1.0 - material.Metallic);
}

vec3 SubSurfaceScatteringData(Material material, vec3 N, vec3 L)
{
    float wrap    = mix(0.25, 0.65, clamp(material.SSSProfile, 0.0, 1.0));
    float NdotL   = dot(N, L);
    float wrapped = max(NdotL + wrap, 0.0) / (1.0 + wrap);
    float scatter = wrapped * mix(0.35, 1.0, clamp(material.Thickness, 0.0, 1.0));
    scatter      *= material.SSSWeight * (1.0 - material.Metallic);
    return material.Albedo * material.SSSColor * scatter;
}

vec3 ThinFilm(vec3 spec, Material m, float HdotV)
{
    float t = mix(300.0, 800.0, clamp(m.ThinFilmThickness, 0.0, 1.0));
    vec3  phase = vec3(t) / vec3(580.0, 550.0, 440.0);
    vec3  irid  = 0.5 + 0.5 * cos(6.2831853 * phase * HdotV);
    return mix(spec, spec * irid, m.ThinFilmWeight);
}

vec3 ClearCoat(Material m, vec3 N, vec3 V, vec3 L, vec3 H, vec3 radiance, float NdotL)
{
    if (m.CoatWeight < 1e-3) return vec3(0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NDF   = DistributionGGX(N, H, max(m.CoatRoughness, 0.04));
    float G     = GeometrySmith(N, V, L, m.CoatRoughness);
    vec3  Fc    = fresnelSchlickRoughness(max(dot(H, V), 0.0), vec3(0.04), m.CoatRoughness);
    vec3  spec  = (NDF * G * Fc) / max(4.0 * NdotV * NdotL, 1e-4);
    return spec * m.CoatColor * m.CoatWeight * radiance * NdotL;
}