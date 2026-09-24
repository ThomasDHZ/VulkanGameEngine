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
    uint HDRMapInputIndex;
    uint EnvironmentMapIndex;
    uint BRDFMapId;
    uint CubeMapId;
    uint IrradianceMapId;
    uint PrefilterMapId;
    uint _padIds0;
    uint _padIds1;

    mat4 OrthoProjection;
    mat4 OrthoView;
    mat4 InverseOrthoProjection;
    mat4 InverseOrthoView;
    mat4 InversePerspectiveProjection;
    mat4 InversePerspectiveView;

    vec3  PerspectiveCameraPosition;
    float Time;
    vec3  PerspectiveViewDirection;
    uint  FrameIndex;
    vec2  InvertResolution;
    vec2  _padEnd;
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

layout(set = 1, binding = 0, input_attachment_index = 0) uniform subpassInput PositionInput;      //R16G16B16A16_SFLOAT
layout(set = 1, binding = 1, input_attachment_index = 1) uniform subpassInput AlbedoInput;        //R8G8B8A8_SRGB
layout(set = 1, binding = 2, input_attachment_index = 2) uniform subpassInput NormalDataInput;    //R16G16B16A16_UNORM 
layout(set = 1, binding = 3, input_attachment_index = 3) uniform subpassInput MROInput;           //R16G16B16A16_UNORM
layout(set = 1, binding = 4, input_attachment_index = 4) uniform subpassInput FeatureAInput;      //R16G16B16A16_UNORM
layout(set = 1, binding = 5, input_attachment_index = 5) uniform subpassInput FeatureBInput;      //R16G16B16A16_UNORM
layout(set = 1, binding = 6, input_attachment_index = 6) uniform subpassInput FeatureCInput;      //R16G16B16A16_UNORM
layout(set = 1, binding = 7, input_attachment_index = 7) uniform subpassInput EmissionInput;      //R16G16B16A16_SFLOAT
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

Material UnpackMaterial();
vec3 SheenData(Material material, vec3 N, vec3 V);
vec3 SubSurfaceScatteringData(Material material, vec3 N, vec3 L);
vec3 DirectionalLightFunc(vec3 F0, vec3 V, Material material);
vec3 PointLightFunc(vec3 F0, vec3 V, Material material);
vec3 ImageBasedLighting(vec3 F0, vec3 V, vec3 N, vec3 R, Material material);
vec3 ReconstructWorldPos(float depth);

void main()
{
    Material material = UnpackMaterial();
    if (material.Depth >= 0.9999f)
    {
        vec2 uv = TexCoords;
        vec3 ndc = vec3(uv * 2.0f - 1.0f, 1.0f);
        vec4 viewPos = sceneDataBuffer.InversePerspectiveProjection * vec4(ndc, 1.0);
        viewPos /= viewPos.w;

        vec3 worldDir = normalize((sceneDataBuffer.InversePerspectiveView * vec4(normalize(viewPos.xyz), 0.0)).xyz);
        outColor = vec4(textureLod(CubeMap[sceneDataBuffer.CubeMapId], worldDir, 0.0).rgb, 1.0);
        outBloom = vec4(0.0);
        return;
    }

   // material.Position = ReconstructWorldPos(depth);
//
//        vec3 stored  = subpassLoad(positionInput).rgb;
//vec3 rebuilt = ReconstructWorldPos(depth);
//    if(stored == rebuilt)
//    {
//        outColor(1.0f, 0.0f, 0.0f, 1.0f);
//        outBloom(0.0f);
//        return;
//    }
    vec3 V     = normalize(sceneDataBuffer.PerspectiveCameraPosition - material.Position);
    vec3 N     = material.Normal;
    vec3 iblN  = normalize(mix(N, V, 0.15f));
    vec3 R     = reflect(-V, iblN);
    vec3 F0    = mix(vec3(0.04), material.Albedo, material.Metallic);

    vec3 Lo    = DirectionalLightFunc(F0, V, material) + PointLightFunc(F0, V, material);
    vec3 color = ImageBasedLighting(F0, V, N, R, material) + Lo + material.Emission;

    outColor   = vec4(color, 1.0);
    outBloom   = vec4(material.Emission + max(color - vec3(1.0), vec3(0.0)), 1.0);
}

const float kFeatureEps = 1e-3;
const uint FEAT_COAT  = 1u << 0;
const uint FEAT_SHEEN = 1u << 1;
const uint FEAT_SSS   = 1u << 2;
const uint FEAT_ANISO = 1u << 4;
const uint FEAT_FILM  = 1u << 5;

Material UnpackMaterial()
{
    vec4 positionAttachment     = subpassLoad(PositionInput);
    vec4 albedoAttachment       = subpassLoad(AlbedoInput);
    vec4 normalAttachment       = subpassLoad(NormalDataInput);
    vec4 mroAttachment          = subpassLoad(MROInput);
    vec4 featureAAttachment     = subpassLoad(FeatureAInput);
    vec4 featureBAttachment     = subpassLoad(FeatureBInput);
    vec4 featureCAttachment     = subpassLoad(FeatureCInput);
    vec4 emissionAttachment     = subpassLoad(EmissionInput);
    vec4 depthAttachment        = subpassLoad(depthInput);

    vec2 sheenW_coatW = Unpack8bitPair(featureAAttachment.a);
    vec2 thick_coatR  = Unpack8bitPair(featureBAttachment.a);
    vec2 aniso        = Unpack8bitPair(featureCAttachment.r);
    vec2 film         = Unpack8bitPair(featureCAttachment.g);
    vec2 coatD_sssW   = Unpack8bitPair(featureCAttachment.b);
    vec2 sheenR_prof  = Unpack8bitPair(featureCAttachment.a);

    Material m;
    m.Position         = positionAttachment.rgb;
    m.Depth            = depthAttachment.r;

    m.Albedo           = albedoAttachment.rgb;
    m.Metallic         = mroAttachment.r;
    m.Roughness        = mroAttachment.g;
    m.AmbientOcclusion = mroAttachment.b;
    m.IOR              = mroAttachment.a * 2.0 + 1.0;

    m.Normal           = normalize(OctahedronDecode(albedoAttachment.xy * 2.0 - 1.0));
    m.SelfShadow       = normalAttachment.a;
    m.Emission         = emissionAttachment.rgb;

    m.CoatColor        = vec3(1.0);
    m.CoatWeight       = sheenW_coatW.y;
    m.CoatRoughness    = thick_coatR.y;
    m.CoatDarkening    = coatD_sssW.x;

    m.SheenColor       = featureAAttachment.rgb;
    m.SheenWeight      = sheenW_coatW.x;
    m.SheenRoughness   = sheenR_prof.x;

    m.SSSColor         = featureBAttachment.rgb;
    m.Thickness        = thick_coatR.x;
    m.SSSWeight        = coatD_sssW.y;
    m.SSSProfile       = sheenR_prof.y;

    m.Anisotropy         = aniso.x;
    m.AnisotropyRotation = aniso.y;
    m.ThinFilmWeight     = film.x;
    m.ThinFilmThickness  = film.y;

    m.ShadingModel = 0u;
    m.FeatureMask  = 0u;
    if (m.CoatWeight      > kFeatureEps) m.FeatureMask |= FEAT_COAT;
    if (m.SheenWeight     > kFeatureEps) m.FeatureMask |= FEAT_SHEEN;
    if (m.SSSWeight       > kFeatureEps) m.FeatureMask |= FEAT_SSS;
    if (m.Anisotropy      > kFeatureEps) m.FeatureMask |= FEAT_ANISO;
    if (m.ThinFilmWeight  > kFeatureEps) m.FeatureMask |= FEAT_FILM;

    return m;
}

vec3 DirectionalLightFunc(vec3 F0, vec3 V, Material material)
{
    vec3 Lo = vec3(0.0);
    for (uint i = 0; i < bindlessBuffer.DirectionalLightCount; ++i)
    {
        const DirectionalLightBuffer light = GetDirectionalLight(i);

        vec3 L = normalize(-light.LightDirection);
        vec3 H = normalize(V + L);
        vec3 N = material.Normal;

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
        vec3 subSurfaceScattering = SubSurfaceScatteringData(material, N, L);
        if (NdotL <= 0.0)
        {
            Lo += subSurfaceScattering;
            continue;
        }

        Lo += (kD * material.Albedo / PI + spec) * radiance * NdotL + sheenContrib * radiance * NdotL; + subSurfaceScattering;
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
        vec3 N = material.Normal;

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
        vec3 subSurfaceScattering = SubSurfaceScatteringData(material, N, L);
        if (NdotL <= 0.0)
        {
            Lo += subSurfaceScattering;
            continue;
        }

        Lo += (kD * material.Albedo / PI + spec) * radiance * NdotL + sheenContrib * radiance * NdotL; + subSurfaceScattering;
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

    float subsurfaceStrength = 0.7f;
    vec3  SssIBL = subsurfaceStrength * irradiance * (material.Albedo * material.SSSColor);
    vec3 ambient = (kD * diffuseIBL + specularIBL + sheenIBL) * material.AmbientOcclusion;
    return max(ambient, vec3(0.02) * material.Albedo);
}

vec3 ReconstructWorldPos(float depth)
{
    vec2 uv = TexCoords; // same UV as sky
    vec4 clip = vec4(uv * 2.0 - 1.0, depth, 1.0); // ZO, GLM_FORCE_DEPTH_ZERO_TO_ONE
    vec4 view = sceneDataBuffer.InverseOrthoProjection * clip;
    view.xyz /= max(view.w, 1e-6);
    return (sceneDataBuffer.InverseOrthoView * vec4(view.xyz, 1.0)).xyz;
}

vec3 SheenData(Material material, vec3 N, vec3 V)
{
    float NdotV = max(dot(N, V), 0.0);
    float sheenFresnel = pow(1.0 - NdotV, 2.0);
    return material.SheenColor * 0.08 * sheenFresnel * (1.0 - material.Metallic);
}

vec3 SubSurfaceScatteringData(Material material, vec3 N, vec3 L)
{
    float wrap = 0.45;
    float NdotL_wrap = max(dot(N, L) + wrap, 0.0) / (1.0 + wrap);
    float scatter = NdotL_wrap * material.Thickness * (1.0 - material.Metallic);
    return material.Albedo * material.SSSColor * scatter * 0.35;
}