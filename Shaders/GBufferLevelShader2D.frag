#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 WorldPos; 
layout(location = 1) in vec2 TexCoords;    

layout(location = 0) out vec4 outPosition;           //Position                                                                                   - R16G16B16A16_SFLOAT
layout(location = 1) out vec4 outAlbedo;             //Albedo/Alpha                                                                               - R8G8B8A8_SRGB

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout(constant_id = 0)   const uint DescriptorBindingType0   = SubpassInputDescriptor;
layout(constant_id = 1)   const uint DescriptorBindingType1   = SubpassInputDescriptor;
layout(constant_id = 2)   const uint DescriptorBindingType2   = SubpassInputDescriptor;
layout(constant_id = 3)   const uint DescriptorBindingType3   = SubpassInputDescriptor;
layout(constant_id = 4)   const uint DescriptorBindingType4  = MeshPropertiesDescriptor;
layout(constant_id = 5)   const uint DescriptorBindingType5  = MaterialDescriptor;
layout(constant_id = 6)   const uint DescriptorBindingType6  = DirectionalLightDescriptor;
layout(constant_id = 7)   const uint DescriptorBindingType7  = PointLightDescriptor;
layout(constant_id = 8)   const uint DescriptorBindingType8  = TextureDescriptor;
layout(constant_id = 9)   const uint DescriptorBindingType9  = SkyBoxDescriptor;
layout(constant_id = 10)  const uint DescriptorBindingType10  = IrradianceCubeMapDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = PrefilterDescriptor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionInput;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput albedoInput;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput depthInput;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput skyBoxInput;
layout(binding = 4)  buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 5)  buffer MaterialProperities { MaterialProperitiesBuffer2 materialProperties; } materialBuffer[];
layout(binding = 6)  buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 7)  buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
layout(binding = 8) uniform sampler2D TextureMap[];
layout(binding = 9) uniform samplerCube CubeMap;
layout(binding = 10) uniform samplerCube IrradianceMap;
layout(binding = 11) uniform samplerCube PrefilterMap;

layout(push_constant) uniform SceneDataBuffer 
{
    int   MeshBufferIndex;
    mat4  Projection;
    mat4  View;
    vec3  ViewDirection;
    vec3  CameraPosition;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

//mat3 CalculateTBN(vec3 worldPos, vec2 uv) {
//    vec3 dp1 = dFdx(worldPos);
//    vec3 dp2 = dFdy(worldPos);
//    vec2 duv1 = dFdx(uv);
//    vec2 duv2 = dFdy(uv);
//    vec3 N = vec3(0.0f, 0.0f, 1.0f);  
//    vec3 T = normalize(dp1 * duv2.t - dp2 * duv1.t);
//    vec3 B = -normalize(cross(N, T));
//    return mat3(T, B, N);
//}
//
//vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS, uint heightIdx)
//{
//    const float minLayers = 8.0;
//    const float maxLayers = 64.0;
//    float numLayers = mix(maxLayers, minLayers, abs(viewDirTS.z));
//
//vec2 shiftDirection = viewDirTS.xy * sceneData.HeightScale * -1.0;
//vec2 deltaUV = shiftDirection / numLayers;
//
//    vec2 currentUV = uv;
//    float currentLayerDepth = 0.0;
//    float currentHeight = 1.0 - texture(TextureMap[heightIdx], currentUV).a;
//
//    for (int i = 0; i < 64; ++i)
//    {
//        currentUV -= deltaUV;
//        currentHeight = 1.0 -texture(TextureMap[heightIdx], currentUV).a;
//        currentLayerDepth += 1.0 / numLayers;
//
//        if (currentLayerDepth >= currentHeight)
//            break;
//    }
//
//    vec2 prevUV = currentUV + deltaUV;
//    float afterHeight  = currentHeight - currentLayerDepth;
//    float beforeHeight = 1.0 - texture(TextureMap[heightIdx], prevUV).a - (currentLayerDepth - 1.0/numLayers);
//
//    float weight = afterHeight / (afterHeight + beforeHeight + 1e-5);
//    vec2 finalUV = mix(currentUV, prevUV, weight);
//
//    return clamp(finalUV, 0.005, 0.995);
//}
//
//vec2 OctahedronEncode(vec3 normal) 
//{
//    vec2 f = normal.xy / (abs(normal.x) + abs(normal.y) + abs(normal.z));
//    return (normal.z < 0.0) ? (1.0 - abs(f.yx)) * sign(f) : f;
//}
//
//vec3 OctahedronDecode(vec2 f)
//{
//    vec3 n;
//    n.xy = f.xy;
//    n.z = 1.0 - abs(f.x) - abs(f.y);
//    n.xy = (n.z < 0.0) ? (1.0 - abs(n.yx)) * sign(n.xy) : n.xy;
//    return normalize(n);
//}
//
//float Pack8bitPair(float high, float low) {
//    uint u_high = uint(high * 255.0 + 0.5) & 0xFFu;
//    uint u_low  = uint(low  * 255.0 + 0.5) & 0xFFu;
//    uint combined = (u_high << 8) | u_low;  // high in MSBs, low in LSBs
//    return float(combined) / 65535.0;
//}

void main()
{
    int meshIdx = sceneData.MeshBufferIndex;
    uint matId = meshBuffer[meshIdx].meshProperties.MaterialIndex;
    MaterialProperitiesBuffer2 material = materialBuffer[matId].materialProperties;

//    mat3 TBN = CalculateTBN(WorldPos, TexCoords);
//    vec3 viewDirWS = normalize(sceneData.ViewDirection);
//    vec3 viewDirTS = normalize(transpose(TBN) * viewDirWS);
//    vec2 finalUV = ParallaxOcclusionMapping(TexCoords, viewDirTS, material.NormalDataId);

    vec4 albedoData =         textureLod(TextureMap[material.AlbedoDataId],         TexCoords, 0.0f).rgba;  
//    vec4 normalData =         textureLod(TextureMap[material.NormalDataId],         finalUV, 0.0f).rgba;  
//    vec4 packedMROData =      textureLod(TextureMap[material.PackedMRODataId],      finalUV, 0.0f).rgba;
//    vec4 packedSheenSSSData = textureLod(TextureMap[material.PackedSheenSSSDataId], finalUV, 0.0f).rgba;  
//    vec4 tempMapData        = textureLod(TextureMap[material.UnusedDataId],         finalUV, 0.0f).rgba;  
//    vec4 emissionData =       textureLod(TextureMap[material.EmissionDataId],       finalUV, 0.0f).rgba;   

    if (albedoData.a < 0.1f) discard; 

//    vec2 f = normalData.xy * 2.0 - 1.0;
//    float normalStrength = normalData.b;
//    
//    vec3 tangentNormal = OctahedronDecode(f);
//    tangentNormal.xy *= normalStrength;
//    tangentNormal = normalize(tangentNormal);
//
//    vec3 normalWS = normalize(TBN * tangentNormal);
//    vec2 encodedNormalWS = OctahedronEncode(normalWS);

    outPosition = vec4(WorldPos, 1.0);
    outAlbedo = albedoData;
//    outNormalData = vec4(encodedNormalWS * 0.5 + 0.5, normalStrength, normalData.a);
//    outPackedMRO = packedMROData;
//    outPackedSheenSSS = packedSheenSSSData;
//    outTempMap = tempMapData;
//    outParallaxInfo = vec4(finalUV - TexCoords, 0.0f, 1.0f);
//    outEmission = emissionData;
}
