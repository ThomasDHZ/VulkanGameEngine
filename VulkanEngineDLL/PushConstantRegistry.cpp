#include "PushConstantRegistry.h"
#include "ShaderSystem.h"
#include <algorithm>
#include "MemoryPoolSystem.h"
#include "RenderSystem.h"

PushConstantRegistry& pushConstantRegistry = PushConstantRegistry::Get();

void PushConstantRegistry::RegisterPushConstantValue(const String& sourceName, UpdateFunc func)
{
	registry[sourceName] = std::move(func);
}

void PushConstantRegistry::ApplyPushConstantRules(ShaderPushConstant& pushConstant, const PushConstantContext& pushConstantContext)
{

    auto it = registry.find(pushConstant.PushConstantName);
    if (it != registry.end())
    {
        it->second(pushConstant, pushConstantContext);
    }
    for (auto& pushConstantVariable : pushConstantContext.PushConstantUpdateRules)
    {
        if(shaderSystem.SearchPushConstantForVariableExists(pushConstant.PushConstantName, pushConstantVariable.Variable))
        {
            switch (pushConstantVariable.VariableType)
            {
                case kShaderMember_Int:   shaderSystem.UpdatePushConstantValue<int>(pushConstant,   pushConstantVariable.Variable, shaderSystem.GetPushConstantValue<int>(pushConstantVariable));   break;
                case kShaderMember_Uint:  shaderSystem.UpdatePushConstantValue<uint>(pushConstant,  pushConstantVariable.Variable, shaderSystem.GetPushConstantValue<uint>(pushConstantVariable));  break;
                case kShaderMember_Float: shaderSystem.UpdatePushConstantValue<float>(pushConstant, pushConstantVariable.Variable, shaderSystem.GetPushConstantValue<float>(pushConstantVariable)); break;
                case kShaderMember_Ivec2: shaderSystem.UpdatePushConstantValue<ivec2>(pushConstant, pushConstantVariable.Variable, shaderSystem.GetPushConstantValue<ivec2>(pushConstantVariable)); break;
                case kShaderMember_Ivec3: shaderSystem.UpdatePushConstantValue<ivec3>(pushConstant, pushConstantVariable.Variable, shaderSystem.GetPushConstantValue<ivec3>(pushConstantVariable)); break;
                case kShaderMember_Ivec4: shaderSystem.UpdatePushConstantValue<ivec4>(pushConstant, pushConstantVariable.Variable, shaderSystem.GetPushConstantValue<ivec4>(pushConstantVariable)); break;
                case kShaderMember_Vec2:  shaderSystem.UpdatePushConstantValue<vec2>(pushConstant,  pushConstantVariable.Variable, shaderSystem.GetPushConstantValue<vec2>(pushConstantVariable));  break;
                case kShaderMember_Vec3:  shaderSystem.UpdatePushConstantValue<vec3>(pushConstant,  pushConstantVariable.Variable, shaderSystem.GetPushConstantValue<vec3>(pushConstantVariable));  break;
                case kShaderMember_Vec4:  shaderSystem.UpdatePushConstantValue<vec4>(pushConstant,  pushConstantVariable.Variable, shaderSystem.GetPushConstantValue<vec4>(pushConstantVariable));  break;
                case kShaderMember_Mat2:  shaderSystem.UpdatePushConstantValue<mat2>(pushConstant,  pushConstantVariable.Variable, shaderSystem.GetPushConstantValue<mat2>(pushConstantVariable));  break;
                case kShaderMember_Mat3:  shaderSystem.UpdatePushConstantValue<mat3>(pushConstant,  pushConstantVariable.Variable, shaderSystem.GetPushConstantValue<mat3>(pushConstantVariable));  break;
                case kShaderMember_Mat4:  shaderSystem.UpdatePushConstantValue<mat4>(pushConstant,  pushConstantVariable.Variable, shaderSystem.GetPushConstantValue<mat4>(pushConstantVariable));  break;
                case kShaderMember_bool:  shaderSystem.UpdatePushConstantValue<bool>(pushConstant,  pushConstantVariable.Variable, shaderSystem.GetPushConstantValue<bool>(pushConstantVariable));  break;
            }
        }
    }
    shaderSystem.UpdatePushConstantBuffer(pushConstant);
}

void PushConstantRegistry::RegisterDefaultPushConstantRules()
{
    RegisterPushConstantValue("sceneData", [&](ShaderPushConstant& pushConstant, const PushConstantContext& context)
        {
            VulkanRenderPass renderPass = renderSystem.FindRenderPass(context.RenderPassGuid);
            SceneDataBuffer& sceneDataBuffer = memoryPoolSystem.UpdateSceneDataBuffer();
            sceneDataBuffer.InvertResolution = vec2(1.0f / static_cast<float>(renderPass.RenderPassResolution().x), 1.0f / static_cast<float>(renderPass.RenderPassResolution().y));

            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "MeshBufferIndex", context.MeshId);
            shaderSystem.UpdatePushConstantBuffer(pushConstant);
        });

    RegisterPushConstantValue("irradianceShaderConstants", [&](ShaderPushConstant& pushConstant, const PushConstantContext& context)
        {
            shaderSystem.UpdatePushConstantValue<float>(pushConstant, "sampleDelta", 0.1f);
            shaderSystem.UpdatePushConstantBuffer(pushConstant);
        });

    RegisterPushConstantValue("prefilterSamplerProperties", [&](ShaderPushConstant& pushConstant, const PushConstantContext& context)
        {
            uint32 width = std::max(1, context.RenderPassResolution.x >> context.MipLevel);
            float roughness = static_cast<float>(context.MipLevel) / static_cast<float>(context.MipCount - 1);

            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "CubeMapResolution", context.RenderPassResolution.x);
            shaderSystem.UpdatePushConstantValue<float>(pushConstant, "Roughness", roughness);
            shaderSystem.UpdatePushConstantBuffer(pushConstant);
        });

    pushConstantRegistry.RegisterPushConstantValue("materialBaker", [&](ShaderPushConstant& pushConstant, const PushConstantContext& context)
        {
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "MaterialBakerSubPassIndex", context.DrawIndex);
            shaderSystem.UpdatePushConstantBuffer(pushConstant);
        });
}