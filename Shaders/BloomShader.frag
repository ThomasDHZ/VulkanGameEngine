#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 1;
layout(constant_id = 1) const uint DescriptorBindingType1 = 1;

layout(binding = 0) uniform sampler2D vertBloomTexture;
layout(binding = 1) uniform sampler2D horizontalBloomTexture;

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 outColor;

void main() 
{
    vec3 vertTexture = texture(vertBloomTexture, TexCoords).rgb;
    vec3 horizontalTexture = texture(horizontalBloomTexture, TexCoords).rgb;
    vec3 textureAdd = vertTexture + horizontalTexture;
    outColor = vec4(textureAdd, 1.0);
}
