struct DirectionalLightBuffer
{
    vec3 LightColor;
    vec3 LightDirection;
    float LightIntensity;
};

struct PointLightBuffer
{
    vec3 LightPosition;
    vec3 LightColor;
    float LightRadius;
    float LightIntensity;
};
