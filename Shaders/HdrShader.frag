#version 460
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 
#include "MemoryPoolBindings.glsl"

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 outColor;

const float Gamma    = 2.2;
const float Exposure = 0.6;

vec3 ACESFilm(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec4 a = vec4(1.0f);
        for(int x = 0; x < bindlessBuffer.Texture2DCount; x++)
    {
    a = texture(TextureMap[x], TexCoords).rgba;
    }
    for(int x = 0; x < 6; x++)
    {
    a =  texture(CubeMap[x], vec3(0.0f)).rgba;
    }

    vec3 hdr = texture(TextureMap[sceneDataBuffer.HDRMapInputIndex], TexCoords).rgb;
    hdr *= Exposure;

    vec3 mapped = ACESFilm(hdr);
    mapped = pow(mapped, vec3(1.0 / Gamma));

    outColor = vec4(mapped, 1.0);
}