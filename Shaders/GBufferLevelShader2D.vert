#version 460
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : require

#include "BindlessHelpers.glsl"

layout(push_constant) uniform SceneDataBuffer
{
    uint  MeshBufferIndex;
    uint  CubeMapIndex;
    mat4  Projection;
    mat4  View;
    vec3  ViewDirection;
    vec3  CameraPosition;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

layout (location = 0)  in vec2  VS_Position;
layout (location = 1)  in vec2  VS_UV;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec2  PS_UV;

void main()
{
    MeshPropertiesBuffer meshProps = GetMesh(sceneData.MeshBufferIndex);

    uint materialId = meshProps.MaterialIndex;
    Material material = GetMaterial(materialId);
    CubeMapMaterial cubeMapMaterial = GetCubeMapMaterial(sceneData.CubeMapIndex);

    mat4 meshTransform = meshProps.MeshTransform;

    PS_Position = vec3(meshTransform * vec4(VS_Position.xy, 0.0f, 1.0f));
	PS_UV = VS_UV.xy;

    gl_Position = sceneData.Projection * 
                  sceneData.View *  
                  meshTransform *
                  vec4(VS_Position.xy, 0.0f, 1.0f);
}