#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 
#include "MemoryPoolBindings.glsl"

layout (location = 0) in vec3  WorldPos;
layout (location = 1) in vec2  PS_UV;
layout (location = 2) in vec2  PS_SpriteSize;
layout (location = 3) in flat ivec2 PS_FlipSprite;
layout (location = 4) in vec4  PS_Color;
layout (location = 5) in flat uint  PS_MaterialId;
layout (location = 6) in flat vec4  PS_UVOffset;
layout (location = 7) in flat uint  PS_SpriteId;

layout(location = 0) out vec4 outPosition;      //R16G16B16A16_SFLOAT
layout(location = 1) out vec4 outAlbedo;        //R8G8B8A8_SRGB
layout(location = 2) out vec4 outNormalData;    //R16G16B16A16_UNORM 
layout(location = 3) out vec4 outMRO;           //R16G16B16A16_UNORM
layout(location = 4) out vec4 outFeatureA;      //R16G16B16A16_UNORM
layout(location = 5) out vec4 outFeatureB;      //R16G16B16A16_UNORM
layout(location = 6) out vec4 outFeatureC;      //R16G16B16A16_UNORM
layout(location = 7) out vec4 outEmission;      //R16G16B16A16_SFLOAT

layout(push_constant) uniform SceneDataBuffer
{
    int   MeshBufferIndex;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

#include "BindlessHelpers.glsl"

void main() 
{
    outPosition   = vec4(WorldPos, 1.0);
    outAlbedo     = vec4(1.0f);
    outNormalData = vec4(0.0f);
    outMRO        = vec4(0.0f);
    outFeatureA   = vec4(0.0f);
    outFeatureB   = vec4(0.0f);
    outFeatureC   = vec4(0.0f);
    outEmission   = vec4(0.0f);
}