#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"
#include "MemoryPoolBindings.glsl"

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

layout(push_constant) uniform SceneDataBuffer
{
    int   MeshBufferIndex;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

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

float SampleHeight(uint heightIdx, vec2 uv)
{
    return textureLod(TextureMap[heightIdx], uv, 0.0).a;
}

vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS, uint heightIdx, vec2 minUV, vec2 maxUV)
{
    if (sceneData.UseHeightMap == 0) return uv;

    const float minLayers = 16.0;
    const float maxLayers = 64.0;
    float numLayers = mix(maxLayers, minLayers, abs(viewDirTS.z));
    vec2  deltaUV   = (viewDirTS.xy * sceneData.HeightScale * -1.0) / numLayers;

    vec2  currentUV    = uv;
    float currentDepth = 0.0;
    float height       = SampleHeight(heightIdx, currentUV);

    for (int i = 0; i < 96; ++i)
    {
        currentUV    -= deltaUV;
        currentUV     = clamp(currentUV, minUV, maxUV);
        height        = SampleHeight(heightIdx, currentUV);
        currentDepth += 1.0 / numLayers;
        if (currentDepth >= height) break;
    }

    vec2  prevUV      = clamp(currentUV + deltaUV, minUV, maxUV);
    float afterDepth  = height - currentDepth;
    float beforeDepth = SampleHeight(heightIdx, prevUV) - (currentDepth - 1.0 / numLayers);
    float weight      = afterDepth / (afterDepth - beforeDepth + 1e-5);
    vec2  finalUV     = mix(currentUV, prevUV, weight);

    vec2  spriteSize = max(maxUV - minUV, vec2(1e-5));
    vec2  edgeDist   = min(finalUV - minUV, maxUV - finalUV) / spriteSize;
    float edgeFade   = smoothstep(0.0, 0.05, min(edgeDist.x, edgeDist.y));
    finalUV = uv + (finalUV - uv) * edgeFade;
    return clamp(finalUV, minUV, maxUV);
}

float HeightSelfShadow(vec2 uv, vec3 Lts, uint heightIdx, float startH, vec2 minUV, vec2 maxUV)
{
    if (Lts.z <= 0.0) return 1.0;

    const int steps = 20;
    float step = max(sceneData.HeightScale, 0.05) * 0.02;
    vec2  dUV  = Lts.xy * step;
    float rayH = startH;
    vec2  p    = uv;

    for (int i = 0; i < steps; ++i)
    {
        p    += dUV;
        rayH += Lts.z * step;
        if (any(lessThan(p, minUV)) || any(greaterThan(p, maxUV))) break;

        float h = SampleHeight(heightIdx, p);
        if (h > rayH + 0.02)
            return mix(0.45, 1.0, float(i) / float(steps));
    }
    return 1.0;
}

BakedMaps UnpackBakedMaterial(PackedMaterial p, vec2 uv);
Material UnpackMaterial(BakedMaps m);
vec3 DirectionalLightFunc(vec3 F0, vec3 V, Material material);
vec3 PointLightFunc(vec3 F0, vec3 V, Material material);
vec3 ImageBasedLighting(vec3 F0, vec3 V, vec3 N_ibl, vec3 R, Material material);

void main()
{
    PackedMaterial packed = GetMaterial(PS_MaterialId);

    vec2 minUV = PS_UVOffset.xy;
    vec2 maxUV = PS_UVOffset.xy + PS_UVOffset.zw;

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) UV.x = minUV.x + maxUV.x - UV.x;
    if (PS_FlipSprite.y == 1) UV.y = minUV.y + maxUV.y - UV.y;

    mat3 TBN = mat3(normalize(PS_T), normalize(PS_B), normalize(PS_N));

    vec3 viewDirWS = normalize(sceneDataBuffer.PerspectiveCameraPosition - WorldPos);
    vec3 viewDirTS = normalize(transpose(TBN) * viewDirWS);
    vec2 finalUV   = ParallaxOcclusionMapping(UV, viewDirTS, packed.NormalTextureId, minUV, maxUV);

    BakedMaps m = UnpackBakedMaterial(packed, finalUV);
    if (m.Alpha >= packed.AlphaCutOff || m.Alpha < 0.01) discard;

    vec3 tN = m.TangentNormal;
    tN.xy *= m.NormalStrength;
    tN = normalize(tN);

    Material material = UnpackMaterial(m);
    material.Normal = normalize(TBN * tN);

    vec2 uvSpan = max(maxUV - minUV, vec2(1e-5));
    float worldPerUV = length(PS_SpriteSize / uvSpan);
    float height = m.Height * sceneData.HeightScale * worldPerUV;
    vec3 Lts = normalize(transpose(TBN) * normalize(-GetDirectionalLight(0).LightDirection));
    material.SelfShadow = HeightSelfShadow(finalUV, Lts, packed.NormalTextureId, m.Height, minUV, maxUV);
    material.Position = WorldPos - normalize(PS_N) * height;

    vec3  toCamera  = sceneDataBuffer.PerspectiveCameraPosition - material.Position;
    vec3  N         = material.Normal;
    vec3  V         = dot(toCamera, toCamera) > 1e-6 ? normalize(toCamera) : normalize(-sceneDataBuffer.PerspectiveViewDirection);
    vec3  R         = reflect(-V, N);

    float F0d       = pow((material.IOR - 1.0) / (material.IOR + 1.0), 2.0);
    vec3  F0        = mix(vec3(F0d), material.Albedo, material.Metallic);

    vec3 reflected  = ImageBasedLighting(F0, V, N, R, material)
                    + DirectionalLightFunc(F0, V, material)
                    + PointLightFunc(F0, V, material)
                    + material.Emission;

    float NdotV     = max(dot(N, V), 0.0);
    vec3  F         = fresnelSchlickRoughness(NdotV, vec3(F0d), material.Roughness);
    float Ft        = clamp(1.0 - max(F.r, max(F.g, F.b)), 0.0, 1.0);
    float T         = clamp(material.TransmissionWeight, 0.0, 1.0);

    vec3 absorb     = exp(-material.AttenuationColor * max(material.Thickness, 0.02) * 8.0);
    vec3 color      = reflected * absorb;

    float alpha     = mix(m.Alpha, mix(0.55, 0.12, Ft), T);
    alpha           = clamp(alpha, 0.08, 0.75);

    outAlphaColor   = vec4(color, alpha);
    outAlphaBloom   = vec4(max(color - vec3(1.0), 0.0) * alpha, alpha);
}

const uint NO_MAP = 0xFFFFFFFFu;

BakedMaps UnpackBakedMaterial(PackedMaterial p, vec2 uv)
{
    vec4 albedo = texture(TextureMap[p.AlbedoTextureId], uv, -0.5);
    vec4 nrm    = textureLod(TextureMap[p.NormalTextureId], uv, 0.0);
    vec4 mro    = textureLod(TextureMap[p.MROTextureId], uv, 0.0);
    vec4 coat   = textureLod(TextureMap[p.ClearCoatOrTranslucentTextureId], uv, 0.0);
    vec4 sssCol = textureLod(TextureMap[p.SubSurfaceScatteringOrTranslucentPropertiesTextureId], uv, 0.0);
    vec4 sssPr  = textureLod(TextureMap[p.SubSurfaceScatteringPropertiesTextureId], uv, 0.0);
    vec4 sheen  = textureLod(TextureMap[p.SheenTextureId], uv, 0.0);
    vec4 aniso  = textureLod(TextureMap[p.AnisotropyTextureId], uv, 0.0);
    vec4 emis   = textureLod(TextureMap[p.EmissionTextureId], uv, 0.0);
    vec4 atten = (p.TranslucentTextureId != NO_MAP) ? textureLod(TextureMap[p.TranslucentTextureId], uv, 0.0) : vec4(0.0);
    vec4 trans = (p.TranslucentPropertiesTextureId != NO_MAP) ? textureLod(TextureMap[p.TranslucentPropertiesTextureId], uv, 0.0) : vec4(0.0);

    BakedMaps m;
    m.Albedo             = albedo.rgb;
    m.Alpha              = albedo.a;
    m.Emission           = emis.rgb * emis.a;
    m.TangentNormal      = OctahedronDecode(nrm.xy * 2.0 - 1.0);
    m.NormalStrength     = nrm.b;
    m.Height             = nrm.a;
    m.Metallic           = mro.r;
    m.Roughness          = mro.g;
    m.AO                 = mro.b;
    m.IORNorm            = mro.a;
    m.SheenColor         = sheen.rgb;
    m.SheenWeight        = sheen.a;
    m.SSSColor           = sssCol.rgb;
    m.SSSWeight          = sssPr.r;
    m.SSSProfile         = sssPr.g;
    m.Thickness          = (p.TranslucentPropertiesTextureId != NO_MAP) ? trans.g : sssPr.b;
    m.SheenRoughness     = sssPr.a;
    m.CoatWeight         = coat.r;
    m.CoatRoughness      = coat.g;
    m.CoatDarkening      = coat.b;
    m.Anisotropy         = aniso.r;
    m.AnisotropyRotation = aniso.g;
    m.ThinFilmWeight     = aniso.b;
    m.ThinFilmThickness  = aniso.a;
    m.AttenuationColor   = atten.rgb;
    m.TransmissionWeight = trans.r;
    m.AttenuationDistance = trans.b;
    m.FeatureMask        = p.FeatureMask;
    return m;
}

Material UnpackMaterial(BakedMaps m)
{
    Material material;
    material.Albedo              = m.Albedo;
    material.Alpha               = m.Alpha;
    material.Metallic            = m.Metallic;
    material.Roughness           = m.Roughness;
    material.AmbientOcclusion    = m.AO;
    material.IOR                 = m.IORNorm * 2.0 + 1.0;
    material.Normal              = vec3(0.0, 0.0, 1.0); // filled by the caller from TBN
    material.Emission            = m.Emission;
    material.SheenColor          = m.SheenColor;
    material.SheenRoughness      = m.SheenRoughness;
    material.SheenWeight         = m.SheenWeight;
    material.SSSColor            = m.SSSColor;
    material.SSSWeight           = m.SSSWeight;
    material.SSSProfile          = m.SSSProfile;
    material.Thickness           = m.Thickness;
    material.CoatColor           = vec3(1.0);
    material.CoatWeight          = m.CoatWeight;
    material.CoatRoughness       = m.CoatRoughness;
    material.CoatDarkening       = m.CoatDarkening;
    material.AttenuationColor    = m.AttenuationColor;
    material.AttenuationDistance = m.AttenuationDistance;
    material.TransmissionWeight  = m.TransmissionWeight;
    material.Anisotropy          = m.Anisotropy;
    material.AnisotropyRotation  = m.AnisotropyRotation;
    material.ThinFilmWeight      = m.ThinFilmWeight;
    material.ThinFilmThickness   = m.ThinFilmThickness;
    material.FeatureMask         = m.FeatureMask;
    material.SelfShadow          = 1.0;
    material.ShadingModel        = 0u;
    return material;
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

        float d = max(distance, 0.05);
        float window = clamp(1.0 - pow(d / light.LightRadius, 4.0), 0.0, 1.0);
        window *= window;
        vec3 radiance = light.LightColor * light.LightIntensity * (window / (d * d)) * material.SelfShadow;
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