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

layout(location = 0) in vec3 aPos;
layout(location = 0) out vec3 vWorldPos;

layout(push_constant) uniform IrradianceShaderConstants {
    float sampleDelta;
} irradianceShaderConstants;

mat4 MVP[6] = {
    {{ 0.000000,  0.000000,  1.000000,  1.000000},
     { 0.000000, -1.000000,  0.000000,  0.000000},
     {-1.000000,  0.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000, -0.100100,  0.000000}},  // +X

    {{ 0.000000,  0.000000, -1.000000, -1.000000},
     { 0.000000, -1.000000,  0.000000,  0.000000},
     { 1.000000,  0.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000, -0.100100,  0.000000}},  // -X

    {{ 1.000000,  0.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000,  1.000000,  1.000000},
     { 0.000000,  1.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000, -0.100100,  0.000000}},  // +Y

    {{ 1.000000,  0.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000, -1.000000, -1.000000},
     { 0.000000, -1.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000, -0.100100,  0.000000}},  // -Y

    {{ 1.000000,  0.000000,  0.000000,  0.000000},
     { 0.000000, -1.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000,  1.000000,  1.000000},
     { 0.000000,  0.000000, -0.100100,  0.000000}},  // +Z

    {{-1.000000,  0.000000,  0.000000,  0.000000},
     { 0.000000, -1.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000, -1.000000, -1.000000},
     { 0.000000,  0.000000, -0.100100,  0.000000}}   // -Z
};

void main()
{
    vWorldPos = aPos;
    vec4 clipPos = MVP[gl_ViewIndex] * vec4(aPos * 5000, 1.0f);
    gl_Position = clipPos.xyww;
}