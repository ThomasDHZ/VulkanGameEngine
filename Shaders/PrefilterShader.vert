#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_multiview : enable

layout (location = 0) in vec3 aPos;
layout(location = 0) out vec3 WorldPos;

layout(push_constant) uniform PrefilterSamplerProperties {
    uint CubeMapResolution;
    float Roughness;
} prefilterSamplerProperties;

// Standard 90° perspective projection (shared for all views)
const mat4 captureProjection = mat4(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, -1.0, -1.0,  // Standard OpenGL Z mapping
    0.0, 0.0, -0.1, 0.0     // Near 0.1
);

// 6 view matrices (lookAt from origin to each face)
// Order: +X, -X, +Y, -Y, +Z, -Z
const mat4 captureViews[6] = mat4[](
    mat4( 0.0,  0.0, -1.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
         -1.0,  0.0,  0.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // +X

    mat4( 0.0,  0.0,  1.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
          1.0,  0.0,  0.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // -X

    mat4( 1.0,  0.0,  0.0, 0.0,
          0.0,  0.0, -1.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // +Y

    mat4( 1.0,  0.0,  0.0, 0.0,
          0.0,  0.0,  1.0, 0.0,
          0.0,  1.0,  0.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // -Y

    mat4( 1.0,  0.0,  0.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
          0.0,  0.0, -1.0, 0.0,
          0.0,  0.0,  0.0, 1.0),  // +Z

    mat4(-1.0,  0.0,  0.0, 0.0,
          0.0, -1.0,  0.0, 0.0,
          0.0,  0.0,  1.0, 0.0,
          0.0,  0.0,  0.0, 1.0)   // -Z
);

void main()
{
    WorldPos = aPos;

    // Full MVP = projection * view[gl_ViewIndex]
    mat4 view = captureViews[gl_ViewIndex];
    vec4 pos = captureProjection * view * vec4(aPos, 1.0);

    gl_Position = pos.xyww;  // Force depth = w for cubemap projection
}