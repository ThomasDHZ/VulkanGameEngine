#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_NV_shader_buffer_load : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "BindlessHelpers.glsl"

layout(push_constant) uniform BloomPushConst
{
    uint vertTextureIndex;
    uint horTextureIndex;
} pushConst;

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 outColor;

void main() 
{
    vec3 vertTexture = texture(TextureMap[pushConst.vertTextureIndex], TexCoords).rgb;
    vec3 horizontalTexture = texture(TextureMap[pushConst.horTextureIndex], TexCoords).rgb;
    vec3 textureAdd = vertTexture + horizontalTexture;
    outColor = vec4(textureAdd, 1.0);
}
