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

layout(location = 0) in  vec3 WorldPos;
layout(location = 1) in  vec2 TexCoords;

layout(location = 0) out vec4 OutColor;
#include "BindlessHelpers.glsl"
const uint NO_MAP = 0xFFFFFFFFu;
void main()
{
    MeshProperitiesBuffer mesh           = GetMesh(directionalLightSceneDataDepthBuffer.MeshBufferIndex);
    PackedMaterial        packedMaterial = GetMaterial(mesh.MaterialIndex);

    vec4 albedo = texture(TextureMap[packedMaterial.AlbedoTextureId], TexCoords, -0.5);
    if (albedo.a >= packedMaterial.AlphaCutOff || albedo.a < 0.01) discard;

    vec3 atten = (packedMaterial.TranslucentTextureId != NO_MAP) ? textureLod(TextureMap[packedMaterial.TranslucentTextureId], TexCoords, 0.0).rgb : vec3(1.0);
    float T = (packedMaterial.TranslucentPropertiesTextureId != NO_MAP) ? textureLod(TextureMap[packedMaterial.TranslucentPropertiesTextureId], TexCoords, 0.0).r : 0.0;
    OutColor = vec4(vec3(0.0f, 0.0f, 0.7f) * T, 1.0);
}