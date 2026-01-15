#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 0;
layout(constant_id = 1) const uint DescriptorBindingType1 = 1;
layout(constant_id = 2) const uint DescriptorBindingType2 = 2;

layout (location = 0)  in vec2  VS_Position;
layout (location = 1)  in vec2  VS_UV;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec2  PS_UV;

#include "MaterialPropertiesBuffer.glsl"

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

struct MeshProperitiesBuffer
{
	int	   MaterialIndex;
	mat4   MeshTransform;
};

layout(binding = 0) buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 1) uniform sampler2D TextureMap[];
layout(binding = 2) buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];

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