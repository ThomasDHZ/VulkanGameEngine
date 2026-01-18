#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 0;
layout(constant_id = 1) const uint DescriptorBindingType1 = 1;
layout(constant_id = 2) const uint DescriptorBindingType2 = 2;
layout(constant_id = 3) const uint DescriptorBindingType3 = 4;

layout (location = 0)  in vec3  VS_Position;
layout (location = 1)  in vec2  VS_UV;

layout(location = 0)  out vec2 PS_WorldPos;
layout (location = 1) out vec2   PS_UV;
//layout(push_constant) uniform SPFDirectionalLightPushConstant 
//{
//    int MeshBufferIndex;
//    int LightBufferIndex;
//    mat4 LightProjection;
//    mat4 LightView;
//    vec3 LightDirection;
//}spfDirectionalLightPushConstant;

layout(push_constant) uniform SPFPointLightPushConstant 
{
    int MeshBufferIndex;
    int LightBufferIndex;
    mat4 LightProjection;
    mat4 LightView;
}spfPointLightPushConstant;

#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 
#include "Lights.glsl"

layout(binding = 0) buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 1) uniform sampler2D TextureMap[];
layout(binding = 2) buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];
layout(binding = 3) buffer PointLight { PointLightBuffer pointLightProperties; } pointLightsBuffer[];
void main()
{
    int meshIndex = spfPointLightPushConstant.MeshBufferIndex;
    mat4 meshTransform = meshBuffer[meshIndex].meshProperties.MeshTransform;

    vec4 worldPos = meshTransform * vec4(VS_Position.xy, 0.0f, 1.0f);
    PS_WorldPos = worldPos.xy;

    gl_Position = spfPointLightPushConstant.LightProjection *
                  spfPointLightPushConstant.LightView *
                  worldPos;
}