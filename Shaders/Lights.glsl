struct DirectionalLightBuffer
{
    vec3 LightColor;
    vec3 LightDirection;
    float LightIntensity;
    float ShadowStrength;       
    float ShadowBias;          
    float ShadowSoftness;
    uint  LightActive;
};

struct PointLightBuffer
{
    vec3  LightPosition;
    vec3  LightColor;
    float LightRadius;
    float LightIntensity;
    float ShadowStrength;      
    float ShadowBias;          
    float ShadowSoftness;
    int   LightLayer;
    uint  LightActive;
};
