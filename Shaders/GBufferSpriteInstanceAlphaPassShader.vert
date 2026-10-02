
#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 
#include "MemoryPoolBindings.glsl"
 
layout (location = 0)  in vec2  VS_SpritePosition;
layout (location = 1)  in vec4  VS_UVOffset; // vec4(vec2(StartUV.x, StartUV.y), vec2(UVEnd.x, UVEnd.y))
layout (location = 2)  in vec2  VS_SpriteSize;
layout (location = 3)  in ivec2 VS_FlipSprite;
layout (location = 4)  in vec4  VS_Color;
layout (location = 5)  in mat4  VS_InstanceTransform;
layout (location = 9)  in uint  VS_MaterialId;
layout (location = 10) in uint  VS_SpriteId;
layout (location = 11) in uint  VS_SpriteLayer;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec2  PS_UV;
layout (location = 2) out vec2  PS_SpriteSize;
layout (location = 3) out ivec2 PS_FlipSprite;
layout (location = 4) out vec4  PS_Color;
layout (location = 5) out uint  PS_MaterialId;
layout (location = 6) out vec4  PS_UVOffset;
layout (location = 7) out uint  PS_SpriteId;
layout (location = 8) out vec3  PS_T;
layout (location = 9) out vec3  PS_B;
layout (location = 10) out vec3 PS_N;

layout(constant_id = 1)  const uint VertexInputRate = 1;
layout(constant_id = 0)  const uint VertexAttributeLocation0 = 0;
layout(constant_id = 2)  const uint VertexAttributeLocation1 = 0;
layout(constant_id = 4)  const uint VertexAttributeLocation2 = 0;
layout(constant_id = 6)  const uint VertexAttributeLocation3 = 0;
layout(constant_id = 8)  const uint VertexAttributeLocation4 = 0;
layout(constant_id = 10) const uint VertexAttributeLocation5 = 0;
layout(constant_id = 12) const uint VertexAttributeLocation9 = 0;
layout(constant_id = 14) const uint VertexAttributeLocation10 = 0;


layout(push_constant) uniform SceneDataBuffer
{
    int   MeshBufferIndex;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

#include "BindlessHelpers.glsl"

struct Vertex2D
{
	vec2 Position;
	vec2 UV;
};

void main() 
{
    Vertex2D vertex = Vertex2D(vec2(0.0f), vec2(0.0f));
    switch(gl_VertexIndex) 
	{
        case 0: vertex = Vertex2D(vec2(0.0f           , VS_SpriteSize.y), vec2(VS_UVOffset.x                , VS_UVOffset.y                )); break; 
        case 1: vertex = Vertex2D(vec2(VS_SpriteSize.x, VS_SpriteSize.y), vec2(VS_UVOffset.x + VS_UVOffset.z, VS_UVOffset.y                )); break;
        case 2: vertex = Vertex2D(vec2(VS_SpriteSize.x, 0.0f           ), vec2(VS_UVOffset.x + VS_UVOffset.z, VS_UVOffset.y + VS_UVOffset.w)); break;
        case 3: vertex = Vertex2D(vec2(0.0f           , 0.0f           ), vec2(VS_UVOffset.x			    , VS_UVOffset.y + VS_UVOffset.w)); break;
    }

    vec4 world = VS_InstanceTransform * vec4(vertex.Position.xy, VS_SpriteLayer, 1.0);
    
    vec3 T = normalize((VS_InstanceTransform * vec4(1.0, 0.0, 0.0, 0.0)).xyz);
    if (VS_FlipSprite.x == 1) T = -T;
    
    vec3 B = normalize((VS_InstanceTransform * vec4(0.0, 1.0, 0.0, 0.0)).xyz);
    if (VS_FlipSprite.y == 1) B = -B;
    
    vec3 N = normalize(cross(T, B));
    
    PS_Position = world.xyz;
	PS_UV = vertex.UV;
    PS_SpriteSize = VS_SpriteSize;
	PS_FlipSprite = VS_FlipSprite;
	PS_Color = VS_Color;
	PS_MaterialId = VS_MaterialId;
	PS_UVOffset = VS_UVOffset;
    PS_SpriteId = VS_SpriteId;
    PS_T = T;
    PS_B = B;
    PS_N = N;

    gl_Position = sceneDataBuffer.OrthoProjection *
                  sceneDataBuffer.OrthoView *
                  world;
}