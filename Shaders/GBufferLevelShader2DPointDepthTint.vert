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

layout (location = 0)  in vec2  VS_Position;
layout (location = 1)  in vec2  VS_UV;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec2  PS_UV;

#include "BindlessHelpers.glsl"

void main()
{
    MeshProperitiesBuffer mesh = GetMesh(pointLightSceneDataDepthBuffer.MeshBufferIndex);

    vec4 world = mesh.MeshTransform * vec4(VS_Position.xy, float(mesh.MeshLayerIndex) * SpriteLayerSpacing, 1.0);

    PS_Position = world.xyz;
    PS_UV       = VS_UV;

    const PointLightBuffer light = GetPointLight(pointLightSceneDataDepthBuffer.PointLightBufferIndex);
    gl_Position = light.LightSpaceMatrix[pointLightSceneDataDepthBuffer.PointLightViewIndex] * world;
 
}