#version 460
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable


#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 
#include "MemoryPoolBindings.glsl"

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 outColor;

float near = 0.1; 
float far  = 100.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{             
float d =  texture(TextureMap[57], TexCoords).r;
outColor = vec4(fract(d * 128.0), fract(d * 32.0), fract(d * 8.0), 1.0);
}