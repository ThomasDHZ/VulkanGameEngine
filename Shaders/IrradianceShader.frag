#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_NV_shader_buffer_load : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "BindlessHelpers.glsl"

layout(push_constant) uniform IrradianceShaderConstants 
{
    uint CubeMapIndex;
    float sampleDelta;
} irradianceShaderConstants;

layout(location = 0) in vec3 WorldPos;
layout(location = 0) out vec4 outColor;


void main() 
{
    CubeMapMaterial CubeMapMaterial = GetCubeMapMaterial(irradianceShaderConstants.CubeMapIndex);

    vec3 N = normalize(WorldPos);
    vec3 up    = abs(N.y) < 0.999f ? vec3(0.0f, 1.0f, 0.0f) : vec3(0.0f, 0.0f, 1.0f);
    vec3 right = normalize(cross(up, N));
    up         = cross(N, right);

    vec3 irradiance = vec3(0.0f);
    float sampleDelta = irradianceShaderConstants.sampleDelta > 0.0f ? irradianceShaderConstants.sampleDelta : 0.025f;

    float nrSamples = 0.0f;
    for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta)
    {
        for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta)
        {
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            irradiance += texture(CubeMaps[CubeMapMaterial.CubeMapId], sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = irradiance = (PI / float(nrSamples)) * irradiance;
    outColor = vec4(irradiance, 1.0f);
}
