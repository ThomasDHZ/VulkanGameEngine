  #version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Constants.glsl"

layout (location = 0) in vec3 aPos;
layout(location = 0) out vec3 TexCoords;

layout(constant_id = 0) const uint DescriptorBindingType0 = SkyBoxDescriptor;
layout(binding = 0) uniform samplerCube CubeMap;

layout(push_constant) uniform SceneDataBuffer {
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
} sceneData;

void main() {
	TexCoords = aPos;
	vec4 pos = sceneData.Projection * sceneData.View * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}
