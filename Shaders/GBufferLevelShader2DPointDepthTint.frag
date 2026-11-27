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

layout(location = 0) in  vec3 WorldPos;
layout(location = 1) in  vec2 TexCoords;

layout(location = 0) out vec4 OutColor;
#include "BindlessHelpers.glsl"
const uint NO_MAP = 0xFFFFFFFFu;
void main()
{
    MeshProperitiesBuffer mesh           = GetMesh(pointLightSceneDataDepthBuffer.MeshBufferIndex);
    PackedMaterial        packedMaterial = GetMaterial(mesh.MaterialIndex);

    vec4 albedo = texture(TextureMap[packedMaterial.AlbedoTextureId], TexCoords, -0.5);
    if (albedo.a >= packedMaterial.AlphaCutOff || albedo.a < 0.01) discard;

    vec3 attenuationColor             =  max((packedMaterial.TranslucentTextureId != NO_MAP) ? textureLod(TextureMap[packedMaterial.TranslucentTextureId], TexCoords, 0.0).rgb : vec3(1.0), vec3(1e-4));
    vec4 translucentPropertiesTexture = textureLod(TextureMap[packedMaterial.TranslucentPropertiesTextureId], TexCoords, 0.0);
    float thickness                   = translucentPropertiesTexture.g;
    float transmissionWeight          = translucentPropertiesTexture.r;

    vec3 sigmaA                       = -log(attenuationColor) / thickness;
    vec3 T                            = exp(-sigmaA * thickness);
    float coverage                    = clamp(transmissionWeight, 0.0, 1.0);
    OutColor = vec4(T, coverage);
}