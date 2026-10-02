struct ImportMaterial
{
    vec3  Albedo;
    vec3  ClearcoatTint;
    vec3  SheenColor;
    vec3  SSSColor;
    vec3  AttenuationColor;
    vec3  Emission;

    float Metallic;
    float Roughness;
    float AmbientOcclusion;
    float IOR;
    float NormalStrength;
    float Height;

    float CoatWeight;
    float CoatRoughness;
    float CoatDarkening;

    float SheenWeight;
    float SheenRoughness;

    float SSSWeight;
    float SSSProfile;
    float Thickness;

    float TransmissionWeight;
    float AttenuationDistance;

    float Anisotropy;
    float AnisotropyRotation;
    float ThinFilmWeight;
    float ThinFilmThickness;
    float EmissionIntensity;
    float AlphaCutoff;

    uint  AlbedoMap;
    uint  NormalMap;
    uint  HeightMap;
    uint  AlphaMap;
    uint  MetallicMap;
    uint  RoughnessMap;
    uint  AmbientOcclusionMap;
    uint  EmissionMap;
    uint  ClearCoatColorMap;
    uint  ClearCoatPropertiesMap;
    uint  SheenMap;
    uint  SheenPropertiesMap;
    uint  SSSColorMap;
    uint  SSSPropertiesMap;
    uint  AttenuationColorMap;
    uint  AttenuationPropertiesMap;
    uint  AnisotropyPropertiesMap;
    uint  IORMap;

    uint  ShadingModel;
    uint  FeatureMask;

    // runtime-only — never stored in the pool
    vec3  NormalTS;
    float Alpha;
    float IORNorm;
};

struct TextureMetadata
{
    uint Width;
    uint Height;
    uint Depth;
    uint MipLevels;
    uint LayerCount;
    uint Format;
    uint TextureType;
    uint ArrayIndex;
};

struct BakedMaps
{
    vec3  Albedo;
    float Alpha;
    vec3  Emission;
    vec3  TangentNormal;
    float NormalStrength;
    float Height;
    float Metallic, Roughness, AO, IORNorm;
    vec3  SheenColor;
    float SheenWeight;
    vec3  SSSColor;
    float SSSWeight, SSSProfile, Thickness, SheenRoughness;
    float CoatWeight, CoatRoughness, CoatDarkening;
      vec3  AttenuationColor;
       float AttenuationDistance;
          float TransmissionWeight;
    float Anisotropy, AnisotropyRotation, ThinFilmWeight, ThinFilmThickness;
    uint  FeatureMask;
};

struct PackedMaterial
{
    uint AlbedoTextureId;
    uint NormalTextureId;
    uint MROTextureId;
    uint ClearCoatOrTranslucentTextureId;
    uint SubSurfaceScatteringOrTranslucentPropertiesTextureId;
    uint SubSurfaceScatteringPropertiesTextureId;
    uint SheenTextureId;
    uint AnisotropyTextureId;
    uint EmissionTextureId;
    uint TranslucentTextureId;
    uint TranslucentPropertiesTextureId;
    uint ShadingModel;
    uint FeatureMask;
    vec3 ClearcoatTint;
    float IOR;
    float AlphaCutOff;
};

struct Material
{
    vec3  Position;

    float Depth;

    vec3  Albedo;
    float Alpha;
    float Metallic;

    vec3  Normal;
    float Roughness;

    vec3  Emission;
    float AmbientOcclusion;

    float Specular;
    float IOR;
    float SelfShadow;
    float _pad0;

    vec3  CoatColor;
    float CoatWeight;
    float CoatRoughness;
    float CoatDarkening;
    float _pad1;
    float _pad2;

    vec3  SheenColor;
    float SheenWeight;
    float SheenRoughness;
    float _pad3;
    float _pad4;

    vec3  SSSColor;
    float SSSWeight;
    float Thickness;
    float SSSProfile;
    float TransmissionWeight;
    float AttenuationDistance;

    vec3  AttenuationColor;
    float Anisotropy;
    float AnisotropyRotation;
    float ThinFilmWeight;
    float ThinFilmThickness;
    float _pad5;

    uint  ShadingModel;
    uint  FeatureMask;
};

struct CubeMapMaterial
{
    uint CubeMapId;
    uint IrradianceMapId;
    uint PrefilterMapId;
};

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

float CoatFresnel(vec3 N, vec3 V, float weight)
{
    return weight * fresnelSchlickRoughness(max(dot(N, V), 0.0), vec3(0.04), 0.0).x;
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
    float wrap    = mix(0.25, 0.65, clamp(material.SSSProfile, 0.0, 1.0));
    float NdotL   = dot(N, L);
    float wrapped = max(NdotL + wrap, 0.0) / (1.0 + wrap);
    float scatter = wrapped * mix(0.35, 1.0, clamp(material.Thickness, 0.0, 1.0));
    scatter      *= material.SSSWeight * (1.0 - material.Metallic);
    return material.Albedo * material.SSSColor * scatter;
}

vec3 ThinFilm(vec3 spec, Material material, float HdotV)
{
    float t = mix(300.0, 800.0, clamp(material.ThinFilmThickness, 0.0, 1.0));
    vec3  phase = vec3(t) / vec3(580.0, 550.0, 440.0);
    vec3  irid  = 0.5 + 0.5 * cos(6.2831853 * phase * HdotV);
    return mix(spec, spec * irid, material.ThinFilmWeight);
}

vec3 ClearCoat(Material material, vec3 N, vec3 V, vec3 L, vec3 H, vec3 radiance, float NdotL)
{
    if (material.CoatWeight < 1e-3) return vec3(0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NDF   = DistributionGGX(N, H, max(material.CoatRoughness, 0.04));
    float G     = GeometrySmith(N, V, L, material.CoatRoughness);
    vec3  Fc    = fresnelSchlickRoughness(max(dot(H, V), 0.0), vec3(0.04), material.CoatRoughness);
    vec3  spec  = (NDF * G * Fc) / max(4.0 * NdotV * NdotL, 1e-4);
    return spec * material.CoatColor * material.CoatWeight * radiance * NdotL;
}

