const float kFeatureEps = 1e-3f;
const uint FEAT_COAT         = 1u << 0;
const uint FEAT_SHEEN        = 1u << 1;
const uint FEAT_SSS          = 1u << 2;
const uint FEAT_TRANSMISSION = 1u << 3;
const uint FEAT_ANISO        = 1u << 4;
const uint FEAT_FILM         = 1u << 5;
const uint FEAT_TWO_SIDED    = 1u << 6;
const uint FEAT_COAT_NORMAL  = 1u << 7;

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
    float Specular;
    float IOR;
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

    uint  AlbedoMap;
    uint  MetallicMap;
    uint  RoughnessMap;
    uint  ThicknessMap;
    uint  SSSColorMap;
    uint  SSSPropertiesMap;
    uint  AttenuationColorMap;
    uint  SheenMap;
    uint  SheenPropertiesMap;
    uint  ClearCoatColorMap;
    uint  ClearCoatPropertiesMap;
    uint  AnisotropyPropertiesMap;
    uint  AmbientOcclusionMap;
    uint  NormalMap;
    uint  AlphaMap;
    uint  EmissionMap;
    uint  HeightMap;
    uint  ShadingModel;
    uint  FeatureMask;
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
    uint AlbedoDataId;
    uint NormalDataId;
    uint MRODataId;
    uint ClearCoatDataId;
    uint FeatureADataId;
    uint FeatureBDataId;
    uint FeatureCDataId;
    uint EmissionDataId;
    uint ShadingModel;
    uint FeatureMask;  


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

