#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_NV_shader_buffer_load : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "BindlessHelpers.glsl"

layout(push_constant) uniform FrameBufferTextureIndexPushConst
{
    uint FrameBufferTextureIndex;
} pushConst;

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(texture(TextureMap[pushConst.FrameBufferTextureIndex], TexCoords).rgb, 1.0f);
}
