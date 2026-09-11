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
    uint  HDRMapInputIndex;
    uint  FrameBufferIndex;
    uint  BRDFMapId;
    uint  CubeMapId;
    uint  IrradianceMapId;
    uint  PrefilterMapId;
    mat4  Projection;
    mat4  View;
    mat4  InverseProjection;
    mat4  InverseView;
    vec3  CameraPosition;
    vec3  ViewDirection;
    vec2  InvertResolution;
    float Time;
    uint  FrameIndex;
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

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outAlbedo;
layout(location = 2) out vec4 outNormalData;
layout(location = 3) out vec4 outPackedMRO;
layout(location = 4) out vec4 outPackedSheenSSS;
layout(location = 5) out vec4 outTempMap;
layout(location = 6) out vec4 outParallaxInfo;
layout(location = 7) out vec4 outEmission;

#include "BindlessHelpers.glsl"

mat3 CalculateTBN(vec3 worldPos, vec2 uv)
{
    vec3 dp1 = dFdx(worldPos);
    vec3 dp2 = dFdy(worldPos);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 N = normalize(cross(dp1, dp2));
    vec3 T = duv1.y * dp2 - duv2.y * dp1;
    if (dot(T, T) < 1e-8)
        T = normalize(cross(abs(N.y) < 0.99 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0), N));
    else
        T = normalize(T);
    vec3 B = normalize(cross(N, T));
    return mat3(T, B, N);
}

vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS, uint heightIdx)
{
    if (sceneData.UseHeightMap == 0)
        return uv;

    vec2 tileUV     = fract(uv);
    vec2 tileOrigin = uv - tileUV;

    float numLayers = mix(64.0, 8.0, abs(viewDirTS.z));
    vec2  deltaUV   = (viewDirTS.xy * sceneData.HeightScale * -1.0) / numLayers;

    vec2  currentUV    = tileUV;
    float currentDepth = 0.0;
    float height       = 1.0 - textureLod(TextureMap[heightIdx], currentUV + tileOrigin, 0.0).a;

    for (int i = 0; i < 64; ++i)
    {
        currentUV    -= deltaUV;
        height        = 1.0 - textureLod(TextureMap[heightIdx], currentUV + tileOrigin, 0.0).a;
        currentDepth += 1.0 / numLayers;
        if (currentDepth >= height)
            break;
    }

    vec2  prevUV      = currentUV + deltaUV;
    float afterDepth  = height - currentDepth;
    float beforeDepth = (1.0 - textureLod(TextureMap[heightIdx], prevUV + tileOrigin, 0.0).a)
                        - (currentDepth - 1.0 / numLayers);
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
    float step = max(sceneData.HeightScale, 0.05) * 0.02;
    vec2  dUV  = Lts.xy * step;
    float rayH = startH;
    vec2  local = tileUV;

    for (int i = 0; i < steps; ++i)
    {
        local += dUV;
        rayH  += Lts.z * step;

        vec2 sampleUV = tileOrigin + fract(local);
        float h = textureLod(TextureMap[heightIdx], sampleUV, 0.0).a; 
        if (h > rayH + 0.02) return mix(0.55, 1.0, float(i) / float(steps));
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

void main()
{
    MeshProperitiesBuffer mesh     = GetMesh(sceneData.MeshBufferIndex);
    PackedMaterial        material = GetMaterial(mesh.MaterialIndex);

    mat3 TBN = CalculateTBN(WorldPos, TexCoords);

    vec3 viewDirWS = normalize(sceneDataBuffer.CameraPosition - WorldPos);
    vec3 viewDirTS = normalize(transpose(TBN) * viewDirWS);
    vec2 finalUV   = ParallaxOcclusionMapping(TexCoords, viewDirTS, material.NormalDataId);

    vec4  albedoData         = texture(TextureMap[material.AlbedoDataId],            finalUV, -0.5).rgba;
    vec3  normalData         = textureLod(TextureMap[material.NormalDataId],         finalUV, 0.0).rgb;
    vec3  packedMROData      = textureLod(TextureMap[material.PackedMRODataId],      finalUV, 0.0f).rgb;   
    vec4  packedSheenSSSData = textureLod(TextureMap[material.PackedSheenSSSDataId], finalUV, 0.0).rgba;
    vec4  tempMapData        = textureLod(TextureMap[material.UnusedDataId],         finalUV, 0.0).rgba;
    vec4  emissionData       = textureLod(TextureMap[material.EmissionDataId],       finalUV, 0.0).rgba;
    float heightRaw          = textureLod(TextureMap[material.NormalDataId],         finalUV, 0.0).a;

    if (albedoData.a < 0.1)
        discard;

    vec3 tangentNormal = OctahedronDecode(normalData.xy * 2.0 - 1.0);
    tangentNormal.xy  *= normalData.b;
    tangentNormal      = normalize(tangentNormal);

    vec3 normalWS         = normalize(TBN * tangentNormal);
    vec2 encodedNormalWS  = OctahedronEncode(normalWS);

    vec3 Lws = normalize(-GetDirectionalLight(0).LightDirection);
    vec3 Lts = normalize(transpose(TBN) * Lws);
    float selfShadow = HeightSelfShadowTiled(finalUV, Lts, material.NormalDataId, heightRaw);

    outPosition      = vec4(WorldPos, 1.0);
    outAlbedo        = albedoData;
    outNormalData = vec4(encodedNormalWS * 0.5 + 0.5, normalData.b, heightRaw);
    outPackedMRO  = vec4(packedMROData, selfShadow);
    outPackedSheenSSS = packedSheenSSSData;
    outTempMap       = tempMapData;
    outParallaxInfo  = vec4(finalUV - TexCoords, 0.0, 1.0);
    outEmission      = emissionData;
}