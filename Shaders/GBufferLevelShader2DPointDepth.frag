#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"
#include "MemoryPoolBindings.glsl"

layout(push_constant) uniform DirectionalLightSceneDataDepthBuffer
{
    int   MeshBufferIndex;
    int   DirectionalLightBufferIndex;
    int   UseHeightMap;
    float HeightScale;
} directionalLightSceneDataDepthBuffer;

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