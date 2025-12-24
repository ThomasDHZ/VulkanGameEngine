#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 outColor;

layout(constant_id = 0) const uint DescriptorBindingType0 = 1;
layout(binding = 0) uniform sampler2D BloomTexture;

layout(push_constant) uniform BloomSettings
{
    uint  blurDirection;  // 0 = horizontal, 1 = vertical
    float blurScale;     // Controls blur radius (e.g., 1.0–4.0)
    float blurStrength;  // Overall intensity multiplier
} bloomSettings;

const float weights[3] = float[](0.227027, 0.1945946, 0.1216216);

void main()
{
    vec2 texOffset = bloomSettings.blurScale / vec2(textureSize(BloomTexture, 0));

    // Horizontal if blurDirection == 0, vertical if 1
    texOffset *= (bloomSettings.blurDirection != 0u) ? vec2(0.0, 1.0) : vec2(1.0, 0.0);

    vec3 result = texture(BloomTexture, UV).rgb * weights[0];

    result += texture(BloomTexture, UV + texOffset * 1.0).rgb * weights[1];
    result += texture(BloomTexture, UV - texOffset * 1.0).rgb * weights[1];
    result += texture(BloomTexture, UV + texOffset * 2.0).rgb * weights[2];
    result += texture(BloomTexture, UV - texOffset * 2.0).rgb * weights[2];

    outColor = vec4(result * bloomSettings.blurStrength, 1.0);
}