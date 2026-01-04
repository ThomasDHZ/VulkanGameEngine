  #version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 TexCoords;
layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform samplerCube CubeMap;

layout(push_constant) uniform SceneDataBuffer {
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
} sceneData;

void main() 
{
    vec3 color = texture(CubeMap, TexCoords).rgb;
    FragColor = vec4(color, 1.0f);
}