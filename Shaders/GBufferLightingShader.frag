#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 1;
layout(constant_id = 1) const uint DescriptorBindingType1 = 3;
layout(constant_id = 2) const uint DescriptorBindingType2 = 4;

layout(location = 0) in  vec2 TexCoords;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBloom;

//layout(location = 0) in vec4 PositionDataMap;
//layout(location = 1) in vec4 AlbedoMap;
//layout(location = 2) in vec4 NormalMap;
//layout(location = 3) in vec4 MatRoughAOHeightMap;
//layout(location = 4) in vec4 EmissionMap;
//layout(location = 4) in vec4 SDFMap;

layout(push_constant) uniform GBufferSceneDataBuffer
{
	uint DirectionalLightCount;
    uint PointLightCount;
}gBufferSceneDataBuffer;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0)) + 1.0;
    denom = (3.1415927410125732421875 * denom) * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float nom = NdotV;
    float denom = (NdotV * (1.0 - k)) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float param = NdotV;
    float param_1 = roughness;
    float ggx2 = GeometrySchlickGGX(param, param_1);
    float param_2 = NdotL;
    float param_3 = roughness;
    float ggx1 = GeometrySchlickGGX(param_2, param_3);
    return ggx1 * ggx2;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + ((max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0));
}

mat3 TBN = mat3(
    vec3(1.0, 0.0, 0.0),  // Tangent  (X)
    vec3(0.0, 1.0, 0.0),  // Bitangent (Y)
    vec3(0.0, 0.0, 1.0)   // Normal    (Z)
);

#include "Lights.glsl"

layout(binding = 0) uniform sampler2D TextureMap[];
layout(binding = 1) buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 2) buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
void main()
{
    vec3 positionDataMap = texture(TextureMap[0], TexCoords).rgb;
    vec3 albedoMap = texture(TextureMap[1], TexCoords).rgb;
    vec3 normalMap = texture(TextureMap[2], TexCoords).rgb * 2.0f - 1.0f;
    float metallicMap = 0.0f;//texture(TextureMap[3], TexCoords).r;
    float roughnessMap = 0.5f;//texture(TextureMap[3], TexCoords).g;
    float ambientOcclusionMap = texture(TextureMap[3], TexCoords).b;
    float heightMap = texture(TextureMap[3], TexCoords).a;
    vec3 emissionMap = texture(TextureMap[4], TexCoords).rgb;
    float specularMap = texture(TextureMap[4], TexCoords).a;

    vec3 N = normalize(normalMap);
    vec3 V = normalize(vec3(0.3f, 0.3f, 1.0f)); 
    
    vec3 Lo = vec3(0.0f); 
    vec3 F0 = vec3(0.04f);
    F0 = mix(F0, albedoMap, metallicMap);
    for(int x = 0; x < gBufferSceneDataBuffer.DirectionalLightCount; x++)
    {
        const DirectionalLightBuffer directionalLight = directionalLightBuffer[x].directionalLightProperties;
        vec3 L = normalize(directionalLight.LightDirection);
        vec3 H = normalize(V + L);
        vec3 radiance = directionalLight.LightColor * directionalLight.LightIntensity;

        float NDF = DistributionGGX(N, H, roughnessMap);
        float G   = GeometrySmith(N, V, L, roughnessMap);
        vec3  F   = fresnelSchlickRoughness(max(dot(H, V), 0.0f), F0, roughnessMap);
        vec3 specular = (NDF * G * F) / (4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f);

        vec3 kS = F;
        vec3 kD = (vec3(1.0f) - kS) * (1.0f - metallicMap);
        float NdotL = max(dot(N, L), 0.0f);
        Lo += (kD * albedoMap / PI + specular) * radiance * NdotL;
    }

for (int x = 0; x < gBufferSceneDataBuffer.PointLightCount; x++)
{
    const PointLightBuffer pointLight = pointLightBuffer[x].pointLightProperties;

    vec3 fragPos = positionDataMap.xyz;
    vec3 toLight = pointLight.LightPosition - fragPos;
    float distance = length(toLight);

    if (distance > pointLight.LightRadius) {
        continue;
    }

    vec3 L = normalize(toLight);
    vec3 H = normalize(V + L);

    float attenuation = 1.0f - (distance / pointLight.LightRadius);
    attenuation = max(attenuation, 0.0f);
    attenuation = attenuation * attenuation;

    vec3 radiance = pointLight.LightColor * pointLight.LightIntensity * attenuation;

    float NDF = DistributionGGX(N, H, roughnessMap);
    float G   = GeometrySmith(N, V, L, roughnessMap);
    vec3 F    = fresnelSchlickRoughness(max(dot(H, V), 0.0f), F0, roughnessMap);

    vec3 specular = (NDF * G * F) / (4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f);

    vec3 kS = F;
    vec3 kD = (vec3(1.0f) - kS) * (1.0f - metallicMap);

    float NdotL = 1.0f;
    Lo += (kD * albedoMap / PI + specular) * radiance * NdotL;

    vec2 lightUV = (fragPos.xy - pointLight.LightPosition.xy) / pointLight.LightRadius;
    lightUV = lightUV * 0.5f + 0.5f;

    float penumbra = 0.05f;
    float sdfValue = texture(TextureMap[5], lightUV).r;
    float shadow = smoothstep(-0.05, 0.05, sdfValue);
    Lo *= shadow;
}

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughnessMap);
    vec3 kS = F;
    vec3 kD = 1.0f - kS;
    kD *= 1.0f - metallicMap;

    vec3 irradiance   = vec3(0.36f);
    vec3 diffuse      = irradiance * albedoMap;

    vec3  L       = normalize(directionalLightBuffer[0].directionalLightProperties.LightDirection);
    vec3  H       = normalize(V + L);
    float NDF     = DistributionGGX(N, H, roughnessMap);
    float G       = GeometrySmith(N, V, L, roughnessMap);
    vec3 specular = (NDF * G * F) / (4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f);
    specular *= specularMap;

    vec3 ambient = emissionMap + ((kD * diffuse + specular) * ambientOcclusionMap);
    vec3 color = ambient + Lo;

    vec3 bloomColor = color - emissionMap;
    bloomColor = emissionMap + max(vec3(0.0f), bloomColor - vec3(1.0f));

    outColor = vec4(color, 1.0f);
    outBloom = vec4(bloomColor, 1.0f);
}