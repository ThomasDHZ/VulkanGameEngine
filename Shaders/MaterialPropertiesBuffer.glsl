struct ImportMaterialBuffer
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

struct Material
{
    uint AlbedoDataId;             //Albedo/Alpha                                                                               - R8G8B8A8_SRGB
    uint NormalDataId;             //Normal/NormalStrength/Height                                                               - R16G16B16A16_UNORM
    uint PackedMRODataId;          //vec4(Metallic/Rough, AO/ClearcoatTint, ClearcoatStrength/ClearcoatRoughness, unused)       - R16G16B16A16_UNORM
    uint PackedSheenSSSDataId;     //vec4(sheenColor.r/sheenColor.g, sheenColor.b/sheenIntensity, sss.r/sss.g, sss.b/thickness) - R16G16B16A16_UNORM
    uint UnusedDataId;             //vec4(                                                                                    ) - R16G16B16A16_UNORM
    uint EmissionDataId;           //Emission                                                                                   - R8G8B8A8_SRGB
};

struct CubeMapMaterial
{
    uint CubeMapId;
    uint IrradianceId;
    uint PrefilterId;
};