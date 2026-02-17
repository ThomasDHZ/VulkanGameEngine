struct ImportMaterial
{
    vec3  Albedo;
    vec3  SheenColor;
    vec3  SubSurfaceScatteringColor;
    vec3  Emission;
    float ClearcoatTint;
    float Metallic;
    float Roughness;
    float AmbientOcclusion;
    float ClearcoatStrength;
    float ClearcoatRoughness;
    float SheenIntensity;
    float Thickness;
    float Anisotropy;
    float AnisotropyRotation;
    float NormalStrength;
    float HeightScale;
    float Height;
    float Alpha;

    uint AlbedoMap;
    uint MetallicMap;
    uint RoughnessMap;
    uint ThicknessMap;
    uint SubSurfaceScatteringColorMap;
    uint SheenMap;
    uint ClearCoatMap;
    uint AnisotropyMap;
    uint AmbientOcclusionMap;
    uint NormalMap;
    uint AlphaMap;
    uint EmissionMap;
    uint HeightMap;
};

struct PackedMaterial
{
    uint AlbedoDataId;             //Albedo/Alpha                                                                               - R8G8B8A8_SRGB
    uint NormalDataId;             //Normal/NormalStrength/Height                                                               - R16G16B16A16_UNORM
    uint PackedMRODataId;          //vec4(Metallic/Rough, AO/ClearcoatTint, ClearcoatStrength/ClearcoatRoughness, unused)       - R16G16B16A16_UNORM
    uint PackedSheenSSSDataId;     //vec4(sheenColor.r/sheenColor.g, sheenColor.b/sheenIntensity, sss.r/sss.g, sss.b/thickness) - R16G16B16A16_UNORM
    uint UnusedDataId;             //vec4(                                                                                    ) - R16G16B16A16_UNORM
    uint EmissionDataId;           //Emission                                                                                   - R8G8B8A8_SRGB
};

struct Material
{
    vec3 Position;
    vec3 Albedo;
    vec3 Normal;
    vec3 ParallaxInfo;
    vec3 Emission;
    vec3 Sheen;
    vec3 SubSurfaceScattering;

    float Height;
    float Metallic;
    float Roughness;
    float AmbientOcclusion;
    float ClearCoatTint;
    float ClearcoatStrength;
    float ClearcoatRoughness;
    float SheenIntensity;
    float Thickness;      
    float ShiftedHeight;
};

struct CubeMapMaterial
{
    uint CubeMapId;
    uint IrradianceMapId;
    uint PrefilterMapId;
};