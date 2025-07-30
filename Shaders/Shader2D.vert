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
layout (location = 2) out vec2  PS_SpriteSize;
layout (location = 3) out ivec2 PS_FlipSprite;
layout (location = 4) out vec4  PS_Color;
layout (location = 5) out uint  PS_MaterialID;
layout (location = 6) out vec4  PS_UVOffset;

layout(push_constant) uniform SceneDataBuffer
{
    int	 MeshBufferIndex;
	mat4 Projection;
	mat4 View;
	vec3 CameraPosition;
}sceneData;

struct MeshProperitiesBuffer
{
	int	   MaterialIndex;
	mat4   MeshTransform;
};

struct MaterialProperitiesBuffer
{
	vec3 Albedo;
	float Metallic;
	float Roughness;
	float AmbientOcclusion;
	vec3 Emission;
	float Alpha;

	uint AlbedoMap;
	uint MetallicRoughnessMap;
	uint MetallicMap;
	uint RoughnessMap;
	uint AmbientOcclusionMap;
	uint NormalMap;
	uint DepthMap;
	uint AlphaMap;
	uint EmissionMap;
	uint HeightMap;
};

struct Vertex2D
{
	vec2 Position;
	vec2 UV;
};

layout(binding = 0) readonly buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 1) uniform sampler2D TextureMap[];
layout(binding = 2) uniform MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];

void main() 
{
	MeshProperities meshProperties = meshBuffer[sceneData.MeshBufferIndex]. 
    PS_Position = vec3(VS_InstanceTransform * vec4(vertex.Position.xy, 0.0f, 1.0f));
	PS_UV = vertex.UV;
    PS_SpriteSize = VS_SpriteSize;
	PS_FlipSprite = VS_FlipSprite;
	PS_Color = VS_Color;
	PS_MaterialID = VS_MaterialID;
	PS_UVOffset = VS_UVOffset;

    gl_Position = sceneData.Projection * 
                  sceneData.View *  
                  VS_InstanceTransform *
                  vec4(vertex.Position.xy, 0.0f, 1.0f);
}