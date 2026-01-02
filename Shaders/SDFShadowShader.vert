#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 0;
layout(constant_id = 1) const uint DescriptorBindingType1 = 1;
layout(constant_id = 2) const uint DescriptorBindingType2 = 2;
layout(constant_id = 3) const uint DescriptorBindingType3 = 3;

layout (location = 0)  in vec3  VS_Position;
layout (location = 1)  in vec2  VS_UV;

layout (location = 0) out vec3  PS_Position;

layout(push_constant) uniform SPFDirectionalLightPushConstant 
{
    int MeshBufferIndex;
    int LightBufferIndex;
    mat4 LightProjection;
    mat4 LightView;
    vec2 LightDirection;
}spfDirectionalLightPushConstant;

#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"
#include "Lights.glsl"

layout(binding = 0) buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 1) uniform sampler2D TextureMap[];
layout(binding = 2) buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];
layout(binding = 3) buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];

void main()
{
    int meshIndex = spfDirectionalLightPushConstant.MeshBufferIndex;
    uint materialId = meshBuffer[meshIndex].meshProperties.MaterialIndex;
    mat4 meshTransform = meshBuffer[meshIndex].meshProperties.MeshTransform;
	MaterialProperitiesBuffer material = materialBuffer[materialId].materialProperties;

    vec4  albedoMap =			(material.AlbedoMap			  != 0xFFFFFFFFu) ? texture(TextureMap[material.AlbedoMap], VS_UV).rgba		 : vec4(material.Albedo, 1.0f);
    vec3  normalMap =			(material.NormalMap			  != 0xFFFFFFFFu) ? texture(TextureMap[material.NormalMap], VS_UV).rgb		 : vec3(0.0, 0.0, 1.0);
	float specularMap =			(material.SpecularMap		  != 0xFFFFFFFFu) ? texture(TextureMap[material.SpecularMap], VS_UV).r		 : material.Specular;
    float metallicMap =			(material.MetallicMap		  != 0xFFFFFFFFu) ? texture(TextureMap[material.MetallicMap], VS_UV).r		 : material.Metallic;
    float roughnessMap =		(material.RoughnessMap		  != 0xFFFFFFFFu) ? texture(TextureMap[material.RoughnessMap], VS_UV).r		 : material.Roughness;
    float ambientOcclusionMap = (material.AmbientOcclusionMap != 0xFFFFFFFFu) ? texture(TextureMap[material.AmbientOcclusionMap], VS_UV).r : material.AmbientOcclusion;
	float heightMap =			(material.HeightMap			  != 0xFFFFFFFFu) ? texture(TextureMap[material.HeightMap], VS_UV).r			 : material.Height;
    vec3  emissionMap =			(material.EmissionMap		  != 0xFFFFFFFFu) ? texture(TextureMap[material.EmissionMap], VS_UV).rgb		 : material.Emission;
	float alphaMap =			(material.AlphaMap			  != 0xFFFFFFFFu) ? texture(TextureMap[material.AlphaMap], VS_UV).r			 : material.Alpha;

    PS_Position = vec3(meshTransform * vec4(VS_Position.xy, 0.0f, 1.0f));
    gl_Position = spfDirectionalLightPushConstant.LightProjection * 
                  spfDirectionalLightPushConstant.LightView *  
                  meshTransform *
                  vec4(VS_Position.xy, 0.0f, 1.0f);
}