#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 1;

layout(binding = 0) uniform sampler2D sdfSampler;

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 outColor;

const float Gamma = 2.2;
const float Exposure = 1.0;
void main() 
{
	float dist = texture(sdfSampler, TexCoords).r;
	if (dist > 0.0) outColor = vec4(0.0, dist * 0.2, 0.0, 1.0f);  // green outside
	else outColor = vec4(-dist * 0.2, 0.0, 0.0, 1.0f);             // red inside
	if (abs(dist) < 0.02) outColor = vec4(1.0, 1.0, 0.0, 1.0f);
}
