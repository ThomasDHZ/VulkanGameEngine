#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout (location = 0) in vec3  WorldPos;
layout (location = 1) in vec2  PS_UV;
layout (location = 2) in vec2  PS_SpriteSize;
layout (location = 3) in flat ivec2 PS_FlipSprite;
layout (location = 4) in vec4  PS_Color;
layout (location = 5) in flat uint  PS_MaterialID;
layout (location = 6) in flat vec4  PS_UVOffset;

layout(location = 0) out vec4 outPosition;           //Position                                                                                   - R16G16B16A16_SFLOAT
layout(location = 1) out vec4 outAlbedo;             //Albedo/Alpha                                                                               - R8G8B8A8_SRGB
layout(location = 2) out vec4 outNormalData;         //Normal/Height/unused                                                                       - R16G16B16A16_UNORM 
layout(location = 3) out vec4 outPackedMRO;          //vec4(Metallic/Rough, AO/ClearcoatTint, ClearcoatStrength/ClearcoatRoughness, unused)       - R16G16B16A16_UNORM
layout(location = 4) out vec4 outPackedSheenSSS;     //vec4(sheenColor.r/sheenColor.g, sheenColor.b/sheenIntensity, sss.r/sss.g, sss.b/thickness) - R16G16B16A16_UNORM
layout(location = 5) out vec4 outTempMap;            //vec4(                                                                                    ) - R16G16B16A16_UNORM
layout(location = 6) out vec4 outParallaxInfo;       //ParallaxUV/Height                                                                          - R16G16B16A16_UNORM
layout(location = 7) out vec4 outEmission;           //Emission                                                                                   - R16G16B16A16_UNORM

layout(constant_id = 0)  const uint DescriptorBindingType0  = SceneDataDescriptor;
layout(constant_id = 1)  const uint DescriptorBindingType1  = MemoryPoolDescriptor;
layout(constant_id = 2)  const uint DescriptorBindingType2  = TextureDescriptor;
layout(constant_id = 3)  const uint DescriptorBindingType3  = Texture3DDescriptor;
layout(constant_id = 4)  const uint DescriptorBindingType4  = SkyBoxDescriptor;

layout(std430, binding = 0)  buffer SceneDataBuffer 
{ 
	uint BRDFMapId;
	uint CubeMapId;
	uint IrradianceMapId;
	uint PrefilterMapId;
	mat4  Projection;
	mat4  View;
	mat4  InverseProjection;
	mat4  InverseView;
	vec3  CameraPosition;
	vec3  ViewDirection;
    vec2  InvertResolution;
	float Time;
	uint  FrameIndex;
}sceneDataBuffer;
layout(binding = 1)  buffer BindlessBuffer 
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
layout(binding = 2) uniform sampler2D TextureMap[];
layout(binding = 3) uniform sampler3D Texture3DMap[];
layout(binding = 4) uniform samplerCube CubeMap[];

layout(set = 1, binding = 0, input_attachment_index = 0) uniform subpassInput positionInput;
layout(set = 1, binding = 1, input_attachment_index = 1) uniform subpassInput albedoInput;
layout(set = 1, binding = 2, input_attachment_index = 2) uniform subpassInput normalInput;
layout(set = 1, binding = 3, input_attachment_index = 3) uniform subpassInput packedMROInput;
layout(set = 1, binding = 4, input_attachment_index = 4) uniform subpassInput packedSheenSSSInput;
layout(set = 1, binding = 5, input_attachment_index = 5) uniform subpassInput tempInput;
layout(set = 1, binding = 6, input_attachment_index = 6) uniform subpassInput parallaxUVInfoInput;
layout(set = 1, binding = 7, input_attachment_index = 7) uniform subpassInput emissionInput;
layout(set = 1, binding = 8, input_attachment_index = 8) uniform subpassInput depthInput;

layout(push_constant) uniform SceneDataBuffer
{
    int   MeshBufferIndex;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

#include "BindlessHelpers.glsl"

vec4 SampleTexture(uint textureIndex, vec2 uv)
{
    TextureMetadata meta = Get2DTextureMetadata(textureIndex);

    if (meta.TextureType == 0) // 2D
    {
        return texture(TextureMap[meta.ArrayIndex], uv);
    }
    return vec4(1.0, 0.0, 1.0, 1.0); // error pink
}

mat3 TBN = mat3(
    vec3(1.0, 0.0, 0.0),   // Tangent   (along X/UV.x)
    vec3(0.0, 1.0, 0.0),   // Bitangent (along Y/UV.y)
    vec3(0.0, 0.0, 1.0)    // Normal    (+Z)
);

vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS, uint heightIdx)
{
    if (sceneData.UseHeightMap == 0) return uv;

    const float minLayers = 16.0;
    const float maxLayers = 64.0;  // Lower than tiled version — sprites don't need as many for performance
    float numLayers = mix(maxLayers, minLayers, abs(viewDirTS.z));

    vec2 shiftDirection = viewDirTS.xy * sceneData.HeightScale * -1.0;
    vec2 deltaUV        = shiftDirection / numLayers;

    vec2  currentUV      = uv;
    float currentDepth   = 0.0;
    // Consistent .a channel for height
    float height         = 1.0 - textureLod(TextureMap[heightIdx], currentUV, 0.0).a;

    int maxSteps = 96;
    for (int i = 0; i < maxSteps; ++i)
    {
        currentUV    -= deltaUV;
        height        = 1.0 - textureLod(TextureMap[heightIdx], currentUV, 0.0).a;
        currentDepth += 1.0 / numLayers;

        if (currentDepth >= height) break;
    }

    // Refinement step
    vec2  prevUV       = currentUV + deltaUV;
    float afterDepth   = height - currentDepth;
    float beforeDepth  = (1.0 - textureLod(TextureMap[heightIdx], prevUV, 0.0).a) 
                         - (currentDepth - 1.0/numLayers);

    float weight       = afterDepth / (afterDepth - beforeDepth + 1e-5);
    vec2  finalUV      = mix(currentUV, prevUV, weight);

    // Optional: Soft clamp/fade near texture edges to prevent minor bleeding
    // (uncomment if you see artifacts at sprite borders)
     vec2 edgeDist = min(finalUV, 1.0 - finalUV);
     float edgeFade = smoothstep(0.0, 0.05, min(edgeDist.x, edgeDist.y));
     finalUV = uv + (finalUV - uv) * edgeFade;

    return finalUV;
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

void main() 
{
    PackedMaterial material = GetMaterial(PS_MaterialID);

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) UV.x = PS_UVOffset.x + PS_UVOffset.z - (UV.x - PS_UVOffset.x);
    if (PS_FlipSprite.y == 1) UV.y = PS_UVOffset.y + PS_UVOffset.w - (UV.y - PS_UVOffset.y);

    vec3 viewDirWS = normalize(sceneDataBuffer.ViewDirection);
    vec3 viewDirTS = normalize(transpose(TBN) * viewDirWS);
    vec2 finalUV = ParallaxOcclusionMapping(UV, viewDirTS, material.NormalDataId);

    vec4 albedoData           = texture(TextureMap[material.AlbedoDataId],            finalUV, -0.5f).rgba;    
    vec3 normalData           = textureLod(TextureMap[material.NormalDataId],         finalUV, 0.0f).rgb;    
    vec4 packedMROData        = textureLod(TextureMap[material.PackedMRODataId],      finalUV, 0.0f).rgba;   
    vec4 packedSheenSSSData   = textureLod(TextureMap[material.PackedSheenSSSDataId], finalUV, 0.0f).rgba;    
    vec4 tempMapData          = textureLod(TextureMap[material.UnusedDataId],         finalUV, 0.0f).rgba;    
    vec4 emissionData         = textureLod(TextureMap[material.EmissionDataId],       finalUV, 0.0f).rgba;
    float height              = textureLod(TextureMap[material.NormalDataId],         finalUV, 0.0f).a;
    if (albedoData.a < 0.1f) discard; 

    vec2 f = normalData.xy * 2.0f - 1.0f;
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
    outParallaxInfo = vec4(finalUV - UV, 0.0f, 1.0);
    outEmission = emissionData;
}