struct MaterialProperitiesBuffer
{
    vec3 Albedo;
    vec3 SheenColor;
    vec3 SubSurfaceScattering;
    vec3 Emission;
    vec3 ClearcoatTint;
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
    uint SubSurfaceScatteringMap;
    uint SheenMap;
    uint ClearCoatMap;
    uint AmbientOcclusionMap;
    uint NormalMap;
    uint AlphaMap;
    uint EmissionMap;
    uint HeightMap;
};