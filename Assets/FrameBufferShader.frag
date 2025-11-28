#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 1;

layout(binding = 0) uniform sampler2D FrameBufferTexture;

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 outColor;

const float Gamma = 2.2f;
const float Exposure = 1.0f;

void main() 
{
    vec3 color = vec3(1.0f) - exp(-texture(FrameBufferTexture, TexCoords).rgb * Exposure);
    color = pow(color, vec3(1.0f / Gamma));
    outColor = vec4(color, 1.0f);
}
