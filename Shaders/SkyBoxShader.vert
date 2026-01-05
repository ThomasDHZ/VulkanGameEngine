#version 450
#extension GL_ARB_separate_shader_objects : enable
#include "Constants.glsl"

layout(location = 0) in vec3 aPos;
layout(location = 0) out vec2 TexCoords;

layout(push_constant) uniform SceneDataBuffer {
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
} sceneData;

void main() {
    TexCoords = vec2((gl_VertexIndex & 1) << 1, gl_VertexIndex >> 1) * 2.0 - 1.0;
    gl_Position = vec4(TexCoords, 0.9999, 1.0);
}