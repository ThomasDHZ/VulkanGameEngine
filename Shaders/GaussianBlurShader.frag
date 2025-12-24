#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 outColor;

layout(constant_id = 0) const uint DescriptorBindingType0 = 1;
layout(binding = 0) uniform sampler2D BloomTexture;

layout(push_constant) uniform BloomSettings
{
    float blurScale;
    float blurStrength;
} bloomSettings;

void main()
{
    vec2 texOffset = bloomSettings.blurScale / vec2(textureSize(BloomTexture, 0)) * vec2(1.0, 0.0);  // Horizontal

    vec3 result = texture(BloomTexture, UV).rgb * 0.227027;

    result += texture(BloomTexture, UV + texOffset * 1.0).rgb * 0.1945946;
    result += texture(BloomTexture, UV - texOffset * 1.0).rgb * 0.1945946;
    result += texture(BloomTexture, UV + texOffset * 2.0).rgb * 0.1216216;
    result += texture(BloomTexture, UV - texOffset * 2.0).rgb * 0.1216216;

    outColor = vec4(result * bloomSettings.blurStrength, 1.0);
}