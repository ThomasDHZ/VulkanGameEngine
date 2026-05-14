#include "PushConstantRegistry.h"
#include "ShaderSystem.h"
#include <algorithm>

PushConstantRegistry& pushConstantRegistry = PushConstantRegistry::Get();

void PushConstantRegistry::RegisterPushConstantValue(const PushConstantResolverEnum& sourceName, UpdateFunc func)
{
	registry[sourceName] = std::move(func);
}

void PushConstantRegistry::ApplyPushConstantRules(ShaderPushConstant& pushConstant, const PushConstantContext& pushConstantContext, const Vector<PushConstantUpdateRule>& pushConstantRuleList)
{
	for (const auto& pushConstantRule : pushConstantRuleList)
	{
		auto it = registry.find(pushConstantRule.SourceId);
		if (it != registry.end())
		{
            if(it->second) it->second(pushConstant, pushConstantContext, pushConstantRule.Variable);
            else std::cerr << "Push Constant variable not handled: " << pushConstantRule.Variable << "\n";
		}
        else HandleSimplePushConstant(pushConstant, pushConstantRule);
	}
    shaderSystem.UpdatePushConstantBuffer(pushConstant);
}

void PushConstantRegistry::RegisterDefaultPushConstantRules()
{
    RegisterPushConstantValue(kPushConst_MeshId, [&](ShaderPushConstant& pushConstant, const PushConstantContext& context, const String& shaderVariable)
        {
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, shaderVariable, context.MeshId);
        });

    RegisterPushConstantValue(kPushConst_RenderPassResolution, [&](ShaderPushConstant& pushConstant, const PushConstantContext& context, const String& shaderVariable)
        {
            uint32 width = std::max(1, context.RenderPassResolution.x >> context.MipLevel);
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, shaderVariable, width);
        });
}

void PushConstantRegistry::HandleSimplePushConstant(ShaderPushConstant& pushConstant, const PushConstantUpdateRule& pushConstantRule)
{
    switch (pushConstantRule.SourceId)
    {
        case shaderInt:   UpdatePushConstantVector<1,    int>  (pushConstant, pushConstantRule); break;
        case shaderUint:  UpdatePushConstantVector<1,    uint> (pushConstant, pushConstantRule); break;
        case shaderFloat: UpdatePushConstantVector<1,    float>(pushConstant, pushConstantRule); break;
        case shaderIvec2: UpdatePushConstantVector<2,    int>  (pushConstant, pushConstantRule); break;
        case shaderIvec3: UpdatePushConstantVector<2,    int>  (pushConstant, pushConstantRule); break;
        case shaderIvec4: UpdatePushConstantVector<4,    int>  (pushConstant, pushConstantRule); break;
        case shaderVec2:  UpdatePushConstantVector<2,    float>(pushConstant, pushConstantRule); break;
        case shaderVec3:  UpdatePushConstantVector<3,    float>(pushConstant, pushConstantRule); break;
        case shaderVec4:  UpdatePushConstantVector<4,    float>(pushConstant, pushConstantRule); break;
        case shaderMat2:  UpdatePushConstantMatrix<2, 2, float>(pushConstant, pushConstantRule); break;
        case shaderMat3:  UpdatePushConstantMatrix<3, 3, float>(pushConstant, pushConstantRule); break;
        case shaderMat4:  UpdatePushConstantMatrix<4, 4, float>(pushConstant, pushConstantRule); break;
        case shaderbool:  
        {
            bool value = false;
            if (pushConstantRule.Value == "true" || pushConstantRule.Value == "1") value = true;
            else if (pushConstantRule.Value == "false" || pushConstantRule.Value == "0") value = false;
            else std::cerr << "Failed to parse bool: " << pushConstantRule.Value << "\n";

            shaderSystem.UpdatePushConstantValue<bool>(pushConstant, pushConstantRule.Variable, value);
            break;
        }
        default: std::cerr << "Unsupported push constant type for: " << pushConstantRule.Variable << std::endl; break;
    }
}