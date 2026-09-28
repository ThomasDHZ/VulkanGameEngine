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

layout(set = 1, binding = 0, input_attachment_index = 0) uniform subpassInput PositionInput;
layout(set = 1, binding = 1, input_attachment_index = 1) uniform subpassInput AlbedoInput;
layout(set = 1, binding = 2, input_attachment_index = 2) uniform subpassInput NormalDataInput;
layout(set = 1, binding = 3, input_attachment_index = 3) uniform subpassInput MROInput;
layout(set = 1, binding = 4, input_attachment_index = 4) uniform subpassInput FeatureAInput;
layout(set = 1, binding = 5, input_attachment_index = 5) uniform subpassInput FeatureBInput;
layout(set = 1, binding = 6, input_attachment_index = 6) uniform subpassInput FeatureCInput;
layout(set = 1, binding = 7, input_attachment_index = 7) uniform subpassInput EmissionInput;
layout(set = 1, binding = 8, input_attachment_index = 8) uniform subpassInput depthInput;

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBloom;

#include "BindlessHelpers.glsl"

const float IBL_EXPOSURE = 2.5;
const float kFeatureEps  = 1e-3;
const uint FEAT_COAT         = 1u << 0;
const uint FEAT_SHEEN        = 1u << 1;
const uint FEAT_SSS          = 1u << 2;
const uint FEAT_TRANSMISSION = 1u << 3;
const uint FEAT_ANISO        = 1u << 4;
const uint FEAT_FILM         = 1u << 5;
const uint FEAT_TWO_SIDED    = 1u << 6;
const uint FEAT_COAT_NORMAL  = 1u << 7;
const uint FEAT_ALBEDO_ONLY  = 1u << 8;

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

float Pack8bitPair(float high, float low)
{
    uint u_high = uint(high * 255.0 + 0.5) & 0xFFu;
    uint u_low  = uint(low  * 255.0 + 0.5) & 0xFFu;
    return float((u_high << 8) | u_low) / 65535.0;
}

vec2 Unpack8bitPair(float packed)
{
    uint combined = uint(packed * 65535.0 + 0.5);
    return vec2(float((combined >> 8) & 0xFFu) / 255.0,
                float(combined & 0xFFu) / 255.0);
}

void AnisoFrame(vec3 N, float rotation, out vec3 T, out vec3 B)
{
    vec3 up = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    T = normalize(cross(up, N));
    B = cross(N, T);
    float a = rotation * 6.2831853;
    float c = cos(a), s = sin(a);
    vec3 T2 = normalize(c * T + s * B);
    B = cross(N, T2);
    T = T2;
}

float DistributionGGX_Aniso(vec3 N, vec3 T, vec3 B, vec3 H, float rough, float aniso)
{
    float at = max(rough * (1.0 + aniso), 0.001);
    float ab = max(rough * (1.0 - aniso), 0.001);
    float ToH = dot(T, H);
    float BoH = dot(B, H);
    float NoH = max(dot(N, H), 0.0);
    float d = ToH * ToH / (at * at) + BoH * BoH / (ab * ab) + NoH * NoH;
    return 1.0 / max(3.14159265 * at * ab * d * d, 1e-6);
}


Material UnpackMaterial();
vec3 DirectionalLightFunc(vec3 F0, vec3 V, Material material);
vec3 PointLightFunc(vec3 F0, vec3 V, Material material);
vec3 ImageBasedLighting(vec3 F0, vec3 V, vec3 N, vec3 R, Material material);
vec3 SheenData(Material material, vec3 N, vec3 V);
vec3 SubSurfaceScatteringData(Material material, vec3 N, vec3 L);
vec3 ThinFilm(vec3 spec, Material m, float HdotV);
vec3 ClearCoat(Material m, vec3 N, vec3 V, vec3 L, vec3 H, vec3 radiance, float NdotL);
vec3 ReconstructWorldPos(float depth);

void main()
{
    Material material = UnpackMaterial();
    if (material.Depth >= 0.9999)
    {
        vec3 ndc = vec3(TexCoords * 2.0 - 1.0, 1.0);
        vec4 viewPos = sceneDataBuffer.InversePerspectiveProjection * vec4(ndc, 1.0);
        viewPos /= viewPos.w;
        vec3 worldDir = normalize((sceneDataBuffer.InversePerspectiveView * vec4(normalize(viewPos.xyz), 0.0)).xyz);
        outColor = vec4(textureLod(CubeMap[sceneDataBuffer.CubeMapId], worldDir, 0.0).rgb, 1.0);
        outBloom = vec4(0.0);
        return;
    }
    if ((material.FeatureMask & FEAT_ALBEDO_ONLY) != 0u)
    {
        outColor = vec4(material.Albedo, 1.0);
        outBloom = vec4(0.0f);
        return;
    }

    vec3 V  = normalize(sceneDataBuffer.PerspectiveCameraPosition - material.Position);
    vec3 N  = material.Normal;
    vec3 R  = reflect(-V, normalize(mix(N, V, 0.15)));
    vec3 F0 = mix(vec3(0.04), material.Albedo, material.Metallic);
//
    vec3 Lo    = DirectionalLightFunc(F0, V, material);
     Lo    += PointLightFunc(F0, V, material);
    vec3 color = ImageBasedLighting(F0, V, N, R, material) + Lo + material.Emission;

    outColor = vec4(color, 1.0);
    outBloom = vec4(material.Emission + max(Lo - vec3(1.0), vec3(0.0)), 1.0);
}

Material UnpackMaterial()
{
    vec4 positionAttachment = subpassLoad(PositionInput);
    vec4 albedoAttachment   = subpassLoad(AlbedoInput);
    vec4 normalAttachment   = subpassLoad(NormalDataInput);
    vec4 mroAttachment      = subpassLoad(MROInput);
    vec4 featureAAttachment = subpassLoad(FeatureAInput);
    vec4 featureBAttachment = subpassLoad(FeatureBInput);
    vec4 featureCAttachment = subpassLoad(FeatureCInput);
    vec4 emissionAttachment = subpassLoad(EmissionInput);
    vec4 depthAttachment    = subpassLoad(depthInput);

    vec2 sssW_coatW    = Unpack8bitPair(featureAAttachment.a);
    vec2 thick_coatR   = Unpack8bitPair(featureBAttachment.a);
    vec2 aniso         = Unpack8bitPair(featureCAttachment.r);
    vec2 film          = Unpack8bitPair(featureCAttachment.g);
    vec2 coatD_prof    = Unpack8bitPair(featureCAttachment.b);
    vec2 sheenR_sheenW = Unpack8bitPair(featureCAttachment.a);

    Material m;
    m.Depth            = depthAttachment.r;
    m.Position         = ReconstructWorldPos(m.Depth);
    m.Albedo           = albedoAttachment.rgb;
    m.Metallic         = mroAttachment.r;
    m.Roughness        = mroAttachment.g;
    m.AmbientOcclusion = mroAttachment.b;
    m.IOR              = mroAttachment.a * 2.0 + 1.0;

    m.Normal       = normalize(OctahedronDecode(normalAttachment.xy * 2.0 - 1.0));
    m.Emission     = emissionAttachment.rgb;

    m.SheenColor     = featureAAttachment.rgb;
    m.SheenRoughness = sheenR_sheenW.x;
    m.SheenWeight    = sheenR_sheenW.y;

    m.SSSColor       = featureBAttachment.rgb;
    m.SSSWeight      = sssW_coatW.x;
    m.SSSProfile     = coatD_prof.y;
    m.Thickness      = thick_coatR.x;

    m.CoatWeight     = sssW_coatW.y;
    m.CoatRoughness  = thick_coatR.y;
    m.CoatDarkening  = coatD_prof.x;

    m.Anisotropy         = aniso.x;
    m.AnisotropyRotation = aniso.y;
    m.ThinFilmWeight     = film.x;
    m.ThinFilmThickness  = film.y;

    m.FeatureMask  = uint(normalAttachment.b * 65535.0 + 0.5);
    m.SelfShadow   = normalAttachment.a;
    m.ShadingModel = 0u;
    return m;
} 

vec3 DirectionalLightFunc(vec3 F0, vec3 V, Material material)
{
    vec3 Lo = vec3(0.0);
    for (uint x = 0; x < bindlessBuffer.DirectionalLightCount; ++x)
    {
        const DirectionalLightBuffer light = GetDirectionalLight(x);
        vec3 L = normalize(-light.LightDirection);
        vec3 H = normalize(V + L);
        vec3 N = material.Normal;
        vec3 T = vec3(0.0f);
        vec3 B = vec3(0.0f);

        vec3 radiance = light.LightColor * light.LightIntensity * material.SelfShadow;
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;

        AnisoFrame(N, material.AnisotropyRotation, T, B);
        //float NDF  = DistributionGGX(N, H, material.Roughness);
        float NDF = DistributionGGX_Aniso(N, T, B, H, material.Roughness, material.Anisotropy);
        Lo += SubSurfaceScatteringData(material, N, L) * radiance;

        float G    = GeometrySmith(N, V, L, material.Roughness);
        vec3  F    = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, material.Roughness);
        vec3  kD   = (vec3(1.0) - F) * (1.0 - material.Metallic);
        
        float NdotV = max(dot(N, V), 0.0);
        vec3  spec = (NDF * G * F) / max(4.0 * NdotV * NdotL, 1e-4);
        spec = ThinFilm(spec, material, max(dot(H, V), 0.0));

        Lo += SheenData(material, N, V) * radiance * NdotL;
        Lo += (kD * material.Albedo / PI + spec) * radiance * NdotL;
    }
    return Lo;
}

vec3 PointLightFunc(vec3 F0, vec3 V, Material material)
{
    vec3 Lo = vec3(0.0);
    for (uint x = 0; x < bindlessBuffer.PointLightCount; ++x)
    {
        const PointLightBuffer light = GetPointLight(x);
        vec3  toLight  = light.LightPosition - material.Position;
        float distance = length(toLight);
        if (distance > light.LightRadius) continue;

        vec3 L = toLight / max(distance, 1e-4);
        vec3 H = normalize(V + L);
        vec3 N = material.Normal;
        vec3 T = vec3(0.0f);
        vec3 B = vec3(0.0f);

        float atten = 1.0 - clamp(distance / light.LightRadius, 0.0, 1.0);
        atten *= atten;

        vec3 radiance = light.LightColor * light.LightIntensity * atten * material.SelfShadow;
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;

        AnisoFrame(N, material.AnisotropyRotation, T, B);
        float NDF = DistributionGGX_Aniso(N, T, B, H, material.Roughness, material.Anisotropy);
        Lo += SubSurfaceScatteringData(material, N, L) * radiance;

        float G     = GeometrySmith(material.Normal, V, L, material.Roughness);
        vec3  F     = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, material.Roughness);
        vec3  kD    = (vec3(1.0) - F) * (1.0 - material.Metallic);

        float NdotV = max(dot(material.Normal, V), 0.0);
        vec3  spec = (NDF * G * F) / max(4.0 * NdotV * NdotL, 1e-4);
        spec = ThinFilm(spec, material, max(dot(H, V), 0.0));
         
        Lo += SheenData(material, N, V) * radiance * NdotL;
        Lo += (kD * material.Albedo / PI + spec) * radiance * NdotL;
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
    specularIBL = ThinFilm(specularIBL, material, max(dot(N, V), 0.0));

    vec3 ambient = (kD * diffuseIBL + specularIBL) * material.AmbientOcclusion;
    ambient += SheenData(material, N, V) * irradiance * IBL_EXPOSURE * 0.25;
    ambient += material.SSSWeight * material.SSSColor * material.Albedo * irradiance * IBL_EXPOSURE * 0.25;
    return max(ambient, vec3(0.02) * material.Albedo);
}

vec3 SheenData(Material material, vec3 N, vec3 V)
{
    float NdotV = max(dot(N, V), 0.0);
    float rough = max(material.SheenRoughness, 0.04);
    float fresnel = pow(1.0 - NdotV, 2.0 + 3.0 * rough);
    return material.SheenColor * material.SheenWeight * fresnel * (1.0 - material.Metallic);
}

vec3 SubSurfaceScatteringData(Material material, vec3 N, vec3 L)
{
  //  if (material.SSSWeight <= kFeatureEps) return vec3(0.0);

    float wrap     = mix(0.25, 0.65, clamp(material.SSSProfile, 0.0, 1.0));
    float NdotL    = dot(N, L);
    float wrapped  = max(NdotL + wrap, 0.0) / (1.0 + wrap);
    float scatter  = wrapped * mix(0.35, 1.0, clamp(material.Thickness, 0.0, 1.0));
    scatter       *= material.SSSWeight * (1.0 - material.Metallic);

    return material.Albedo * material.SSSColor * scatter;
}

vec3 ThinFilm(vec3 spec, Material m, float HdotV)
{
   // if (m.ThinFilmWeight < 1e-3) return spec;
    float t = mix(300.0, 800.0, clamp(m.ThinFilmThickness, 0.0, 1.0));
    vec3  phase = vec3(t) / vec3(580.0, 550.0, 440.0);
    vec3  irid  = 0.5 + 0.5 * cos(6.2831853 * phase * HdotV);
    return mix(spec, spec * irid, m.ThinFilmWeight);
}

vec3 ClearCoat(Material m, vec3 N, vec3 V, vec3 L, vec3 H, vec3 radiance, float NdotL)
{
    if (m.CoatWeight < 1e-3) return vec3(0.0);

    float NdotV = max(dot(N, V), 0.0);
    float NDF   = DistributionGGX(N, H, max(m.CoatRoughness, 0.04));
    float G     = GeometrySmith(N, V, L, m.CoatRoughness);
    vec3  Fc    = fresnelSchlickRoughness(max(dot(H, V), 0.0), vec3(0.04), m.CoatRoughness);
    vec3  spec  = (NDF * G * Fc) / max(4.0 * NdotV * NdotL, 1e-4);
    return spec * m.CoatColor * m.CoatWeight * radiance * NdotL;
}

vec3 ReconstructWorldPos(float depth)
{
    vec2 uv  = gl_FragCoord.xy * sceneDataBuffer.InvertResolution;
    vec2 ndc = uv * 2.0 - 1.0; 
    vec4 view = sceneDataBuffer.InverseOrthoProjection * vec4(ndc, depth, 1.0);
    view.xyz /= max(view.w, 1e-6);
    return (sceneDataBuffer.InverseOrthoView * vec4(view.xyz, 1.0)).xyz;
}