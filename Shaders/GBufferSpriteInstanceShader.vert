
#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"

layout (location = 0)  in vec2  VS_SpritePosition;
layout (location = 1)  in vec4  VS_UVOffset; // vec4(vec2(StartUV.x, StartUV.y), vec2(UVEnd.x, UVEnd.y))
layout (location = 2)  in vec2  VS_SpriteSize;
layout (location = 3)  in ivec2 VS_FlipSprite;
layout (location = 4)  in vec4  VS_Color;
layout (location = 5)  in mat4  VS_InstanceTransform;
layout (location = 9)  in uint  VS_MaterialID;

layout (location = 0) out vec3  PS_Position;
layout (location = 1) out vec2  PS_UV;
layout (location = 2) out vec2  PS_SpriteSize;
layout (location = 3) out ivec2 PS_FlipSprite;
layout (location = 4) out vec4  PS_Color;
layout (location = 5) out uint  PS_MaterialID;
layout (location = 6) out vec4  PS_UVOffset;

layout(constant_id = 0) const uint VertexAttributeLocation0 = 0;
layout(constant_id = 1) const uint VertexInputRateLocation0 = 1;
layout(constant_id = 2) const uint VertexAttributeLocation1 = 0;
layout(constant_id = 3) const uint VertexInputRateLocation1 = 1;
layout(constant_id = 4) const uint VertexAttributeLocation2 = 0;
layout(constant_id = 5) const uint VertexInputRateLocation2 = 1;
layout(constant_id = 6) const uint VertexAttributeLocation3 = 0;
layout(constant_id = 7) const uint VertexInputRateLocation3 = 1;
layout(constant_id = 8) const uint VertexAttributeLocation4 = 0;
layout(constant_id = 9) const uint VertexInputRateLocation4 = 1;
layout(constant_id = 10) const uint VertexAttributeLocation5 = 0;
layout(constant_id = 11) const uint VertexInputRateLocation5 = 1;
layout(constant_id = 12) const uint VertexAttributeLocation9 = 0;
layout(constant_id = 13) const uint VertexInputRateLocation9 = 1;

layout(constant_id = 14)  const uint DescriptorBindingType0   = SubpassInputDescriptor;
layout(constant_id = 15)  const uint DescriptorBindingType1   = SubpassInputDescriptor;
layout(constant_id = 16)  const uint DescriptorBindingType2   = SubpassInputDescriptor;
layout(constant_id = 17)  const uint DescriptorBindingType3   = SubpassInputDescriptor;
layout(constant_id = 18)  const uint DescriptorBindingType4   = SubpassInputDescriptor;
layout(constant_id = 19)  const uint DescriptorBindingType5   = SubpassInputDescriptor;
layout(constant_id = 20)  const uint DescriptorBindingType6   = SubpassInputDescriptor;
layout(constant_id = 21)  const uint DescriptorBindingType7   = SubpassInputDescriptor;
layout(constant_id = 22)  const uint DescriptorBindingType8   = MeshPropertiesDescriptor;
layout(constant_id = 23)  const uint DescriptorBindingType9   = MaterialDescriptor;
layout(constant_id = 24)  const uint DescriptorBindingType10  = DirectionalLightDescriptor;
layout(constant_id = 25)  const uint DescriptorBindingType11  = PointLightDescriptor;
layout(constant_id = 26)  const uint DescriptorBindingType12  = TextureDescriptor;
layout(constant_id = 27)  const uint DescriptorBindingType13  = SkyBoxDescriptor;
layout(constant_id = 28)  const uint DescriptorBindingType14  = IrradianceCubeMapDescriptor;
layout(constant_id = 29)  const uint DescriptorBindingType15  = PrefilterDescriptor;

layout(binding = 8)  buffer MeshProperities { MeshProperitiesBuffer meshProperties; } meshBuffer[];
layout(binding = 9)  buffer MaterialProperities { MaterialProperitiesBuffer materialProperties; } materialBuffer[];
layout(binding = 10)  buffer DirectionalLight { DirectionalLightBuffer directionalLightProperties; } directionalLightBuffer[];
layout(binding = 11)  buffer PointLight { PointLightBuffer pointLightProperties; } pointLightBuffer[];
layout(binding = 12) uniform sampler2D TextureMap[];
layout(binding = 13) uniform samplerCube CubeMap;
layout(binding = 14) uniform samplerCube IrradianceMap;
layout(binding = 15) uniform samplerCube PrefilterMap;

layout(push_constant) uniform SceneDataBuffer 
{
    int MeshBufferIndex;
    mat4 Projection;
    mat4 View;
          vec3  ViewDirection;
    vec3 CameraPosition;
    int   UseHeightMap;
    float HeightScale;
} sceneData;

struct Vertex2D
{
	vec2 Position;
	vec2 UV;
};

void main() 
{
    Vertex2D vertex = Vertex2D(vec2(0.0f), vec2(0.0f));
    switch(gl_VertexIndex) 
	{
        case 0: vertex = Vertex2D(vec2(VS_SpritePosition.x                  , VS_SpritePosition.y + VS_SpriteSize.y), vec2(VS_UVOffset.x                , VS_UVOffset.y                )); break; 
        case 1: vertex = Vertex2D(vec2(VS_SpritePosition.x + VS_SpriteSize.x, VS_SpritePosition.y + VS_SpriteSize.y), vec2(VS_UVOffset.x + VS_UVOffset.z, VS_UVOffset.y                )); break;
        case 2: vertex = Vertex2D(vec2(VS_SpritePosition.x + VS_SpriteSize.x, VS_SpritePosition.y                  ), vec2(VS_UVOffset.x + VS_UVOffset.z, VS_UVOffset.y + VS_UVOffset.w)); break;
        case 3: vertex = Vertex2D(vec2(VS_SpritePosition.x                  , VS_SpritePosition.y                  ), vec2(VS_UVOffset.x			    , VS_UVOffset.y + VS_UVOffset.w)); break;
    }

    PS_Position = vec3(VS_InstanceTransform * vec4(vertex.Position.xy, 0.0f, 1.0f));
	PS_UV = vertex.UV;
    PS_SpriteSize = VS_SpriteSize;
	PS_FlipSprite = VS_FlipSprite;
	PS_Color = VS_Color;
	PS_MaterialID = VS_MaterialID;
	PS_UVOffset = VS_UVOffset;

    gl_Position = sceneData.Projection * 
                  sceneData.View *  
                  VS_InstanceTransform *
                  vec4(vertex.Position.xy, 0.0f, 1.0f);
}