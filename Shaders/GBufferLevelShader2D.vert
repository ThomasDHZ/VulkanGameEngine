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
layout(constant_id = 9)   const uint DescriptorBindingType9  = MeshPropertiesDescriptor;
layout(constant_id = 10)  const uint DescriptorBindingType10  = MaterialDescriptor;
layout(constant_id = 11)  const uint DescriptorBindingType11  = DirectionalLightDescriptor;
layout(constant_id = 12)  const uint DescriptorBindingType12  = PointLightDescriptor;
layout(constant_id = 13)  const uint DescriptorBindingType13  = TextureDescriptor;
layout(constant_id = 14)  const uint DescriptorBindingType14  = SkyBoxDescriptor;
layout(constant_id = 15)  const uint DescriptorBindingType15  = IrradianceCubeMapDescriptor;
layout(constant_id = 16)  const uint DescriptorBindingType16  = PrefilterDescriptor;

layout(binding = 9)  buffer MeshProperities 
{ 
    uint MeshOffset;     
    uint MeshCount;
    uint MeshSize;    
    uint Data[]; 
} meshBuffer;
layout(binding = 10) buffer MaterialProperties
{
    uint MaterialOffset; 
    uint MaterialCount;
    uint MaterialSize;      
    uint Data[];            
} materialBuffer;
layout(binding = 11)  buffer DirectionalLight 
{ 
    uint LightOffset; 
    uint LightCount;
    uint LightSize;      
    uint Data[];    
} directionalLightBuffer;
layout(binding = 12)  buffer PointLight 
{ 
    uint LightOffset; 
    uint LightCount;
    uint LightSize;      
    uint Data[];    
} 
pointLightBuffer;
layout(binding = 13) uniform sampler2D TextureMap[];
layout(binding = 14) uniform samplerCube CubeMap;
layout(binding = 15) uniform samplerCube IrradianceMap;
layout(binding = 16) uniform samplerCube PrefilterMap;

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

uint ReadUint(uint uintBase, uint offsetUints)
{
    return materialBuffer.Data[uintBase + offsetUints];
}

float ReadFloat(uint uintBase, uint offsetUints)
{
    return uintBitsToFloat(materialBuffer.Data[uintBase + offsetUints]);
}

vec2 ReadVec2(uint uintBase, uint offsetUints)
{
    return vec2(
        ReadFloat(uintBase, offsetUints + 0u),
        ReadFloat(uintBase, offsetUints + 1u)
    );
}

vec3 ReadVec3(uint uintBase, uint offsetUints)
{
    return vec3(
        ReadFloat(uintBase, offsetUints + 0u),
        ReadFloat(uintBase, offsetUints + 1u),
        ReadFloat(uintBase, offsetUints + 2u)
    );
}

vec4 ReadVec4(uint uintBase, uint offsetUints)
{
    return vec4(
        ReadFloat(uintBase, offsetUints + 0u),
        ReadFloat(uintBase, offsetUints + 1u),
        ReadFloat(uintBase, offsetUints + 2u),
        ReadFloat(uintBase, offsetUints + 3u)
    );
}

mat3 ReadMat3(uint uintBase, uint offsetUints)
{
    return mat3(
        ReadVec3(uintBase, offsetUints + 0u),   // column 0
        ReadVec3(uintBase, offsetUints + 3u),   // column 1
        ReadVec3(uintBase, offsetUints + 6u)    // column 2
    );
}

mat4 ReadMat4(uint uintBase, uint offsetUints)
{
    return mat4(
        ReadVec4(uintBase, offsetUints +  0u),  // column 0
        ReadVec4(uintBase, offsetUints +  4u),  // column 1
        ReadVec4(uintBase, offsetUints +  8u),  // column 2
        ReadVec4(uintBase, offsetUints + 12u)   // column 3
    );
}

MeshProperitiesBuffer GetMesh(uint index) 
{
    MeshProperitiesBuffer mesh;

    if (index >= meshBuffer.MeshCount) 
    {
        mesh.MaterialIndex = 0u;
        mesh.MeshTransform = mat4(0.0);
        return mesh;
    }

    uint startUint = meshBuffer.MeshOffset / 4;
    uint strideUint = meshBuffer.MeshSize / 4; 
    uint base = startUint + index * 17;

    mesh.MaterialIndex = meshBuffer.Data[(index * 17) + 0u];
    mesh.MeshTransform = mat4(
        meshBuffer.Data[(index * 17) + 1u],  meshBuffer.Data[(index * 17) + 2u],  meshBuffer.Data[(index * 17) + 3u],  meshBuffer.Data[(index * 17) + 4u],
        meshBuffer.Data[(index * 17) + 5u],  meshBuffer.Data[(index * 17) + 6u],  meshBuffer.Data[(index * 17) + 7u],  meshBuffer.Data[(index * 17) + 8u],
        meshBuffer.Data[(index * 17) + 9u],  meshBuffer.Data[(index * 17) + 10u], meshBuffer.Data[(index * 17) + 11u], meshBuffer.Data[(index * 17) + 12u],
        meshBuffer.Data[(index * 17) + 13u], meshBuffer.Data[(index * 17) + 14u], meshBuffer.Data[(index * 17) + 15u], meshBuffer.Data[(index * 17) + 16u]);

    return mesh;
}

MaterialProperitiesBuffer2 GetMaterial(uint index)
{
    MaterialProperitiesBuffer2 mat;
    mat.AlbedoDataId          = ~0u;
    mat.NormalDataId          = ~0u;
    mat.PackedMRODataId       = ~0u;
    mat.PackedSheenSSSDataId  = ~0u;
    mat.UnusedDataId          = ~0u;
    mat.EmissionDataId        = ~0u;
    if (index >= materialBuffer.MaterialCount)
    {
        return mat;
    }

    const uint baseByteLocation = (index * (materialBuffer.MaterialSize / 4));
    mat.AlbedoDataId          = materialBuffer.Data[baseByteLocation + 0u];
    mat.NormalDataId          = materialBuffer.Data[baseByteLocation + 1u];
    mat.PackedMRODataId       = materialBuffer.Data[baseByteLocation + 2u];
    mat.PackedSheenSSSDataId  = materialBuffer.Data[baseByteLocation + 3u];
    mat.UnusedDataId          = materialBuffer.Data[baseByteLocation + 4u];
    mat.EmissionDataId        = materialBuffer.Data[baseByteLocation + 5u];
    return mat;
}

DirectionalLightBuffer GetDirectionalLight(uint index) 
{
    DirectionalLightBuffer light;
    if (index >= directionalLightBuffer.LightCount) 
    {
        light.LightColor     = vec3(0.0);
        light.LightDirection = vec3(0.0);
        light.LightIntensity = 0.0;
        light.ShadowStrength = 0.0;
        light.ShadowBias     = 0.0;
        light.ShadowSoftness = 0.0;
        return light;
    }

    const uint baseByteLocation = (index * (directionalLightBuffer.LightSize / 4));
    light.LightColor     = vec3(uintBitsToFloat(directionalLightBuffer.Data[baseByteLocation + 0u]), uintBitsToFloat(directionalLightBuffer.Data[baseByteLocation + 1u]), uintBitsToFloat(directionalLightBuffer.Data[baseByteLocation + 2u]));
    light.LightDirection = vec3(uintBitsToFloat(directionalLightBuffer.Data[baseByteLocation + 3u]), uintBitsToFloat(directionalLightBuffer.Data[baseByteLocation + 4u]), uintBitsToFloat(directionalLightBuffer.Data[baseByteLocation + 5u]));
    light.LightIntensity = uintBitsToFloat(directionalLightBuffer.Data[baseByteLocation + 6u]);
    light.ShadowStrength = uintBitsToFloat(directionalLightBuffer.Data[baseByteLocation + 7u]);
    light.ShadowBias     = uintBitsToFloat(directionalLightBuffer.Data[baseByteLocation + 8u]);
    light.ShadowSoftness = uintBitsToFloat(directionalLightBuffer.Data[baseByteLocation + 9u]);
    return light;
}

PointLightBuffer GetPointLight(uint index) 
{
    PointLightBuffer light;
    if (index >= pointLightBuffer.LightCount) 
    {
        light.LightPosition  = vec3(960, 540, 0.0);
        light.LightColor     = vec3(0.0, 0.55, 0.9);
        light.LightRadius    = 2000.0;
        light.LightIntensity = 5.0;
        light.ShadowStrength = 0.0;
        light.ShadowBias     = 0.0;
        light.ShadowSoftness = 0.0;
        return light;
    }

    const uint baseByteLocation = (index * (pointLightBuffer.LightSize / 4));
    light.LightPosition  = vec3(uintBitsToFloat(pointLightBuffer.Data[baseByteLocation + 0u]), uintBitsToFloat(pointLightBuffer.Data[baseByteLocation + 1u]), uintBitsToFloat(pointLightBuffer.Data[baseByteLocation + 2u]));
    light.LightColor     = vec3(uintBitsToFloat(pointLightBuffer.Data[baseByteLocation + 3u]), uintBitsToFloat(pointLightBuffer.Data[baseByteLocation + 4u]), uintBitsToFloat(pointLightBuffer.Data[baseByteLocation + 5u]));
    light.LightRadius    = uintBitsToFloat(pointLightBuffer.Data[baseByteLocation + 6u]);
    light.LightIntensity = uintBitsToFloat(pointLightBuffer.Data[baseByteLocation + 7u]);
    light.ShadowStrength = uintBitsToFloat(pointLightBuffer.Data[baseByteLocation + 8u]);
    light.ShadowBias     = uintBitsToFloat(pointLightBuffer.Data[baseByteLocation + 9u]);
    light.ShadowSoftness = uintBitsToFloat(pointLightBuffer.Data[baseByteLocation + 10u]);
    return light;
}

void main()
{
    MeshProperitiesBuffer mesh = GetMesh(sceneData.MeshBufferIndex);
    MaterialProperitiesBuffer2 material = GetMaterial(mesh.MaterialIndex);

    PS_Position = vec3(mesh.MeshTransform * vec4(VS_Position.xy, 0.0f, 1.0f));
	PS_UV = VS_UV.xy;

    gl_Position = sceneData.Projection * 
                  sceneData.View *  
                  mesh.MeshTransform *
                  vec4(VS_Position.xy, 0.0f, 1.0f);
}