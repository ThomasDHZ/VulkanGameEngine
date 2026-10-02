#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 
#include "MemoryPoolBindings.glsl"

layout(push_constant) uniform SceneDataBuffer
{
    int   MeshBufferIndex;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

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
    MeshProperitiesBuffer mesh = GetMesh(sceneData.MeshBufferIndex);

    vec4 world = mesh.MeshTransform * vec4(VS_Position.xy, 0.0, 1.0);
    vec3 T = normalize((mesh.MeshTransform * vec4(1.0, 0.0, 0.0, 0.0)).xyz);
    vec3 B = normalize((mesh.MeshTransform * vec4(0.0, 1.0, 0.0, 0.0)).xyz);
    vec3 N = normalize(cross(T, B));

    PS_Position = world.xyz;
    PS_UV       = VS_UV;
    PS_T = T;
    PS_B = B;
    PS_N = N;

    gl_Position = sceneDataBuffer.OrthoProjection *
                  sceneDataBuffer.OrthoView *
                  world;
}