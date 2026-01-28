#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

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
layout(location = 2) out vec4 outNormalData;         //Normal/Height/unused                                                                      - R16G16B16A16_UNORM 
layout(location = 3) out vec4 outPackedMRO;          //vec4(Metallic/Rough, AO/ClearcoatTint, ClearcoatStrength/ClearcoatRoughness, unused)       - R16G16B16A16_UNORM
layout(location = 4) out vec4 outPackedSheenSSS;     //vec4(sheenColor.r/sheenColor.g, sheenColor.b/sheenIntensity, sss.r/sss.g, sss.b/thickness) - R16G16B16A16_UNORM
//layout(location = 5) out vec4 outTempMap;            //vec4(                                                                                    ) - R16G16B16A16_UNORM
//layout(location = 6) out vec4 outParallaxInfo;       //ParallaxUV/Height                                                                          - R16G16B16A16_UNORM
//layout(location = 7) out vec4 outEmission;           //Emission                                                                                   - R8G8B8A8_SRGB


layout(constant_id = 0)   const uint DescriptorBindingType0   = SubpassInputDescriptor;
layout(constant_id = 1)   const uint DescriptorBindingType1   = SubpassInputDescriptor;
layout(constant_id = 2)   const uint DescriptorBindingType2   = SubpassInputDescriptor;
layout(constant_id = 3)   const uint DescriptorBindingType3   = SubpassInputDescriptor;
layout(constant_id = 4)   const uint DescriptorBindingType4   = SubpassInputDescriptor;
layout(constant_id = 5)   const uint DescriptorBindingType5   = SubpassInputDescriptor;
layout(constant_id = 6)   const uint DescriptorBindingType6   = SubpassInputDescriptor;
layout(constant_id = 7)   const uint DescriptorBindingType7   = MeshPropertiesDescriptor;
layout(constant_id = 8)   const uint DescriptorBindingType8   = MaterialDescriptor;
layout(constant_id = 9)   const uint DescriptorBindingType9   = DirectionalLightDescriptor;
layout(constant_id = 10)   const uint DescriptorBindingType10   = PointLightDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = TextureDescriptor;
layout(constant_id = 12)  const uint DescriptorBindingType12  = SkyBoxDescriptor;
layout(constant_id = 13)  const uint DescriptorBindingType13  = IrradianceCubeMapDescriptor;
layout(constant_id = 14)  const uint DescriptorBindingType14  = PrefilterDescriptor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionInput;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput albedoInput;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput normalInput;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput packedMROInput;
layout(input_attachment_index = 4, binding = 4) uniform subpassInput packedSheenSSS;
layout(input_attachment_index = 5, binding = 5) uniform subpassInput depthInput;
layout(input_attachment_index = 6, binding = 6) uniform subpassInput skyBoxInput;
layout(binding = 7)  buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 8)  buffer MaterialProperities { MaterialProperitiesBuffer2 materialProperties; } materialBuffer[];
layout(binding = 9)  buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 10)  buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
layout(binding = 11) uniform sampler2D TextureMap[];
layout(binding = 12) uniform samplerCube CubeMap;
layout(binding = 13) uniform samplerCube IrradianceMap;
layout(binding = 14) uniform samplerCube PrefilterMap;

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

mat3 TBN = mat3(
    vec3(1.0, 0.0, 0.0),   // Tangent   (along X/UV.x)
    vec3(0.0, 1.0, 0.0),   // Bitangent (along Y/UV.y)
    vec3(0.0, 0.0, 1.0)    // Normal    (+Z)
);

//vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS, uint heightIdx)
//{
//    if (sceneData.UseHeightMap == 0) return uv;
//
//    const float minLayers = 16.0f;
//    const float maxLayers = 96.0f;
//    float numLayers = mix(maxLayers, minLayers, abs(viewDirTS.z));
//
//    vec2 shiftDirection = viewDirTS.xy * sceneData.HeightScale * -1.0f;
//    // Remove or loosen this clamp — it was killing offset on grazed angles / low height values
//    // shiftDirection = clamp(shiftDirection, vec2(-0.06f), vec2(0.06f));
//
//    vec2 deltaUV = shiftDirection / numLayers;
//
//    vec2 currentUV = uv;
//    float currentLayerDepth = 0.0f;
//    float currentHeight = 1.0f - textureLod(TextureMap[heightIdx], currentUV, 0.0f).a;
//
//    for (int x = 0; x < 96; ++x) 
//    {
//        currentUV -= deltaUV;
//        currentHeight = 1.0f - textureLod(TextureMap[heightIdx], currentUV, 0.0f).a;
//        currentLayerDepth += 1.0f / numLayers;
//        if (currentLayerDepth >= currentHeight) break;
//    }
//
//    vec2 prevUV = currentUV + deltaUV;
//    float afterHeight = currentHeight - currentLayerDepth;
//    float beforeHeight = 1.0f - textureLod(TextureMap[heightIdx], prevUV, 0.0f).a - (currentLayerDepth - 1.0f/numLayers);
//
//    float weight = afterHeight / (afterHeight + beforeHeight + 1e-5f);
//    vec2 finalUV = mix(currentUV, prevUV, weight);
//
//    return clamp(finalUV, 0.0f, 1.0f);
//}
//
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
void main() {
    MaterialProperitiesBuffer2 material = materialBuffer[PS_MaterialID].materialProperties;

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) UV.x = PS_UVOffset.x + PS_UVOffset.z - (UV.x - PS_UVOffset.x);
    if (PS_FlipSprite.y == 1) UV.y = PS_UVOffset.y + PS_UVOffset.w - (UV.y - PS_UVOffset.y);

   // vec3 viewDirWS = normalize(sceneData.ViewDirection);
   // vec3 viewDirTS = normalize(transpose(TBN) * viewDirWS);
    vec2 finalUV = UV;//ParallaxOcclusionMapping(UV, viewDirTS, material.NormalDataId);

      vec4  a =   textureLod(TextureMap[0],         finalUV, 0.0f).rgba;  
      vec4  a1 =   textureLod(TextureMap[1],         finalUV, 0.0f).rgba;  
      vec4  a2 =   textureLod(TextureMap[2],         finalUV, 0.0f).rgba;  
      vec4  a3 =   textureLod(TextureMap[3],         finalUV, 0.0f).rgba;  
      vec4  a4 =   textureLod(TextureMap[4],         finalUV, 0.0f).rgba;  
      vec4  a5 =   textureLod(TextureMap[5],         finalUV, 0.0f).rgba;  
      vec4  a6 =   textureLod(TextureMap[6],         finalUV, 0.0f).rgba;  
      vec4  a7 =   textureLod(TextureMap[7],         finalUV, 0.0f).rgba;  
      vec4  a8 =   textureLod(TextureMap[8],         finalUV, 0.0f).rgba;  
      vec4  a9 =   textureLod(TextureMap[9],         finalUV, 0.0f).rgba;  
      vec4  a10 =   textureLod(TextureMap[10],         finalUV, 0.0f).rgba;  
      vec4  a11 =   textureLod(TextureMap[11],         finalUV, 0.0f).rgba;  
      vec4  a12 =   textureLod(TextureMap[12],         finalUV, 0.0f).rgba;  
      vec4  a13 =   textureLod(TextureMap[13],         finalUV, 0.0f).rgba;  
      vec4  a14 =   textureLod(TextureMap[14],         finalUV, 0.0f).rgba;  
      vec4  a15 =   textureLod(TextureMap[15],         finalUV, 0.0f).rgba;  
      vec4  a16 =   textureLod(TextureMap[16],         finalUV, 0.0f).rgba;  
      vec4  a17 =   textureLod(TextureMap[17],         finalUV, 0.0f).rgba;  

    vec4 albedoData           = textureLod(TextureMap[material.AlbedoDataId],         finalUV, 0.0f).rgba;    
    vec4 normalData           = textureLod(TextureMap[material.NormalDataId],         finalUV, 0.0f).rgba;    
    vec4 packedMROData        = textureLod(TextureMap[material.PackedMRODataId],      finalUV, 0.0f).rgba;   
    vec4 packedSheenSSSData = textureLod(TextureMap[material.PackedSheenSSSDataId], finalUV, 0.0f).rgba;    
//    vec4 tempMapData        = textureLod(TextureMap[material.UnusedDataId],         finalUV, 0.0f).rgba;    
//    vec4 emissionData       = textureLod(TextureMap[material.EmissionDataId],       finalUV, 0.0f).rgba;    
    if (albedoData.a < 0.1f) discard; 

    vec2 f = normalData.xy * 2.0 - 1.0;
    float normalStrength = normalData.b;
    
    vec3 tangentNormal = OctahedronDecode(f);
    tangentNormal.xy *= normalStrength;
    tangentNormal = normalize(tangentNormal);

   // vec3 normalWS = normalize(TBN * tangentNormal);
    vec2 encodedNormalWS = OctahedronEncode(tangentNormal);


        const vec2 unpackMRO_Metallic_Rough                        = Unpack8bitPair(packedMROData.r);
    const vec2 unpackMRO_AO_ClearCoatTint                      = Unpack8bitPair(packedMROData.g);

    outPosition = vec4(WorldPos, 1.0);
    outAlbedo = albedoData;
    outNormalData = vec4(encodedNormalWS * 0.5 + 0.5, normalStrength, 1.0f);
    outPackedMRO = vec4(unpackMRO_Metallic_Rough.r, unpackMRO_Metallic_Rough.g, unpackMRO_AO_ClearCoatTint.r, unpackMRO_AO_ClearCoatTint.g);
    outPackedSheenSSS = packedSheenSSSData;
//    outTempMap = tempMapData;
//    outParallaxInfo = vec4(finalUV - UV, 0.0f, 1.0f);
//    outEmission = emissionData;
}