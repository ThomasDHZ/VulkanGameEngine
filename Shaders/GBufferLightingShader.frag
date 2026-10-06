#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"
#include "MemoryPoolBindings.glsl"

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

Material UnpackMaterial();
vec3 DirectionalLightFunc(vec3 F0, vec3 V, Material material);
vec3 PointLightFunc(vec3 F0, vec3 V, Material material);
vec3 ImageBasedLighting(vec3 F0, vec3 V, vec3 N_ibl, vec3 R, Material material);

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
        outBloom = vec4(0.0);
        return;
    }

    vec3  toCamera  = sceneDataBuffer.PerspectiveCameraPosition - material.Position;
    vec3  N         = material.Normal;
    vec3  V         = dot(toCamera, toCamera) > 1e-6 ? normalize(toCamera) : normalize(-sceneDataBuffer.PerspectiveViewDirection);
    vec3  R         = reflect(-V, N);
    float F0d       = pow((material.IOR - 1.0) / (material.IOR + 1.0), 2.0);
    vec3  F0        = mix(vec3(F0d), material.Albedo, material.Metallic);

    //vec3 Lo         = DirectionalLightFunc(F0, V, material);
    vec3 Lo           = PointLightFunc(F0, V, material);
    vec3 color      = ImageBasedLighting(F0, V, N, R, material) + Lo + material.Emission;

     const PointLightBuffer light = GetPointLight(0);
    outColor = vec4(Lo, 1.0);
    outBloom        = vec4(material.Emission + max(color - vec3(1.0), vec3(0.0)), 1.0);
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
    m.Position         = positionAttachment.xyz;
    m.Albedo           = albedoAttachment.rgb;
    m.Metallic         = mroAttachment.r;
    m.Roughness        = mroAttachment.g;
    m.AmbientOcclusion = mroAttachment.b;
    m.IOR              = mroAttachment.a * 2.0 + 1.0;
    m.Normal           = normalize(OctahedronDecode(normalAttachment.xy * 2.0 - 1.0));
    m.Emission         = emissionAttachment.rgb;

    m.SheenColor     = featureAAttachment.rgb;
    m.SheenRoughness = sheenR_sheenW.x;
    m.SheenWeight    = sheenR_sheenW.y;
    m.SSSColor       = featureBAttachment.rgb;
    m.SSSWeight      = sssW_coatW.x;
    m.SSSProfile     = coatD_prof.y;
    m.Thickness      = thick_coatR.x;
    m.CoatColor      = vec3(1.0);
    m.CoatWeight     = sssW_coatW.y;
    m.CoatRoughness  = thick_coatR.y;
    m.CoatDarkening  = coatD_prof.x;
    m.Anisotropy         = aniso.x;
    m.AnisotropyRotation = aniso.y;
    m.ThinFilmWeight     = film.x;
    m.ThinFilmThickness  = film.y;
    m.FeatureMask = uint(normalAttachment.b * 65535.0 + 0.5);
    m.SelfShadow  = normalAttachment.a;
    m.ShadingModel = 0u;
    return m;
}

vec3 DirectionalLightFunc(vec3 F0, vec3 V, Material material)
{
    vec3 Lo = vec3(0.0);
    for (uint x = 0; x < bindlessBuffer.DirectionalLightCount; ++x)
    {
        const DirectionalLightBuffer light = GetDirectionalLight(x);
        if(light.LightActive != 1u) continue;

        vec3 L = normalize(-light.LightDirection);
        vec3 H = normalize(V + L);
        vec3 N = material.Normal;
        vec3 T, B;

        vec3 radiance = light.LightColor * light.LightIntensity * material.SelfShadow;
        float NdotL   = max(dot(N, L), 0.0);

        Lo += SubSurfaceScatteringData(material, N, L) * radiance;
        float back = max(-dot(N, L), 0.0);
        Lo += material.Albedo * material.SSSColor * material.SSSWeight * back * material.Thickness * radiance *exp(-1.0 / max(material.Thickness, 0.05));

        if (NdotL <= 0.0) continue;

        AnisoFrame(N, material.AnisotropyRotation, T, B);
        float NDF = DistributionGGX_Aniso(N, T, B, H, material.Roughness, material.Anisotropy);
        float G   = GeometrySmith(N, V, L, material.Roughness);
        vec3  F   = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, material.Roughness);
        vec3  kD  = (vec3(1.0) - F) * (1.0 - material.Metallic);
        float NdotV = max(dot(N, V), 0.0);
        vec3  spec  = (NDF * G * F) / max(4.0 * NdotV * NdotL, 1e-4);
        spec = ThinFilm(spec, material, max(dot(H, V), 0.0));

        vec3 base = (kD * material.Albedo / PI + spec) * radiance * NdotL;
        if (material.CoatWeight > 0.0)
        {
            float coatF = CoatFresnel(N, V, material.CoatWeight);
            base *= mix(1.0, material.CoatDarkening, material.CoatWeight);
            base *= (1.0 - coatF);
        }
        base += SheenData(material, N, V) * radiance * NdotL;

        Lo += base;
        Lo += ClearCoat(material, N, V, L, H, radiance, NdotL);
    }
    return Lo;
}

vec3 PointLightFunc(vec3 F0, vec3 V, Material material)
{
    const float kLayerSpacing = 64.0;
    vec3 Lo = vec3(0.0);

    for (uint x = 0; x < bindlessBuffer.PointLightCount; ++x)
    {
        const PointLightBuffer light = GetPointLight(x);
        if (light.LightActive != 1u) continue;

        float spriteLayer = material.Position.z / kLayerSpacing;
        float lightLayer  = 1; // uint/float layer index, not world Z
        float layerDelta  = abs(spriteLayer - lightLayer);

        vec2 toLightXY = light.LightPosition.xy - material.Position.xy;
        float distance = length(toLightXY);
        if (distance > light.LightRadius) continue;

        float atten = 1.0 - clamp(distance / light.LightRadius, 0.0, 1.0);
        atten *= atten;
        atten *= exp(-layerDelta * layerDelta); // 1 on the light's layer, ~0.37 one layer away, ~0.02 two away

        vec3 L = normalize(vec3(toLightXY, (lightLayer - spriteLayer) * kLayerSpacing));
        vec3 H = normalize(V + L);
        vec3 N = material.Normal;
        vec3 T, B;

        vec3 radiance = light.LightColor * light.LightIntensity * atten * material.SelfShadow;
        float NdotL = max(dot(N, L), 0.25); // billboard facing camera; geometric NdotL is ~0 for a 2D light

        Lo += SubSurfaceScatteringData(material, N, L) * radiance;
        float back = max(-dot(N, L), 0.0);
        Lo += material.Albedo * material.SSSColor * material.SSSWeight * back * material.Thickness * radiance * exp(-1.0 / max(material.Thickness, 0.05));

        AnisoFrame(N, material.AnisotropyRotation, T, B);
        float NDF = DistributionGGX_Aniso(N, T, B, H, material.Roughness, material.Anisotropy);
        float G   = GeometrySmith(N, V, L, material.Roughness);
        vec3  F   = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, material.Roughness);
        vec3  kD  = (vec3(1.0) - F) * (1.0 - material.Metallic);
        float NdotV = max(dot(N, V), 0.0);
        vec3  spec  = (NDF * G * F) / max(4.0 * NdotV * NdotL, 1e-4);
        spec = ThinFilm(spec, material, max(dot(H, V), 0.0));

        vec3 base = (kD * material.Albedo / PI + spec) * radiance * NdotL;
        if (material.CoatWeight > 0.0)
        {
            float coatF = CoatFresnel(N, V, material.CoatWeight);
            base *= mix(1.0, material.CoatDarkening, material.CoatWeight);
            base *= (1.0 - coatF);
        }
        base += SheenData(material, N, V) * radiance * NdotL;

        Lo += base;
        Lo += ClearCoat(material, N, V, L, H, radiance, NdotL);
    }
    return Lo;
}

vec3 ImageBasedLighting(vec3 F0, vec3 V, vec3 N_ibl, vec3 R, Material material)
{
    vec3 F  = fresnelSchlickRoughness(max(dot(N_ibl, V), 0.0), F0, material.Roughness);
    vec3 kD = (vec3(1.0) - F) * (1.0 - material.Metallic);

    vec3 irradiance = texture(CubeMap[sceneDataBuffer.IrradianceMapId], N_ibl).rgb;
    vec3 diffuseIBL = material.Albedo * irradiance * IBL_EXPOSURE;

    float maxLod = float(textureQueryLevels(CubeMap[sceneDataBuffer.PrefilterMapId]) - 1);
    float lod    = clamp(material.Roughness * maxLod, 0.0, maxLod);
    vec3  prefiltered = textureLod(CubeMap[sceneDataBuffer.PrefilterMapId], R, lod).rgb;

    vec2 brdf = texture(TextureMap[sceneDataBuffer.BRDFMapId],
                        vec2(max(dot(N_ibl, V), 0.0), material.Roughness)).rg;
    vec3 specularIBL = prefiltered * (F * brdf.x + brdf.y) * IBL_EXPOSURE;
    specularIBL = ThinFilm(specularIBL, material, max(dot(N_ibl, V), 0.0));

    vec3 ambient = (kD * diffuseIBL + specularIBL) * material.AmbientOcclusion;
    if (material.CoatWeight > 0.0)
    {
        float coatF   = CoatFresnel(N_ibl, V, material.CoatWeight);
        float coatLod = clamp(material.CoatRoughness * maxLod, 0.0, maxLod);
        vec3  coatPref = textureLod(CubeMap[sceneDataBuffer.PrefilterMapId], R, coatLod).rgb;
        ambient *= mix(1.0, material.CoatDarkening, material.CoatWeight);
        ambient *= (1.0 - coatF);
        ambient += coatPref * coatF * IBL_EXPOSURE * material.CoatColor;
    }
    ambient += SheenData(material, N_ibl, V) * irradiance * IBL_EXPOSURE * 0.25;
    ambient += material.SSSWeight * material.SSSColor * material.Albedo * irradiance * IBL_EXPOSURE * 0.25;
    return max(ambient, vec3(0.02) * material.Albedo);
}