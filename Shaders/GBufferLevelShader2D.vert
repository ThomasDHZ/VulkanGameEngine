#version 460
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout(set = 0, binding = 0) uniform sampler2D   TextureMap[];
layout(set = 0, binding = 1) uniform samplerCube CubeMaps[];
layout(set = 0, binding = 2) buffer              ScenePropertiesBuffer 
{ 
    MeshProperitiesBuffer meshProperties[]; 
    Material material[];
    CubeMapMaterial cubeMapMaterial[];
    DirectionalLightBuffer directionalLightProperties[];
    PointLightBuffer pointLightProperties[];
} 
scenePropertiesBuffer;

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
    uint meshIndex = sceneData.MeshBufferIndex;
    uint materialIndex = scenePropertiesBuffer.meshProperties[meshIndex].MaterialIndex;
    Material material = scenePropertiesBuffer.material[materialIndex];
    CubeMapMaterial cubeMapMaterial =  scenePropertiesBuffer.cubeMapMaterial[sceneData.CubeMapIndex];
    mat4 meshTransform = scenePropertiesBuffer.meshProperties[sceneData.MeshBufferIndex].MeshTransform;

    PS_Position = vec3(meshTransform * vec4(VS_Position.xy, 0.0f, 1.0f));
	PS_UV = VS_UV.xy;

    gl_Position = sceneData.Projection * 
                  sceneData.View *  
                  meshTransform *
                  vec4(VS_Position.xy, 0.0f, 1.0f);
}