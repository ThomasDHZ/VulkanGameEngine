#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_shader_8bit_storage : require

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

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout(constant_id = 0)   const uint DescriptorBindingType0   = SubpassInputDescriptor;
layout(constant_id = 1)   const uint DescriptorBindingType1   = SubpassInputDescriptor;
layout(constant_id = 2)   const uint DescriptorBindingType2   = SubpassInputDescriptor;
layout(constant_id = 3)   const uint DescriptorBindingType3   = SubpassInputDescriptor;
layout(constant_id = 4)   const uint DescriptorBindingType4   = SubpassInputDescriptor;
layout(constant_id = 5)   const uint DescriptorBindingType5   = SubpassInputDescriptor;
layout(constant_id = 6)   const uint DescriptorBindingType6   = SubpassInputDescriptor;
layout(constant_id = 7)   const uint DescriptorBindingType7   = SubpassInputDescriptor;
layout(constant_id = 8)   const uint DescriptorBindingType8   = SubpassInputDescriptor;
layout(constant_id = 9)   const uint DescriptorBindingType9   = MeshPropertiesDescriptor;
layout(constant_id = 10)  const uint DescriptorBindingType10  = MaterialDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = DirectionalLightDescriptor;
layout(constant_id = 12)  const uint DescriptorBindingType12  = PointLightDescriptor;
layout(constant_id = 13)  const uint DescriptorBindingType13  = TextureDescriptor;
layout(constant_id = 14)  const uint DescriptorBindingType14  = SkyBoxDescriptor;
layout(constant_id = 15)  const uint DescriptorBindingType15  = IrradianceCubeMapDescriptor;
layout(constant_id = 16)  const uint DescriptorBindingType16  = PrefilterDescriptor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionInput;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput albedoInput;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput normalInput;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput packedMROInput;
layout(input_attachment_index = 4, binding = 4) uniform subpassInput packedSheenSSSInput;
layout(input_attachment_index = 5, binding = 5) uniform subpassInput tempInput;
layout(input_attachment_index = 6, binding = 6) uniform subpassInput parallaxUVInfoInput;
layout(input_attachment_index = 7, binding = 7) uniform subpassInput emissionInput;
layout(input_attachment_index = 8, binding = 8) uniform subpassInput depthInput;
layout(binding = 9)  buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 10) buffer MaterialProperties
{
    uint MaterialOffset;     // byte offset to start of materials (typically 12)
    uint MaterialCount;
    uint MaterialSize;       // bytes per material, e.g. 24
    uint Data[];             // ← this line MUST be here; unsized array at the end
} materialBuffer;
layout(binding = 11)  buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 12)  buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
layout(binding = 13) uniform sampler2D TextureMap[];
layout(binding = 14) uniform samplerCube CubeMap;
layout(binding = 15) uniform samplerCube IrradianceMap;
layout(binding = 16) uniform samplerCube PrefilterMap;

layout(push_constant) uniform SceneDataBuffer
{
    int   MeshBufferIndex;
    mat4  Projection;
    mat4  View;
    vec3  ViewDirection;
    vec3  CameraPosition;
    int   UseHeightMap;
    float HeightScale;
    int   Buffer1;
} sceneData;

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

uint ReadUint(uint uintBase, uint offsetUints)
{
    return materialBuffer.Data[uintBase + offsetUints];
}

float ReadFloat(uint uintBase, uint offsetUints)
{
    return uintBitsToFloat(materialBuffer.Data[uintBase + offsetUints]);
}

vec2 ReadVec2(uint uintBase, uint offsetUints)
{
    return vec2(
        ReadFloat(uintBase, offsetUints + 0u),
        ReadFloat(uintBase, offsetUints + 1u)
    );
}

vec3 ReadVec3(uint uintBase, uint offsetUints)
{
    return vec3(
        ReadFloat(uintBase, offsetUints + 0u),
        ReadFloat(uintBase, offsetUints + 1u),
        ReadFloat(uintBase, offsetUints + 2u)
    );
}

vec4 ReadVec4(uint uintBase, uint offsetUints)
{
    return vec4(
        ReadFloat(uintBase, offsetUints + 0u),
        ReadFloat(uintBase, offsetUints + 1u),
        ReadFloat(uintBase, offsetUints + 2u),
        ReadFloat(uintBase, offsetUints + 3u)
    );
}

mat3 ReadMat3(uint uintBase, uint offsetUints)
{
    return mat3(
        ReadVec3(uintBase, offsetUints + 0u),   // column 0
        ReadVec3(uintBase, offsetUints + 3u),   // column 1
        ReadVec3(uintBase, offsetUints + 6u)    // column 2
    );
}

mat4 ReadMat4(uint uintBase, uint offsetUints)
{
    return mat4(
        ReadVec4(uintBase, offsetUints +  0u),  // column 0
        ReadVec4(uintBase, offsetUints +  4u),  // column 1
        ReadVec4(uintBase, offsetUints +  8u),  // column 2
        ReadVec4(uintBase, offsetUints + 12u)   // column 3
    );
}

MaterialProperitiesBuffer2 GetMaterial(uint index)
{
    MaterialProperitiesBuffer2 mat;
    mat.AlbedoDataId          = ~0u;
    mat.NormalDataId          = ~0u;
    mat.PackedMRODataId       = ~0u;
    mat.PackedSheenSSSDataId  = ~0u;
    mat.UnusedDataId          = ~0u;
    mat.EmissionDataId        = ~0u;
    if (index >= materialBuffer.MaterialCount)
    {
        return mat;
    }

    index = 1;
    mat.AlbedoDataId          = materialBuffer.Data[(index * 6) + 0u];
    mat.NormalDataId          = materialBuffer.Data[(index * 6) + 1u];
    mat.PackedMRODataId       = materialBuffer.Data[(index * 6) + 2u];
    mat.PackedSheenSSSDataId  = materialBuffer.Data[(index * 6) + 3u];
    mat.UnusedDataId          = materialBuffer.Data[(index * 6) + 4u];
    mat.EmissionDataId        = materialBuffer.Data[(index * 6) + 5u];
    return mat;
}

void main()
{
    int meshIdx = sceneData.MeshBufferIndex;
    uint matId = meshBuffer[meshIdx].meshProperties.MaterialIndex;
    MaterialProperitiesBuffer2 material = GetMaterial(matId);

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
 