#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension VK_EXT_dynamic_color_write_enable : enable

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 vertColor;
layout(location = 1) out vec4 horizontalColor;

layout(constant_id = 0) const uint DescriptorBindingType0 = 1;
layout(constant_id = 1) const uint DescriptorBindingType1 = 1;

layout(binding = 0) uniform sampler2D vertTexture;
layout(binding = 1) uniform sampler2D horizontalTexture;

layout(push_constant) uniform BloomSettings
{
    uint  blurDirection;  // 1 = horizontal, 0 = vertical
    float blurScale;     // Controls blur radius (e.g., 1.0–4.0)
    float blurStrength;  // Overall intensity multiplier
} bloomSettings;

const float weights[3] = float[](0.227027, 0.1945946, 0.1216216);

void main()
{
    if(bloomSettings.blurDirection == 0)
    {
        vec2 texOffset = bloomSettings.blurScale / vec2(textureSize(vertTexture, 0));
        texOffset *= (bloomSettings.blurDirection != 0u) ? vec2(0.0, 1.0) : vec2(1.0, 0.0);

        vec3 result = texture(vertTexture, UV).rgb * weights[0];
        result += texture(vertTexture, UV + texOffset * 1.0).rgb * weights[1];
        result += texture(vertTexture, UV - texOffset * 1.0).rgb * weights[1];
        result += texture(vertTexture, UV + texOffset * 2.0).rgb * weights[2];
        result += texture(vertTexture, UV - texOffset * 2.0).rgb * weights[2];

        vertColor = vec4(result * bloomSettings.blurStrength, 1.0);
        horizontalColor = vec4(result * bloomSettings.blurStrength, 1.0);
    }
    else
    {
        vec2 texOffset = bloomSettings.blurScale / vec2(textureSize(horizontalTexture, 0));
        texOffset *= (bloomSettings.blurDirection != 0u) ? vec2(0.0, 1.0) : vec2(1.0, 0.0);

        vec3 result = texture(horizontalTexture, UV).rgb * weights[0];
        result += texture(horizontalTexture, UV + texOffset * 1.0).rgb * weights[1];
        result += texture(horizontalTexture, UV - texOffset * 1.0).rgb * weights[1];
        result += texture(horizontalTexture, UV + texOffset * 2.0).rgb * weights[2];
        result += texture(horizontalTexture, UV - texOffset * 2.0).rgb * weights[2];

        vertColor = vec4(result * bloomSettings.blurStrength, 1.0);
        horizontalColor = vec4(result * bloomSettings.blurStrength, 1.0);
    }
}