#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"

layout(binding = 0) uniform sampler2D TextureMap[];
layout(binding = 1) uniform samplerCube CubeMaps[];
layout(binding = 2) buffer ScenePropertiesBuffer
{
    uint MeshPropertiesOffset;
    uint MaterialOffset;
    uint CubeMapMaterialOffset;
    uint DirectionalLightOffset;
    uint PointLightOffset;

    uint MeshPropertiesCount;
    uint MaterialCount;
    uint CubeMapMaterialCount;
    uint DirectionalLightCount;
    uint PointLightCount;

    uint MeshPropertiesSize;
    uint MaterialSize;
    uint CubeMapMaterialSize;
    uint DirectionalLightSize;
    uint PointLightSize;

    uint ByteData[];
} scenePropertiesBuffer;

uint UnpackUint(uint base, uint offset) 
{
    return scenePropertiesBuffer.ByteData[base + offset + 0];
}

float UnpackFloat(uint base, uint offset) 
{
    return uintBitsToFloat(scenePropertiesBuffer.ByteData[base + offset + 0]);
}

vec3 UnpackVec3(uint base, uint offset) 
{
    return vec3(
        uintBitsToFloat(scenePropertiesBuffer.ByteData[base + offset + 0]),
        uintBitsToFloat(scenePropertiesBuffer.ByteData[base + offset + 1]),
        uintBitsToFloat(scenePropertiesBuffer.ByteData[base + offset + 2])
    );
}

vec4 UnpackVec4(uint base, uint offset) 
{
    return vec4(
        uintBitsToFloat(scenePropertiesBuffer.ByteData[base + offset + 0]),
        uintBitsToFloat(scenePropertiesBuffer.ByteData[base + offset + 1]),
        uintBitsToFloat(scenePropertiesBuffer.ByteData[base + offset + 2]),
        uintBitsToFloat(scenePropertiesBuffer.ByteData[base + offset + 3])
    );
}

mat4 UnpackMat4(uint base, uint offset) {
    return mat4(
        UnpackVec4(base, offset + 0),
        UnpackVec4(base, offset + 4),
        UnpackVec4(base, offset + 8),
        UnpackVec4(base, offset + 12)
    );
}

MeshPropertiesBuffer GetMesh(uint index) {
    MeshPropertiesBuffer mesh;

    if (index >= scenePropertiesBuffer.MeshPropertiesCount) 
    {
        mesh.MaterialIndex = 0u;
        mesh.MeshTransform = mat4(0.0);
        return mesh;
    }

    uint startUint = scenePropertiesBuffer.MeshPropertiesOffset / 4;
    uint strideUint = scenePropertiesBuffer.MeshPropertiesSize; 
    uint base = startUint + index * strideUint;

    mesh.MeshTransform = UnpackMat4(base, 0);
    mesh.MaterialIndex = UnpackUint(base, 16);
    return mesh;
}

Material GetMaterial(uint index) 
{
    Material mat;

    if (index >= scenePropertiesBuffer.MaterialCount) {
        mat.AlbedoDataId       = 0u;
        mat.NormalDataId       = 0u;
        mat.PackedMRODataId    = 0u;
        mat.PackedSheenSSSDataId = 0u;
        mat.UnusedDataId       = 0u;
        mat.EmissionDataId     = 0u;
        return mat;
    }

    uint startUint = scenePropertiesBuffer.MaterialOffset / 4;
    uint strideUint = scenePropertiesBuffer.MaterialSize;

    uint base = startUint + index * strideUint;

    mat.AlbedoDataId       = UnpackUint(base, 0);
    mat.NormalDataId       = UnpackUint(base, 1);
    mat.PackedMRODataId    = UnpackUint(base, 2);
    mat.PackedSheenSSSDataId = UnpackUint(base, 3);
    mat.UnusedDataId       = UnpackUint(base, 4);
    mat.EmissionDataId     = UnpackUint(base, 5);

    return mat;
}

CubeMapMaterial GetCubeMapMaterial(uint index) 
{
    CubeMapMaterial mat;

    if (index >= scenePropertiesBuffer.CubeMapMaterialCount) 
    {
        mat.CubeMapId    = 0u;
        mat.IrradianceId = 0u;
        mat.PrefilterId  = 0u;
        return mat;
    }

    uint startUint = scenePropertiesBuffer.CubeMapMaterialOffset / 4;
    uint strideUint = scenePropertiesBuffer.CubeMapMaterialSize;

    uint base = startUint + index * strideUint;

    mat.CubeMapId    = UnpackUint(base, 0);
    mat.IrradianceId = UnpackUint(base, 1);
    mat.PrefilterId  = UnpackUint(base, 2);

    return mat;
}

DirectionalLightBuffer GetDirectionalLight(uint index) {
    DirectionalLightBuffer light;

    if (index >= scenePropertiesBuffer.DirectionalLightCount) 
    {
        light.LightColor     = vec3(0.0);
        light.LightDirection = vec3(0.0);
        light.LightIntensity = 0.0;
        light.ShadowStrength = 0.0;
        light.ShadowBias     = 0.0;
        light.ShadowSoftness = 0.0;
        return light;
    }

    uint startUint = scenePropertiesBuffer.DirectionalLightOffset / 4;
    uint strideUint = scenePropertiesBuffer.DirectionalLightSize;

    uint base = startUint + index * strideUint;

    light.LightColor     = UnpackVec3(base, 0);
    light.LightDirection = UnpackVec3(base, 3);
    light.LightIntensity = UnpackFloat(base, 6);
    light.ShadowStrength = UnpackFloat(base, 7);
    light.ShadowBias     = UnpackFloat(base, 8);
    light.ShadowSoftness = UnpackFloat(base, 9);

    return light;
}

PointLightBuffer GetPointLight(uint index) 
{
    PointLightBuffer light;

    if (index >= scenePropertiesBuffer.PointLightCount) 
    {
        light.LightPosition  = vec3(0.0);
        light.LightColor     = vec3(0.0);
        light.LightRadius    = 0.0;
        light.LightIntensity = 0.0;
        light.ShadowStrength = 0.0;
        light.ShadowBias     = 0.0;
        light.ShadowSoftness = 0.0;
        return light;
    }

    uint startUint = scenePropertiesBuffer.PointLightOffset / 4;
    uint strideUint = scenePropertiesBuffer.PointLightSize;
    uint base = startUint + index * strideUint;

    light.LightPosition  = UnpackVec3(base, 0);
    light.LightColor     = UnpackVec3(base, 3);
    light.LightRadius    = UnpackFloat(base, 6);
    light.LightIntensity = UnpackFloat(base, 7);
    light.ShadowStrength = UnpackFloat(base, 8);
    light.ShadowBias     = UnpackFloat(base, 9);
    light.ShadowSoftness = UnpackFloat(base, 10);
    return light;
}
