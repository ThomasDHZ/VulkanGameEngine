#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_NV_shader_buffer_load : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "BindlessHelpers.glsl"

layout(push_constant) uniform BloomSettings
{
    uint  blurVertTextureIndex;
    uint  blurHorTextureIndex;
    uint  blurDirection;  // 1 = horizontal, 0 = vertical
    float blurScale;     // Controls blur radius (e.g., 1.0–4.0)
    float blurStrength;  // Overall intensity multiplier
} bloomSettings;

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 vertColor;
layout(location = 1) out vec4 horizontalColor;

const float weights[3] = float[](0.227027, 0.1945946, 0.1216216);

void main()
{
    if(bloomSettings.blurDirection == 0)
    {
        vec2 texOffset = bloomSettings.blurScale / vec2(textureSize(TextureMap[bloomSettings.blurVertTextureIndex], 0));
        texOffset *= (bloomSettings.blurDirection != 0u) ? vec2(0.0, 1.0) : vec2(1.0, 0.0);

        vec3 result = texture(TextureMap[bloomSettings.blurVertTextureIndex], UV).rgb * weights[0];
        result += texture(TextureMap[bloomSettings.blurVertTextureIndex], UV + texOffset * 1.0).rgb * weights[1];
        result += texture(TextureMap[bloomSettings.blurVertTextureIndex], UV - texOffset * 1.0).rgb * weights[1];
        result += texture(TextureMap[bloomSettings.blurVertTextureIndex], UV + texOffset * 2.0).rgb * weights[2];
        result += texture(TextureMap[bloomSettings.blurVertTextureIndex], UV - texOffset * 2.0).rgb * weights[2];

        vertColor = vec4(result * bloomSettings.blurStrength, 1.0);
        horizontalColor = vec4(result * bloomSettings.blurStrength, 1.0);
    }
    else
    {
        vec2 texOffset = bloomSettings.blurScale / vec2(textureSize(TextureMap[bloomSettings.blurHorTextureIndex], 0));
        texOffset *= (bloomSettings.blurDirection != 0u) ? vec2(0.0, 1.0) : vec2(1.0, 0.0);

        vec3 result = texture(TextureMap[bloomSettings.blurHorTextureIndex], UV).rgb * weights[0];
        result += texture(TextureMap[bloomSettings.blurHorTextureIndex], UV + texOffset * 1.0).rgb * weights[1];
        result += texture(TextureMap[bloomSettings.blurHorTextureIndex], UV - texOffset * 1.0).rgb * weights[1];
        result += texture(TextureMap[bloomSettings.blurHorTextureIndex], UV + texOffset * 2.0).rgb * weights[2];
        result += texture(TextureMap[bloomSettings.blurHorTextureIndex], UV - texOffset * 2.0).rgb * weights[2];

        vertColor = vec4(result * bloomSettings.blurStrength, 1.0);
        horizontalColor = vec4(result * bloomSettings.blurStrength, 1.0);
    }
}