#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = 0;
layout(constant_id = 1) const uint DescriptorBindingType1 = 1;
layout(constant_id = 2) const uint DescriptorBindingType2 = 2;
layout(constant_id = 3) const uint DescriptorBindingType3 = 4;

layout(location = 0) in vec2 worldPos;
layout(location = 1) in vec2 UV;
layout(location = 0) out float outDistance;

layout(push_constant) uniform SPFPointLightPushConstant 
{
    int MeshBufferIndex;
    int LightBufferIndex;
    mat4 LightProjection;
    mat4 LightView;
}spfPointLightPushConstant;

#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"
#include "Lights.glsl"

layout(binding = 0) buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 1) uniform sampler2D TextureMap[];
layout(binding = 2) buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];
layout(binding = 3) buffer PointLight { PointLightBuffer pointLightProperties; } pointLightsBuffer[];

void main() 
{
    int meshIndex = spfPointLightPushConstant.MeshBufferIndex;
    uint materialId = meshBuffer[meshIndex].meshProperties.MaterialIndex;
    MaterialProperitiesBuffer material = materialBuffer[materialId].materialProperties;

    float alpha = (material.AlphaMap != 0xFFFFFFFFu) 
        ? texture(TextureMap[material.AlphaMap], UV).r 
        : material.Alpha;
//
//    if (alpha <= 0.0f) {
//        discard;
//    }

    vec2 lightPos = pointLightsBuffer[spfPointLightPushConstant.LightBufferIndex].pointLightProperties.LightPosition.xy;

    float distToLight = length(worldPos - lightPos);
    outDistance = -distToLight;
}
