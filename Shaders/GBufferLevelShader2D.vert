#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

layout (location = 0)  in vec2  VS_Position;
layout (location = 1)  in vec2  VS_UV;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec2  PS_UV;


#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout(constant_id = 0)   const uint DescriptorBindingType0   = SubpassInputDescriptor;
layout(constant_id = 1)   const uint DescriptorBindingType1   = SubpassInputDescriptor;
layout(constant_id = 2)   const uint DescriptorBindingType2   = SubpassInputDescriptor;
layout(constant_id = 3)   const uint DescriptorBindingType3   = SubpassInputDescriptor;
layout(constant_id = 4)   const uint DescriptorBindingType4   = SubpassInputDescriptor;
layout(constant_id = 5)   const uint DescriptorBindingType5  = MeshPropertiesDescriptor;
layout(constant_id = 6)   const uint DescriptorBindingType6  = MaterialDescriptor;
layout(constant_id = 7)   const uint DescriptorBindingType7  = DirectionalLightDescriptor;
layout(constant_id = 8)   const uint DescriptorBindingType8  = PointLightDescriptor;
layout(constant_id = 9)   const uint DescriptorBindingType9  = TextureDescriptor;
layout(constant_id = 10)   const uint DescriptorBindingType10  = SkyBoxDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = IrradianceCubeMapDescriptor;
layout(constant_id = 12)  const uint DescriptorBindingType12  = PrefilterDescriptor;

layout(binding = 5)  buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 6)  buffer MaterialProperities { MaterialProperitiesBuffer2 materialProperties; } materialBuffer[];
layout(binding = 7)  buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 8)  buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
layout(binding = 9) uniform sampler2D TextureMap[];
layout(binding = 10) uniform samplerCube CubeMap;
layout(binding = 11) uniform samplerCube IrradianceMap;
layout(binding = 12) uniform samplerCube PrefilterMap;

layout(push_constant) uniform SceneDataBuffer 
{
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
    vec3  ViewDirection;
    vec3 CameraPosition;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

void main()
{
    int meshIndex = sceneData.MeshBufferIndex;
    mat4 meshTransform = meshBuffer[meshIndex].meshProperties.MeshTransform;

    PS_Position = vec3(meshTransform * vec4(VS_Position.xy, 0.0f, 1.0f));
	PS_UV = VS_UV.xy;

    gl_Position = sceneData.Projection * 
                  sceneData.View *  
                  meshTransform *
                  vec4(VS_Position.xy, 0.0f, 1.0f);
}