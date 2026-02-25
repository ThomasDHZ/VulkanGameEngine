#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

const int BrdfMapBinding              = 1;
const int DirectionalShadowMapBinding = 2;
const int SDFShadowMapBinding         = 3;

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBloom;

layout(constant_id = 0)   const uint DescriptorBindingType0   = SubpassInputDescriptor;
layout(constant_id = 1)   const uint DescriptorBindingType1   = SubpassInputDescriptor;
layout(constant_id = 2)   const uint DescriptorBindingType2   = SubpassInputDescriptor;
layout(constant_id = 3)   const uint DescriptorBindingType3   = SubpassInputDescriptor;
layout(constant_id = 4)   const uint DescriptorBindingType4   = SubpassInputDescriptor;
layout(constant_id = 5)   const uint DescriptorBindingType5   = SubpassInputDescriptor;
layout(constant_id = 6)   const uint DescriptorBindingType6   = SubpassInputDescriptor;
layout(constant_id = 7)   const uint DescriptorBindingType7   = SubpassInputDescriptor;
layout(constant_id = 8)   const uint DescriptorBindingType8   = SubpassInputDescriptor;
layout(constant_id = 9)   const uint DescriptorBindingType9   = MemoryPoolDescriptor;
layout(constant_id = 10)  const uint DescriptorBindingType10  = TextureDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = BRDFDescriptor;
layout(constant_id = 12)  const uint DescriptorBindingType12  = SkyBoxDescriptor;
layout(constant_id = 13)  const uint DescriptorBindingType13  = IrradianceCubeMapDescriptor;
layout(constant_id = 14)  const uint DescriptorBindingType14  = PrefilterDescriptor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionInput;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput albedoInput;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput normalInput;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput packedMROInput;
layout(input_attachment_index = 4, binding = 4) uniform subpassInput packedSheenSSSInput;
layout(input_attachment_index = 5, binding = 5) uniform subpassInput tempInput;
layout(input_attachment_index = 6, binding = 6) uniform subpassInput parallaxUVInfoInput;
layout(input_attachment_index = 7, binding = 7) uniform subpassInput emissionInput;
layout(input_attachment_index = 8, binding = 8) uniform subpassInput depthInput;
layout(binding = 9)  buffer BindlessBuffer 
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
    uint64_t UnusedDataInShader1; //Used in CPU Side, Sprite Instance Data.
    uint64_t UnusedDataInShader2; //Used in CPU Side, Sprite Instance Data.
    uint Data[]; 
} bindlessBuffer;
layout(binding = 10) uniform sampler2D TextureMap[];
layout(binding = 11) uniform sampler2D BRDFMap;
layout(binding = 12) uniform samplerCube CubeMap;
layout(binding = 13) uniform samplerCube IrradianceMap;
layout(binding = 14) uniform samplerCube PrefilterMap;

layout(push_constant) uniform GBufferSceneDataBuffer
{
    int  Isolate;
    vec2  InvertResolution; 
    vec3  PerspectiveViewDirection;
    vec3  OrthographicCameraPosition;
    uint  DirectionalLightCount;
    uint  PointLightCount;
    mat4  InvProjection;
    mat4  InvView;
} gBufferSceneDataBuffer;

MeshProperitiesBuffer GetMesh(uint index) 
{
    MeshProperitiesBuffer mesh;
    if (index >= bindlessBuffer.MeshCount) 
    {
        mesh.MaterialIndex = 0u;
        mesh.MeshTransform = mat4(0.0);
        return mesh;
    }

    const uint baseByteLocation = (uint(bindlessBuffer.MeshOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.MeshSize / 4));
    mesh.MaterialIndex = bindlessBuffer.Data[baseByteLocation + 0u];
    mesh.MeshTransform = mat4(
        bindlessBuffer.Data[baseByteLocation + 1u],  bindlessBuffer.Data[baseByteLocation + 2u],  bindlessBuffer.Data[baseByteLocation + 3u],  bindlessBuffer.Data[baseByteLocation + 4u],
        bindlessBuffer.Data[baseByteLocation + 5u],  bindlessBuffer.Data[baseByteLocation + 6u],  bindlessBuffer.Data[baseByteLocation + 7u],  bindlessBuffer.Data[baseByteLocation + 8u],
        bindlessBuffer.Data[baseByteLocation + 9u],  bindlessBuffer.Data[baseByteLocation + 10u], bindlessBuffer.Data[baseByteLocation + 11u], bindlessBuffer.Data[baseByteLocation + 12u],
        bindlessBuffer.Data[baseByteLocation + 13u], bindlessBuffer.Data[baseByteLocation + 14u], bindlessBuffer.Data[baseByteLocation + 15u], bindlessBuffer.Data[baseByteLocation + 16u]);
    return mesh;
}

PackedMaterial GetMaterial(uint index)
{
    PackedMaterial mat;
    mat.AlbedoDataId          = ~0u;
    mat.NormalDataId          = ~0u;
    mat.PackedMRODataId       = ~0u;
    mat.PackedSheenSSSDataId  = ~0u;
    mat.UnusedDataId          = ~0u;
    mat.EmissionDataId        = ~0u;
    if (index >= bindlessBuffer.MaterialCount)
    {
        return mat;
    }

    const uint baseByteLocation = (uint(bindlessBuffer.MaterialOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.MaterialSize / 4));
    mat.AlbedoDataId          = bindlessBuffer.Data[baseByteLocation + 0u];
    mat.NormalDataId          = bindlessBuffer.Data[baseByteLocation + 1u];
    mat.PackedMRODataId       = bindlessBuffer.Data[baseByteLocation + 2u];
    mat.PackedSheenSSSDataId  = bindlessBuffer.Data[baseByteLocation + 3u];
    mat.UnusedDataId          = bindlessBuffer.Data[baseByteLocation + 4u];
    mat.EmissionDataId        = bindlessBuffer.Data[baseByteLocation + 5u];
    return mat;
}

DirectionalLightBuffer GetDirectionalLight(uint index) 
{
    DirectionalLightBuffer light;
    if (index >= bindlessBuffer.DirectionalLightCount) 
    {
        light.LightColor     = vec3(0.0);
        light.LightDirection = vec3(0.0);
        light.LightIntensity = 0.0;
        light.ShadowStrength = 0.0;
        light.ShadowBias     = 0.0;
        light.ShadowSoftness = 0.0;
        return light;
    }

    const uint baseByteLocation = (uint(bindlessBuffer.DirectionalLightOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.DirectionalLightSize / 4));
    light.LightColor     = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 0u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 1u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 2u]));
    light.LightDirection = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 3u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 4u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 5u]));
    light.LightIntensity = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 6u]);
    light.ShadowStrength = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 7u]);
    light.ShadowBias     = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 8u]);
    light.ShadowSoftness = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 9u]);
    return light;
}

PointLightBuffer GetPointLight(uint index) 
{
    PointLightBuffer light;
    if (index >= bindlessBuffer.PointLightCount) 
    {
        light.LightPosition  = vec3(0.0);
        light.LightColor     = vec3(0.0);
        light.LightRadius    = 0.0;
        light.LightIntensity = 0.0;
        light.ShadowStrength = 0.0;
        light.ShadowBias     = 0.0;
        light.ShadowSoftness = 0.0;
        return light;
    }

    const uint baseByteLocation = (uint(bindlessBuffer.PointLightOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.PointLightSize / 4));
    light.LightPosition  = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 0u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 1u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 2u]));
    light.LightColor     = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 3u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 4u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 5u]));
    light.LightRadius    = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 6u]);
    light.LightIntensity = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 7u]);
    light.ShadowStrength = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 8u]);
    light.ShadowBias     = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 9u]);
    light.ShadowSoftness = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 10u]);
    return light;
}

TextureMetadata Get2DTextureMetadata(uint index)
{
    TextureMetadata textureMetaData;
    if (index >= bindlessBuffer.Texture2DCount) 
    {
        textureMetaData.Width       = 0;
        textureMetaData.Height      = 0;
        textureMetaData.Depth       = 0;
        textureMetaData.MipLevels   = 1;
        textureMetaData.LayerCount  = 1;
        textureMetaData.Format      = 0;
        textureMetaData.TextureType = 0;
        textureMetaData.ArrayIndex  = 0;
        return textureMetaData;
    }

    const uint baseByteLocation = (uint(bindlessBuffer.Texture2DOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.Texture2DSize / 4));
    textureMetaData.Width       = bindlessBuffer.Data[baseByteLocation + 0u];
    textureMetaData.Height      = bindlessBuffer.Data[baseByteLocation + 1u];
    textureMetaData.Depth       = bindlessBuffer.Data[baseByteLocation + 2u];
    textureMetaData.MipLevels   = bindlessBuffer.Data[baseByteLocation + 3u];
    textureMetaData.LayerCount  = bindlessBuffer.Data[baseByteLocation + 4u];
    textureMetaData.Format      = bindlessBuffer.Data[baseByteLocation + 5u];
    textureMetaData.TextureType = bindlessBuffer.Data[baseByteLocation + 6u];
    textureMetaData.ArrayIndex  = bindlessBuffer.Data[baseByteLocation + 7u];
    return textureMetaData;
}

TextureMetadata Get3DTextureMetadata(uint index)
{
    TextureMetadata textureMetaData;
    if (index >= bindlessBuffer.Texture3DCount) 
    {
        textureMetaData.Width       = 0;
        textureMetaData.Height      = 0;
        textureMetaData.Depth       = 0;
        textureMetaData.MipLevels   = 1;
        textureMetaData.LayerCount  = 1;
        textureMetaData.Format      = 0;
        textureMetaData.TextureType = 0;
        textureMetaData.ArrayIndex  = 0;
        return textureMetaData;
    }

    const uint baseByteLocation = (uint(bindlessBuffer.Texture3DOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.Texture3DSize / 4));
    textureMetaData.Width       = bindlessBuffer.Data[baseByteLocation + 0u];
    textureMetaData.Height      = bindlessBuffer.Data[baseByteLocation + 1u];
    textureMetaData.Depth       = bindlessBuffer.Data[baseByteLocation + 2u];
    textureMetaData.MipLevels   = bindlessBuffer.Data[baseByteLocation + 3u];
    textureMetaData.LayerCount  = bindlessBuffer.Data[baseByteLocation + 4u];
    textureMetaData.Format      = bindlessBuffer.Data[baseByteLocation + 5u];
    textureMetaData.TextureType = bindlessBuffer.Data[baseByteLocation + 6u];
    textureMetaData.ArrayIndex  = bindlessBuffer.Data[baseByteLocation + 7u];
    return textureMetaData;
}

TextureMetadata GetCubeMapTextureMetadata(uint index)
{
    TextureMetadata textureMetaData;
    if (index >= bindlessBuffer.TextureCubeMapCount) 
    {
        textureMetaData.Width       = 0;
        textureMetaData.Height      = 0;
        textureMetaData.Depth       = 0;
        textureMetaData.MipLevels   = 1;
        textureMetaData.LayerCount  = 1;
        textureMetaData.Format      = 0;
        textureMetaData.TextureType = 0;
        textureMetaData.ArrayIndex  = 0;
        return textureMetaData;
    }

    const uint baseByteLocation = (uint(bindlessBuffer.TextureCubeMapOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.TextureCubeMapSize / 4));
    textureMetaData.Width       = bindlessBuffer.Data[baseByteLocation + 0u];
    textureMetaData.Height      = bindlessBuffer.Data[baseByteLocation + 1u];
    textureMetaData.Depth       = bindlessBuffer.Data[baseByteLocation + 2u];
    textureMetaData.MipLevels   = bindlessBuffer.Data[baseByteLocation + 3u];
    textureMetaData.LayerCount  = bindlessBuffer.Data[baseByteLocation + 4u];
    textureMetaData.Format      = bindlessBuffer.Data[baseByteLocation + 5u];
    textureMetaData.TextureType = bindlessBuffer.Data[baseByteLocation + 6u];
    textureMetaData.ArrayIndex  = bindlessBuffer.Data[baseByteLocation + 7u];
    return textureMetaData;
}

vec4 SampleTexture(uint textureIndex, vec2 uv)
{
    TextureMetadata meta = Get2DTextureMetadata(textureIndex);

    if (meta.TextureType == 0) // 2D
    {
        return texture(TextureMap[meta.ArrayIndex], uv);
    }
    return vec4(1.0, 0.0, 1.0, 1.0); // error pink
}

vec2 Unpack8bitPair(float packed) {
    uint combined = uint(packed * 65535.0 + 0.5);
    float high = float((combined >> 8) & 0xFFu) / 255.0;
    float low  = float(combined & 0xFFu) / 255.0;
    return vec2(high, low);
}

vec3 OctahedronDecode(vec2 f)
{
    vec3 n;
    n.xy = f.xy;
    n.z = 1.0 - abs(f.x) - abs(f.y);
    n.xy = (n.z < 0.0) ? (1.0 - abs(n.yx)) * sign(n.xy) : n.xy;
    return normalize(n);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f)) + 1.0f;
    denom = PI * denom * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0f - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0f, 1.0f), 5.0f);
}

float SchlickWeight(float cosTheta) {
    return pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

mat3 ReconstructTBN(vec3 normalWS)
{
    vec3 N = normalWS;
    vec3 T = normalize(vec3(1.0f, 0.0f, 0.0f));
    vec3 B = normalize(cross(N, T));
    T = normalize(cross(B, N));
    return mat3(T, B, N);
}

vec3 SampleSkyboxViewDependent(vec3 viewDirWS)
{
    vec3 skyDir = reflect(viewDirWS, vec3(0,1,0));
    skyDir.y = max(skyDir.y, 0.1);
    
    float lod = mix(2.0, 6.0, abs(skyDir.y)); 
    return textureLod(CubeMap, skyDir, lod).rgb;
}

float DisneyDiffuse(float NdotV, float NdotL, float LdotH, float roughness) {
    float fd90 = 0.5 + 2.0 * roughness * LdotH * LdotH;
    float lightScatter = (1.0 + (fd90 - 1.0) * pow(1.0 - NdotL, 5.0));
    float viewScatter  = (1.0 + (fd90 - 1.0) * pow(1.0 - NdotV, 5.0));
    return (lightScatter * viewScatter) / PI;
}

Material UnpackMaterial();
vec3 DirectionalLightFunc(vec3 F0, vec3 V, vec3 R, vec2 finalUV, Material material);
vec3 PointLightFunc(vec3 F0, vec3 V, vec3 R, vec2 finalUV, Material material);
vec3 ImageBasedLighting(vec3 F0, vec3 V, vec3 R, Material material);

float DirectionalSelfShadow(vec2 finalUV, vec3 normalWS, uint lightIndex, float currentHeight)
{
    if (currentHeight < 0.001f) return 1.0f;

    const DirectionalLightBuffer light = GetDirectionalLight(lightIndex);
    mat3 worldToTangent = transpose(ReconstructTBN(normalWS));
    vec3 lightDirWS = normalize(light.LightDirection);
    vec3 lightDirTS = normalize(worldToTangent * lightDirWS);

    if (dot(lightDirTS, vec3(0,0,1)) < 0.1f) return 0.5f;  // softer backface

    const int maxSteps = 48;
    const float stepSize = 0.04f;
    float shadow = 1.0f;
    vec2 marchUV = finalUV;
    vec2 deltaUV = lightDirTS.xy * stepSize;
    float bias = light.ShadowBias * 0.5f;
    float rayHeight = currentHeight;

    for (int x = 0; x < maxSteps; ++x)
    {
        marchUV += deltaUV;
        rayHeight += stepSize;
        if (rayHeight > currentHeight + bias)
        {
            shadow = mix(0.3f, 1.0f, float(x) / float(maxSteps));  // soft falloff
            break;
        }
    }
    return shadow;
}

float PointSelfShadow(vec2 finalUV, vec3 lightDirTS, uint lightIndex, float currentHeight)
{
    if (currentHeight < 0.001f) return 1.0f;

    const PointLightBuffer light = GetPointLight(lightIndex);
    float NdotLTS = dot(lightDirTS, vec3(0,0,1));
    if (NdotLTS < 0.1f) return 0.6f;

    const int maxSteps = 32;
    const float stepSize = 0.04f;
    float shadow = 1.0f;
    vec2 marchUV = finalUV;
    vec2 deltaUV = lightDirTS.xy * stepSize;
    float bias = 0.02f;
    float rayHeight = currentHeight;

    for (int x = 0; x < maxSteps; ++x)
    {
        marchUV += deltaUV;
        if (any(lessThan(marchUV, vec2(0.0))) || any(greaterThan(marchUV, vec2(1.0)))) break;

        rayHeight += stepSize;
        if (rayHeight > currentHeight + bias)
        {
            shadow = mix(0.4f, 1.0f, float(x) / float(maxSteps));
            break;
        }
    }
    return shadow;
}

void main()
{

    const float depth = subpassLoad(depthInput).r;
    if (depth >= 0.9999f) {
        vec3 ndc = vec3(gl_FragCoord.xy * gBufferSceneDataBuffer.InvertResolution * 2.0 - 1.0, 1.0);
        vec4 viewPos = gBufferSceneDataBuffer.InvProjection * vec4(ndc, 1.0);
        viewPos /= viewPos.w;
        vec3 viewDir = normalize(viewPos.xyz);
        vec3 worldDir = normalize((gBufferSceneDataBuffer.InvView * vec4(viewDir, 0.0)).xyz);
        vec3 sky = textureLod(CubeMap, worldDir, 0.0).rgb;

        outColor = vec4(sky, 1.0);
        outBloom = vec4(0.0);
        return;
    }

    Material material = UnpackMaterial();
    vec3 N = material.Normal;
    float clearcoatStrength   = 0.0;
    float clearcoatRoughness  = 0.05;

    vec2 parallaxOffset = material.ParallaxInfo.xy;
    vec2 screenUV = gl_FragCoord.xy * gBufferSceneDataBuffer.InvertResolution;
    vec2 finalUV = screenUV + material.ParallaxInfo.xy;  

    // View reconstruction
    vec2 ndc = screenUV * 2.0 - 1.0;
    vec4 clip = vec4(ndc, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = gBufferSceneDataBuffer.InvProjection * clip;
    viewPos /= viewPos.w;
    vec3 V = normalize(-viewPos.xyz);

    vec3 R = reflect(-V, N);
    vec3 F0 = mix(vec3(0.04), material.Albedo, material.Metallic);

    vec3 Lo = vec3(0.0);
    Lo += DirectionalLightFunc(F0, V, R, finalUV, material);
    Lo += PointLightFunc(F0, V, R, finalUV, material);
    vec3 ambient = ImageBasedLighting(F0, V, R, material);

    vec3  color = ambient + Lo;
    outColor = vec4(color, 1.0);

    vec3  bloomColor = max(vec3(0.0f), color - vec3(1.0f));
    outBloom = vec4(material.Emission.rgb, 1.0f);
} 

Material UnpackMaterial()
{
    Material material;
    const vec4 packedMRO = subpassLoad(packedMROInput);
    const vec4 packedSheenSSS = subpassLoad(packedSheenSSSInput);

    const vec2 unpackMRO_Metallic_Rough                        = Unpack8bitPair(packedMRO.r);
    const vec2 unpackMRO_AO_ClearCoatTint                      = Unpack8bitPair(packedMRO.g);
    const vec2 unpackMRO_ClearCoatStrength_ClearCoatRoughness  = Unpack8bitPair(packedMRO.b);

    const vec2 SheenSSS_SheenColorR_SheenColorG                = Unpack8bitPair(packedSheenSSS.r);
    const vec2 SheenSSS_SheenColorB_SheenIntensity             = Unpack8bitPair(packedSheenSSS.g);
    const vec2 SheenSSS_SSSR_SSSG                              = Unpack8bitPair(packedSheenSSS.b);
    const vec2 SheenSSS_SSSB_Thickness                         = Unpack8bitPair(packedSheenSSS.a);

    material.Position            = subpassLoad(positionInput).rgb;
    material.Albedo              = subpassLoad(albedoInput).rgb;
    material.Normal              = subpassLoad(normalInput).rgb;
    material.ParallaxInfo        = subpassLoad(parallaxUVInfoInput).rgb;
    material.Emission            = subpassLoad(emissionInput).rgb;
    material.Height              = subpassLoad(normalInput).a;

    material.Metallic = unpackMRO_Metallic_Rough.x;
    material.Roughness = unpackMRO_Metallic_Rough.y;
    material.AmbientOcclusion = unpackMRO_AO_ClearCoatTint.x;
    material.ClearCoatTint = unpackMRO_AO_ClearCoatTint.y;
    material.ClearcoatStrength = unpackMRO_ClearCoatStrength_ClearCoatRoughness.x;
    material.ClearcoatRoughness = unpackMRO_ClearCoatStrength_ClearCoatRoughness.y;
    material.Sheen = vec3(SheenSSS_SheenColorR_SheenColorG.x, SheenSSS_SheenColorR_SheenColorG.y, SheenSSS_SheenColorB_SheenIntensity.x);
    material.SheenIntensity = SheenSSS_SheenColorB_SheenIntensity.y;
    material.SubSurfaceScattering = vec3(SheenSSS_SSSR_SSSG.x, SheenSSS_SSSR_SSSG.y, SheenSSS_SSSB_Thickness.x);
    material.Thickness = SheenSSS_SSSB_Thickness.y;
    material.ShiftedHeight = material.ParallaxInfo.z;

    vec3 N = OctahedronDecode(material.Normal.xy * 2.0f - 1.0f);
    material.Normal = normalize(N);

    return material;
}

vec3 DirectionalLightFunc(vec3 F0, vec3 V, vec3 R, vec2 finalUV, Material material)
{
    float clearcoatStrength   = 0.0;
    float clearcoatRoughness  = 0.05;

    vec3 Lo = vec3(0.0f);
    for (uint x = 0; x < gBufferSceneDataBuffer.DirectionalLightCount; ++x)
    {
        const DirectionalLightBuffer light = GetDirectionalLight(x);

        vec3 L = normalize(light.LightDirection);
        vec3 H = normalize(V + L);

        float NdotV = max(dot(material.Normal, V), 0.0f);
        float NdotL = max(dot(material.Normal, L), 0.0f);
        float LdotH = max(dot(L, H), 0.0f);

        if (NdotL <= 0.0f) continue;

        float selfShadow = DirectionalSelfShadow(finalUV, material.Normal, int(x), material.ShiftedHeight);
        float microShadow = NdotL;
        float combinedShadow = selfShadow * (0.4f + 0.6f * microShadow);
        vec3 radiance = light.LightColor * light.LightIntensity * selfShadow;

        float NDF = DistributionGGX(material.Normal, H, material.Roughness);
        float G = GeometrySmith(material.Normal, V, L, material.Roughness);
        vec3 F = fresnelSchlickRoughness(NdotV, F0, material.Roughness);
        vec3 specular = (NDF * G * F) / max(4.0f * NdotV * NdotL, 0.0001f);

        vec3 coatH = normalize(V + L);
        float coatNdotL = max(dot(material.Normal, L), 0.0f);
        float coatNdotV = max(dot(material.Normal, V), 0.0f);
        float coatNdotH = max(dot(material.Normal, coatH), 0.0f);

        float coatD = DistributionGGX(material.Normal, coatH, clearcoatStrength);
        float coatG = GeometrySmith(material.Normal, V, L, clearcoatStrength);
        float coatF = 0.04 + (1.0 - 0.04) * pow(1.0 - max(dot(coatH, V), 0.0), 5.0);

        float  coatSpec = (coatD * coatG * coatF) / max(4.0 * coatNdotV * coatNdotL, 0.0001f);
        vec3  clearcoatContrib = vec3(coatSpec) * clearcoatStrength;

        float disneyDiff = DisneyDiffuse(NdotV, NdotL, LdotH, material.Roughness);
        vec3  diffuse = material.Albedo * disneyDiff;

        float subsurfaceStrength = 0.7f;
        float subsurfaceWrap = 0.5f;
        float NdotL_wrap = max(NdotL + subsurfaceWrap, 0.0) / (1.0 + subsurfaceWrap);
        vec3  sssColor = material.Albedo * material.SubSurfaceScattering;
        vec3  sssContrib = sssColor * subsurfaceStrength * NdotL_wrap;

        vec3  baseDiffuse = mix(diffuse, sssContrib, subsurfaceStrength) * (1.0 - material.Metallic);

        float sheenIntensity = 0.4f;
        vec3  sheenColor = mix(material.Sheen, material.Albedo, 0.5);
        float sheenFactor = pow(1.0 - NdotV, 5.0);
        vec3  sheenContrib = sheenColor * material.SheenIntensity * sheenFactor;

        Lo += (diffuse + specular + clearcoatContrib + sheenContrib + sssContrib) * radiance * NdotL;
    }
    return Lo;
}

vec3 PointLightFunc(vec3 F0, vec3 V, vec3 R, vec2 finalUV, Material material)
{
    float clearcoatStrength   = 0.0;
    float clearcoatRoughness  = 0.05;

    vec3 Lo = vec3(0.0f);
    for (uint x = 0; x < gBufferSceneDataBuffer.PointLightCount; ++x)
    {
        const PointLightBuffer light = GetPointLight(x);

        vec3 toLight = light.LightPosition - material.Position;
        float distance = length(toLight);
        if (distance > light.LightRadius) continue;

        vec3 L = normalize(toLight);
        vec3 H = normalize(V + L);
        float NdotL = max(dot(material.Normal, L), 0.0f);
        if (NdotL <= 0.0f) continue;

        float attenuation = 1.0f - (distance / light.LightRadius);
        attenuation = max(attenuation, 0.0f);
        attenuation *= attenuation;

        vec3 lightDirTS = normalize(transpose(ReconstructTBN(material.Normal)) * L);
        float pomSelfShadow = PointSelfShadow(finalUV, lightDirTS, x, material.ShiftedHeight);
        
        float distanceFactor = 1.0 - (distance / light.LightRadius);
        float softFactor = smoothstep(0.0, 0.2, distanceFactor);

        float combinedShadow = pomSelfShadow * softFactor;
        vec3 radiance = light.LightColor * light.LightIntensity * attenuation;

        float NDF = DistributionGGX(material.Normal, H, material.Roughness);
        float G = GeometrySmith(material.Normal, V, L, material.Roughness);
        vec3 F = fresnelSchlickRoughness(max(dot(H, V), 0.0f), F0, material.Roughness);
        vec3 specular = (NDF * G * F) / max(4.0f * max(dot(material.Normal, V), 0.0f) * NdotL, 0.0001f);
        vec3 kS = F;
        vec3 kD = (vec3(1.0f) - kS) * (1.0f - material.Metallic);

        Lo += (kD * material.Albedo / PI + specular) * radiance * NdotL;
    }
    return Lo;
}
      
vec3 ImageBasedLighting(vec3 F0, vec3 V, vec3 R, Material material)
{
    float clearcoatStrength   = 0.0;
    float clearcoatRoughness  = 0.05;

    vec3  N = material.Normal;
    vec3  sheenColor = mix(material.Sheen, material.Albedo, 0.5);
    float sheenIntensity = 0.4f;

    vec3  F = fresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, material.Roughness);
    vec3  kS = F;
    vec3  kD = (vec3(1.0f) - kS) * (1.0f - material.Metallic);

    vec3  irradiance = texture(IrradianceMap, N).rgb;
    vec3  diffuseIBL = (material.Albedo / PI) * irradiance;

    float maxLod = textureQueryLevels(PrefilterMap) - 1.0;
    float lod = material.Roughness * maxLod;
    lod = clamp(lod, 0.0, maxLod);
    vec3  prefilteredColor = textureLod(PrefilterMap, R, lod).rgb;

    vec2  brdf = texture(BRDFMap, vec2(max(dot(N, V), 0.0f), material.Roughness)).rg;
    vec3  specularIBL = prefilteredColor * (F * brdf.x + brdf.y);

    float iblSheenFactor = pow(1.0 - max(dot(N, V), 0.0f), 5.0);
    vec3  iblSheenContrib = sheenColor * sheenIntensity * iblSheenFactor * irradiance;

    float subsurfaceStrength = 0.7f;
    vec3  iblSSSContrib = subsurfaceStrength * irradiance * (material.Albedo * material.SubSurfaceScattering);

    float coatNdotV = max(dot(N, V), 0.0f);
    vec3  coatR = reflect(-V, N);
    float coatLod = clearcoatRoughness * (textureQueryLevels(PrefilterMap) - 1.0);
    coatLod = clamp(coatLod, 0.0, textureQueryLevels(PrefilterMap) - 1.0);

    vec3  coatPrefilter = textureLod(PrefilterMap, coatR, coatLod).rgb;

    vec2  coatBRDF = texture(TextureMap[BrdfMapBinding], vec2(coatNdotV, clearcoatRoughness)).rg;
    float coatF = 0.04 + (1.0 - 0.04) * pow(1.0 - coatNdotV, 5.0);

    vec3  clearcoatIBL = coatPrefilter * (coatF * coatBRDF.x + coatBRDF.y) * clearcoatStrength;

    vec3  ambient = (kD * (diffuseIBL + iblSSSContrib) + specularIBL + iblSheenContrib) * material.AmbientOcclusion + clearcoatIBL * material.AmbientOcclusion;
    return max(ambient, vec3(0.02) * material.Albedo);
}