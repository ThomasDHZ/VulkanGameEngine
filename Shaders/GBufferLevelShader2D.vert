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
#include "SceneDataBuffer.glsl"
layout(push_constant) uniform SceneDataPushConstant
{
    int      MeshBufferIndex;
    float    LevelHeightScale;
    uint64_t SceneDataBufferPtr;
    uint64_t MeshPropertiesBufferPtr;
    uint64_t LightPropertiesBufferPtr;
    uint64_t TextureHeaderBufferPtr;
} sceneDataPushConstant;

SceneDataBuffer sceneData = GetSceneDataBuffer(sceneDataPushConstant.SceneDataBufferPtr);

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

layout (location = 0)  in vec2  VS_Position;
layout (location = 1)  in vec2  VS_UV;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec2  PS_UV;
layout (location = 2) out vec3  PS_T;
layout (location = 3) out vec3  PS_B;
layout (location = 4) out vec3  PS_N;

#include "BindlessHelpers.glsl"

vec4 SampleTexture(uint textureIndex, vec2 uv)
{
    TextureMetadata meta = Get2DTextureMetadata(textureIndex);

    if (meta.TextureType == 0) // 2D
    {
        return texture(TextureMap[meta.ArrayIndex], uv);
    }
    return vec4(1.0, 0.0, 1.0, 1.0); // error pink
}

void main()
{
    MeshProperitiesBuffer mesh = GetMesh(sceneDataPushConstant.MeshBufferIndex);

    vec4 world = mesh.MeshTransform * vec4(VS_Position.xy, float(mesh.MeshLayerIndex) * SpriteLayerSpacing, 1.0);
    vec3 T = normalize((mesh.MeshTransform * vec4(1.0, 0.0, 0.0, 0.0)).xyz);
    vec3 B = normalize((mesh.MeshTransform * vec4(0.0, 1.0, 0.0, 0.0)).xyz);
    vec3 N = normalize(cross(T, B));

    PS_Position = world.xyz;
    PS_UV       = VS_UV;
    PS_T = T;
    PS_B = B;
    PS_N = N;

    gl_Position = sceneData.OrthoProjection *
                  sceneData.OrthoView *
                  world;
}