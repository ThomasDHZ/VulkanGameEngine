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

layout(push_constant) uniform DirectionalLightSceneDataDepthBuffer
{
    int   MeshBufferIndex;
    int   DirectionalLightBufferIndex;
    float LevelHeightScale;
    uint64_t SceneDataBufferPtr;
    uint64_t MeshPropertiesBufferPtr;
    uint64_t LightPropertiesBufferPtr;
    uint64_t TextureHeaderBufferPtr;
} directionalLightSceneDataDepthBuffer;

SceneDataBuffer sceneData = GetSceneDataBuffer(directionalLightSceneDataDepthBuffer.SceneDataBufferPtr);

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

layout(location = 0) in vec3 WorldPos;
layout(location = 1) in vec2 TexCoords;

#include "BindlessHelpers.glsl"

void main()
{
    MeshProperitiesBuffer mesh           = GetMesh(directionalLightSceneDataDepthBuffer.MeshBufferIndex);
    PackedMaterial        packedMaterial = GetMaterial(mesh.MaterialIndex);

    vec4 albedoTexture = texture(TextureMap[packedMaterial.AlbedoTextureId], TexCoords, -0.5);
    if (albedoTexture.a < packedMaterial.AlphaCutOff) discard;
}