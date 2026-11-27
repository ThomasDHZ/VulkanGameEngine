#ifndef SceneDataBufferInclude
#define SceneDataBufferInclude

struct SceneDataBuffer
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
};

layout(buffer_reference, std430, buffer_reference_align = 16) readonly buffer SceneDataBufferRef
{
    SceneDataBuffer data;
};

SceneDataBuffer GetSceneDataBuffer(uint64_t bufferPtr)
{
    SceneDataBufferRef ref = SceneDataBufferRef(bufferPtr);
    return ref.data;
}
#endif
