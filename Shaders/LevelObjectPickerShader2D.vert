#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout (location = 0)  in vec2  VS_Position;
layout (location = 1)  in vec2  VS_UV;
layout (location = 2)  in vec4  VS_Guid;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec2  PS_UV;
layout (location = 2) out vec4  PS_Guid;

layout(push_constant) uniform SceneDataBuffer {
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
} sceneData;

layout(binding = 0) buffer meshPropertiesBuffer
{
	int	   MaterialIndex;
	mat4   MeshTransform;
} meshProperties[];
layout(binding = 1) uniform sampler2D TextureMap[];
layout(binding = 2) uniform MaterialPropertiesBuffer {
    vec3 Albedo;
    float Metallic;
    float Roughness;
    float AmbientOcclusion;
    vec3 Emission;
    float Alpha;
    uint AlbedoMap;
    uint MetallicRoughnessMap;
    uint MetallicMap;
    uint RoughnessMap;
    uint AmbientOcclusionMap;
    uint NormalMap;
    uint DepthMap;
    uint AlphaMap;
    uint EmissionMap;
    uint HeightMap;
} materialBuffer[];

void main()
{
    int meshIndex = sceneData.MeshBufferIndex;
    mat4 meshTransform = meshProperties[meshIndex].MeshTransform;

    PS_Position = vec3(meshTransform * vec4(VS_Position.xy, 0.0f, 1.0f));
	PS_UV = VS_UV.xy;
    PS_Guid = VS_Guid;

    gl_Position = sceneData.Projection * 
                  sceneData.View *  
                  meshTransform *
                  vec4(VS_Position.xy, 0.0f, 1.0f);
}