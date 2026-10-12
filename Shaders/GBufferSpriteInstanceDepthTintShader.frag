#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"
#include "MemoryPoolBindings.glsl"

layout(location = 0)  in vec3       WorldPos;
layout(location = 1)  in vec2       PS_UV;
layout(location = 2)  in vec2       PS_SpriteSize;
layout(location = 3)  in flat ivec2 PS_FlipSprite;
layout(location = 4)  in vec4       PS_Color;
layout(location = 5)  in flat uint  PS_MaterialId;
layout(location = 6)  in flat vec4  PS_UVOffset;
layout(location = 7)  in flat uint  PS_SpriteId;
layout(location = 8)  in vec3       PS_T;
layout(location = 9)  in vec3       PS_B;
layout(location = 10) in vec3       PS_N;

layout(location = 0) out vec4 OutColor;
#include "BindlessHelpers.glsl"

const uint NO_MAP = 0xFFFFFFFFu;
void main()
{
    PackedMaterial packedMaterial = GetMaterial(PS_MaterialId);

    vec2 minUV = PS_UVOffset.xy;
    vec2 maxUV = PS_UVOffset.xy + PS_UVOffset.zw;

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) UV.x = minUV.x + maxUV.x - UV.x;
    if (PS_FlipSprite.y == 1) UV.y = minUV.y + maxUV.y - UV.y;

    vec4 albedo = texture(TextureMap[packedMaterial.AlbedoTextureId], UV, -0.5);
    if (albedo.a >= packedMaterial.AlphaCutOff || albedo.a < 0.01) discard;

    vec3 atten = (packedMaterial.TranslucentTextureId != NO_MAP) ? textureLod(TextureMap[packedMaterial.TranslucentTextureId], UV, 0.0).rgb : vec3(1.0);
    float T = (packedMaterial.TranslucentPropertiesTextureId != NO_MAP) ? textureLod(TextureMap[packedMaterial.TranslucentPropertiesTextureId], UV, 0.0).r : 0.0;
    OutColor = vec4(vec3(0.0f, 0.0f, 0.7f) * T, 1.0);
}
