#version 460
#extension GL_ARB_separate_shader_objects : enable
//#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 1;
layout(constant_id = 1) const uint DescriptorBindingType1 = 1;
layout(constant_id = 2) const uint DescriptorBindingType2 = 1;
layout(constant_id = 3) const uint DescriptorBindingType3 = 1;
layout(constant_id = 4) const uint DescriptorBindingType4 = 1;

layout(binding = 0) uniform sampler2D PositionDataMap;
layout(binding = 1) uniform sampler2D AlbedoMap;
layout(binding = 2) uniform sampler2D NormalMap;
layout(binding = 3) uniform sampler2D MatRoughAOMap;
layout(binding = 4) uniform sampler2D EmissionMap;
layout(location = 0) out vec2 fragTexCoord;

void main() 
{
    fragTexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(fragTexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
}