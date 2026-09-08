#include "IblRenderSystem.h"
#include "RenderSystem.h"

IblRenderSystem& iblRenderSystem = IblRenderSystem::Get();

void IblRenderSystem::StartUp()
{
    _brdfRenderPassId = renderSystem.LoadRenderPass("RenderPass/BRDFRenderPass.json");
    _environmentToCubeMapRenderPassId = renderSystem.LoadRenderPass("RenderPass/EnvironmentToCubeMapRenderPass.json");
    _irradianceMapRenderPassId = renderSystem.LoadRenderPass("RenderPass/IrradianceRenderPass.json");
    _prefilterMapRenderPassId = renderSystem.LoadRenderPass("RenderPass/PrefilterRenderPass.json");
    textureSystem.GenerateTexture(_brdfRenderPassId);
}

Vector<RenderPassNode> IblRenderSystem::CreateDrawCommands(VkCommandBuffer& commandBuffer, const float& deltaTime)
{
    return Vector<RenderPassNode>();
}

void IblRenderSystem::SetEnvironment(VkGuid environmentMapGuid)
{
    textureSystem.GenerateTexture(_environmentToCubeMapRenderPassId);
}
