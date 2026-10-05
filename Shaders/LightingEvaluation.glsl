#ifndef LIGHTING_EVAL_GLSL
#define LIGHTING_EVAL_GLSL

// Shared direct/IBL evaluation for the deferred lighting pass and the forward alpha passes.
// Alpha shaders must set material.Position from the interpolated world position before calling.

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

vec3 PerspectiveView(vec3 worldPos)
{
    vec3 toCam = sceneDataBuffer.PerspectiveCameraPosition - worldPos;
    if (dot(toCam, toCam) > 1e-6)
        return normalize(toCam);
    return normalize(-sceneDataBuffer.PerspectiveViewDirection);
}

float LambdaAniso(vec3 T, vec3 B, vec3 N, vec3 W, float at, float ab)
{
    float ToW = dot(T, W);
    float BoW = dot(B, W);
    float NoW = max(abs(dot(N, W)), 1e-4);
    float a2  = ToW * ToW * at * at + BoW * BoW * ab * ab;
    return (-1.0 + sqrt(1.0 + a2 / (NoW * NoW))) * 0.5;
}

float GeometrySmithAniso(vec3 T, vec3 B, vec3 N, vec3 V, vec3 L, float rough, float aniso)
{
    if (abs(aniso) < kFeatureEps)
        return GeometrySmith(N, V, L, rough);

    float at = max(rough * (1.0 + aniso), 0.001);
    float ab = max(rough * (1.0 - aniso), 0.001);
    return 1.0 / (1.0 + LambdaAniso(T, B, N, V, at, ab) + LambdaAniso(T, B, N, L, at, ab));
}

vec3 EvaluateDirect(vec3 F0, vec3 V, vec3 L, vec3 radiance, Material material)
{
    vec3 N = material.Normal;
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);

    vec3 Lo = SubSurfaceScatteringData(material, N, L) * radiance;

    float back = max(-dot(N, L), 0.0);
    Lo += material.Albedo * material.SSSColor * material.SSSWeight * back
        * material.Thickness * radiance * exp(-1.0 / max(material.Thickness, 0.05));

    if (NdotL <= 0.0)
        return Lo;

    vec3 T, B;
    AnisoFrame(N, material.AnisotropyRotation, T, B);
    float NDF   = DistributionGGX_Aniso(N, T, B, H, material.Roughness, material.Anisotropy);
    float G     = GeometrySmithAniso(T, B, N, V, L, material.Roughness, material.Anisotropy);
    vec3  F     = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, material.Roughness);
    float NdotV = max(dot(N, V), 0.0);

    float sssCut   = clamp(material.SSSWeight, 0.0, 1.0);
    float sheenCut = clamp(material.SheenWeight, 0.0, 1.0) * (1.0 - material.Metallic);
    vec3  kD       = (vec3(1.0) - F) * (1.0 - material.Metallic) * (1.0 - sssCut) * (1.0 - sheenCut);

    vec3 spec = (NDF * G * F) / max(4.0 * NdotV * NdotL, 1e-4);
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
    return Lo;
}

vec3 DirectionalLightFunc(vec3 F0, vec3 V, Material material)
{
    vec3 Lo = vec3(0.0);
    for (uint x = 0; x < bindlessBuffer.DirectionalLightCount; ++x)
    {
        const DirectionalLightBuffer light = GetDirectionalLight(x);
        if (light.LightActive != 1u) continue;
        if (dot(light.LightDirection, light.LightDirection) < 1e-8) continue;

        vec3 L = normalize(-light.LightDirection);
        vec3 radiance = light.LightColor * light.LightIntensity * material.SelfShadow;
        Lo += EvaluateDirect(F0, V, L, radiance, material);
    }
    return Lo;
}

vec3 PointLightFunc(vec3 F0, vec3 V, Material material)
{
    vec3 Lo = vec3(0.0);
    for (uint x = 0; x < bindlessBuffer.PointLightCount; ++x)
    {
        const PointLightBuffer light = GetPointLight(x);
        if (light.LightActive != 1u) continue;

        vec3  toLight  = light.LightPosition - material.Position;
        float distance = length(toLight);
        if (distance > light.LightRadius) continue;

        vec3 L = toLight / max(distance, 1e-4);
        float atten = 1.0 - clamp(distance / light.LightRadius, 0.0, 1.0);
        atten *= atten;
        vec3 radiance = light.LightColor * light.LightIntensity * atten * material.SelfShadow;
        Lo += EvaluateDirect(F0, V, L, radiance, material);
    }
    return Lo;
}

vec3 ImageBasedLighting(vec3 F0, vec3 V, vec3 N_ibl, vec3 R, Material material)
{
    vec3 F  = fresnelSchlickRoughness(max(dot(N_ibl, V), 0.0), F0, material.Roughness);
    float sssCut   = clamp(material.SSSWeight, 0.0, 1.0);
    float sheenCut = clamp(material.SheenWeight, 0.0, 1.0) * (1.0 - material.Metallic);
    vec3 kD = (vec3(1.0) - F) * (1.0 - material.Metallic) * (1.0 - sssCut) * (1.0 - sheenCut);

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
    return ambient;
}

// Coverage for a forward-blended surface. Transmission follows fresnel instead of
// the albedo alpha, and it is not clamped into a forced 0.08..0.75 window.
// Beer-Lambert of the background cannot be done here: this pass does not sample
// the opaque lighting target. The blend shows that target through `alpha`.
float TransmissionCoverage(Material material, float albedoAlpha, float NdotV, float F0d)
{
    float T = clamp(material.TransmissionWeight, 0.0, 1.0);
    vec3  F = fresnelSchlickRoughness(NdotV, vec3(F0d), material.Roughness);
    float Ft = clamp(1.0 - max(F.r, max(F.g, F.b)), 0.0, 1.0);
    float glass = 1.0 - Ft;
    return clamp(mix(albedoAlpha, glass, T), 0.0, 1.0);
}

#endif