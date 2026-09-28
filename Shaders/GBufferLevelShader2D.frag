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

layout(push_constant) uniform Push
{
    int   MeshBufferIndex;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

layout(location = 0) in vec3 WorldPos;
layout(location = 1) in vec2 TexCoords;

layout(location = 0) out vec4 outPosition;      // R16G16B16A16_SFLOAT
layout(location = 1) out vec4 outAlbedo;        // R8G8B8A8_SRGB
layout(location = 2) out vec4 outNormalData;    // R16G16B16A16_UNORM
layout(location = 3) out vec4 outMRO;           // R16G16B16A16_UNORM
layout(location = 4) out vec4 outFeatureA;      // R16G16B16A16_UNORM
layout(location = 5) out vec4 outFeatureB;      // R16G16B16A16_UNORM
layout(location = 6) out vec4 outFeatureC;      // R16G16B16A16_UNORM
layout(location = 7) out vec4 outEmission;      // R16G16B16A16_SFLOAT

#include "BindlessHelpers.glsl"

mat3 CalculateTBN(vec3 worldPos, vec2 uv)
{
    vec3 dp1  = dFdx(worldPos);
    vec3 dp2  = dFdy(worldPos);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 N = normalize(cross(dp1, dp2));
    vec3 T = duv1.y * dp2 - duv2.y * dp1;
    if (dot(T, T) < 1e-8) T = normalize(cross(abs(N.y) < 0.99 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0), N));
    else T = normalize(T);
    vec3 B = normalize(cross(N, T));
    return mat3(T, B, N);
}

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

vec2 Unpack8bitPair(float packed)
{
    uint combined = uint(packed * 65535.0 + 0.5);
    float high = float((combined >> 8) & 0xFFu) / 255.0;
    float low  = float(combined & 0xFFu) / 255.0;
    return vec2(high, low);
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

    mat3 TBN = CalculateTBN(WorldPos, TexCoords);
    vec3 viewDirWS = normalize(sceneDataBuffer.PerspectiveCameraPosition - WorldPos);
    vec3 viewDirTS = normalize(transpose(TBN) * viewDirWS);
    vec2 finalUV   = ParallaxOcclusionMapping(TexCoords, viewDirTS, packedMaterial.NormalTextureId);

    BakedMaps m = UnpackBakedMaterial(packedMaterial, finalUV);
    if (m.Alpha < packedMaterial.AlphaCutOff) discard;

    vec3 tN = m.TangentNormal;
    tN.xy *= m.NormalStrength;
    tN = normalize(tN);

    vec3 normalWS = normalize(TBN * tN);
    vec3 Lws = normalize(-GetDirectionalLight(0).LightDirection);
    vec3 Lts = normalize(transpose(TBN) * Lws);
    
    float selfShadow = 1.0f;
    //if (sceneData.UseHeightMap != 0)
    selfShadow = HeightSelfShadowTiled(finalUV, Lts, packedMaterial.NormalTextureId, m.Height);

    outPosition   = vec4(WorldPos, 1.0);
    outAlbedo     = vec4(m.Albedo, m.Alpha);
    outEmission   = vec4(m.Emission, 1.0);
    outNormalData = vec4(OctahedronEncode(normalWS) * 0.5 + 0.5, float(m.FeatureMask) / 65535.0, selfShadow);
    outMRO        = vec4(m.Metallic, m.Roughness, m.AO, m.IORNorm);
    outFeatureA   = vec4(m.SheenColor, Pack8bitPair(m.SSSWeight, m.CoatWeight));
    outFeatureB   = vec4(m.SSSColor,   Pack8bitPair(m.Thickness, m.CoatRoughness));
    outFeatureC   = vec4(Pack8bitPair(m.Anisotropy, m.AnisotropyRotation), Pack8bitPair(m.ThinFilmWeight, m.ThinFilmThickness),  Pack8bitPair(m.CoatDarkening, m.SSSProfile), Pack8bitPair(m.SheenRoughness, m.SheenWeight));
}