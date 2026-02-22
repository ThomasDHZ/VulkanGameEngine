#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_shader_8bit_storage : require

layout (location = 0)  in vec2  VS_Position;
layout (location = 1)  in vec2  VS_UV;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec2  PS_UV;

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout(constant_id = 0)   const uint DescriptorBindingType0   = SubpassInputDescriptor;
layout(constant_id = 1)   const uint DescriptorBindingType1   = SubpassInputDescriptor;
layout(constant_id = 2)   const uint DescriptorBindingType2   = SubpassInputDescriptor;
layout(constant_id = 3)   const uint DescriptorBindingType3   = SubpassInputDescriptor;
layout(constant_id = 4)   const uint DescriptorBindingType4   = SubpassInputDescriptor;
layout(constant_id = 5)   const uint DescriptorBindingType5   = SubpassInputDescriptor;
layout(constant_id = 6)   const uint DescriptorBindingType6   = SubpassInputDescriptor;
layout(constant_id = 7)   const uint DescriptorBindingType7   = SubpassInputDescriptor;
layout(constant_id = 8)   const uint DescriptorBindingType8   = SubpassInputDescriptor;
layout(constant_id = 9)   const uint DescriptorBindingType9   = MemoryPoolDescriptor;
layout(constant_id = 10)  const uint DescriptorBindingType10  = TextureDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = BRDFDescriptor;
layout(constant_id = 12)  const uint DescriptorBindingType12  = SkyBoxDescriptor;
layout(constant_id = 13)  const uint DescriptorBindingType13  = IrradianceCubeMapDescriptor;
layout(constant_id = 14)  const uint DescriptorBindingType14  = PrefilterDescriptor;

layout(binding = 9)  buffer BindlessBuffer 
{ 
    uint MeshOffset;     
    uint MeshCount;
    uint MeshSize;   
    uint MaterialOffset; 
    uint MaterialCount;
    uint MaterialSize;  
    uint DirectionalLightOffset; 
    uint DirectionalLightCount;
    uint DirectionalLightSize;   
    uint PointLightOffset; 
    uint PointLightCount;
    uint PointLightSize;     
    uint Data[]; 
} bindlessBuffer;
layout(binding = 10) uniform sampler2D TextureMap[];
layout(binding = 11) uniform sampler2D BRDFMap;
layout(binding = 12) uniform samplerCube CubeMap;
layout(binding = 13) uniform samplerCube IrradianceMap;
layout(binding = 14) uniform samplerCube PrefilterMap;

layout(push_constant) uniform SceneDataBuffer
{
    int   MeshBufferIndex;
    mat4  Projection;
    mat4  View;
    vec3  ViewDirection;
    vec3  CameraPosition;
    int   UseHeightMap;
    float HeightScale;
    int   Buffer1;
} sceneData;

MeshProperitiesBuffer GetMesh(uint index) 
{
    MeshProperitiesBuffer mesh;
    if (index >= bindlessBuffer.MeshCount) 
    {
        mesh.MaterialIndex = 0u;
        mesh.MeshTransform = mat4(0.0);
        return mesh;
    }

    const uint baseByteLocation = (bindlessBuffer.MeshOffset/4)+ (index * (bindlessBuffer.MeshSize / 4));
    mesh.MaterialIndex = bindlessBuffer.Data[baseByteLocation + 0u];
    mesh.MeshTransform = mat4(
        bindlessBuffer.Data[baseByteLocation + 1u],  bindlessBuffer.Data[baseByteLocation + 2u],  bindlessBuffer.Data[baseByteLocation + 3u],  bindlessBuffer.Data[baseByteLocation + 4u],
        bindlessBuffer.Data[baseByteLocation + 5u],  bindlessBuffer.Data[baseByteLocation + 6u],  bindlessBuffer.Data[baseByteLocation + 7u],  bindlessBuffer.Data[baseByteLocation + 8u],
        bindlessBuffer.Data[baseByteLocation + 9u],  bindlessBuffer.Data[baseByteLocation + 10u], bindlessBuffer.Data[baseByteLocation + 11u], bindlessBuffer.Data[baseByteLocation + 12u],
        bindlessBuffer.Data[baseByteLocation + 13u], bindlessBuffer.Data[baseByteLocation + 14u], bindlessBuffer.Data[baseByteLocation + 15u], bindlessBuffer.Data[baseByteLocation + 16u]);

    return mesh;
}

PackedMaterial GetMaterial(uint index)
{
    PackedMaterial mat;
    mat.AlbedoDataId          = ~0u;
    mat.NormalDataId          = ~0u;
    mat.PackedMRODataId       = ~0u;
    mat.PackedSheenSSSDataId  = ~0u;
    mat.UnusedDataId          = ~0u;
    mat.EmissionDataId        = ~0u;
    if (index >= bindlessBuffer.MaterialCount)
    {
        return mat;
    }

    const uint baseByteLocation = (bindlessBuffer.MaterialOffset/4)+ (index * (bindlessBuffer.MaterialSize / 4));
    mat.AlbedoDataId          = bindlessBuffer.Data[baseByteLocation + 0u];
    mat.NormalDataId          = bindlessBuffer.Data[baseByteLocation + 1u];
    mat.PackedMRODataId       = bindlessBuffer.Data[baseByteLocation + 2u];
    mat.PackedSheenSSSDataId  = bindlessBuffer.Data[baseByteLocation + 3u];
    mat.UnusedDataId          = bindlessBuffer.Data[baseByteLocation + 4u];
    mat.EmissionDataId        = bindlessBuffer.Data[baseByteLocation + 5u];
    return mat;
}

DirectionalLightBuffer GetDirectionalLight(uint index) 
{
    DirectionalLightBuffer light;
    if (index >= bindlessBuffer.DirectionalLightCount) 
    {
        light.LightColor     = vec3(0.0);
        light.LightDirection = vec3(0.0);
        light.LightIntensity = 0.0;
        light.ShadowStrength = 0.0;
        light.ShadowBias     = 0.0;
        light.ShadowSoftness = 0.0;
        return light;
    }

    const uint baseByteLocation = (bindlessBuffer.DirectionalLightOffset/4)+ (index * (bindlessBuffer.DirectionalLightSize / 4));
    light.LightColor     = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 0u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 1u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 2u]));
    light.LightDirection = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 3u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 4u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 5u]));
    light.LightIntensity = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 6u]);
    light.ShadowStrength = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 7u]);
    light.ShadowBias     = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 8u]);
    light.ShadowSoftness = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 9u]);
    return light;
}

PointLightBuffer GetPointLight(uint index) 
{
    PointLightBuffer light;
    if (index >= bindlessBuffer.PointLightCount) 
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

    const uint baseByteLocation = (bindlessBuffer.PointLightOffset/4)+ (index * (bindlessBuffer.PointLightSize / 4));
    light.LightPosition  = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 0u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 1u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 2u]));
    light.LightColor     = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 3u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 4u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 5u]));
    light.LightRadius    = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 6u]);
    light.LightIntensity = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 7u]);
    light.ShadowStrength = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 8u]);
    light.ShadowBias     = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 9u]);
    light.ShadowSoftness = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 10u]);
    return light;
}

void main()
{
    MeshProperitiesBuffer mesh = GetMesh(sceneData.MeshBufferIndex);
    PackedMaterial material = GetMaterial(mesh.MaterialIndex);

    PS_Position = vec3(mesh.MeshTransform * vec4(VS_Position.xy, 0.0f, 1.0f));
	PS_UV = VS_UV.xy;

    gl_Position = sceneData.Projection * 
                  sceneData.View *  
                  mesh.MeshTransform *
                  vec4(VS_Position.xy, 0.0f, 1.0f);
}