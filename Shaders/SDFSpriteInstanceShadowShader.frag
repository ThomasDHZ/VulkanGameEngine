#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 0;
layout(constant_id = 1) const uint DescriptorBindingType1 = 1;
layout(constant_id = 2) const uint DescriptorBindingType2 = 2;
layout(constant_id = 3) const uint DescriptorBindingType3 = 4;

layout (location = 0) in vec3  PS_Position;
layout (location = 1) in vec2  PS_UV;
layout (location = 2) in vec2  PS_SpriteSize;
layout (location = 3) in flat ivec2 PS_FlipSprite;
layout (location = 4) in vec4  PS_Color;
layout (location = 5) in flat uint  PS_MaterialID;
layout (location = 6) in vec4  PS_UVOffset;

layout(push_constant) uniform DirectionalLightPushConstant 
{
    int MeshBufferIndex;
    int LightBufferIndex;
    mat4 LightProjection;
    mat4 LightView;
}directionalLightPushConstant;

#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"
#include "Lights.glsl"

layout(binding = 0) buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 1) uniform sampler2D TextureMap[];
layout(binding = 2) buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];
layout(binding = 3) buffer PointLight { PointLightBuffer pointLightProperties; } pointLightsBuffer[];
void main()
{
	MaterialProperitiesBuffer material = materialBuffer[PS_MaterialID].materialProperties;

    float alphaMap =			(material.AlphaMap			  != 0xFFFFFFFFu) ? texture(TextureMap[material.AlphaMap], PS_UV).r				 : material.Alpha;
    if (alphaMap.r == 0.0)
	{
        discard;
	}
}