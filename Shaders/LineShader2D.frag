#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 
#include "MemoryPoolBindings.glsl"

layout(push_constant) uniform SceneDataBuffer
{
    int   MeshBufferIndex;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

layout (location = 0)  in vec4  PS_Position;
layout (location = 1)  in vec4  PS_Color;  

layout(location = 0) out vec4 outColor;

#include "BindlessHelpers.glsl"

void main()
{
    outColor = PS_Position;
}
 