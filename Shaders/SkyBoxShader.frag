#version 460
#extension GL_ARB_separate_shader_objects : enable

#include "Constants.glsl"

layout(location = 0) in vec3 TexCoords;
layout(location = 0) out vec4 FragColor;

layout(constant_id = 0) const uint DescriptorBindingType0 = SkyBoxDescriptor;
layout(binding = 0) uniform samplerCube CubeMap;

layout(push_constant) uniform SkyBoxViewData {
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
} skyBoxViewData;

void main() 
{
    vec3 color = texture(CubeMap, normalize(TexCoords)).rgb;
    FragColor = vec4(color, 1.0f);
}