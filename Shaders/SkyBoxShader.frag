#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 FragColor;

layout(constant_id = 0)   const uint DescriptorBindingType0   = SubpassInputDescriptor;
layout(constant_id = 1)   const uint DescriptorBindingType1   = SubpassInputDescriptor;
layout(constant_id = 2)   const uint DescriptorBindingType2   = SubpassInputDescriptor;
layout(constant_id = 3)   const uint DescriptorBindingType3   = SubpassInputDescriptor;
layout(constant_id = 4)   const uint DescriptorBindingType4   = SubpassInputDescriptor;
layout(constant_id = 5)   const uint DescriptorBindingType5   = SubpassInputDescriptor;
layout(constant_id = 6)   const uint DescriptorBindingType6   = SubpassInputDescriptor;
layout(constant_id = 7)   const uint DescriptorBindingType7   = SubpassInputDescriptor;
layout(constant_id = 8)   const uint DescriptorBindingType8   = SubpassInputDescriptor;
layout(constant_id = 9)   const uint DescriptorBindingType9   = SubpassInputDescriptor;
layout(constant_id = 10)  const uint DescriptorBindingType10  = MeshPropertiesDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = MaterialDescriptor;
layout(constant_id = 12)  const uint DescriptorBindingType12  = DirectionalLightDescriptor;
layout(constant_id = 13)  const uint DescriptorBindingType13  = PointLightDescriptor;
layout(constant_id = 14)  const uint DescriptorBindingType14  = TextureDescriptor;
layout(constant_id = 15)  const uint DescriptorBindingType15  = SkyBoxDescriptor;
layout(constant_id = 16)  const uint DescriptorBindingType16  = IrradianceCubeMapDescriptor;
layout(constant_id = 17)  const uint DescriptorBindingType17  = PrefilterDescriptor;

layout(binding = 10)  buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 11)  buffer MaterialProperities { MaterialProperitiesBuffer2 materialProperties; } materialBuffer[];
layout(binding = 12)  buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 13)  buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
layout(binding = 14) uniform sampler2D TextureMap[];
layout(binding = 15) uniform samplerCube CubeMap;
layout(binding = 16) uniform samplerCube IrradianceMap;
layout(binding = 17) uniform samplerCube PrefilterMap;

layout(push_constant) uniform SkyBoxViewData {
    mat4 InverseProjection;
    mat4 InverseView;
    mat3 Buffer1;
    int  Buffer2;
} skyBoxViewData;

void main() 
{
    vec3 ndc = vec3(TexCoords * 2.0f - 1.0f, 1.0f);
    vec4 viewSpace = skyBoxViewData.InverseProjection * vec4(ndc, 1.0f);
    viewSpace /= viewSpace.w;

    vec3 viewDir = normalize(viewSpace.xyz);
    vec3 worldDir = normalize((skyBoxViewData.InverseView * vec4(viewDir, 0.0f)).xyz);
    vec3 skyColor = textureLod(CubeMap, worldDir, 0.0f).rgb;
  
    FragColor = vec4(skyColor, 1.0);
}