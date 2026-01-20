#version 460
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable

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
layout(constant_id = 9)   const uint DescriptorBindingType9   = SubpassInputDescriptor;
layout(constant_id = 10)  const uint DescriptorBindingType10  = MeshPropertiesDescriptor;
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
layout(input_attachment_index = 3, binding = 3) uniform subpassInput matRoughAOInput;
layout(input_attachment_index = 4, binding = 4) uniform subpassInput parallaxUVInfoInput;
layout(input_attachment_index = 5, binding = 5) uniform subpassInput emissionInput;
layout(input_attachment_index = 6, binding = 6) uniform subpassInput tempInput;
layout(input_attachment_index = 7, binding = 7) uniform subpassInput tempInput2;
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

layout(push_constant) uniform GBufferSceneDataBuffer
{
    vec2  InvertResolution;
    vec3  ViewDirection;
    uint  DirectionalLightCount;
    uint  PointLightCount;
    mat4  InvProjection;
    mat4  InvView;
} gBufferSceneDataBuffer;

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

float DirectionalSelfShadow(vec2 finalUV, vec3 normalWS, int lightIndex)
{
    vec4 parallaxInfo = subpassLoad(parallaxUVInfoInput);
    float currentHeight = parallaxInfo.z;

    if (currentHeight < 0.001f) return 1.0f;

    mat3 worldToTangent = transpose(ReconstructTBN(normalWS));
    vec3 lightDirWS = normalize(directionalLightBuffer[lightIndex].directionalLightProperties.LightDirection);
    vec3 lightDirTS = normalize(worldToTangent * lightDirWS);

    if (dot(lightDirTS, vec3(0,0,1)) < 0.05f) return 0.4f;

    const int maxSteps = 32;
    const float stepSize = 0.05f;
    float shadow = 1.0f;
    vec2 marchUV = finalUV;
    vec2 deltaUV = lightDirTS.xy * stepSize;
    float bias = directionalLightBuffer[lightIndex].directionalLightProperties.ShadowBias * 0.4f;

    float rayHeight = currentHeight;

    for (int x = 0; x < maxSteps; ++x)
    {
        marchUV += deltaUV;
        rayHeight += stepSize;

        if (rayHeight > currentHeight + bias * 0.3f)
        {
            shadow *= 0.3f + 0.1f * float(x) / float(maxSteps);
            break;
        }
    }
    return shadow;
}

float PointSelfShadow(vec2 finalUV, vec3 lightDirTS, float currentHeight)
{
    if (currentHeight < 0.001f) return 1.0f;

    float NdotLTS = dot(lightDirTS, vec3(0,0,1));
    if (NdotLTS < 0.08f) return 0.5f; // softer threshold for point lights

    const int maxSteps = 24;
    const float stepSize = 0.035f;
    const float startOffset = -0.03f;
    const float shadowBias = 0.015f;

    float shadow = 1.0f;
    vec2 marchUV = finalUV;
    vec2 deltaUV = lightDirTS.xy * stepSize;

    float rayHeight = currentHeight + startOffset;

    for (int x = 0; x < maxSteps; ++x)
    {
        marchUV += deltaUV;
        if (any(lessThan(marchUV, vec2(0.0))) || any(greaterThan(marchUV, vec2(1.0))))
            break;

        rayHeight += stepSize;

        if (rayHeight > currentHeight + shadowBias)
        {
            float falloff = float(x) / float(maxSteps);
            shadow *= 0.45f + 0.55f * (1.0f - falloff);
            break;
        }
    }

    return clamp(shadow, 0.0f, 1.0f);
}

float DisneyDiffuse(float NdotV, float NdotL, float LdotH, float roughness) {
    float fd90 = 0.5 + 2.0 * roughness * LdotH * LdotH;
    float lightScatter = (1.0 + (fd90 - 1.0) * pow(1.0 - NdotL, 5.0));
    float viewScatter  = (1.0 + (fd90 - 1.0) * pow(1.0 - NdotV, 5.0));
    return (lightScatter * viewScatter) / PI;
}

void main()
{
    vec3 V = normalize(gBufferSceneDataBuffer.ViewDirection);

    float depth = subpassLoad(depthInput).r;
    if (depth >= 0.99995f)
    {
        outColor = vec4(subpassLoad(skyBoxInput).rgb, 1.0);
        return;
    }

    vec4 parallaxInfo = subpassLoad(parallaxUVInfoInput);
    vec2 parallaxOffset = parallaxInfo.xy;
    float shiftedHeight = parallaxInfo.z;  

    vec2 screenUV = gl_FragCoord.xy * gBufferSceneDataBuffer.InvertResolution;
    vec2 finalUV = screenUV + parallaxOffset;

    vec3 position = subpassLoad(positionInput).rgb;
    vec3 albedo = subpassLoad(albedoInput).rgb;
    float metallic = subpassLoad(matRoughAOInput).r;
    float roughness = subpassLoad(matRoughAOInput).g;
    float ambientOcclusion = subpassLoad(matRoughAOInput).b;
    vec3 emission = subpassLoad(emissionInput).rgb;

    float clearcoatStrength   = 0.0;  // 0.0–0.5 typical (how visible the coat is)
    float clearcoatRoughness  = 0.05;

    vec3 normal = subpassLoad(normalInput).rgb * 2.0f - 1.0f;
    vec3 N = normalize(normal);

    mat3 TBN = ReconstructTBN(N);
    vec3 R = reflect(-V, N);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);    // Standard dielectric

    vec3 Lo = vec3(0.0f);
    for (uint x = 0; x < gBufferSceneDataBuffer.DirectionalLightCount; ++x)
    {
        const DirectionalLightBuffer light = directionalLightBuffer[x].directionalLightProperties;
        vec3 L = normalize(light.LightDirection);
        vec3 H = normalize(V + L);

        float NdotV = max(dot(N, V), 0.0f);
        float NdotL = max(dot(N, L), 0.0f);
        float LdotH = max(dot(L, H), 0.0f);

        if (NdotL <= 0.0f) continue;

        float selfShadow = DirectionalSelfShadow(finalUV, N, int(x));
        float microShadow = NdotL;
        float combinedShadow = selfShadow * (0.4f + 0.6f * microShadow);
        vec3 radiance = light.LightColor * light.LightIntensity * combinedShadow;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
        vec3 specular = (NDF * G * F) / max(4.0f * NdotV * NdotL, 0.0001f);

        vec3 coatH = normalize(V + L);
        float coatNdotL = max(dot(N, L), 0.0f);
        float coatNdotV = max(dot(N, V), 0.0f);
        float coatNdotH = max(dot(N, coatH), 0.0f);

        float coatD = DistributionGGX(N, coatH, clearcoatRoughness);
        float coatG = GeometrySmith(N, V, L, clearcoatRoughness);
        float coatF = 0.04 + (1.0 - 0.04) * pow(1.0 - max(dot(coatH, V), 0.0), 5.0);

        float  coatSpec = (coatD * coatG * coatF) / max(4.0 * coatNdotV * coatNdotL, 0.0001f);
        vec3  clearcoatContrib = vec3(coatSpec) * clearcoatStrength;

        float disneyDiff = DisneyDiffuse(NdotV, NdotL, LdotH, roughness);
        vec3  diffuse = albedo * disneyDiff;

        float subsurfaceStrength = 0.7f;
        float subsurfaceWrap = 0.5f;
        float NdotL_wrap = max(NdotL + subsurfaceWrap, 0.0) / (1.0 + subsurfaceWrap);
        vec3  sssColor = albedo * vec3(1.0, 0.7, 0.6);
        vec3  sssContrib = sssColor * subsurfaceStrength * NdotL_wrap;

        vec3  baseDiffuse = mix(diffuse, sssContrib, subsurfaceStrength) * (1.0 - metallic);

        float sheenIntensity = 0.4f;
        vec3  sheenColor = mix(vec3(1.0), albedo, 0.5);
        float sheenFactor = pow(1.0 - NdotV, 5.0);
        vec3  sheenContrib = sheenColor * sheenIntensity * sheenFactor;

        Lo += (baseDiffuse + specular + sheenContrib) * radiance * NdotL;
        Lo += clearcoatContrib * radiance * combinedShadow * coatNdotL;
    }
    for (uint x = 0; x < gBufferSceneDataBuffer.PointLightCount; ++x)
    {
        const PointLightBuffer light = pointLightBuffer[x].pointLightProperties;
        vec3 toLight = light.LightPosition - position;
        float distance = length(toLight);
        if (distance > light.LightRadius) continue;

        vec3 L = normalize(toLight);
        vec3 H = normalize(V + L);
        float NdotL = max(dot(N, L), 0.0f);
        if (NdotL <= 0.0f) continue;

        float attenuation = 1.0f - (distance / light.LightRadius);
        attenuation = max(attenuation, 0.0f);
        attenuation *= attenuation;

        vec3 lightDirTS = normalize(transpose(ReconstructTBN(N)) * L);
        float pomSelfShadow = PointSelfShadow(finalUV, lightDirTS, shiftedHeight);
        
        float distanceFactor = 1.0 - (distance / light.LightRadius);
        float softFactor = smoothstep(0.0, 0.2, distanceFactor);

        float combinedShadow = pomSelfShadow * softFactor;
        vec3 radiance = light.LightColor * light.LightIntensity * attenuation * combinedShadow;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlickRoughness(max(dot(H, V), 0.0f), F0, roughness);
        vec3 specular = (NDF * G * F) / max(4.0f * max(dot(N, V), 0.0f) * NdotL, 0.0001f);
        vec3 kS = F;
        vec3 kD = (vec3(1.0f) - kS) * (1.0f - metallic);

       // Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3  sheenColor = mix(vec3(1.0), albedo, 0.5);
    float sheenIntensity = 0.4f;

    vec3  F = fresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughness);
    vec3  kS = F;
    vec3  kD = (vec3(1.0f) - kS) * (1.0f - metallic);

    vec3  irradiance = texture(IrradianceMap, N).rgb;
    vec3  diffuseIBL = (albedo / PI) * irradiance;

    float maxLod = textureQueryLevels(PrefilterMap) - 1.0;
    float lod = roughness * maxLod;
    lod = clamp(lod, 0.0, maxLod);
    vec3  prefilteredColor = textureLod(PrefilterMap, R, lod).rgb;

    vec2  brdf = texture(TextureMap[BrdfMapBinding], vec2(max(dot(N, V), 0.0f), roughness)).rg;
    vec3  specularIBL = prefilteredColor * (F * brdf.x + brdf.y);

    float iblSheenFactor = pow(1.0 - max(dot(N, V), 0.0f), 5.0);
    vec3  iblSheenContrib = sheenColor * sheenIntensity * iblSheenFactor * irradiance;

    float subsurfaceStrength = 0.7f;
    vec3  iblSSSContrib = subsurfaceStrength * irradiance * (albedo * vec3(1.0, 0.7, 0.6));

    float coatNdotV = max(dot(N, V), 0.0f);
    vec3  coatR = reflect(-V, N);
    float coatLod = clearcoatRoughness * (textureQueryLevels(PrefilterMap) - 1.0);
    coatLod = clamp(coatLod, 0.0, textureQueryLevels(PrefilterMap) - 1.0);

    vec3  coatPrefilter = textureLod(PrefilterMap, coatR, coatLod).rgb;

    vec2  coatBRDF = texture(TextureMap[BrdfMapBinding], vec2(coatNdotV, clearcoatRoughness)).rg;
    float coatF = 0.04 + (1.0 - 0.04) * pow(1.0 - coatNdotV, 5.0);

    vec3  clearcoatIBL = coatPrefilter * (coatF * coatBRDF.x + coatBRDF.y) * clearcoatStrength;

    vec3  ambient = emission + (kD * (diffuseIBL + iblSSSContrib) + specularIBL + iblSheenContrib) * ambientOcclusion + clearcoatIBL * ambientOcclusion;
    ambient = max(ambient, vec3(0.02) * albedo);

    vec3  color = ambient + Lo;
    outColor = vec4(color, 1.0f);

    vec3  bloomColor = max(vec3(0.0f), color - vec3(1.0f));
    outBloom = vec4(bloomColor, 1.0f);
}