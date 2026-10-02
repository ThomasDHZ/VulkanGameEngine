#version 460
#extension GL_EXT_multiview : enable
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 
#include "MemoryPoolBindings.glsl"

layout (location = 0) in vec3 aPos;
layout(location = 0) out vec3 WorldPos;

layout(push_constant) uniform PrefilterSamplerProperties {
    uint CubeMapResolution;
    float Roughness;
} prefilterSamplerProperties;

const mat4 captureProjection = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, -1.0, -1.0, 
    0.0, 0.0, -0.1, 0.0    
);

const mat4 captureViews[6] = mat4[](
    mat4( 0.0,  0.0, -1.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
         -1.0,  0.0,  0.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // +X

    mat4( 0.0,  0.0,  1.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
          1.0,  0.0,  0.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // -X

    mat4( 1.0,  0.0,  0.0, 0.0,
          0.0,  0.0, -1.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // +Y

    mat4( 1.0,  0.0,  0.0, 0.0,
          0.0,  0.0,  1.0, 0.0,
          0.0,  1.0,  0.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // -Y

    mat4( 1.0,  0.0,  0.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
          0.0,  0.0, -1.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // +Z

    mat4(-1.0,  0.0,  0.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
          0.0,  0.0,  1.0, 0.0,
          0.0,  0.0,  0.0, 1.0)   // -Z
);

void main()
{
    WorldPos = aPos;

    mat4 view = captureViews[gl_ViewIndex];
    vec4 pos = captureProjection * view * vec4(aPos, 1.0);

    gl_Position = pos.xyww;
}