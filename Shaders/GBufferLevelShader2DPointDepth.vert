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

layout (location = 0)  in vec2  VS_Position;
layout (location = 1)  in vec2  VS_UV;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec2  PS_UV;

#include "BindlessHelpers.glsl"

void main()
{
    MeshProperitiesBuffer mesh = GetMesh(directionalLightSceneDataDepthBuffer.MeshBufferIndex);
    const DirectionalLightBuffer light = GetDirectionalLight(0);

    vec4 world = mesh.MeshTransform * vec4(VS_Position.xy, float(mesh.MeshLayerIndex) * SpriteLayerSpacing, 1.0);

    PS_Position = world.xyz;
    PS_UV       = VS_UV;
    gl_Position = light.LightSpaceMatrix *
                  world;
}