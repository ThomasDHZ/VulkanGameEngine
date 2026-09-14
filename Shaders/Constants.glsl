#ifndef CONSTANTS_GLSL
#define CONSTANTS_GLSL
#extension GL_ARB_separate_shader_objects : enable
const float PI = 3.14159265359;
const uint MeshPropertiesDescriptor = 0;
const uint TextureDescriptor = 1;
const uint MaterialDescriptor = 2;
const uint DirectionalLightDescriptor = 3;
const uint PointLightDescriptor = 4;
const uint kSpotLightDescriptor = 5;
const uint VertexDescsriptor = 6;
const uint IndexDescriptor = 7;
const uint TransformDescriptor = 8;
const uint SkyBoxDescriptor = 9;
const uint IrradianceCubeMapDescriptor = 10;
const uint PrefilterDescriptor = 11;
const uint SubpassInputDescriptor = 12;
const uint BRDFDescriptor = 13;
const uint EnvironmentMapDescriptor = 14;
const uint MemoryPoolDescriptor = 15;
const uint Texture3DDescriptor = 16;
const uint SceneDataDescriptor = 17;

const uint FEAT_COAT         = 1u << 0;
const uint FEAT_SHEEN        = 1u << 1;
const uint FEAT_SSS          = 1u << 2;
const uint FEAT_TRANSMISSION = 1u << 3;
const uint FEAT_ANISO        = 1u << 4;
const uint FEAT_FILM         = 1u << 5;
const uint FEAT_TWO_SIDED    = 1u << 6;
const uint FEAT_COAT_NORMAL  = 1u << 7;

#define saturate(x) clamp(x, 0.0, 1.0)
#endif
