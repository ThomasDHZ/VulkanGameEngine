#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_NV_shader_buffer_load : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "BindlessHelpers.glsl"

layout(push_constant) uniform SceneDataBuffer
{
    uint MeshBufferIndex;
    uint CubeMapIndex;
    mat4 Projection;
    mat4 View;
    vec3 ViewDirection;
    vec3 CameraPosition;
    int UseHeightMap;
    float HeightScale;
} sceneData;

layout(location = 0) in vec3 WorldPos; 
layout(location = 1) in vec2 TexCoords;    

layout(location = 0) out vec4 outPosition;           //Position                                                                                   - R16G16B16A16_SFLOAT
layout(location = 1) out vec4 outAlbedo;             //Albedo/Alpha                                                                               - R8G8B8A8_SRGB
layout(location = 2) out vec4 outNormalData;         //Normal/Height/unused                                                                       - R16G16B16A16_UNORM 
layout(location = 3) out vec4 outPackedMRO;          //vec4(Metallic/Rough, AO/ClearcoatTint, ClearcoatStrength/ClearcoatRoughness, unused)       - R16G16B16A16_UNORM
layout(location = 4) out vec4 outPackedSheenSSS;     //vec4(sheenColor.r/sheenColor.g, sheenColor.b/sheenIntensity, sss.r/sss.g, sss.b/thickness) - R16G16B16A16_UNORM
layout(location = 5) out vec4 outTempMap;            //vec4(                                                                                    ) - R16G16B16A16_UNORM
layout(location = 6) out vec4 outParallaxInfo;       //ParallaxUV/Height                                                                          - R16G16B16A16_UNORM
layout(location = 7) out vec4 outEmission;           //Emission                                                                                   - R16G16B16A16_UNORM

mat3 CalculateTBN(vec3 worldPos, vec2 uv);
vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS, uint heightIdx);
vec2 OctahedronEncode(vec3 normal);
vec3 OctahedronDecode(vec2 f);
float Pack8bitPair(float high, float low);
vec2 Unpack8bitPair(float packed);

void main()
{
    MeshPropertiesBuffer meshProps = GetMesh(sceneData.MeshBufferIndex);

    uint materialId = meshProps.MaterialIndex;
    PackedMaterial material = GetMaterial(materialId);
    CubeMapMaterial cubeMapMaterial = GetCubeMapMaterial(sceneData.CubeMapIndex);

    mat3 TBN = CalculateTBN(WorldPos, TexCoords);
    vec3 viewDirWS = normalize(sceneData.ViewDirection);
    vec3 viewDirTS = normalize(transpose(TBN) * viewDirWS);
    vec2 finalUV =  ParallaxOcclusionMapping(TexCoords, viewDirTS, material.NormalDataId);

    vec4 albedoData           = texture(TextureMap[material.AlbedoDataId],            finalUV, -0.5f).rgba;    
    vec3 normalData           = textureLod(TextureMap[material.NormalDataId],         finalUV, 0.0f).rgb;    
    vec4 packedMROData        = textureLod(TextureMap[material.PackedMRODataId],      finalUV, 0.0f).rgba;   
    vec4 packedSheenSSSData   = textureLod(TextureMap[material.PackedSheenSSSDataId], finalUV, 0.0f).rgba;    
    vec4 tempMapData          = textureLod(TextureMap[material.UnusedDataId],         finalUV, 0.0f).rgba;    
    vec4 emissionData         = textureLod(TextureMap[material.EmissionDataId],       finalUV, 0.0f).rgba;
    float height              = textureLod(TextureMap[material.NormalDataId],         finalUV, 0.0f).a;
    if (albedoData.a < 0.1f) discard; 

    vec2 f = normalData.xy * 2.0 - 1.0;
    float normalStrength = normalData.b;
    
    vec3 tangentNormal = OctahedronDecode(f);
    tangentNormal.xy *= normalStrength;
    tangentNormal = normalize(tangentNormal);

    vec3 normalWS = normalize(TBN * tangentNormal);
    vec2 encodedNormalWS = OctahedronEncode(normalWS);

    outPosition = vec4(WorldPos, 1.0);
    outAlbedo = albedoData;
    outNormalData = vec4(encodedNormalWS * 0.5 + 0.5, normalData.b, height);
    outPackedMRO = packedMROData;
    outPackedSheenSSS = packedSheenSSSData;
    outTempMap = tempMapData;
    outParallaxInfo = vec4(finalUV - TexCoords, 0.0f, 1.0);
    outEmission = emissionData;
}

 
mat3 CalculateTBN(vec3 worldPos, vec2 uv) {
    vec3 dp1 = dFdx(worldPos);
    vec3 dp2 = dFdy(worldPos);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);
    vec3 N = vec3(0.0f, 0.0f, 1.0f);  
    vec3 T = normalize(dp1 * duv2.t - dp2 * duv1.t);
    vec3 B = -normalize(cross(N, T));
    return mat3(T, B, N);
}

vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS, uint heightIdx)
{
    if (sceneData.UseHeightMap == 0) return uv;

    // Tile-local coordinates
    vec2 tileUV     = fract(uv);               // [0,1] inside current tile
    vec2 tileOrigin = uv - tileUV;             // Base UV of this tile

    const float minLayers = 8.0;
    const float maxLayers = 64.0;
    float numLayers = mix(maxLayers, minLayers, abs(viewDirTS.z));

    vec2 P       = viewDirTS.xy * sceneData.HeightScale * -1.0;
    vec2 deltaUV = P / numLayers;

    vec2 currentUV     = tileUV;
    float currentDepth = 0.0;
    float height       = 1.0 - textureLod(TextureMap[heightIdx], currentUV + tileOrigin, 0.0).a;

    int maxSteps = 64;
    for (int i = 0; i < maxSteps; ++i)
    {
        currentUV     -= deltaUV;
        height         = 1.0 - textureLod(TextureMap[heightIdx], currentUV + tileOrigin, 0.0).a;
        currentDepth  += 1.0 / numLayers;

        if (currentDepth >= height) break;
    }

    // Linear interpolate between last two steps
    vec2 prevUV        = currentUV + deltaUV;
    float afterDepth   = height - currentDepth;
    float beforeDepth  = (1.0 - textureLod(TextureMap[heightIdx], prevUV + tileOrigin, 0.0).a) 
                         - (currentDepth - 1.0/numLayers);

    float weight       = afterDepth / (afterDepth - beforeDepth + 1e-5);
    vec2 finalLocalUV  = mix(currentUV, prevUV, weight);

    // Clamp to stay strictly inside tile (prevents bleeding)
    finalLocalUV = clamp(finalLocalUV, vec2(0.0), vec2(1.0));

    // Reconstruct full UV
    return tileOrigin + finalLocalUV;
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
    n.z = 1.0 - abs(f.x) - abs(f.y);
    n.xy = (n.z < 0.0) ? (1.0 - abs(n.yx)) * sign(n.xy) : n.xy;
    return normalize(n);
}

float Pack8bitPair(float high, float low) {
    uint u_high = uint(high * 255.0 + 0.5) & 0xFFu;
    uint u_low  = uint(low  * 255.0 + 0.5) & 0xFFu;
    uint combined = (u_high << 8) | u_low;  // high in MSBs, low in LSBs
    return float(combined) / 65535.0;
}

vec2 Unpack8bitPair(float packed) {
    uint combined = uint(packed * 65535.0 + 0.5);
    float high = float((combined >> 8) & 0xFFu) / 255.0;
    float low  = float(combined & 0xFFu) / 255.0;
    return vec2(high, low);
}