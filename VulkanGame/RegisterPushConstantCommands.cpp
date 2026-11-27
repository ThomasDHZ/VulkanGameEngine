#include "RegisterPushConstantCommands.h"
#include <MemoryPoolSystem.h>

void RegisterPushConstantCommands()
{
    pushConstantRegistry.RegisterPushConstantValue("sceneDataPushConstant", [](ShaderPushConstant& pushConstant, const PushConstantContext& context)
        {
            VulkanRenderPass renderPass = renderSystem.FindRenderPass(context.RenderPassGuid);
            SceneDataBuffer& sceneDataBuffer = memoryPoolSystem.UpdateSceneDataBuffer();
            sceneDataBuffer.InvertResolution = vec2(1.0f / static_cast<float>(renderPass.RenderPassResolution().x), 1.0f / static_cast<float>(renderPass.RenderPassResolution().y));

            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "MeshBufferIndex", context.MeshId);
            shaderSystem.UpdatePushConstantValue<float>(pushConstant, "LevelHeightScale", context.LevelHeightScale);
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "SceneDataBufferPtr", context.SceneDataBufferPtr);
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "MeshPropertiesBufferPtr", context.MeshPropertiesBufferPtr);
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "LightPropertiesBufferPtr", context.LightPropertiesBufferPtr);
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "TextureHeaderBufferPtr", context.TextureHeaderBufferPtr);
            shaderSystem.UpdatePushConstantBuffer(pushConstant);
        });

    pushConstantRegistry.RegisterPushConstantValue("irradianceShaderConstants", [](ShaderPushConstant& pushConstant, const PushConstantContext& context)
        {
            shaderSystem.UpdatePushConstantValue<float>(pushConstant, "sampleDelta", 0.1f);
            shaderSystem.UpdatePushConstantBuffer(pushConstant);
        });

    pushConstantRegistry.RegisterPushConstantValue("prefilterSamplerProperties", [](ShaderPushConstant& pushConstant, const PushConstantContext& context)
        {
            uint32 width = std::max(1, context.RenderPassResolution.x >> context.MipLevel);
            float roughness = static_cast<float>(context.MipLevel) / static_cast<float>(context.MipCount - 1);

            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "CubeMapResolution", context.RenderPassResolution.x);
            shaderSystem.UpdatePushConstantValue<float>(pushConstant, "Roughness", roughness);
            shaderSystem.UpdatePushConstantBuffer(pushConstant);
        });

    pushConstantRegistry.RegisterPushConstantValue("directionalLightSceneDataDepthBuffer", [](ShaderPushConstant& pushConstant, const PushConstantContext& context)
        {
            shaderSystem.UpdatePushConstantValue<int>(pushConstant, "MeshBufferIndex", context.MeshId);
            shaderSystem.UpdatePushConstantValue<int>(pushConstant, "DirectionalLightBufferIndex", context.LightId);
            shaderSystem.UpdatePushConstantValue<float>(pushConstant, "LevelHeightScale", context.LevelHeightScale);
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "SceneDataBufferPtr", context.SceneDataBufferPtr);
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "MeshPropertiesBufferPtr", context.MeshPropertiesBufferPtr);
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "LightPropertiesBufferPtr", context.LightPropertiesBufferPtr);
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "TextureHeaderBufferPtr", context.TextureHeaderBufferPtr);
            shaderSystem.UpdatePushConstantBuffer(pushConstant);
        });

    pushConstantRegistry.RegisterPushConstantValue("pointLightSceneDataDepthBuffer", [](ShaderPushConstant& pushConstant, const PushConstantContext& context)
        {
            shaderSystem.UpdatePushConstantValue<int>(pushConstant, "MeshBufferIndex", context.MeshId);
            shaderSystem.UpdatePushConstantValue<int>(pushConstant, "PointLightBufferIndex", context.LightId);
            shaderSystem.UpdatePushConstantBuffer(pushConstant);
        });


    pushConstantRegistry.RegisterPushConstantValue("lightingPushConstant", [](ShaderPushConstant& pushConstant, const PushConstantContext& context)
        {
            // shaderSystem.UpdatePushConstantValue<int>(pushConstant, "PointLightShadowMapCellSize", context.LightViewIndex);
            shaderSystem.UpdatePushConstantValue<int>(pushConstant, "PointLightViewIndex", context.LightId);
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "SceneDataBufferPtr", context.SceneDataBufferPtr);
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "MeshPropertiesBufferPtr", context.MeshPropertiesBufferPtr);
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "LightPropertiesBufferPtr", context.LightPropertiesBufferPtr);
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "TextureHeaderBufferPtr", context.TextureHeaderBufferPtr);
            shaderSystem.UpdatePushConstantBuffer(pushConstant);
        });

    pushConstantRegistry.RegisterPushConstantValue("materialBaker", [](ShaderPushConstant& pushConstant, const PushConstantContext& context)
        {
            shaderSystem.UpdatePushConstantValue<uint>(pushConstant, "MaterialBakerSubPassIndex", context.DrawIndex);
            shaderSystem.UpdatePushConstantBuffer(pushConstant);
        });
}
