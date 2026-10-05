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
#include "LightingEvaluation.glsl"

const uint NO_MAP = 0xFFFFFFFFu;

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
    vec4 atten  = (p.TranslucentTextureId != NO_MAP) ? textureLod(TextureMap[p.TranslucentTextureId], uv, 0.0) : vec4(0.0);
    vec4 trans  = (p.TranslucentPropertiesTextureId != NO_MAP) ? textureLod(TextureMap[p.TranslucentPropertiesTextureId], uv, 0.0) : vec4(0.0);

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

Material UnpackMaterial(BakedMaps m, vec3 worldPos, vec3 coatTint)
{
    Material material;
    material.Position            = worldPos;
    material.Albedo              = m.Albedo;
    material.Alpha               = m.Alpha;
    material.Metallic            = m.Metallic;
    material.Roughness           = m.Roughness;
    material.AmbientOcclusion    = m.AO;
    material.IOR                 = m.IORNorm * 2.0 + 1.0;
    material.Normal              = vec3(0.0, 0.0, 1.0);
    material.Emission            = m.Emission;
    material.SheenColor          = m.SheenColor;
    material.SheenRoughness      = m.SheenRoughness;
    material.SheenWeight         = m.SheenWeight;
    material.SSSColor            = m.SSSColor;
    material.SSSWeight           = m.SSSWeight;
    material.SSSProfile          = m.SSSProfile;
    material.Thickness           = m.Thickness;
    material.CoatColor           = coatTint;
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

bool IsTransmission(BakedMaps m)
{
    return m.TransmissionWeight > kFeatureEps || (m.FeatureMask & FEAT_TRANSMISSION) != 0u;
}

void main()
{
    PackedMaterial packed = GetMaterial(PS_MaterialId);

    vec2 minUV = PS_UVOffset.xy;
    vec2 maxUV = PS_UVOffset.xy + PS_UVOffset.zw;

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) UV.x = minUV.x + maxUV.x - UV.x;
    if (PS_FlipSprite.y == 1) UV.y = minUV.y + maxUV.y - UV.y;

    mat3 TBN = mat3(normalize(PS_T), normalize(PS_B), normalize(PS_N));
    vec3 viewDirWS = PerspectiveView(WorldPos);
    vec3 viewDirTS = normalize(transpose(TBN) * viewDirWS);
    vec2 finalUV   = ParallaxOcclusionMapping(UV, viewDirTS, packed.NormalTextureId, minUV, maxUV);

    BakedMaps m = UnpackBakedMaterial(packed, finalUV);
    m.Albedo *= PS_Color.rgb;
    m.Alpha  *= PS_Color.a;

    // Opaque pass owns alpha >= cutoff. This pass owns the blended remainder,
    // plus any transmission material even when the albedo alpha is opaque.
    if (!IsTransmission(m) && (m.Alpha >= packed.AlphaCutOff || m.Alpha < 0.01))
        discard;

    if ((m.FeatureMask & FEAT_ALBEDO_ONLY) != 0u)
    {
        outAlphaColor = vec4(m.Albedo, m.Alpha);
        outAlphaBloom = vec4(0.0);
        return;
    }

    vec3 tN = m.TangentNormal;
    tN.xy *= m.NormalStrength;
    tN = normalize(tN);

    vec3 coatTint = dot(packed.ClearcoatTint, packed.ClearcoatTint) > 1e-6 ? packed.ClearcoatTint : vec3(1.0);
    Material material = UnpackMaterial(m, WorldPos, coatTint);
    material.Normal = normalize(TBN * tN);
    if ((material.FeatureMask & FEAT_TWO_SIDED) != 0u && dot(material.Normal, viewDirWS) < 0.0)
        material.Normal = -material.Normal;

    DirectionalLightBuffer key = GetDirectionalLight(0);
    if (key.LightActive == 1u && dot(key.LightDirection, key.LightDirection) > 1e-8)
    {
        vec3 Lts = normalize(transpose(TBN) * normalize(-key.LightDirection));
        material.SelfShadow = HeightSelfShadow(finalUV, Lts, packed.NormalTextureId, m.Height, minUV, maxUV);
    }

    vec3 N = material.Normal;
    vec3 V = viewDirWS;
    vec3 R = reflect(-V, N);

    float F0d = pow((material.IOR - 1.0) / (material.IOR + 1.0), 2.0);
    vec3  F0  = mix(vec3(F0d), material.Albedo, material.Metallic);

    vec3 lit = DirectionalLightFunc(F0, V, material) + PointLightFunc(F0, V, material);
    vec3 ibl = ImageBasedLighting(F0, V, N, R, material);
    vec3 color = lit + ibl + material.Emission;

    float alpha = TransmissionCoverage(material, m.Alpha, max(dot(N, V), 0.0), F0d);
    outAlphaColor = vec4(color, alpha);
    outAlphaBloom = vec4((material.Emission + max(lit + ibl - vec3(1.0), vec3(0.0))) * alpha, alpha);
}