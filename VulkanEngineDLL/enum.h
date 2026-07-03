#pragma once

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

enum DescriptorBindingTypeEnum
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