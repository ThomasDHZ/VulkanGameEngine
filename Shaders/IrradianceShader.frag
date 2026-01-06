#version 460

#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_multiview : enable

#include "Constants.glsl"

layout(constant_id = 0) const uint DescriptorBindingType0 = SkyBoxDescriptor;
layout(binding = 0) uniform samplerCube environmentMap;

layout(location = 0) in vec3 WorldPos;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform IrradianceShaderConstants {
    float sampleDelta;
} irradianceShaderConstants;

void main() 
{
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
            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0f / nrSamples);
    outColor = vec4(irradiance, 1.0f);
}
