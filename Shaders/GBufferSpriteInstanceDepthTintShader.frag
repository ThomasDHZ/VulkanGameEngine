#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"
#include "MemoryPoolBindings.glsl"

layout(location = 0)  in vec2       PS_UV;
layout(location = 1)  in flat ivec2 PS_FlipSprite;
layout(location = 2)  in flat uint  PS_MaterialId;
layout(location = 3)  in flat vec4  PS_UVOffset;

layout(location = 0) out vec4 OutColor;

layout(push_constant) uniform DirectionalLightSceneDataDepthBuffer
{
    int   MeshBufferIndex;
    int   DirectionalLightBufferIndex;
    int   UseHeightMap;
    float HeightScale;
} directionalLightSceneDataDepthBuffer;

#include "BindlessHelpers.glsl"

const float IBL_EXPOSURE = 2.5;
const float kFeatureEps  = 1e-3;
const uint FEAT_COAT         = 1u << 0;
const uint FEAT_SHEEN        = 1u << 1;
const uint FEAT_SSS          = 1u << 2;
const uint FEAT_TRANSMISSION = 1u << 3;
const uint FEAT_ANISO        = 1u << 4;
const uint FEAT_FILM         = 1u << 5;
const uint FEAT_TWO_SIDED    = 1u << 6;
const uint FEAT_COAT_NORMAL  = 1u << 7;
const uint FEAT_ALBEDO_ONLY  = 1u << 8;

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
    OutColor = vec4(atten * T, 1.0);
}