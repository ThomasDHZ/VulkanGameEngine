#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 0;
layout(constant_id = 1) const uint DescriptorBindingType1 = 1;
layout(constant_id = 2) const uint DescriptorBindingType2 = 2;

layout(location = 0) in vec3 inPS_Position; 
layout(location = 1) in vec2 inPS_UV;    

layout(location = 0) out vec4 AlbedoMap;
layout(location = 1) out vec4 NormalMap;
layout(location = 2) out vec4 MatRoughAOEmissiveMap;
layout(location = 3) out vec4 EnvironmentMap;

layout(push_constant) uniform SceneDataBuffer {
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
} sceneData;

struct MeshProperitiesBuffer
{
	int	   MaterialIndex;
	mat4   MeshTransform;
};

struct MaterialProperitiesBuffer
{
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
};

layout(binding = 0) buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 1) uniform sampler2D TextureMap[];
layout(binding = 2) buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];

void main()
{
    vec4 albedoColor = texture(TextureMap[1], inPS_UV);
    vec3 Albedo = albedoColor.rgb;
    float Alpha = albedoColor.a;

    AlbedoMap = vec4(Albedo, 1.0f);
	NormalMap = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	MatRoughAOEmissiveMap = vec4(0.0f, 1.0f, 0.0f, 1.0f);
	EnvironmentMap = vec4(0.0f, 0.0f, 1.0f, 1.0f);
}