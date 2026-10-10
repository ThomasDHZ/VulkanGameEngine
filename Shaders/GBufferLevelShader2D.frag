#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"
#include "MemoryPoolBindings.glsl"

layout(push_constant) uniform SceneDataBuffer
{
    int   MeshBufferIndex;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

layout(location = 0) in vec3 WorldPos;
layout(location = 1) in vec2 TexCoords;
layout(location = 2) in vec3  PS_T;
layout(location = 3) in vec3  PS_B;
layout(location = 4) in vec3  PS_N;

layout(location = 0) out vec4 outPosition;      // R16G16B16A16_SFLOAT
layout(location = 1) out vec4 outAlbedo;        // R8G8B8A8_SRGB
layout(location = 2) out vec4 outNormalData;    // R16G16B16A16_UNORM
layout(location = 3) out vec4 outMRO;           // R16G16B16A16_UNORM
layout(location = 4) out vec4 outFeatureA;      // R16G16B16A16_UNORM
layout(location = 5) out vec4 outFeatureB;      // R16G16B16A16_UNORM
layout(location = 6) out vec4 outFeatureC;      // R16G16B16A16_UNORM
layout(location = 7) out vec4 outEmission;      // R16G16B16A16_SFLOAT

#include "BindlessHelpers.glsl"

float SampleHeight(uint heightIdx, vec2 uv)
{
    return textureLod(TextureMap[heightIdx], uv, 0.0).a;
}

vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS, uint heightIdx)
{
    if (sceneData.UseHeightMap == 0) return uv;

    vec2 tileUV     = fract(uv);
    vec2 tileOrigin = uv - tileUV;

    float numLayers = mix(64.0, 8.0, abs(viewDirTS.z));
    vec2  deltaUV   = (viewDirTS.xy * sceneData.HeightScale * -1.0) / numLayers;

    vec2  currentUV    = tileUV;
    float currentDepth = 0.0;
    float height       = SampleHeight(heightIdx, currentUV + tileOrigin);

    for (int i = 0; i < 64; ++i)
    {
        currentUV    -= deltaUV;
        height        = SampleHeight(heightIdx, currentUV + tileOrigin);
        currentDepth += 1.0 / numLayers;
        if (currentDepth >= height) break;
    }

    vec2  prevUV      = currentUV + deltaUV;
    float afterDepth  = height - currentDepth;
    float beforeDepth = SampleHeight(heightIdx, prevUV + tileOrigin) - (currentDepth - 1.0 / numLayers);
    float weight      = afterDepth / (afterDepth - beforeDepth + 1e-5);
    vec2  localUV     = clamp(mix(currentUV, prevUV, weight), 0.0, 1.0);

    return tileOrigin + localUV;
}

float HeightSelfShadowTiled(vec2 uv, vec3 Lts, uint heightIdx, float startH)
{
    if (Lts.z <= 0.0) return 1.0;

    vec2 tileUV     = fract(uv);
    vec2 tileOrigin = uv - tileUV;

    const int steps = 16;
    float step = max(sceneData.HeightScale, 0.05) * 0.08;
    vec2  dUV  = Lts.xy * step;
    float rayH = startH;
    vec2  local = tileUV;

    for (int i = 0; i < steps; ++i)
    {
        local += dUV;
        rayH  += Lts.z * step;

        vec2  sampleUV = tileOrigin + fract(local);
        float h        = SampleHeight(heightIdx, sampleUV);
        if (h > rayH + 0.05) return mix(0.35, 1.0, float(i) / float(steps));
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

    BakedMaps m;
    m.Albedo = albedo.rgb;
    m.Alpha  = albedo.a;
    m.Emission = emis.rgb * emis.a;

    m.TangentNormal  = OctahedronDecode(nrm.xy * 2.0 - 1.0);
    m.NormalStrength = nrm.b;
    m.Height         = nrm.a;

    m.Metallic = mro.r;
    m.Roughness = mro.g;
    m.AO = mro.b;
    m.IORNorm = mro.a;

    m.SheenColor     = sheen.rgb;
    m.SheenWeight    = sheen.a;

    m.SSSColor       = sssCol.rgb;
    m.SSSWeight      = sssPr.r;
    m.SSSProfile     = sssPr.g;
    m.Thickness      = sssPr.b;
    m.SheenRoughness = sssPr.a;

    m.CoatWeight     = coat.r;
    m.CoatRoughness  = coat.g;
    m.CoatDarkening  = coat.b;

    m.Anisotropy         = aniso.r;
    m.AnisotropyRotation = aniso.g;
    m.ThinFilmWeight     = aniso.b;
    m.ThinFilmThickness  = aniso.a;

    m.FeatureMask = p.FeatureMask;
    return m;
}

void main()
{
    MeshProperitiesBuffer mesh           = GetMesh(sceneData.MeshBufferIndex);
    PackedMaterial        packedMaterial = GetMaterial(mesh.MaterialIndex);

    mat3 TBN = mat3(normalize(PS_T), normalize(PS_B), normalize(PS_N));
    vec3 viewDirWS = normalize(sceneDataBuffer.PerspectiveCameraPosition - WorldPos);
    vec3 viewDirTS = normalize(transpose(TBN) * viewDirWS);
    vec2 finalUV   = ParallaxOcclusionMapping(TexCoords, viewDirTS, packedMaterial.NormalTextureId);

    BakedMaps m = UnpackBakedMaterial(packedMaterial, finalUV);
    if (m.Alpha < packedMaterial.AlphaCutOff) discard;

    vec3 tN = m.TangentNormal;
    tN.xy *= m.NormalStrength;
    tN = normalize(tN);
    vec3 normalWS = normalize(TBN * tN);

    vec3 Lts = normalize(transpose(TBN) * normalize(-GetDirectionalLight(0).LightDirection));
    float selfShadow = HeightSelfShadowTiled(finalUV, Lts, packedMaterial.NormalTextureId, m.Height);

    vec3 perspectiveWorldPos = WorldPos;

        vec2 uvDx = dFdx(TexCoords);
        vec2 uvDy = dFdy(TexCoords);
        float uvSpan = max(0.5 * (length(uvDx) + length(uvDy)), 1e-5);
        float worldSpan = 0.5 * (length(dFdx(WorldPos)) + length(dFdy(WorldPos)));
        float height = m.Height * sceneData.HeightScale * (worldSpan / uvSpan);
        perspectiveWorldPos = WorldPos - normalize(PS_N) * height;

    outPosition = vec4(perspectiveWorldPos, 1.0);
    outAlbedo     = vec4(m.Albedo, m.Alpha);
    outEmission   = vec4(m.Emission, 1.0);
    outNormalData = vec4(OctahedronEncode(normalWS) * 0.5 + 0.5, float(m.FeatureMask) / 65535.0, selfShadow);
    outMRO        = vec4(m.Metallic, m.Roughness, m.AO, m.IORNorm);
    outFeatureA   = vec4(m.SheenColor, Pack8bitPair(m.SSSWeight, m.CoatWeight));
    outFeatureB   = vec4(m.SSSColor,   Pack8bitPair(m.Thickness, m.CoatRoughness));
    outFeatureC   = vec4(Pack8bitPair(m.Anisotropy, m.AnisotropyRotation), Pack8bitPair(m.ThinFilmWeight, m.ThinFilmThickness),  Pack8bitPair(m.CoatDarkening, m.SSSProfile), Pack8bitPair(m.SheenRoughness, m.SheenWeight));
}