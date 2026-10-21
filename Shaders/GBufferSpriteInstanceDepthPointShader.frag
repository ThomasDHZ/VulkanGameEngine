#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"
layout(std430, binding = 0) buffer SceneDataBuffer
{
   uint HDRMapInputIndex;
    uint EnvironmentMapIndex;
    uint BRDFMapId;
    uint CubeMapId;
    uint IrradianceMapId;
    uint PrefilterMapId;
	uint DirectionalLightShadowMapId;
	uint DirectionalLightShadowTintMapId;
	uint PointLightShadowMapId;
	uint PointLightShadowTintMapId;
    uint _pad0;
    uint _pad1;

    mat4 OrthoProjection;
    mat4 OrthoView;
    mat4 InverseOrthoProjection;
    mat4 InverseOrthoView;
    mat4 InversePerspectiveProjection;
    mat4 InversePerspectiveView;

    vec3  PerspectiveCameraPosition;
    float Time;
    vec3  PerspectiveViewDirection;
    uint  FrameIndex;
    vec2  InvertResolution;
    vec2  _padEnd;
} sceneDataBuffer;

layout(binding = 1) buffer BindlessBuffer
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

layout(push_constant) uniform PointLightSceneDataDepthBuffer
{
    int   MeshBufferIndex;
    int   PointLightBufferIndex;
    int   PointLightViewIndex;
    int   UseHeightMap;
    float HeightScale;
} pointLightSceneDataDepthBuffer;

#include "BindlessHelpers.glsl"

void main()
{
    PackedMaterial packedMaterial = GetMaterial(PS_MaterialId);

    vec2 minUV = PS_UVOffset.xy;
    vec2 maxUV = PS_UVOffset.xy + PS_UVOffset.zw;

    vec2 UV = PS_UV;
    if (PS_FlipSprite.x == 1) UV.x = minUV.x + maxUV.x - UV.x;
    if (PS_FlipSprite.y == 1) UV.y = minUV.y + maxUV.y - UV.y;

    vec4 albedo = texture(TextureMap[packedMaterial.AlbedoTextureId], UV, -0.5);
    if (albedo.a < packedMaterial.AlphaCutOff) discard;

    const PointLightBuffer light = GetPointLight(0);
    float dist = length(WorldPos - vec3(light.LightPosition.xy, light.LightLayer * SpriteLayerSpacing));
    gl_FragDepth = clamp(dist / light.LightRadius, 0.0, 1.0);
}
