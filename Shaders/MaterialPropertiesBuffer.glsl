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

struct PackedMaterial
{
    uint AlbedoTextureId;
    uint NormalTextureId;
    uint MROTextureId;
    uint ClearCoatOrTranslucentTextureId;
    uint SubSurfaceScatteringOrTranslucentPropertiesTextureId;
    uint SheenTextureId;
    uint AnisotropyTextureId;
    uint EmissionTextureId;
    uint ShadingModel;
    uint FeatureMask;
    uint _pad0;
    uint _pad1;
    vec3  ClearcoatTint;
    float SheenRoughness;
    float SSSWeight;
    float SSSProfile;
    float IOR;
    float AlphaCutOff;
};

struct Material
{
    vec3  Position;
    float Depth;

    vec3  Albedo;
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

