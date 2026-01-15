#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

#include "MaterialPropertiesBuffer.glsl"

layout (location = 0)   in vec2  VS_SpritePosition;
layout (location = 1)   in vec4  VS_UVOffset; //vec4(vec2(StartUV.x, StartUV.y), vec2(UVEnd.x, UVEnd.y))
layout (location = 2)   in vec2  VS_SpriteSize;
layout (location = 3)   in ivec2 VS_FlipSprite;
layout (location = 4)   in vec4  VS_Color;
layout (location = 5)   in mat4  VS_InstanceTransform;
layout (location = 9)   in uint  VS_MaterialID;
layout (location = 10)  in vec4  VS_Guid;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec2  PS_UV;
layout (location = 2) out vec2  PS_SpriteSize;
layout (location = 3) out ivec2 PS_FlipSprite;
layout (location = 4) out vec4  PS_Color;
layout (location = 5) out uint  PS_MaterialID;
layout (location = 6) out vec4  PS_UVOffset;
layout (location = 7) out vec4  PS_Guid;


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
    Vertex2D vertex = Vertex2D(vec2(0.0f), vec2(0.0f));
    switch(gl_VertexIndex) 
	{
        case 0: vertex = Vertex2D(vec2(VS_SpritePosition.x                  , VS_SpritePosition.y + VS_SpriteSize.y), vec2(VS_UVOffset.x                , VS_UVOffset.y                )); break; 
        case 1: vertex = Vertex2D(vec2(VS_SpritePosition.x + VS_SpriteSize.x, VS_SpritePosition.y + VS_SpriteSize.y), vec2(VS_UVOffset.x + VS_UVOffset.z, VS_UVOffset.y                )); break;
        case 2: vertex = Vertex2D(vec2(VS_SpritePosition.x + VS_SpriteSize.x, VS_SpritePosition.y                  ), vec2(VS_UVOffset.x + VS_UVOffset.z, VS_UVOffset.y + VS_UVOffset.w)); break;
        case 3: vertex = Vertex2D(vec2(VS_SpritePosition.x                  , VS_SpritePosition.y                  ), vec2(VS_UVOffset.x			    , VS_UVOffset.y + VS_UVOffset.w)); break;
    }

    PS_Position = vec3(VS_InstanceTransform * vec4(vertex.Position.xy, 0.0f, 1.0f));
	PS_UV = vertex.UV;
    PS_SpriteSize = VS_SpriteSize;
	PS_FlipSprite = VS_FlipSprite;
	PS_Color = VS_Color;
	PS_MaterialID = VS_MaterialID;
	PS_UVOffset = VS_UVOffset;
	PS_Guid = VS_Guid;

    gl_Position = sceneData.Projection * 
                  sceneData.View *  
                  VS_InstanceTransform *
                  vec4(vertex.Position.xy, 0.0f, 1.0f);
}