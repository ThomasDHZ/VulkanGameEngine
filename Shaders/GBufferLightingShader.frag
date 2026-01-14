#version 460
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable

#include "Constants.glsl"
#include "Lights.glsl"

const int BrdfMapBinding              = 1;
const int DirectionalShadowMapBinding = 2;
const int SDFShadowMapBinding         = 3;

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBloom;

layout(constant_id = 0) const uint DescriptorBindingType0 = SubpassInputDescriptor;
layout(constant_id = 1) const uint DescriptorBindingType1 = SubpassInputDescriptor;
layout(constant_id = 2) const uint DescriptorBindingType2 = SubpassInputDescriptor;
layout(constant_id = 3) const uint DescriptorBindingType3 = SubpassInputDescriptor;
layout(constant_id = 4) const uint DescriptorBindingType4 = SubpassInputDescriptor;
layout(constant_id = 5) const uint DescriptorBindingType5 = SubpassInputDescriptor;
layout(constant_id = 6) const uint DescriptorBindingType6 = TextureDescriptor;
layout(constant_id = 7) const uint DescriptorBindingType7 = DirectionalLightDescriptor;
layout(constant_id = 8) const uint DescriptorBindingType8 = PointLightDescriptor;
layout(constant_id = 9) const uint DescriptorBindingType9 = SkyBoxDescriptor;
layout(constant_id = 10) const uint DescriptorBindingType10 = IrradianceCubeMapDescriptor;
layout(constant_id = 11) const uint DescriptorBindingType11 = PrefilterDescriptor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionInput;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput albedoInput;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput normalInput;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput matRoughInput;
layout(input_attachment_index = 4, binding = 4) uniform subpassInput emissionInput;
layout(input_attachment_index = 5, binding = 5) uniform subpassInput depthInput;

layout(binding = 6) uniform sampler2D TextureMap[];
layout(binding = 7) buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 8) buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];

layout(binding = 9) uniform samplerCube CubeMap;
layout(binding = 10) uniform samplerCube IrradianceMap;
layout(binding = 11) uniform samplerCube PrefilterMap;

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

float SelfShadow(vec2 screenUV, vec3 normalWS, int lightIndex)
{
    vec4 matData = subpassLoad(matRoughInput);

    float currentHeight = matData.r;
    if (currentHeight < 0.001f) return 1.0f;

    mat3 worldToTangent = transpose(ReconstructTBN(normalWS));
    vec3 lightDirWS = normalize(directionalLightBuffer[lightIndex].directionalLightProperties.LightDirection);
    vec3 lightDirTS = normalize(worldToTangent * lightDirWS);

    if (dot(lightDirTS, vec3(0,0,1)) < 0.05f) return 0.4f;

    const int maxSteps = 32;
    const float stepSize = 0.05f;

    float shadow = 1.0f;
    vec2 marchUV = screenUV;
    vec2 deltaUV = lightDirTS.xy * stepSize;
    float bias = directionalLightBuffer[lightIndex].directionalLightProperties.ShadowBias * 0.4f;
    for (int x = 0; x < maxSteps; ++x)
    {
        marchUV += deltaUV;
        float marchedHeight = matData.r + bias * float(x);
        if (marchedHeight > currentHeight + bias * 0.3f)
        {
            shadow *= 0.3f + 0.1f * float(x) / float(maxSteps);
            break;
        }
    }
    return shadow;
}

void main()
{
    vec3 V = normalize(gBufferSceneDataBuffer.ViewDirection);
    vec3 skyColor = SampleSkyboxViewDependent(V);

    vec3  position = subpassLoad(positionInput).rgb;
    vec3  albedo = subpassLoad(albedoInput).rgb;
    vec3  normal = subpassLoad(normalInput).rgb * 2.0f - 1.0f;
    float metallic = 0.0f;//subpassLoad(matRoughInput, pixelOffset).r;
    float roughness = 1.0f;//subpassLoad(matRoughInput, pixelOffset).g;
    float ambientOcclusion = subpassLoad(matRoughInput).b;
    float height = subpassLoad(matRoughInput).r;
    vec3  emission = subpassLoad(emissionInput).rgb;
    float depth = subpassLoad(depthInput).r;

    vec3 N = normalize(normal);
    vec3 R = reflect(-V, N);

    vec3 Lo = vec3(0.0f);
    vec3 F0 = mix(vec3(0.04f), albedo, metallic);
    for (uint x = 0; x < gBufferSceneDataBuffer.DirectionalLightCount; ++x)
    {
        const DirectionalLightBuffer light = directionalLightBuffer[x].directionalLightProperties;
        vec2 screenUV = gl_FragCoord.xy * gBufferSceneDataBuffer.InvertResolution;

        vec3 L = normalize(light.LightDirection);
        vec3 H = normalize(V + L);

        float NdotL = max(dot(N, L), 0.0f);
        if (NdotL <= 0.0f) continue;

        float selfShadow = SelfShadow(screenUV, N, int(x));
        vec3 radiance = light.LightColor * light.LightIntensity * selfShadow;
        
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);

        vec3 F = fresnelSchlickRoughness(max(dot(H, V), 0.0f), F0, roughness);
        vec3 specular = (NDF * G * F) / max(4.0f * max(dot(N, V), 0.0f) * NdotL, 0.0001f);
        vec3 kS = F;
        vec3 kD = (vec3(1.0f) - kS) * (1.0f - metallic);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    for (uint x = 0; x < gBufferSceneDataBuffer.PointLightCount; ++x)
    {
        const PointLightBuffer light = pointLightBuffer[x].pointLightProperties;

        vec3 toLight = light.LightPosition - position;
        float distance = length(toLight);

        if (distance > light.LightRadius) continue;

        vec3 L = normalize(toLight);
        vec3 H = normalize(V + L);

        float attenuation = 1.0f - (distance / light.LightRadius);
        attenuation = max(attenuation, 0.0f);
        attenuation *= attenuation;

        vec3 radiance = light.LightColor * light.LightIntensity * attenuation;
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);

        vec3 F = fresnelSchlickRoughness(max(dot(H, V), 0.0f), F0, roughness);
        vec3 specular = (NDF * G * F) / max(4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f), 0.0001f);
        vec3 kS = F;
        vec3 kD = (vec3(1.0f) - kS) * (1.0f - metallic);
        float NdotL = max(dot(N, L), 0.0f);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughness);
    vec3 kS = F;
    vec3 kD = (vec3(1.0f) - kS) * (1.0f - metallic);

    vec3 irradiance = texture(IrradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0f;
    vec3 prefilteredColor = textureLod(PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;

    vec2 brdf = texture(TextureMap[BrdfMapBinding], vec2(max(dot(N, V), 0.0f), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    vec3 ambient = emission + (kD * diffuse + specular) * ambientOcclusion;

    vec3 color = ambient + Lo;
   // color = mix(skyColor * 0.4f, color, clamp(subpassLoad(depthInput).r * 1.2f, 0.0, 1.0));
    outColor = vec4(color, 1.0f);

    vec3 bloomColor = max(vec3(0.0f), color - vec3(1.0f));
    outBloom = vec4(bloomColor, 1.0f);
}