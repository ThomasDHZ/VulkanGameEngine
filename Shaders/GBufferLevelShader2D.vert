#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout (location = 0)  in vec2  VS_Position;
layout (location = 1)  in vec2  VS_UV;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec2  PS_UV;


#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout(constant_id = 8)   const uint DescriptorBindingType8   = MeshPropertiesDescriptor;
layout(binding = 8)  buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];

layout(push_constant) uniform SceneDataBuffer 
{
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
          vec3  ViewDirection;
    vec3 CameraPosition;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

void main()
{
    int meshIndex = sceneData.MeshBufferIndex;
    mat4 meshTransform = meshBuffer[meshIndex].meshProperties.MeshTransform;

    PS_Position = vec3(meshTransform * vec4(VS_Position.xy, 0.0f, 1.0f));
	PS_UV = VS_UV.xy;

    gl_Position = sceneData.Projection * 
                  sceneData.View *  
                  meshTransform *
                  vec4(VS_Position.xy, 0.0f, 1.0f);
}