#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 FragColor;

layout(constant_id = 0)   const uint DescriptorBindingType0 = SubpassInputDescriptor;
layout(constant_id = 1)   const uint DescriptorBindingType1   = SubpassInputDescriptor;
layout(constant_id = 2)   const uint DescriptorBindingType2   = SubpassInputDescriptor;
layout(constant_id = 3)   const uint DescriptorBindingType3   = SubpassInputDescriptor;
layout(constant_id = 4)   const uint DescriptorBindingType4   = SubpassInputDescriptor;
layout(constant_id = 5)   const uint DescriptorBindingType5   = SubpassInputDescriptor;
layout(constant_id = 6)   const uint DescriptorBindingType6   = SubpassInputDescriptor;
layout(constant_id = 7)   const uint DescriptorBindingType7   = SubpassInputDescriptor;
layout(constant_id = 8)   const uint DescriptorBindingType8   = SubpassInputDescriptor;
layout(constant_id = 9)   const uint DescriptorBindingType9   = MeshPropertiesDescriptor;
layout(constant_id = 10)  const uint DescriptorBindingType10  = MaterialDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = DirectionalLightDescriptor;
layout(constant_id = 12)  const uint DescriptorBindingType12  = PointLightDescriptor;
layout(constant_id = 13)  const uint DescriptorBindingType13  = TextureDescriptor;
layout(constant_id = 14)  const uint DescriptorBindingType14  = SkyBoxDescriptor;
layout(constant_id = 15)  const uint DescriptorBindingType15  = IrradianceCubeMapDescriptor;
layout(constant_id = 16)  const uint DescriptorBindingType16  = PrefilterDescriptor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionInput;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput albedoInput;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput normalInput;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput packedMROInput;
layout(input_attachment_index = 4, binding = 4) uniform subpassInput packedSheenSSSInput;
layout(input_attachment_index = 5, binding = 5) uniform subpassInput parallaxInfoInput;
layout(input_attachment_index = 6, binding = 6) uniform subpassInput emissionInput;
layout(input_attachment_index = 7, binding = 7) uniform subpassInput depthInput;
layout(input_attachment_index = 8, binding = 8) uniform subpassInput skyBoxInput;
layout(binding = 9)   buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 10)  buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];
layout(binding = 11)  buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 12)  buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
layout(binding = 13) uniform sampler2D TextureMap[];
layout(binding = 14) uniform samplerCube CubeMap;
layout(binding = 15) uniform samplerCube IrradianceMap;
layout(binding = 16) uniform samplerCube PrefilterMap;

layout(push_constant) uniform SkyBoxViewData {
    mat4 InverseProjection;
    mat4 InverseView;
} skyBoxViewData;

void main() 
{
    float depth = subpassLoad(depthInput).r;
    if (depth < 0.99995f) 
    { 
        FragColor = vec4(0.0f);
        return;
    }

    vec3 ndc = vec3(TexCoords * 2.0f - 1.0f, 1.0f);
    vec4 viewSpace = skyBoxViewData.InverseProjection * vec4(ndc, 1.0f);
    viewSpace /= viewSpace.w;

    vec3 viewDir = normalize(viewSpace.xyz);
    vec3 worldDir = normalize((skyBoxViewData.InverseView * vec4(viewDir, 0.0f)).xyz);
    vec3 skyColor = textureLod(CubeMap, worldDir, 0.0f).rgb;
  
    FragColor = vec4(skyColor, 1.0);
}