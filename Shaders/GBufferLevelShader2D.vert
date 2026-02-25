#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

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

    const uint baseByteLocation = (uint(bindlessBuffer.MeshOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.MeshSize / 4));
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

    const uint baseByteLocation = (uint(bindlessBuffer.MaterialOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.MaterialSize / 4));
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

    const uint baseByteLocation = (uint(bindlessBuffer.DirectionalLightOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.DirectionalLightSize / 4));
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

    const uint baseByteLocation = (uint(bindlessBuffer.PointLightOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.PointLightSize / 4));
    light.LightPosition  = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 0u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 1u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 2u]));
    light.LightColor     = vec3(uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 3u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 4u]), uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 5u]));
    light.LightRadius    = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 6u]);
    light.LightIntensity = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 7u]);
    light.ShadowStrength = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 8u]);
    light.ShadowBias     = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 9u]);
    light.ShadowSoftness = uintBitsToFloat(bindlessBuffer.Data[baseByteLocation + 10u]);
    return light;
}

TextureMetadata Get2DTextureMetadata(uint index)
{
    TextureMetadata textureMetaData;
    if (index >= bindlessBuffer.Texture2DCount) 
    {
        textureMetaData.Width       = 0;
        textureMetaData.Height      = 0;
        textureMetaData.Depth       = 0;
        textureMetaData.MipLevels   = 1;
        textureMetaData.LayerCount  = 1;
        textureMetaData.Format      = 0;
        textureMetaData.TextureType = 0;
        textureMetaData.ArrayIndex  = 0;
        return textureMetaData;
    }

    const uint baseByteLocation = (uint(bindlessBuffer.Texture2DOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.Texture2DSize / 4));
    textureMetaData.Width       = bindlessBuffer.Data[baseByteLocation + 0u];
    textureMetaData.Height      = bindlessBuffer.Data[baseByteLocation + 1u];
    textureMetaData.Depth       = bindlessBuffer.Data[baseByteLocation + 2u];
    textureMetaData.MipLevels   = bindlessBuffer.Data[baseByteLocation + 3u];
    textureMetaData.LayerCount  = bindlessBuffer.Data[baseByteLocation + 4u];
    textureMetaData.Format      = bindlessBuffer.Data[baseByteLocation + 5u];
    textureMetaData.TextureType = bindlessBuffer.Data[baseByteLocation + 6u];
    textureMetaData.ArrayIndex  = bindlessBuffer.Data[baseByteLocation + 7u];
    return textureMetaData;
}

TextureMetadata Get3DTextureMetadata(uint index)
{
    TextureMetadata textureMetaData;
    if (index >= bindlessBuffer.Texture3DCount) 
    {
        textureMetaData.Width       = 0;
        textureMetaData.Height      = 0;
        textureMetaData.Depth       = 0;
        textureMetaData.MipLevels   = 1;
        textureMetaData.LayerCount  = 1;
        textureMetaData.Format      = 0;
        textureMetaData.TextureType = 0;
        textureMetaData.ArrayIndex  = 0;
        return textureMetaData;
    }

    const uint baseByteLocation = (uint(bindlessBuffer.Texture3DOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.Texture3DSize / 4));
    textureMetaData.Width       = bindlessBuffer.Data[baseByteLocation + 0u];
    textureMetaData.Height      = bindlessBuffer.Data[baseByteLocation + 1u];
    textureMetaData.Depth       = bindlessBuffer.Data[baseByteLocation + 2u];
    textureMetaData.MipLevels   = bindlessBuffer.Data[baseByteLocation + 3u];
    textureMetaData.LayerCount  = bindlessBuffer.Data[baseByteLocation + 4u];
    textureMetaData.Format      = bindlessBuffer.Data[baseByteLocation + 5u];
    textureMetaData.TextureType = bindlessBuffer.Data[baseByteLocation + 6u];
    textureMetaData.ArrayIndex  = bindlessBuffer.Data[baseByteLocation + 7u];
    return textureMetaData;
}

TextureMetadata GetCubeMapTextureMetadata(uint index)
{
    TextureMetadata textureMetaData;
    if (index >= bindlessBuffer.TextureCubeMapCount) 
    {
        textureMetaData.Width       = 0;
        textureMetaData.Height      = 0;
        textureMetaData.Depth       = 0;
        textureMetaData.MipLevels   = 1;
        textureMetaData.LayerCount  = 1;
        textureMetaData.Format      = 0;
        textureMetaData.TextureType = 0;
        textureMetaData.ArrayIndex  = 0;
        return textureMetaData;
    }

    const uint baseByteLocation = (uint(bindlessBuffer.TextureCubeMapOffset - bindlessBuffer.MeshOffset) / 4) + (index * (bindlessBuffer.TextureCubeMapSize / 4));
    textureMetaData.Width       = bindlessBuffer.Data[baseByteLocation + 0u];
    textureMetaData.Height      = bindlessBuffer.Data[baseByteLocation + 1u];
    textureMetaData.Depth       = bindlessBuffer.Data[baseByteLocation + 2u];
    textureMetaData.MipLevels   = bindlessBuffer.Data[baseByteLocation + 3u];
    textureMetaData.LayerCount  = bindlessBuffer.Data[baseByteLocation + 4u];
    textureMetaData.Format      = bindlessBuffer.Data[baseByteLocation + 5u];
    textureMetaData.TextureType = bindlessBuffer.Data[baseByteLocation + 6u];
    textureMetaData.ArrayIndex  = bindlessBuffer.Data[baseByteLocation + 7u];
    return textureMetaData;
}

vec4 SampleTexture(uint textureIndex, vec2 uv)
{
    TextureMetadata meta = Get2DTextureMetadata(textureIndex);

    if (meta.TextureType == 0) // 2D
    {
        return texture(TextureMap[meta.ArrayIndex], uv);
    }
    return vec4(1.0, 0.0, 1.0, 1.0); // error pink
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