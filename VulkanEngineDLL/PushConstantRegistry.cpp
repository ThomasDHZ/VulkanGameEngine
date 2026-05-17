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
}

void PushConstantRegistry::RegisterDefaultPushConstantRules()
{
    RegisterPushConstantValue("sceneData", [&](ShaderPushConstant& pushConstant, const PushConstantContext& context)
        {
            VulkanRenderPass renderPass = renderSystem.FindRenderPass(context.RenderPassGuid);
            SceneDataBuffer& sceneDataBuffer = memoryPoolSystem.UpdateSceneDataBuffer();
            sceneDataBuffer.InvertResolution = vec2(1.0f / static_cast<float>(renderPass.RenderPassResolution.x), 1.0f / static_cast<float>(renderPass.RenderPassResolution.y));

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
}