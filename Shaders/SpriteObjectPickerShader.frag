#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout (location = 0) in vec3       PS_Position;
layout (location = 1) in vec2       PS_UV;
layout (location = 2) in vec2       PS_SpriteSize;
layout (location = 3) in flat ivec2 PS_FlipSprite;
layout (location = 4) in vec4       PS_Color;
layout (location = 5) in flat uint  PS_MaterialID;
layout (location = 6) in flat vec4  PS_UVOffset;
layout (location = 7) in flat vec4  PS_Guid;

layout(location = 0) out vec4 OutputColor;

layout(push_constant) uniform SceneDataBuffer
{
	int	 MeshBufferIndex;
	mat4 Projection;
	mat4 View;
	vec3 CameraPosition;
}sceneData;

layout(binding = 0) buffer MeshProperitiesBuffer
{
	int	   MaterialIndex;
	mat4   MeshTransform;
} meshProperities[];
layout(binding = 1) uniform sampler2D TextureMap[];


void main() 
{
    OutputColor = PS_Guid;
}