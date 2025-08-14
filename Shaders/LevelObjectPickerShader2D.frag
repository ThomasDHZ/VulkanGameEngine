#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3       PS_Position; 
layout(location = 1) in vec2       PS_UV;    
layout(location = 2) in flat vec4  PS_Guid;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform SceneDataBuffer {
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
} sceneData;

layout(binding = 0) buffer MeshProperitiesBuffer
{
	int	   MaterialIndex;
	mat4   MeshTransform;
} meshProperities[];
layout(binding = 1) uniform sampler2D TextureMap[];
layout(binding = 2) buffer MaterialProperitiesBuffer {
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
    outColor = PS_Guid;
}