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

enum PushConstantMemberType
{
    kPushConstantVarUnknown,
    kPushConstantVarInt,
    kPushConstantVarUint,
    kPushConstantVarFloat,
    kPushConstantVarIvec2,
    kPushConstantVarIvec3,
    kPushConstantVarIvec4,
    kPushConstantVarVec2,
    kPushConstantVarVec3,
    kPushConstantVarVec4,
    kPushConstantVarMat2,
    kPushConstantVarMat3,
    kPushConstantVarMat4,
    kPushConstantVarBool
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