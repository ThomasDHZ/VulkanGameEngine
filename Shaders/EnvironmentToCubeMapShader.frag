#version 460
#extension GL_ARB_separate_shader_objects : enable

#include "Constants.glsl"
layout(constant_id = 0)  const uint DescriptorBindingType0  = TextureDescriptor;
layout(binding = 0) uniform sampler2D EnvironmentMap;

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
    
    vec3 color = texture(EnvironmentMap, uv).rgb;
    FragColor = vec4(color, 1.0f);
}