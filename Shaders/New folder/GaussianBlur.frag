#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 UV;
layout(location = 0) out vec4 outColor;
layout(binding = 0) uniform sampler2D BloomTexture;

layout(push_constant) uniform BloomSettings
{
    float blurScale;
    float blurStrength;
} bloomSettings;

const float blurWeights[25] = float[] (
	0.00390625f, 0.01562500f, 0.02343750f, 0.01562500f, 0.00390625f,
	0.01562500f, 0.06250000f, 0.09375000f, 0.06250000f, 0.01562500f,
	0.02343750f, 0.09375000f, 0.14062500f, 0.09375000f, 0.02343750f,
	0.01562500f, 0.06250000f, 0.09375000f, 0.06250000f, 0.01562500f,
    0.00390625f, 0.01562500f, 0.02343750f, 0.01562500f, 0.00390625f);

const vec2 offsets[25] = { 
    {-2.0f, -2.0f}, {-1.0f, -2.0f}, { 0.0f, -2.0f}, { 1.0f, -2.0f}, { 1.0f, -2.0f},
    {-2.0f, -1.0f}, {-1.0f, -1.0f}, { 0.0f, -1.0f}, { 1.0f, -1.0f}, { 1.0f, -1.0f},
    {-2.0f,  0.0f}, {-1.0f,  0.0f}, { 0.0f,  0.0f}, { 1.0f,  0.0f}, { 1.0f,  0.0f},
    {-2.0f,  1.0f}, {-1.0f,  1.0f}, { 0.0f,  1.0f}, { 1.0f,  1.0f}, { 1.0f,  1.0f},
    {-2.0f,  2.0f}, {-1.0f,  2.0f}, { 0.0f,  2.0f}, { 1.0f,  2.0f}, { 1.0f,  2.0f}};

void main()
{  
     vec2 tex_offset = 1.0 / textureSize(BloomTexture, 0) * bloomSettings.blurScale;

     vec3 result = vec3(0.0f);
     for(int x = 1; x < 25; x++)
     {
        result += (texture(BloomTexture, UV + (tex_offset * offsets[x])).rgb) * blurWeights[x] * bloomSettings.blurStrength;
     }

     outColor = vec4(result, 1.0);
}