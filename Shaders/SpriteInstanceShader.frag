#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 0;
layout(constant_id = 1) const uint DescriptorBindingType1 = 1;
layout(constant_id = 2) const uint DescriptorBindingType2 = 2;

layout (location = 0) in vec3  PS_Position;
layout (location = 1) in vec2  PS_UV;
layout (location = 2) in vec2  PS_SpriteSize;
layout (location = 3) in flat ivec2 PS_FlipSprite;
layout (location = 4) in vec4  PS_Color;
layout (location = 5) in flat uint  PS_MaterialID;
layout (location = 6) in flat vec4  PS_UVOffset;

layout(location = 0) out vec4 OutputColor;

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
	MaterialProperitiesBuffer material = materialBuffer[PS_MaterialID].materialProperties;

	vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) 
	{
		UV.x = PS_UVOffset.x + PS_UVOffset.z - (UV.x - PS_UVOffset.x);
    }
    if (PS_FlipSprite.y == 1) 
	{
		UV.y = PS_UVOffset.y + PS_UVOffset.w - (UV.y - PS_UVOffset.y);
    }

	vec4 albedoColor = texture(TextureMap[material.AlbedoMap], UV);
	material.Albedo = albedoColor.rgb;
	material.Alpha = albedoColor.a;
	
	if(material.Alpha == 0.0f)
	{
		discard;
	}
  
    OutputColor = vec4(material.Albedo.rgb, 1.0f);
}