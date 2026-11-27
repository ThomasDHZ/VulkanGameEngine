#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"
#include "MemoryPool.glsl"
#include "SceneDataBuffer.glsl"

layout(push_constant) uniform PointLightSceneDataDepthBuffer
{
    int      MeshBufferIndex;
    int      PointLightBufferIndex;
    int      PointLightViewIndex;
    float    LevelHeightScale;
    uint64_t SceneDataBufferPtr;
    uint64_t MeshPropertiesBufferPtr;
    uint64_t LightPropertiesBufferPtr;
    uint64_t TextureHeaderBufferPtr;
} pointLightSceneDataDepthBuffer;

SceneDataBuffer sceneData = GetSceneDataBuffer(pointLightSceneDataDepthBuffer.SceneDataBufferPtr);

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


layout(location = 0)  in vec3       WorldPos;
layout(location = 1)  in vec2       PS_UV;
layout(location = 2)  in vec2       PS_SpriteSize;
layout(location = 3)  in flat ivec2 PS_FlipSprite;
layout(location = 4)  in vec4       PS_Color;
layout(location = 5)  in flat uint  PS_MaterialId;
layout(location = 6)  in flat vec4  PS_UVOffset;
layout(location = 7)  in flat uint  PS_SpriteId;
layout(location = 8)  in vec3       PS_T;
layout(location = 9)  in vec3       PS_B;
layout(location = 10) in vec3       PS_N;

#include "BindlessHelpers.glsl"

void main()
{
    PackedMaterial packedMaterial = GetMaterial(PS_MaterialId);

    vec2 minUV = PS_UVOffset.xy;
    vec2 maxUV = PS_UVOffset.xy + PS_UVOffset.zw;

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) UV.x = minUV.x + maxUV.x - UV.x;
    if (PS_FlipSprite.y == 1) UV.y = minUV.y + maxUV.y - UV.y;

    vec4 albedo = texture(TextureMap[packedMaterial.AlbedoTextureId], UV, -0.5);
    if (albedo.a < packedMaterial.AlphaCutOff) discard;

    const PointLightBuffer light = GetPointLight(0);
    float dist = length(WorldPos - vec3(light.LightPosition.xy, light.LightLayer * SpriteLayerSpacing));
    gl_FragDepth = clamp(dist / light.LightRadius, 0.0, 1.0);
}
