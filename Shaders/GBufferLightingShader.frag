#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"

layout(std430, binding = 0) buffer SceneDataBuffer
{
    uint  HDRMapInputIndex;
    uint  FrameBufferIndex;
    uint  BRDFMapId;
    uint  CubeMapId;
    uint  IrradianceMapId;
    uint  PrefilterMapId;
    mat4  Projection;
    mat4  View;
    mat4  InverseProjection;
    mat4  InverseView;
    vec3  CameraPosition;
    vec3  ViewDirection;
    vec2  InvertResolution;
    float Time;
    uint  FrameIndex;
} sceneDataBuffer;

layout(binding = 1) buffer BindlessBuffer
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
    uint64_t SpriteInstanceOffset;
    uint SpriteInstanceCount;
    uint SpriteInstanceSize;
    uint Data[];
} bindlessBuffer;

layout(binding = 2) uniform samplerCube CubeMap[];
layout(binding = 3) uniform sampler2D TextureMap[];
layout(binding = 4) uniform sampler3D Texture3DMap[];

layout(set = 1, binding = 0, input_attachment_index = 0) uniform subpassInput positionInput;
layout(set = 1, binding = 1, input_attachment_index = 1) uniform subpassInput albedoInput;
layout(set = 1, binding = 2, input_attachment_index = 2) uniform subpassInput normalInput;
layout(set = 1, binding = 3, input_attachment_index = 3) uniform subpassInput packedMROInput;
layout(set = 1, binding = 4, input_attachment_index = 4) uniform subpassInput packedSheenSSSInput;
layout(set = 1, binding = 5, input_attachment_index = 5) uniform subpassInput tempInput;
layout(set = 1, binding = 6, input_attachment_index = 6) uniform subpassInput parallaxUVInfoInput;
layout(set = 1, binding = 7, input_attachment_index = 7) uniform subpassInput emissionInput;
layout(set = 1, binding = 8, input_attachment_index = 8) uniform subpassInput depthInput;

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBloom;

#include "BindlessHelpers.glsl"

const float IBL_EXPOSURE = 2.5; // match HDR plate by eye

vec3 OctahedronDecode(vec2 f)
{
    vec3 n;
    n.xy = f.xy;
    n.z  = 1.0 - abs(f.x) - abs(f.y);
    n.xy = (n.z < 0.0) ? (1.0 - abs(n.yx)) * sign(n.xy) : n.xy;
    return normalize(n);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a     = roughness * roughness;
    float a2    = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

Material UnpackMaterial();
vec3 SheenData(Material material, vec3 N, vec3 V);
vec3 DirectionalLightFunc(vec3 F0, vec3 V, Material material);
vec3 PointLightFunc(vec3 F0, vec3 V, Material material);
vec3 ImageBasedLighting(vec3 F0, vec3 V, vec3 N, vec3 R, Material material);

void main()
{
    const float depth = subpassLoad(depthInput).r;
    if (depth >= 0.9999)
    {
        vec3 ndc = vec3(gl_FragCoord.xy * sceneDataBuffer.InvertResolution * 2.0 - 1.0, 1.0);
        vec4 viewPos = sceneDataBuffer.InverseProjection * vec4(ndc, 1.0);
        viewPos /= viewPos.w;

        vec3 viewDir  = normalize(viewPos.xyz);
        vec3 worldDir = normalize((sceneDataBuffer.InverseView * vec4(viewDir, 0.0)).xyz);
        vec3 sky      = textureLod(CubeMap[sceneDataBuffer.CubeMapId], worldDir, 0.0).rgb;

        outColor = vec4(sky, 1.0);
        outBloom = vec4(0.0);
        return;
    }

    Material material = UnpackMaterial();

    vec3 V = normalize(sceneDataBuffer.CameraPosition - material.Position);

    vec3 N = material.Normal;
    vec3 iblN = normalize(mix(material.Normal, V, 0.15));
    vec3 R = reflect(-V, iblN);
    vec3 F0 = mix(vec3(0.04), material.Albedo, material.Metallic);

    vec3 Lo = vec3(0.0);
    Lo += DirectionalLightFunc(F0, V, material);
    Lo += PointLightFunc(F0, V, material);
    vec3 ambient = ImageBasedLighting(F0, V, N, R, material);

    vec3 color = ambient + Lo + material.Emission;
    outColor = vec4(color, 1.0);
    outBloom = vec4(material.Emission + max(color - vec3(1.0), vec3(0.0)), 1.0);
}

Material UnpackMaterial()
{
    Material material;

    vec4 packedMRO  = subpassLoad(packedMROInput);
    vec4 normalData = subpassLoad(normalInput);
    vec4 sheenSSS = subpassLoad(packedSheenSSSInput);
    material.Position       = subpassLoad(positionInput).rgb;
    material.Albedo         = subpassLoad(albedoInput).rgb;
    material.Emission       = subpassLoad(emissionInput).rgb;
    material.ParallaxInfo   = subpassLoad(parallaxUVInfoInput).rgb;
    material.Sheen          = sheenSSS.rgb;
    material.SheenIntensity = sheenSSS.a;

    material.Metallic         = packedMRO.r;
    material.Roughness        = clamp(packedMRO.g, 0.04, 1.0);
    material.AmbientOcclusion = packedMRO.b;
    material.SelfShadow       = normalData.a; 

    material.Normal = normalize(OctahedronDecode(normalData.xy * 2.0 - 1.0));
    return material;
}

vec3 DirectionalLightFunc(vec3 F0, vec3 V, Material material)
{
    vec3 Lo = vec3(0.0);
    for (uint i = 0; i < bindlessBuffer.DirectionalLightCount; ++i)
    {
        const DirectionalLightBuffer light = GetDirectionalLight(i);

        vec3 L = normalize(-light.LightDirection);
        vec3 H = normalize(V + L);

        float NdotL = max(dot(material.Normal, L), 0.0);
        if (NdotL <= 0.0) continue;

        float NdotV = max(dot(material.Normal, V), 0.0);

        vec3 radiance = light.LightColor * light.LightIntensity;
        radiance *= material.SelfShadow;

        float NDF = DistributionGGX(material.Normal, H, material.Roughness);
        float G   = GeometrySmith(material.Normal, V, L, material.Roughness);
        vec3  F   = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, material.Roughness);
        vec3  spec = (NDF * G * F) / max(4.0 * NdotV * NdotL, 1e-4);

        vec3 kD = (vec3(1.0) - F) * (1.0 - material.Metallic);
        vec3 sheenContrib = SheenData(material, material.Normal, V);
        Lo += (kD * material.Albedo / PI + spec) * radiance * NdotL + sheenContrib * radiance * NdotL;
    }
    return Lo;
}

vec3 PointLightFunc(vec3 F0, vec3 V, Material material)
{
    vec3 Lo = vec3(0.0);
    for (uint i = 0; i < bindlessBuffer.PointLightCount; ++i)
    {
        const PointLightBuffer light = GetPointLight(i);

        vec3  toLight  = light.LightPosition - material.Position;
        float distance = length(toLight);
        if (distance > light.LightRadius) continue;

        vec3 L = toLight / max(distance, 1e-4);
        vec3 H = normalize(V + L);

        float NdotL = max(dot(material.Normal, L), 0.0);
        if (NdotL <= 0.0) continue;

        float atten = 1.0 - clamp(distance / light.LightRadius, 0.0, 1.0);
        atten *= atten;

        vec3 radiance = light.LightColor * light.LightIntensity * atten;

        float NdotV = max(dot(material.Normal, V), 0.0);
        float NDF   = DistributionGGX(material.Normal, H, material.Roughness);
        float G     = GeometrySmith(material.Normal, V, L, material.Roughness);
        vec3  F     = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, material.Roughness);
        vec3  spec  = (NDF * G * F) / max(4.0 * NdotV * NdotL, 1e-4);

        vec3 kD = (vec3(1.0) - F) * (1.0 - material.Metallic);
        vec3 sheenContrib = SheenData(material, material.Normal, V);
        Lo += (kD * material.Albedo / PI + spec) * radiance * NdotL + sheenContrib * radiance * NdotL;
    }
    return Lo;
}

vec3 ImageBasedLighting(vec3 F0, vec3 V, vec3 N, vec3 R, Material material)
{
    vec3 F  = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, material.Roughness);
    vec3 kD = (vec3(1.0) - F) * (1.0 - material.Metallic);

    vec3 irradiance = texture(CubeMap[sceneDataBuffer.IrradianceMapId], N).rgb;
    vec3 diffuseIBL = material.Albedo * irradiance * IBL_EXPOSURE;

    float maxLod = float(textureQueryLevels(CubeMap[sceneDataBuffer.PrefilterMapId]) - 1);
    float lod    = clamp(material.Roughness * maxLod, 0.0, maxLod);
    vec3  prefiltered = textureLod(CubeMap[sceneDataBuffer.PrefilterMapId], R, lod).rgb;

    vec2 brdf = texture(TextureMap[sceneDataBuffer.BRDFMapId], vec2(max(dot(N, V), 0.0), material.Roughness)).rg;
    vec3 specularIBL = prefiltered * (F * brdf.x + brdf.y) * IBL_EXPOSURE;

    vec3 sheenIBL = SheenData(material, N, V) * irradiance * IBL_EXPOSURE;
    vec3 ambient = (kD * diffuseIBL + specularIBL + sheenIBL) * material.AmbientOcclusion;
    return max(ambient, vec3(0.02) * material.Albedo);
}

vec3 SheenData(Material material, vec3 N, vec3 V)
{
    float NdotV = max(dot(N, V), 0.0);
    float sheenFresnel = pow(1.0 - NdotV, 5.0);
    return material.Sheen * material.SheenIntensity * sheenFresnel * (1.0 - material.Metallic);
}