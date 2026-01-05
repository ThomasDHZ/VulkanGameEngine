#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_debug_printf : enable

layout(constant_id = 0) const uint DescriptorBindingType0 = SkyBoxDescriptor;
layout(binding = 0) uniform samplerCube CubeMap;

layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 outColor;

layout(push_constant) PrefilterSamplerProperties
{
    uint Resolution;
    float Roughness;
}PrefilterSamplerProperties;

void main()
{		
    vec3 N = normalize(WorldPos);
    
    // make the simplifying assumption that V equals R equals the normal 
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    vec3 prefilteredColor = vec3(0.0f);
    float totalWeight = 0.0f;
    
    for(uint x = 0u; x < SAMPLE_COUNT; ++x)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, PrefilterSamplerProperties.Roughness);
        vec3 L  = normalize(2.0f * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0f);
        if(NdotL > 0.0)
        {
            float D   = DistributionGGX(N, H, PrefilterSamplerProperties.Roughness);
            float NdotH = max(dot(N, H), 0.0f);
            float HdotV = max(dot(H, V), 0.0f);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001f; 

            float resolution = 512.0f;
            float saTexel  = 4.0f * PI / (6.0f * PrefilterSamplerProperties.Resolution * PrefilterSamplerProperties.Resolution);
            float saSample = 1.0f / (float(SAMPLE_COUNT) * pdf + 0.0001f);

            float mipLevel = PrefilterSamplerProperties.Roughness == 0.0f ? 0.0f : 0.5f * log2(saSample / saTexel); 
            
            prefilteredColor += textureLod(CubeMap, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;

    FragColor = vec4(prefilteredColor, 1.0);
}
