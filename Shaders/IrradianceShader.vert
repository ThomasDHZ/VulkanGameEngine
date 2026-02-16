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

layout(push_constant) uniform IrradianceShaderConstants 
{
    float sampleDelta;
} irradianceShaderConstants;

layout(location = 0) in vec3 aPos;
layout(location = 0) out vec3 vWorldPos;


mat4 MVP[6] = {
    {{ 0.000000,  0.000000,  1.000000,  1.000000},
     { 0.000000, -1.000000,  0.000000,  0.000000},
     {-1.000000,  0.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000, -0.100100,  0.000000}},  // +X

    {{ 0.000000,  0.000000, -1.000000, -1.000000},
     { 0.000000, -1.000000,  0.000000,  0.000000},
     { 1.000000,  0.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000, -0.100100,  0.000000}},  // -X

    {{ 1.000000,  0.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000,  1.000000,  1.000000},
     { 0.000000,  1.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000, -0.100100,  0.000000}},  // +Y

    {{ 1.000000,  0.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000, -1.000000, -1.000000},
     { 0.000000, -1.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000, -0.100100,  0.000000}},  // -Y

    {{ 1.000000,  0.000000,  0.000000,  0.000000},
     { 0.000000, -1.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000,  1.000000,  1.000000},
     { 0.000000,  0.000000, -0.100100,  0.000000}},  // +Z

    {{-1.000000,  0.000000,  0.000000,  0.000000},
     { 0.000000, -1.000000,  0.000000,  0.000000},
     { 0.000000,  0.000000, -1.000000, -1.000000},
     { 0.000000,  0.000000, -0.100100,  0.000000}}   // -Z
};

void main()
{
    vWorldPos = aPos;
    vec4 clipPos = MVP[gl_ViewIndex] * vec4(aPos * 5000, 1.0f);
    gl_Position = clipPos.xyww;
}