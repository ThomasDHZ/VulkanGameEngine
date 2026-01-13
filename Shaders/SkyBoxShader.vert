#version 460
#extension GL_ARB_separate_shader_objects : enable
#include "Constants.glsl"

layout(location = 0) in vec3 aPos;
layout(location = 0) out vec3 TexCoords;

layout(push_constant) uniform SkyBoxViewData {
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
} skyBoxViewData;

void main() {
    TexCoords = aPos;
    mat4 viewRotOnly = mat4(mat3(skyBoxViewData.View));
    vec4 pos = skyBoxViewData.Projection * viewRotOnly * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}