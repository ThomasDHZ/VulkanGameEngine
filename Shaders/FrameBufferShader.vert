#version 460
#extension GL_ARB_separate_shader_objects : enable
//#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 1;
layout(binding = 0) uniform sampler2D FrameBufferTexture;
layout(location = 0) out vec2 fragTexCoord;

void main() 
{
//    if(gl_VertexIndex == 0)
//	{
//		debugPrintfEXT(": %i \n", 432);
//	}

    fragTexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(fragTexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
}