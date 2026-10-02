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
layout (location = 1)  in vec4  VS_Color;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec4  PS_Color;


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
    PackedMaterial material = GetMaterial(mesh.MaterialIndex);

    PS_Position = vec3(mesh.MeshTransform * vec4(VS_Position.xy, 0.0f, 1.0f));
	PS_Color = VS_Color;

    gl_Position = sceneDataBuffer.OrthoProjection * 
                  sceneDataBuffer.OrthoView *  
                  mesh.MeshTransform *
                  vec4(VS_Position.xy, 0.0f, 1.0f);
}