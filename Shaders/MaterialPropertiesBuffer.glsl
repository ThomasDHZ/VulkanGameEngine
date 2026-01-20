struct MaterialProperitiesBuffer
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
    uint AmbientOcclusionMap;
    uint NormalMap;
    uint AlphaMap;
    uint EmissionMap;
    uint HeightMap;
};