  #version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Constants.glsl"

layout(location = 0) in vec3 TexCoords;
layout(location = 0) out vec4 FragColor;

layout(constant_id = 0) const uint DescriptorBindingType0 = SkyBoxDescriptor;
layout(binding = 0) uniform samplerCube CubeMap;

layout(push_constant) uniform SceneDataBuffer {
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
} sceneData;

void main() 
{
//    vec2 ndc = inUV * 2.0 - 1.0;   // comment this out if inUV already -1..1
//    vec3 dir = normalize(cameraForward + cameraRight * ndc.x + cameraUp * ndc.y);
//    vec3 color = texture(CubeMap, dir).rgb;

    // For testing: output solid red first
     FragColor = vec4(1.0, 0.0, 0.0, 1.0);

  //  FragColor = vec4(color, 1.0);
}