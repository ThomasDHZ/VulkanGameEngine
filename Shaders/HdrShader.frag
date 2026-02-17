#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_NV_shader_buffer_load : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "BindlessHelpers.glsl"

layout(push_constant) uniform HDRPushConst
{
    uint hdrTextureIndex;
    uint bloomTextureIndex;
} pushConst;

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 outColor;

const float Gamma = 2.2;
const float Exposure = 1.0;
void main() 
{
    vec3 hdrColor = texture(TextureMap[pushConst.hdrTextureIndex], TexCoords).rgb;
    vec3 finalColor = hdrColor;
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    mapped = pow(mapped, vec3(1.0 / Gamma));
    outColor = vec4(mapped, 1.0);
}
