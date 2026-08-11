#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout(std430, binding = 0)  buffer SceneDataBuffer 
{ 	uint HDRMapIndex;
	uint FrameBufferIndex;
	uint BRDFMapId;
	uint CubeMapId;
	uint IrradianceMapId;
	uint PrefilterMapId;
	mat4  Projection;
	mat4  View;
	mat4  InverseProjection;
	mat4  InverseView;
	vec3  CameraPosition;
	vec3  ViewDirection;
    vec2  InvertResolution;
	float Time;
	uint  FrameIndex;
}sceneDataBuffer;
layout(binding = 1)  buffer BindlessBuffer 
{ 
    uint64_t MeshOffset;     
    uint MeshCount;
    uint MeshSize;   
    uint64_t MaterialOffset; 
    uint MaterialCount;
    uint MaterialSize;  
    uint64_t DirectionalLightOffset; 
    uint DirectionalLightCount;
    uint DirectionalLightSize;   
    uint64_t PointLightOffset; 
    uint PointLightCount;
    uint PointLightSize;     
    uint64_t Texture2DOffset;
	uint Texture2DCount;
	uint Texture2DSize;
	uint64_t Texture3DOffset;
	uint Texture3DCount;
	uint Texture3DSize;
	uint64_t TextureCubeMapOffset;
	uint TextureCubeMapCount;
	uint TextureCubeMapSize;
    uint64_t SpriteInstanceOffset;
	uint SpriteInstanceCount;
    uint SpriteInstanceSize;
    uint Data[]; 
} bindlessBuffer;
layout(binding = 2) uniform samplerCube CubeMap[];
layout(binding = 3) uniform sampler2D TextureMap[];
layout(binding = 4) uniform sampler3D Texture3DMap[];


layout (location = 0) in vec3  WorldPos;
layout (location = 1) in vec2  PS_UV;
layout (location = 2) in vec2  PS_SpriteSize;
layout (location = 3) in flat ivec2 PS_FlipSprite;
layout (location = 4) in vec4  PS_Color;
layout (location = 5) in flat uint  PS_MaterialId;
layout (location = 6) in flat vec4  PS_UVOffset;
layout (location = 7) in flat uint  PS_SpriteId;

layout(location = 0) out uint outGameObjectId; 

#include "BindlessHelpers.glsl"
#define UINT_MAX 4294967295u
void main() 
{
    PackedMaterial material = GetMaterial(PS_MaterialId);

   vec2 UV = PS_UV;
   if (PS_FlipSprite.x == 1) UV.x = PS_UVOffset.x + PS_UVOffset.z - (UV.x - PS_UVOffset.x);
   if (PS_FlipSprite.y == 1) UV.y = PS_UVOffset.y + PS_UVOffset.w - (UV.y - PS_UVOffset.y);

   vec4 albedoData           = texture(TextureMap[material.AlbedoDataId],            PS_UV, -0.5f).rgba;    
   if (albedoData.a < 0.1f) discard; 
   else outGameObjectId = PS_SpriteId;
}