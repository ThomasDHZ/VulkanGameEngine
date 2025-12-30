#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 1;
layout(constant_id = 1) const uint DescriptorBindingType1 = 1;

layout(binding = 0) uniform sampler2D HDRSceneTexture;
layout(binding = 1) uniform sampler2D BloomTexture;

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 outColor;

const float Gamma = 2.2;
const float Exposure = 1.0;
void main() 
{
    vec3 hdrColor = texture(HDRSceneTexture, TexCoords).rgb;
    vec3 finalColor = hdrColor;
    vec3 mapped = vec3(1.0) - exp(-finalColor * Exposure);
    mapped = pow(mapped, vec3(1.0 / Gamma));
    outColor = vec4(mapped, 1.0);
}
