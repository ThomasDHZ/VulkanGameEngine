#version 460
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 
#include "MemoryPoolBindings.glsl"

layout(location = 0) in vec3 pos;
layout(location = 0) out vec4 FragColor;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() 
{
    vec2 uv = SampleSphericalMap(normalize(pos));
    
    vec3 color = texture(TextureMap[sceneDataBuffer.EnvironmentMapIndex], uv).rgb;
    FragColor = vec4(color, 1.0f);
}