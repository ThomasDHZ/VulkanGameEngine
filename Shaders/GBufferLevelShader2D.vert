#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

layout (location = 0)  in vec2  VS_Position;
layout (location = 1)  in vec2  VS_UV;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec2  PS_UV;

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 


layout(constant_id = 0)  const uint DescriptorBindingType0  = MemoryPoolDescriptor;
layout(constant_id = 1)  const uint DescriptorBindingType1  = MemoryPoolDescriptor;
layout(constant_id = 2)  const uint DescriptorBindingType2  = TextureDescriptor;
layout(constant_id = 3)  const uint DescriptorBindingType3  = Texture3DDescriptor;
layout(constant_id = 4)  const uint DescriptorBindingType4  = SkyBoxDescriptor;

layout(binding = 0)  buffer SceneDataBuffer 
{ 
	mat4  Projection;
	mat4  View;
	mat4  InverseProjection;
	mat4  InverseView;
	vec3  CameraPosition;
	vec3  ViewDirection;
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
layout(binding = 2) uniform sampler2D TextureMap[];
layout(binding = 3) uniform sampler3D Texture3DMap[];
layout(binding = 4) uniform samplerCube CubeMap[];

layout(push_constant) uniform SceneDataBuffer
{
    int   MeshBufferIndex;
    mat4  Projection;
    mat4  View;
    vec3  ViewDirection;
    vec3  CameraPosition;
    int   UseHeightMap;
    float HeightScale;
    int   Buffer1;
} sceneData;

#include "BindlessHelpers.glsl"

vec4 SampleTexture(uint textureIndex, vec2 uv)
{
    TextureMetadata meta = Get2DTextureMetadata(textureIndex);

    if (meta.TextureType == 0) // 2D
    {
        return texture(TextureMap[meta.ArrayIndex], uv);
    }
    return vec4(1.0, 0.0, 1.0, 1.0); // error pink
}

void main()
{
    MeshProperitiesBuffer mesh = GetMesh(sceneData.MeshBufferIndex);
    PackedMaterial material = GetMaterial(mesh.MaterialIndex);

    PS_Position = vec3(mesh.MeshTransform * vec4(VS_Position.xy, 0.0f, 1.0f));
	PS_UV = VS_UV.xy;

    gl_Position = sceneData.Projection * 
                  sceneData.View *  
                  mesh.MeshTransform *
                  vec4(VS_Position.xy, 0.0f, 1.0f);
}