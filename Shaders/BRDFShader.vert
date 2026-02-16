#version 460
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_KHR_Vulkan_GLSL : enable 
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_debug_printf : enable

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl" 

layout(set = 0, binding = 0) uniform sampler2D   TextureMap[];
layout(set = 0, binding = 1) uniform samplerCube CubeMaps[];
layout(set = 0, binding = 2) buffer              ScenePropertiesBuffer 
{ 
    MeshProperitiesBuffer meshProperties[]; 
    Material material[];
    CubeMapMaterial cubeMapMaterial[];
    DirectionalLightBuffer directionalLightProperties[];
    PointLightBuffer pointLightProperties[];
} 
scenePropertiesBuffer;

layout(location = 0) out vec2 TexCoords;

void main() {
	TexCoords = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(TexCoords * 2.0f - 1.0f, 0.0f, 1.0f);
}
