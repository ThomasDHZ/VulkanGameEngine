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

enum GameObjectMemberType
{
   GameObjectVarUnknown,
   GameObjectVarInt,
   GameObjectVarUint,
   GameObjectVarFloat,
   GameObjectVarIvec2,
   GameObjectVarIvec3,
   GameObjectVarIvec4,
   GameObjectVarVec2,
   GameObjectVarVec3,
   GameObjectVarVec4,
   GameObjectVarMat2,
   GameObjectVarMat3,
   GameObjectVarMat4,
   GameObjectVarBool
};

enum TextureUsageTypeEnum : uint32
{
    kUsageType_Undefined,
    kUsageType_SwapChainTexture,
    kUsageType_OffscreenColorTexture,
    kUsageType_DepthBufferTexture,
    kUsageType_GBufferTexture,
    kUsageType_IrradianceTexture,
    kUsageType_PrefilterTexture,
    kUsageType_CubeMap,
    kUsageType_BRDFTexture,
    kUsageType_Texture
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
    kEnvironmentMapDescriptor,
    kBindlessDataDescriptor,
    kTexture3DDescriptor,
    kSceneDataDescriptor
};