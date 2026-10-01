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

layout(push_constant) uniform SceneDataBuffer
{
    int   MeshBufferIndex;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

#include "BindlessHelpers.glsl"

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

vec2 OctahedronEncode(vec3 normal)
{
    vec2 f = normal.xy / (abs(normal.x) + abs(normal.y) + abs(normal.z));
    return (normal.z < 0.0) ? (1.0 - abs(f.yx)) * sign(f) : f;
}

vec3 OctahedronDecode(vec2 f)
{
    vec3 n;
    n.xy = f.xy;
    n.z  = 1.0 - abs(f.x) - abs(f.y);
    n.xy = (n.z < 0.0) ? (1.0 - abs(n.yx)) * sign(n.xy) : n.xy;
    return normalize(n);
}

float Pack8bitPair(float high, float low)
{
    uint u_high = uint(high * 255.0 + 0.5) & 0xFFu;
    uint u_low  = uint(low  * 255.0 + 0.5) & 0xFFu;
    return float((u_high << 8) | u_low) / 65535.0;
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
    m.Albedo         = albedo.rgb;
    m.Alpha          = albedo.a;
    m.Emission       = emis.rgb * emis.a;
    m.TangentNormal  = OctahedronDecode(nrm.xy * 2.0 - 1.0);
    m.NormalStrength = nrm.b;
    m.Height         = nrm.a;
    m.Metallic       = mro.r;
    m.Roughness      = mro.g;
    m.AO             = mro.b;
    m.IORNorm        = mro.a;
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

    PackedMaterial material = GetMaterial(PS_MaterialId);

    vec2 minUV = PS_UVOffset.xy;
    vec2 maxUV = PS_UVOffset.xy + PS_UVOffset.zw;

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) UV.x = minUV.x + maxUV.x - UV.x;
    if (PS_FlipSprite.y == 1) UV.y = minUV.y + maxUV.y - UV.y;

    mat3 TBN = mat3(normalize(PS_T), normalize(PS_B), normalize(PS_N));

    vec3 viewDirWS = normalize(sceneDataBuffer.PerspectiveCameraPosition - WorldPos);
    vec3 viewDirTS = normalize(transpose(TBN) * viewDirWS);
    vec2 finalUV   = ParallaxOcclusionMapping(UV, viewDirTS, material.NormalTextureId, minUV, maxUV);

    BakedMaps m = UnpackBakedMaterial(material, UV);
    if (m.Alpha >= material.AlphaCutOff || m.Alpha < 0.01f) discard;

    vec3 tN = m.TangentNormal;
    tN.xy *= m.NormalStrength;
    tN = normalize(tN);
    vec3 normalWS = normalize(TBN * tN);

    vec3 Lts = normalize(transpose(TBN) * normalize(-GetDirectionalLight(0).LightDirection));
    float selfShadow = HeightSelfShadow(finalUV, Lts, material.NormalTextureId, m.Height, minUV, maxUV);

    outAlphaColor = vec4(m.Albedo, m.Alpha);
    outAlphaBloom = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}