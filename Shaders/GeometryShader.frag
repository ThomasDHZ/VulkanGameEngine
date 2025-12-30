#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 1;
layout(binding = 0) uniform sampler2D TextureMap[];

layout(location = 0) in  vec2 TexCoords;
layout(location = 0) out vec4 outColor;

//layout(location = 0) in vec4 PositionDataMap;
//layout(location = 1) in vec4 AlbedoMap;
//layout(location = 2) in vec4 NormalMap;
//layout(location = 3) in vec4 MatRoughAOHeightMap;
//layout(location = 4) in vec4 EmissionMap;

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

struct MaterialProperitiesBuffer
{
	vec3 Albedo;
	float Specular;
	float Metallic;
	float Roughness;
	float AmbientOcclusion;
	vec3 Emission;
	float Alpha;
	float HeightScale;
	float Height;

	uint AlbedoMap;
	uint SpecularMap;
	uint MetallicMap;
	uint RoughnessMap;
	uint AmbientOcclusionMap;
	uint NormalMap;
	uint AlphaMap;
	uint EmissionMap;
	uint HeightMap;
};

struct DirectionalLightBuffer
{
    vec3  LightColor;
    vec3  LightDirection;
    float LightIntensity;
}

void main() 
{
    vec4  PositionDataMap     = texture(TextureMap[0], TexCoords).rgba;
    vec3  albedoMap           = texture(TextureMap[1], TexCoords).rgb;
    vec3  N                   = normalize(texture(TextureMap[2], TexCoords).rgb + vec3(0.0, 0.0, 1.0));
    float metallicMap         = texture(TextureMap[3], TexCoords).r;
    float roughnessMap        = texture(TextureMap[3], TexCoords).g;
    float ambientOcclusionMap = texture(TextureMap[3], TexCoords).b;
    float heightMap           = texture(TextureMap[3], TexCoords).a;
    vec3  emissionMap         = texture(TextureMap[4], TexCoords).rgb;
    float specularMap         = texture(TextureMap[4], TexCoords).a;
    vec3  V                   = normalize(vec3(0.3, 0.3, 1.0));

    N = normalize(N * 2.0 - 1.0);
    N = normalize(TBN * N);
    vec3 R = reflect(-V, N); 

    vec3 F0 = vec3(0.04f); 
    F0 = mix(F0, albedoMap, metallicMap);

    vec3 lightDir = normalize(vec3(0.5, 0.2, 1.0)); 
    vec3 Lo = vec3(0.0);
    for(int x = 0; x <= 5; x++)
    {
        vec3  lightColor = vec3(1.0, 0.95, 0.9);  
        float lightIntensity = 1.2f;

        vec3 L = lightDir;
        vec3 H = normalize(V + L);
        vec3 radiance = lightColor * lightIntensity;

        float NDF = DistributionGGX(N, H, roughnessMap);
        float G   = GeometrySmith(N, V, L, roughnessMap);
        vec3  F   = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, roughnessMap);

        vec3 specular = (NDF * G * F) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS) * (1.0 - metallicMap);

        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedoMap / PI + specular) * radiance * NdotL;
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughnessMap);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallicMap;

    vec3 irradiance   = vec3(0.36f);
    vec3 diffuse      = irradiance * albedoMap;

    vec3  L   = lightDir;
    vec3  H   = normalize(V + L);
    float NDF = DistributionGGX(N, H, roughnessMap);
    float G   = GeometrySmith(N, V, L, roughnessMap);
    vec3 specular = (NDF * G * F) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);
    specular *= specularMap;

    vec3 ambient      = emissionMap + ((vec3(0.15, 0.18, 0.25) * diffuse + specular));
    vec3 color        = ambient + Lo;

    outColor = vec4(color, 1.0);
}
