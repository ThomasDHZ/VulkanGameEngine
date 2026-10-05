#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_gpu_shader_int64 : require

#include "Lights.glsl"
#include "Constants.glsl"
#include "MeshPropertiesBuffer.glsl"
#include "MaterialPropertiesBuffer.glsl"
#include "MemoryPoolBindings.glsl"

layout(set = 1, binding = 0, input_attachment_index = 0) uniform subpassInput PositionInput;
layout(set = 1, binding = 1, input_attachment_index = 1) uniform subpassInput AlbedoInput;
layout(set = 1, binding = 2, input_attachment_index = 2) uniform subpassInput NormalDataInput;
layout(set = 1, binding = 3, input_attachment_index = 3) uniform subpassInput MROInput;
layout(set = 1, binding = 4, input_attachment_index = 4) uniform subpassInput FeatureAInput;
layout(set = 1, binding = 5, input_attachment_index = 5) uniform subpassInput FeatureBInput;
layout(set = 1, binding = 6, input_attachment_index = 6) uniform subpassInput FeatureCInput;
layout(set = 1, binding = 7, input_attachment_index = 7) uniform subpassInput EmissionInput;
layout(set = 1, binding = 8, input_attachment_index = 8) uniform subpassInput depthInput;

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outBloom;

#include "BindlessHelpers.glsl"
#include "LightingEvaluation.glsl"

Material UnpackMaterial();

void main()
{
    Material material = UnpackMaterial();
    if (material.Depth >= 0.9999)
    {
        vec3 ndc = vec3(TexCoords * 2.0 - 1.0, 1.0);
        vec4 viewPos = sceneDataBuffer.InversePerspectiveProjection * vec4(ndc, 1.0);
        viewPos /= viewPos.w;
        vec3 worldDir = normalize((sceneDataBuffer.InversePerspectiveView * vec4(normalize(viewPos.xyz), 0.0)).xyz);
        outColor = vec4(textureLod(CubeMap[sceneDataBuffer.CubeMapId], worldDir, 0.0).rgb, 1.0);
        outBloom = vec4(0.0);
        return;
    }
    if ((material.FeatureMask & FEAT_ALBEDO_ONLY) != 0u)
    {
        outColor = vec4(material.Albedo, 1.0);
        outBloom = vec4(0.0);
        return;
    }

    vec3 N = material.Normal;
    vec3 V = PerspectiveView(material.Position);
    if ((material.FeatureMask & FEAT_TWO_SIDED) != 0u && dot(N, V) < 0.0) N = -N;
    material.Normal = N;

    vec3 R = reflect(-V, N);
    float F0d = pow((material.IOR - 1.0) / (material.IOR + 1.0), 2.0);
    vec3  F0  = mix(vec3(F0d), material.Albedo, material.Metallic);

    vec3 lit   = DirectionalLightFunc(F0, V, material);
    vec3 ibl   = ImageBasedLighting(F0, V, N, R, material);
    vec3 color = ibl + lit + material.Emission;

    outColor = vec4(color, 1.0);
    outBloom = vec4(material.Emission + max(lit + ibl - vec3(1.0), vec3(0.0)), 1.0);
}

Material UnpackMaterial()
{
    vec4 positionAttachment = subpassLoad(PositionInput);
    vec4 albedoAttachment   = subpassLoad(AlbedoInput);
    vec4 normalAttachment   = subpassLoad(NormalDataInput);
    vec4 mroAttachment      = subpassLoad(MROInput);
    vec4 featureAAttachment = subpassLoad(FeatureAInput);
    vec4 featureBAttachment = subpassLoad(FeatureBInput);
    vec4 featureCAttachment = subpassLoad(FeatureCInput);
    vec4 emissionAttachment = subpassLoad(EmissionInput);
    vec4 depthAttachment    = subpassLoad(depthInput);

    vec2 sssW_coatW    = Unpack8bitPair(featureAAttachment.a);
    vec2 thick_coatR   = Unpack8bitPair(featureBAttachment.a);
    vec2 aniso         = Unpack8bitPair(featureCAttachment.r);
    vec2 film          = Unpack8bitPair(featureCAttachment.g);
    vec2 coatD_prof    = Unpack8bitPair(featureCAttachment.b);
    vec2 sheenR_sheenW = Unpack8bitPair(featureCAttachment.a);

    Material m;
    m.Depth            = depthAttachment.r;
    m.Position         = positionAttachment.xyz;
    m.Albedo           = albedoAttachment.rgb;
    m.Metallic         = mroAttachment.r;
    m.Roughness        = mroAttachment.g;
    m.AmbientOcclusion = mroAttachment.b;
    m.IOR              = mroAttachment.a * 2.0 + 1.0;
    m.Normal           = normalize(OctahedronDecode(normalAttachment.xy * 2.0 - 1.0));
    m.Emission         = emissionAttachment.rgb;

    m.SheenColor     = featureAAttachment.rgb;
    m.SheenRoughness = sheenR_sheenW.x;
    m.SheenWeight    = sheenR_sheenW.y;
    m.SSSColor       = featureBAttachment.rgb;
    m.SSSWeight      = sssW_coatW.x;
    m.SSSProfile     = coatD_prof.y;
    m.Thickness      = thick_coatR.x;
    m.CoatColor      = vec3(1.0);
    m.CoatWeight     = sssW_coatW.y;
    m.CoatRoughness  = thick_coatR.y;
    m.CoatDarkening  = coatD_prof.x;
    m.Anisotropy         = aniso.x;
    m.AnisotropyRotation = aniso.y;
    m.ThinFilmWeight     = film.x;
    m.ThinFilmThickness  = film.y;
    m.FeatureMask = uint(normalAttachment.b * 65535.0 + 0.5);
    m.SelfShadow  = normalAttachment.a;
    m.ShadingModel = 0u;
    return m;
}