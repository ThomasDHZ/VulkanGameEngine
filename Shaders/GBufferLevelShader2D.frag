#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 WorldPos; 
layout(location = 1) in vec2 TexCoords;    

layout(location = 0) out vec4 outPosition;           //Position                                                                                   - R16G16B16A16_SFLOAT
layout(location = 1) out vec4 outAlbedo;             //Albedo/Alpha                                                                               - R8G8B8A8_UNORM
layout(location = 2) out vec4 outNormalData;         //Normal/NormalStrength                                                                      - R16G16B16A16_UNORM
layout(location = 3) out vec4 outPackedMRO;          //vec4(Metallic/Rough, AO/ClearcoatTint, ClearcoatStrength/ClearcoatRoughness, unused)       - R16G16B16A16_UNORM
layout(location = 4) out vec4 outPackedSheenSSS;     //vec4(sheenColor.r/sheenColor.g, sheenColor.b/sheenIntensity, sss.r/sss.g, sss.b/thickness) - R16G16B16A16_UNORM
layout(location = 5) out vec4 TempMap;
layout(location = 6) out vec4 outParallaxInfo;       //ParallaxUV/Height                                                                          - R16G16B16A16_UNORM
layout(location = 7) out vec4 outEmission;           //Emission                                                                                   - R8G8B8A8_UNORM

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
layout(constant_id = 9)   const uint DescriptorBindingType9   = SubpassInputDescriptor;
layout(constant_id = 10)  const uint DescriptorBindingType10   = MeshPropertiesDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = MaterialDescriptor;
layout(constant_id = 12)  const uint DescriptorBindingType12  = DirectionalLightDescriptor;
layout(constant_id = 13)  const uint DescriptorBindingType13  = PointLightDescriptor;
layout(constant_id = 14)  const uint DescriptorBindingType14  = TextureDescriptor;
layout(constant_id = 15)  const uint DescriptorBindingType15  = SkyBoxDescriptor;
layout(constant_id = 16)  const uint DescriptorBindingType16  = IrradianceCubeMapDescriptor;
layout(constant_id = 17)  const uint DescriptorBindingType17  = PrefilterDescriptor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionInput;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput albedoInput;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput normalInput;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput packedMROInput;
layout(input_attachment_index = 4, binding = 4) uniform subpassInput packedSheenSSSInput;
layout(input_attachment_index = 5, binding = 5) uniform subpassInput tempInput;
layout(input_attachment_index = 6, binding = 6) uniform subpassInput parallaxUVInfoInput;
layout(input_attachment_index = 7, binding = 7) uniform subpassInput emissionInput;
layout(input_attachment_index = 8, binding = 8) uniform subpassInput depthInput;
layout(input_attachment_index = 9, binding = 9) uniform subpassInput skyBoxInput;
layout(binding = 10)  buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 11)  buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];
layout(binding = 12)  buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 13)  buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
layout(binding = 14) uniform sampler2D TextureMap[];
layout(binding = 15) uniform samplerCube CubeMap;
layout(binding = 16) uniform samplerCube IrradianceMap;
layout(binding = 17) uniform samplerCube PrefilterMap;

layout(push_constant) uniform SceneDataBuffer 
{
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3  ViewDirection;
    vec3 CameraPosition;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

mat3 CalculateTBN(vec3 worldPos, vec2 uv) {
    vec3 dp1   = dFdx(worldPos);
    vec3 dp2   = dFdy(worldPos);
    vec2 duv1  = dFdx(uv);
    vec2 duv2  = dFdy(uv);

    vec3 N = vec3(0.0f, 0.0f, 1.0f);
    vec3 T = normalize(dp1 * duv2.t - dp2 * duv1.t);
    vec3 B = -normalize(cross(N, T));

    return mat3(T, B, N);
}

vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS, uint heightIdx)
{
    if (sceneData.HeightScale < 0.001f || heightIdx == 0xFFFFFFFFu) 
        return uv;

    const float minLayers = 8.0;
    const float maxLayers = 64.0;
    float numLayers = mix(maxLayers, minLayers, abs(viewDirTS.z));

vec2 shiftDirection = viewDirTS.xy * sceneData.HeightScale * -1.0;
vec2 deltaUV = shiftDirection / numLayers;

    vec2 currentUV = uv;
    float currentLayerDepth = 0.0;
    float currentHeight = 1.0 - texture(TextureMap[heightIdx], currentUV).r;

    for (int i = 0; i < 64; ++i)
    {
        currentUV -= deltaUV;
        currentHeight = 1.0 -texture(TextureMap[heightIdx], currentUV).r;
        currentLayerDepth += 1.0 / numLayers;

        if (currentLayerDepth >= currentHeight)
            break;
    }

    vec2 prevUV = currentUV + deltaUV;
    float afterHeight  = currentHeight - currentLayerDepth;
    float beforeHeight = 1.0 - texture(TextureMap[heightIdx], prevUV).r - (currentLayerDepth - 1.0/numLayers);

    float weight = afterHeight / (afterHeight + beforeHeight + 1e-5);
    vec2 finalUV = mix(currentUV, prevUV, weight);

    return clamp(finalUV, 0.005, 0.995);
}

void main()
{
    int meshIdx = sceneData.MeshBufferIndex;
    uint matId = meshBuffer[meshIdx].meshProperties.MaterialIndex;
    MaterialProperitiesBuffer material = materialBuffer[matId].materialProperties;
    
       mat3 TBN = CalculateTBN(WorldPos, TexCoords);

    vec3 viewDirWS = normalize(sceneData.ViewDirection);
    vec3 viewDirTS = normalize(transpose(TBN) * viewDirWS);

    vec2 finalUV = ParallaxOcclusionMapping(TexCoords, viewDirTS, material.HeightMap);

    vec4  albedo     = (material.AlbedoMap           != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlbedoMap],           finalUV, 0.0f)                   : vec4(material.Albedo, 1.0f);
    vec3 normalTS = (material.NormalMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.NormalMap], finalUV, 0.0).xyz * 2.0 - 1.0 : vec3(0.0, 0.0, 1.0);
    float metallic   = (material.MetallicMap         != 0xFFFFFFFFu) ? textureLod(TextureMap[material.MetallicMap], finalUV, 0.0f).r : material.Metallic;
    float roughness  = (material.MetallicMap         != 0xFFFFFFFFu) ? textureLod(TextureMap[material.MetallicMap], finalUV, 0.0f).g : material.Roughness;
    float ao = (material.AmbientOcclusionMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AmbientOcclusionMap], finalUV, 0.0f).r : material.AmbientOcclusion;
    vec3 emission = (material.EmissionMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.EmissionMap], finalUV, 0.0f).rgb : material.Emission;
    float height = (material.HeightMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.HeightMap], finalUV, 0.0f).r : 0.5f;
    float alpha = (material.AlphaMap != 0xFFFFFFFFu) ? textureLod(TextureMap[material.AlphaMap], finalUV, 0.0f).r : material.Alpha;
    
    if (alpha < 0.5f) discard;

    vec3 normalWS = normalize(TBN * normalTS);
    normalWS.xy *= material.NormalStrength;
    normalWS = normalize(normalWS);
    
    outPosition     = vec4(WorldPos, 1.0f);
    outAlbedo           = albedo;
    outNormalData           = vec4(normalWS * 0.5f + 0.5f, 1.0f);
    outPackedMRO       = vec4(metallic, roughness, ao, 1.0f);
    outPackedSheenSSS             = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    TempMap            = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    outParallaxInfo   = vec4(finalUV - TexCoords, height, 1.0f);
    outEmission         = vec4(emission, 1.0f);
}