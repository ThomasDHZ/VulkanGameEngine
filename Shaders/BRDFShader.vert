#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_NV_shader_buffer_load : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "BindlessHelpers.glsl"

layout(location = 0) out vec2 TexCoords;

void main() {
	TexCoords = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(TexCoords * 2.0f - 1.0f, 0.0f, 1.0f);
}
