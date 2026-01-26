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
layout(constant_id = 4)   const uint DescriptorBindingType4  = MeshPropertiesDescriptor;
layout(constant_id = 5)   const uint DescriptorBindingType5  = MaterialDescriptor;
layout(constant_id = 6)   const uint DescriptorBindingType6  = DirectionalLightDescriptor;
layout(constant_id = 7)   const uint DescriptorBindingType7  = PointLightDescriptor;
layout(constant_id = 8)   const uint DescriptorBindingType8  = TextureDescriptor;
layout(constant_id = 9)   const uint DescriptorBindingType9  = SkyBoxDescriptor;
layout(constant_id = 10)  const uint DescriptorBindingType10  = IrradianceCubeMapDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = PrefilterDescriptor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionInput;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput albedoInput;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput depthInput;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput skyBoxInput;
layout(binding = 4)  buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 5)  buffer MaterialProperities { MaterialProperitiesBuffer2 materialProperties; } materialBuffer[];
layout(binding = 6)  buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 7)  buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
layout(binding = 8) uniform sampler2D TextureMap[];
layout(binding = 9) uniform samplerCube CubeMap;
layout(binding = 10) uniform samplerCube IrradianceMap;
layout(binding = 11) uniform samplerCube PrefilterMap;

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