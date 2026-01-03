struct DirectionalLightBuffer
{
    vec3 LightColor;
    vec3 LightDirection;
    float LightIntensity;
    mat4 LightSpaceMatrix;
};

struct PointLightBuffer
{
    vec3 LightPosition;
    vec3 LightColor;
    float LightRadius;
    float LightIntensity;
};
