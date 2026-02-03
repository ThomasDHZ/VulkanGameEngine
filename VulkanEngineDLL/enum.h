#pragma once


enum ShaderMemberType
{
    shaderUnknown,
    shaderInt,
    shaderUint,
    shaderFloat,
    shaderIvec2,
    shaderIvec3,
    shaderIvec4,
    shaderVec2,
    shaderVec3,
    shaderVec4,
    shaderMat2,
    shaderMat3,
    shaderMat4,
    shaderbool
};


enum ColorChannelUsed
{
    ChannelR = 1,
    ChannelRG,
    ChannelRGB,
    ChannelRGBA
};

enum TextureUsageEnum
{
    kUse_Undefined,
    kUse_2DImageTexture,
    kUse_2DDataTexture,
    kUse_2DRenderedTexture,
    kUse_2DDepthTexture,
    kUse_3DImageTexture,
    kUse_3DDataTexture,
    kUse_3DRenderedTexture,
    kUse_3DDepthTexture,
    kUse_CubeMapTexture,
    kUse_CubeMapDepthTexture
};

enum TextureTypeEnum
{
    TextureType_UNKNOWN,
    TextureType_ColorTexture,
    TextureType_DepthTexture,
    TextureType_SkyboxTexture,
    TextureType_IrradianceMapTexture,
    TextureType_PrefilterMapTexture
};

enum RenderTextureTypeEnum
{
    RenderType_UNKNOWN,
    RenderType_SwapChainTexture,
    RenderType_OffscreenColorTexture,
    RenderType_DepthBufferTexture,
    RenderType_GBufferTexture,
    RenderType_IrradianceTexture,
    RenderType_PrefilterTexture,
    RenderType_CubeMapTexture
};

enum RenderAttachmentTypeEnum
{
    ColorRenderedTexture,
    InputAttachmentTexture,
    ResolveAttachmentTexture,
    DepthRenderedTexture,
    SkipSubPass
};

enum DescriptorBindingPropertiesEnum
{
    kMeshPropertiesDescriptor,
    kTextureDescriptor,
    kMaterialDescriptor,
    kDirectionalLightDescriptor,
    kPointLightDescriptor,
    kSpotLightDescriptor,
    kVertexDescsriptor,
    kIndexDescriptor,
    kTransformDescriptor,
    kSkyBoxDescriptor,
    kIrradianceMapDescriptor,
    kPrefilterMapDescriptor,
    kSubpassInputDescriptor,
    kBRDFMapDescriptor,
    kEnvironmentMapDescriptor
};