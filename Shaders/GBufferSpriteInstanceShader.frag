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

layout(location = 0) in vec3       WorldPos;
layout(location = 1) in vec2       PS_UV;
layout(location = 2) in vec2       PS_SpriteSize;
layout(location = 3) in flat ivec2 PS_FlipSprite;
layout(location = 4) in vec4       PS_Color;
layout(location = 5) in flat uint  PS_MaterialId;
layout(location = 6) in flat vec4  PS_UVOffset;
layout(location = 7) in flat uint  PS_SpriteId;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outAlbedo;
layout(location = 2) out vec4 outNormalData;
layout(location = 3) out vec4 outMRO;
layout(location = 4) out vec4 outFeatureA;
layout(location = 5) out vec4 outFeatureB;
layout(location = 6) out vec4 outFeatureC;
layout(location = 7) out vec4 outEmission;

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

    vec2 deltaUV = (viewDirTS.xy * sceneData.HeightScale * -1.0) / numLayers;

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
        if (h > rayH + 0.02) return mix(0.45, 1.0, float(i) / float(steps));
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
    uint combined = (u_high << 8) | u_low;
    return float(combined) / 65535.0;
}

void main()
{
    PackedMaterial material = GetMaterial(PS_MaterialId);

    vec2 minUV = PS_UVOffset.xy;
    vec2 maxUV = PS_UVOffset.xy + PS_UVOffset.zw;

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) UV.x = minUV.x + maxUV.x - UV.x;
    if (PS_FlipSprite.y == 1) UV.y = minUV.y + maxUV.y - UV.y;

    vec3 N = normalize(sceneDataBuffer.PerspectiveCameraPosition - WorldPos);
    vec3 T = normalize(cross(vec3(0.0, 1.0, 0.0), N));
    if (dot(T, T) < 1e-6) T = normalize(cross(vec3(1.0, 0.0, 0.0), N));
    vec3 B = normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    vec3 viewDirWS = N;
    vec3 viewDirTS = normalize(transpose(TBN) * viewDirWS);
    vec2 finalUV   = ParallaxOcclusionMapping(UV, viewDirTS, material.NormalTextureId, minUV, maxUV);

    vec4 albedoDataMap         = texture(TextureMap[material.AlbedoTextureId], finalUV, -0.5);
    vec4 normalDataMap         = textureLod(TextureMap[material.NormalTextureId], finalUV, 0.0);
    vec4 mroDataMap            = textureLod(TextureMap[material.MROTextureId], finalUV, 0.0);
    vec4 clearCoatColorDataMap = textureLod(TextureMap[material.ClearCoatOrTranslucentTextureId], finalUV, 0.0);
    vec4 sssDataMap            = textureLod(TextureMap[material.SubSurfaceScatteringOrTranslucentPropertiesTextureId], finalUV, 0.0);
    vec4 sssPropertiesDataMap  = textureLod(TextureMap[material.SubSurfaceScatteringPropertiesTextureId], finalUV, 0.0);
    vec4 sheenDataMap          = textureLod(TextureMap[material.SheenTextureId], finalUV, 0.0);
    vec4 anisotropyDataMap     = textureLod(TextureMap[material.AnisotropyTextureId], finalUV, 0.0);
    vec4 emissionDataMap       = textureLod(TextureMap[material.EmissionTextureId], finalUV, 0.0);
    if (albedoDataMap.a < material.AlphaCutOff) discard;

    vec3 tangentNormal = OctahedronDecode(normalDataMap.xy * 2.0 - 1.0);
    tangentNormal.xy  *= normalDataMap.b;
    tangentNormal      = normalize(tangentNormal);

    vec3 normalWS = normalize(TBN * tangentNormal);
    vec3 Lws = normalize(-GetDirectionalLight(0).LightDirection);
    vec3 Lts = normalize(transpose(TBN) * Lws);
    float selfShadow = HeightSelfShadow(finalUV, Lts, material.NormalTextureId, normalDataMap.a, minUV, maxUV);

    outPosition   = vec4(WorldPos, 1.0);
    outAlbedo     = vec4(albedoDataMap.rgb, albedoDataMap.a);
    outNormalData = vec4(OctahedronEncode(normalWS) * 0.5 + 0.5, 0.0, selfShadow);
    outMRO        = mroDataMap;
    outFeatureA   = vec4(sheenDataMap.rgb, Pack8bitPair(sheenDataMap.a, clearCoatColorDataMap.r));
    outFeatureB   = vec4(sssDataMap.rgb,   Pack8bitPair(sssDataMap.a, clearCoatColorDataMap.g));
    outFeatureC   = vec4(Pack8bitPair(anisotropyDataMap.r, anisotropyDataMap.g), Pack8bitPair(anisotropyDataMap.b, anisotropyDataMap.a), Pack8bitPair(clearCoatColorDataMap.b, sssPropertiesDataMap.g), Pack8bitPair(sssPropertiesDataMap.r, sssPropertiesDataMap.b));
    outEmission   = vec4(emissionDataMap.rgb * emissionDataMap.a, 1.0);
}