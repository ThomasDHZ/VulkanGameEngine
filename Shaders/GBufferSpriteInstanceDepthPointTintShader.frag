#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"


layout(location = 0)  in vec2       PS_UV;
layout(location = 1)  in flat ivec2 PS_FlipSprite;
layout(location = 2)  in flat uint  PS_MaterialId;
layout(location = 3)  in flat vec4  PS_UVOffset;

layout(location = 0) out vec4 OutColor;

layout(push_constant) uniform PointLightSceneDataDepthBuffer
{
    int   MeshBufferIndex;
    int   PointLightBufferIndex;
    int   PointLightViewIndex;
    int   UseHeightMap;
    float HeightScale;
} pointLightSceneDataDepthBuffer;

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
} sceneDataBuffer2;

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

    vec3 attenuationColor             =  max((packedMaterial.TranslucentTextureId != NO_MAP) ? textureLod(TextureMap[packedMaterial.TranslucentTextureId], UV, 0.0).rgb : vec3(1.0), vec3(1e-4));
    vec4 translucentPropertiesTexture = textureLod(TextureMap[packedMaterial.TranslucentPropertiesTextureId], UV, 0.0);
    float thickness                   = translucentPropertiesTexture.g;
    float transmissionWeight          = translucentPropertiesTexture.r;

    vec3 sigmaA                       = -log(attenuationColor) / thickness;
    vec3 T                            = exp(-sigmaA * thickness);
    float coverage                    = clamp(transmissionWeight, 0.0, 1.0);
    OutColor = vec4(T, coverage);
}