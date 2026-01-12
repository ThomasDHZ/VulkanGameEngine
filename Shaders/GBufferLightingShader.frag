#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable

#include "Constants.glsl"
#include "Lights.glsl"

layout(location = 0) in  vec2 TexCoords;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBloom;

const int PositionDataMapBinding = 0;
const int AlbedoMapBinding = 1;
const int NormalMapBinding = 2;
const int MatRoughAOHeightMapBinding = 3;
const int EmissionMapBinding = 4;
const int DepthMapBinding = 5;
const int BrdfMapBinding = 6;
const int DirectionalShadowMapBinding = 7;
const int SDFShadowMapBinding = 8;


layout(constant_id = 0) const uint DescriptorBindingType0 = TextureDescriptor;
layout(constant_id = 1) const uint DescriptorBindingType1 = DirectionalLightDescriptor;
layout(constant_id = 2) const uint DescriptorBindingType2 = PointLightDescriptor;
layout(constant_id = 3) const uint DescriptorBindingType3 = SkyBoxDescriptor;
layout(constant_id = 4) const uint DescriptorBindingType4 = IrradianceCubeMapDescriptor;
layout(constant_id = 5) const uint DescriptorBindingType5 = PrefilterDescriptor;
layout(constant_id = 6) const uint DescriptorBindingType6 = SubpassInputDescriptor;
layout(constant_id = 7) const uint DescriptorBindingType7 = SubpassInputDescriptor;
layout(constant_id = 8) const uint DescriptorBindingType8 = SubpassInputDescriptor;
layout(constant_id = 9) const uint DescriptorBindingType9 = SubpassInputDescriptor;
layout(constant_id = 10) const uint DescriptorBindingType10 = SubpassInputDescriptor;
layout(constant_id = 11) const uint DescriptorBindingType11 = SubpassInputDescriptor;

layout(binding = 0) uniform sampler2D   TextureMap[];
layout(binding = 1) buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 2) buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
layout(binding = 3) uniform samplerCube CubeMap;
layout(binding = 4) uniform samplerCube IrradianceMap;
layout(binding = 5) uniform samplerCube PrefilterMap;
layout(input_attachment_index = 0, binding = 6) uniform subpassInput positionInput;
layout(input_attachment_index = 1, binding = 7) uniform subpassInput normalInput;
layout(input_attachment_index = 2, binding = 8) uniform subpassInput albedoInput;
layout(input_attachment_index = 3, binding = 9) uniform subpassInput matRoughInput;
layout(input_attachment_index = 4, binding = 10) uniform subpassInput emissionInput;
layout(input_attachment_index = 5, binding = 11) uniform subpassInput depthInput;

layout(push_constant) uniform GBufferSceneDataBuffer
{
    int UseHeightMap;
    float HeightScale;
    vec3 ViewDirection;
	uint DirectionalLightCount;
    uint PointLightCount;
    mat4 InvProjection;
    mat4 InvView;
}gBufferSceneDataBuffer;

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

vec2 ParallaxOcclusionMapping(vec2 uv, vec3 viewDirTS)
{
    if (gBufferSceneDataBuffer.UseHeightMap == 0) return uv;

    const float minLayers = 16.0f;
    const float maxLayers = 64.0f;
    float numLayers = mix(maxLayers, minLayers, abs(viewDirTS.z));

    float layerDepth = 1.0f / numLayers;
    float currentLayerDepth = 0.0f;
    vec2 P = viewDirTS.xy * gBufferSceneDataBuffer.HeightScale;
    vec2 deltaUV = P / numLayers;

    vec2 currentUV = uv;
    float currentHeight = subpassLoad(matRoughInput).r;

    int x = 0;
    while (currentLayerDepth < currentHeight && x < 32) {
        currentUV -= deltaUV;
        currentHeight = subpassLoad(matRoughInput).r;
        currentLayerDepth += layerDepth;
        x++;
    }

    vec2 prevUV = currentUV + deltaUV;
    float afterDepth = currentHeight - currentLayerDepth;
    float beforeDepth = subpassLoad(matRoughInput).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth + 0.0001f);
    vec2 finalUV = mix(currentUV, prevUV, weight);
    finalUV = clamp(finalUV, vec2(0.005f), vec2(0.995f));

    return clamp(finalUV, vec2(0.01f), vec2(0.99f));
}

void main()
{    
    vec2 baseUV = TexCoords;
    vec3 V = normalize(gBufferSceneDataBuffer.ViewDirection);

    vec2 uv = baseUV;
    if (gBufferSceneDataBuffer.UseHeightMap == 1) {
        uv = ParallaxOcclusionMapping(baseUV, V);
        uv = clamp(uv, vec2(0.01), vec2(0.99));
    }

    float depthMap = texture(TextureMap[DepthMapBinding], baseUV).r;
    bool isBackground = (depthMap >= 0.99999);

    if (isBackground) {
        vec2 ndc = baseUV * 2.0 - 1.0;
        vec4 clipPos = vec4(ndc, 1.0, 1.0);
        vec4 viewPos = gBufferSceneDataBuffer.InvProjection * clipPos;
        viewPos /= viewPos.w;
        vec3 viewDir = normalize(viewPos.xyz);
        vec3 worldDir = normalize(mat3(gBufferSceneDataBuffer.InvView) * viewDir);
        vec3 skyColor = texture(CubeMap, worldDir).rgb;
        outColor = vec4(skyColor, 1.0);
        outBloom = vec4(0.0);
        return;
    }

    vec3 positionDataMap = subpassLoad(positionInput).rgb;
    vec3 albedoMap = subpassLoad(albedoInput).rgb;
    vec3 normalMap =subpassLoad(normalInput).rgb * 2.0f - 1.0f;
    float metallicMap = 0.0f;//subpassLoad(matRoughInput).r;
    float roughnessMap = 1.0f;//subpassLoad(matRoughInput).g;
    float ambientOcclusionMap = subpassLoad(matRoughInput).b;
    float heightMap = subpassLoad(matRoughInput).a;
    vec3 emissionMap = subpassLoad(emissionInput).rgb;
    float specularMap = subpassLoad(emissionInput).a;

    vec3 N = normalize(normalMap);
    vec3 R = reflect(-V, N); 

    vec3 Lo = vec3(0.0f); 
    vec3 F0 = vec3(0.04f);
    float shadow =0.0f;
    F0 = mix(F0, albedoMap, metallicMap);
    for(int x = 0; x < gBufferSceneDataBuffer.DirectionalLightCount; x++)
    {
        const DirectionalLightBuffer directionalLight = directionalLightBuffer[x].directionalLightProperties;

        vec3 fragPos = positionDataMap.xyz;
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

        vec4 lightSpacePos = directionalLight.LightSpaceMatrix * vec4(fragPos, 1.0);
        vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
        projCoords = projCoords * 0.5 + 0.5;

        float shadowDepth = texture(TextureMap[DirectionalShadowMapBinding], projCoords.xy).r;
        float currentDepth = projCoords.z;

        float bias = 0.0005;
         shadow = (currentDepth > shadowDepth + bias) ? 0.0 : 1.0;

        shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(TextureMap[DirectionalShadowMapBinding], 0);
        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                vec2 offset = vec2(x, y) * texelSize;
                float pcfDepth = texture(TextureMap[DirectionalShadowMapBinding], projCoords.xy + offset).r;
                shadow += (currentDepth > pcfDepth + bias) ? 0.0 : 1.0;
            }
        }
        shadow /= 9.0;
        Lo *= shadow;
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

//    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//    projCoords = projCoords * 0.5 + 0.5;
//    float closestDepth = texture(TextureMap[SDFShadowMapBinding], projCoords.xy).r; 
//    float currentDepth = projCoords.z;
//
//    vec3 lightDir = normalize(pointLight.LightPosition - fragPos);
//    float bias = max(0.05 * (1.0 - dot(normalMap, lightDir)), 0.005);
//    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
//
//    vec2 texelSize = 1.0 / textureSize(TextureMap[SDFShadowMapBinding], 0);
//    for(int x = -1; x <= 1; ++x)
//    {
//        for(int y = -1; y <= 1; ++y)
//        {
//            float pcfDepth = texture(TextureMap[SDFShadowMapBinding], projCoords.xy + vec2(x, y) * texelSize).r; 
//            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
//        }    
//    }
//    shadow /= 9.0;
//    
//    if(projCoords.z > 1.0)
//        shadow = 0.0;
//        
 //   float NdotL = max(dot(N, L), 0.0f);
 //   Lo += (kD * albedoMap / PI + specular) * radiance * NdotL;// * visibility;
//    Lo *= shadow;
}

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughnessMap);
    vec3 kS = F;
    vec3 kD = 1.0f - kS;
    kD *= 1.0f - metallicMap;

    vec3 irradiance   = texture(IrradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedoMap;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = texture(PrefilterMap, R,  roughnessMap * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(TextureMap[BrdfMapBinding], vec2(max(dot(N, V), 0.0), roughnessMap)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = emissionMap + ((kD * diffuse + specular) * ambientOcclusionMap);
    vec3 color = ambient + Lo;

    vec3 bloomColor = color - emissionMap;
    bloomColor = emissionMap + max(vec3(0.0f), bloomColor - vec3(1.0f));

    outColor = vec4(color, 1.0);
    outBloom = vec4(bloomColor, 1.0f);
}