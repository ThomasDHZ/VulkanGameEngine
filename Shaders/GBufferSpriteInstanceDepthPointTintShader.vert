
#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 
#include "MemoryPool.glsl"

layout(std430, binding = 0) buffer SceneDataBuffer
{
   uint HDRMapInputIndex;
    uint EnvironmentMapIndex;
    uint BRDFMapId;
    uint CubeMapId;
    uint IrradianceMapId;
    uint PrefilterMapId;
	uint DirectionalLightShadowMapId;
	uint DirectionalLightShadowTintMapId;
	uint PointLightShadowMapId;
	uint PointLightShadowTintMapId;
    uint _pad0;
    uint _pad1;

    mat4 OrthoProjection;
    mat4 OrthoView;
    mat4 InverseOrthoProjection;
    mat4 InverseOrthoView;
    mat4 InversePerspectiveProjection;
    mat4 InversePerspectiveView;

    vec3  PerspectiveCameraPosition;
    float Time;
    vec3  PerspectiveViewDirection;
    uint  FrameIndex;
    vec2  InvertResolution;
    vec2  _padEnd;
} sceneDataBuffer;

layout(binding = 1) buffer BindlessBuffer
{
    uint64_t MeshOffset;
    uint MeshCount;
    uint MeshSize;
    uint64_t MaterialOffset;
    uint MaterialCount;
    uint MaterialSize;
    uint64_t DirectionalLightOffset;
    uint DirectionalLightCount;
    uint DirectionalLightSize;
    uint64_t PointLightOffset;
    uint PointLightCount;
    uint PointLightSize;
    uint64_t Texture2DOffset;
    uint Texture2DCount;
    uint Texture2DSize;
    uint64_t Texture3DOffset;
    uint Texture3DCount;
    uint Texture3DSize;
    uint64_t TextureCubeMapOffset;
    uint TextureCubeMapCount;
    uint TextureCubeMapSize;
    uint64_t SpriteInstanceOffset;
    uint SpriteInstanceCount;
    uint SpriteInstanceSize;
    uint Data[];
} bindlessBuffer;

layout(binding = 2) uniform samplerCube CubeMap[];
layout(binding = 3) uniform sampler2D TextureMap[];
layout(binding = 4) uniform sampler3D Texture3DMap[];
struct SpriteInstance
{
    vec2  SpritePosition;
    vec4  UVOffset;
    vec2  SpriteSize;
    ivec2 FlipSprite;
    vec4  Color;
    mat4  InstanceTransform;
    uint  MaterialId;
    uint  SpriteId;
    uint  SpriteLayer;
};

layout(std430, binding = 5) buffer SpriteInstanceMemoryPool
{
    MemoryPoolHeader Header;
    uint Data[];
} spriteInstanceMemoryPool;

SpriteInstance GetSpriteInstance(uint index)
{
    SpriteInstance sprite;

    if (index >= spriteInstanceMemoryPool.Header.ObjectCount)
    {
        sprite.SpritePosition = vec2(0.0);
        sprite.UVOffset = vec4(0.0);
        sprite.SpriteSize = vec2(0.0);
        sprite.FlipSprite = ivec2(0);
        sprite.Color = vec4(0.0);
        sprite.InstanceTransform = mat4(1.0);
        sprite.MaterialId = 0u;
        sprite.SpriteId = 0u;
        sprite.SpriteLayer = 0u;
        return sprite;
    }

    uint offset = index * (spriteInstanceMemoryPool.Header.ObjectSize / 4u);

    // Manual unpack (order must match your C++ struct layout)
    sprite.SpritePosition.x = uintBitsToFloat(spriteInstanceMemoryPool.Data[offset++]);
    sprite.SpritePosition.y = uintBitsToFloat(spriteInstanceMemoryPool.Data[offset++]);

    sprite.UVOffset.x = uintBitsToFloat(spriteInstanceMemoryPool.Data[offset++]);
    sprite.UVOffset.y = uintBitsToFloat(spriteInstanceMemoryPool.Data[offset++]);
    sprite.UVOffset.z = uintBitsToFloat(spriteInstanceMemoryPool.Data[offset++]);
    sprite.UVOffset.w = uintBitsToFloat(spriteInstanceMemoryPool.Data[offset++]);

    sprite.SpriteSize.x = uintBitsToFloat(spriteInstanceMemoryPool.Data[offset++]);
    sprite.SpriteSize.y = uintBitsToFloat(spriteInstanceMemoryPool.Data[offset++]);

    sprite.FlipSprite.x = int(spriteInstanceMemoryPool.Data[offset++]);
    sprite.FlipSprite.y = int(spriteInstanceMemoryPool.Data[offset++]);

    sprite.Color.x = uintBitsToFloat(spriteInstanceMemoryPool.Data[offset++]);
    sprite.Color.y = uintBitsToFloat(spriteInstanceMemoryPool.Data[offset++]);
    sprite.Color.z = uintBitsToFloat(spriteInstanceMemoryPool.Data[offset++]);
    sprite.Color.w = uintBitsToFloat(spriteInstanceMemoryPool.Data[offset++]);

    // mat4 (column-major, 16 floats)
    for (int col = 0; col < 4; ++col)
        for (int row = 0; row < 4; ++row)
            sprite.InstanceTransform[col][row] = uintBitsToFloat(spriteInstanceMemoryPool.Data[offset++]);

    sprite.MaterialId = spriteInstanceMemoryPool.Data[offset++];
    sprite.SpriteId = spriteInstanceMemoryPool.Data[offset++];
    sprite.SpriteLayer = spriteInstanceMemoryPool.Data[offset++];

    return sprite;
}


layout (location = 0)  in vec2  VS_SpritePosition;
layout (location = 1)  in vec4  VS_UVOffset; // vec4(vec2(StartUV.x, StartUV.y), vec2(UVEnd.x, UVEnd.y))
layout (location = 2)  in vec2  VS_SpriteSize;
layout (location = 3)  in ivec2 VS_FlipSprite;
layout (location = 4)  in vec4  VS_Color;
layout (location = 5)  in mat4  VS_InstanceTransform;
layout (location = 9)  in uint  VS_MaterialId;
layout (location = 10) in uint  VS_SpriteId;
layout (location = 11) in uint  VS_SpriteLayer;

layout(location = 0)  out vec2  PS_UV;
layout(location = 1)  out ivec2 PS_FlipSprite;
layout(location = 2)  out uint  PS_MaterialId;
layout(location = 3)  out vec4  PS_UVOffset;

layout(constant_id = 1)  const uint VertexInputRate = 1;
layout(constant_id = 0)  const uint VertexAttributeLocation0 = 0;
layout(constant_id = 2)  const uint VertexAttributeLocation1 = 0;
layout(constant_id = 4)  const uint VertexAttributeLocation2 = 0;
layout(constant_id = 6)  const uint VertexAttributeLocation3 = 0;
layout(constant_id = 8)  const uint VertexAttributeLocation4 = 0;
layout(constant_id = 10) const uint VertexAttributeLocation5 = 0;
layout(constant_id = 12) const uint VertexAttributeLocation9 = 0;
layout(constant_id = 14) const uint VertexAttributeLocation10 = 0;

layout(push_constant) uniform PointLightSceneDataDepthBuffer
{
    int   MeshBufferIndex;
    int   PointLightBufferIndex;
    int   PointLightViewIndex;
    int   UseHeightMap;
    float HeightScale;
} pointLightSceneDataDepthBuffer;

#include "Constants.glsl"
#include "BindlessHelpers.glsl"

struct Vertex2D
{
	vec2 Position;
	vec2 UV;
};

void main() 
{
    SpriteInstance sprite = GetSpriteInstance(gl_InstanceIndex);

    Vertex2D vertex = Vertex2D(vec2(0.0f), vec2(0.0f));
    switch(gl_VertexIndex) 
	{
        case 0: vertex = Vertex2D(vec2(0.0f               , sprite.SpriteSize.y), vec2(sprite.UVOffset.x                    , sprite.UVOffset.y                    )); break; 
        case 1: vertex = Vertex2D(vec2(sprite.SpriteSize.x, sprite.SpriteSize.y), vec2(sprite.UVOffset.x + sprite.UVOffset.z, sprite.UVOffset.y                    )); break;
        case 2: vertex = Vertex2D(vec2(sprite.SpriteSize.x, 0.0f           ),     vec2(sprite.UVOffset.x + sprite.UVOffset.z, sprite.UVOffset.y + sprite.UVOffset.w)); break;
        case 3: vertex = Vertex2D(vec2(0.0f               , 0.0f           ),     vec2(sprite.UVOffset.x			        , sprite.UVOffset.y + sprite.UVOffset.w)); break;
    }

    vec4 world = sprite.InstanceTransform * vec4(vertex.Position.xy, float(sprite.SpriteLayer) * SpriteLayerSpacing, 1.0);
    vec3 T = normalize((sprite.InstanceTransform * vec4(1.0, 0.0, 0.0, 0.0)).xyz);
    if (sprite.FlipSprite.x == 1) T = -T;

    vec3 B = normalize((sprite.InstanceTransform * vec4(0.0, 1.0, 0.0, 0.0)).xyz);
    if (sprite.FlipSprite.y == 1) B = -B;

    vec3 N = normalize(cross(T, B));
    if (sprite.FlipSprite.x != sprite.FlipSprite.y) N = -N;

	PS_UV = vertex.UV;
	PS_FlipSprite = sprite.FlipSprite;
	PS_MaterialId = sprite.MaterialId;
	PS_UVOffset = sprite.UVOffset;

    const PointLightBuffer light = GetPointLight(pointLightSceneDataDepthBuffer.PointLightBufferIndex);
    gl_Position = light.LightSpaceMatrix[pointLightSceneDataDepthBuffer.PointLightViewIndex] * world;
}