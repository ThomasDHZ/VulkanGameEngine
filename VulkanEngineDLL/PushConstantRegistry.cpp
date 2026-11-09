#include "PushConstantRegistry.h"
#include "ShaderSystem.h"
#include <algorithm>
#include "MemoryPoolSystem.h"
#include "RenderSystem.h"
#include "LightSystem.h"

PushConstantRegistry& pushConstantRegistry = PushConstantRegistry::Get();

void PushConstantRegistry::RegisterPushConstantValue(const String& sourceName, UpdateFunc func)
{
    _registry[sourceName] = std::move(func);
}

void PushConstantRegistry::ApplyPushConstantRules(ShaderPushConstant& pushConstant, const PushConstantContext& pushConstantContext)
{
    auto it = _registry.find(pushConstant.PushConstantName);
    if (it != _registry.end())
    {
        it->second(pushConstant, pushConstantContext);
    }
    for (auto& pushConstantVariable : pushConstantContext.PushConstantUpdateRules)
    {
        if(shaderSystem.SearchPushConstantForVariableExists(pushConstant.PushConstantName, pushConstantVariable.Variable)) shaderSystem.UpdatePushConstantValue(pushConstant, pushConstantVariable);
    }
    shaderSystem.UpdatePushConstantBuffer(pushConstant);
}