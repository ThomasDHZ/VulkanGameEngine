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

layout(location = 0) out vec4 OutputColor;
layout(location = 1) out vec4 OutputBloom;

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
	float HeightScale;

	uint AlbedoMap;
	uint MetallicRoughnessMap;
	uint MetallicMap;
	uint RoughnessMap;
	uint AmbientOcclusionMap;
	uint NormalMap;
	uint AlphaMap;
	uint EmissionMap;
	uint HeightMap;
};

layout(binding = 0) buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 1) uniform sampler2D TextureMap[];
layout(binding = 2) buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];

void main()
{
if (gl_FragCoord.x < 2.0 && gl_FragCoord.y < 2.0) // only one pixel
{
 MeshProperities meshBuffer1 = MeshProperities[0].meshProperties;
  MeshProperities meshBuffer2 = MeshProperities[1].meshProperties;
   MeshProperities meshBuffer3 = MeshProperities[2].meshProperties;
    MeshProperities meshBuffer4 = MeshProperities[3].meshProperties;
    uint count = 2; // adjust to your max expected mesh count
    for (uint i = 0; i < count; ++i)
    {
	        int matIndex = meshBuffer[i].meshProperties.MaterialIndex;
        int matIndex = meshBuffer[i].meshProperties.MaterialIndex;
        if (matIndex < 0) break; // optional sentinel

        debugPrintfEXT("meshBuffer[%u].MaterialIndex = %d\n", i, matIndex);
        // Optionally print transform parts if needed
    }
}

	int meshIndex = sceneData.MeshBufferIndex;
    uint materialId = meshBuffer[meshIndex].meshProperties.MaterialIndex;
	MaterialProperitiesBuffer material = materialBuffer[2].materialProperties;

    vec4 albedoColor = texture(TextureMap[material.AlbedoMap], inPS_UV);
    vec3 Albedo = albedoColor.rgb;
    float Alpha = albedoColor.a;

    if (Alpha == 0.0)
        discard;

    OutputColor = vec4(Albedo, Alpha);
	OutputBloom = vec4(Albedo, Alpha);
}