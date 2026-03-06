#version 460
#extension GL_EXT_multiview : enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout(constant_id = 0)  const uint DescriptorBindingType0  = SceneDataDescriptor;
layout(constant_id = 1)  const uint DescriptorBindingType1  = MemoryPoolDescriptor;
layout(constant_id = 2)  const uint DescriptorBindingType2  = TextureDescriptor;
layout(constant_id = 3)  const uint DescriptorBindingType3  = Texture3DDescriptor;
layout(constant_id = 4)  const uint DescriptorBindingType4  = SkyBoxDescriptor;

layout(std430, binding = 0)  buffer SceneDataBuffer 
{ 
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
layout(binding = 2) uniform sampler2D TextureMap[];
layout(binding = 3) uniform sampler3D Texture3DMap[];
layout(binding = 4) uniform samplerCube CubeMap[];

layout (location = 0) in vec3 aPos;
layout(location = 0) out vec3 WorldPos;

layout(push_constant) uniform PrefilterSamplerProperties {
    uint CubeMapResolution;
    float Roughness;
} prefilterSamplerProperties;

const mat4 captureProjection = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, -1.0, -1.0, 
    0.0, 0.0, -0.1, 0.0  
);

// 6 view matrices (lookAt from origin to each face)
// Order: +X, -X, +Y, -Y, +Z, -Z
const mat4 captureViews[6] = mat4[](
    mat4( 0.0,  0.0, -1.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
         -1.0,  0.0,  0.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // +X

    mat4( 0.0,  0.0,  1.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
          1.0,  0.0,  0.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // -X

    mat4( 1.0,  0.0,  0.0, 0.0,
          0.0,  0.0, -1.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // +Y

    mat4( 1.0,  0.0,  0.0, 0.0,
          0.0,  0.0,  1.0, 0.0,
          0.0,  1.0,  0.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // -Y

    mat4( 1.0,  0.0,  0.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
          0.0,  0.0, -1.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // +Z

    mat4(-1.0,  0.0,  0.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
          0.0,  0.0,  1.0, 0.0,
          0.0,  0.0,  0.0, 1.0)   // -Z
);

void main()
{
    WorldPos = aPos;
    mat4 view = captureViews[gl_ViewIndex];
    vec4 pos = captureProjection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}