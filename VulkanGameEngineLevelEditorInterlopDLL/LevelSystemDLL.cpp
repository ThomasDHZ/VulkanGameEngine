#include "LevelSystemDLL.h"

void LevelSystem_LoadLevel(const char* levelPath)
{
	levelSystem.LoadLevel(levelPath);
}

void LevelSystem_Update(const float& deltaTime)
{
	levelSystem.Update(deltaTime);
}

void LevelSystem_RenderIrradianceMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime)
{
	levelSystem.RenderIrradianceMapRenderPass(commandBuffer, renderPassId, deltaTime);
}

void LevelSystem_RenderPrefilterMapRenderPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, float deltaTime)
{
	levelSystem.RenderPrefilterMapRenderPass(commandBuffer, renderPassId, deltaTime);
}

void LevelSystem_RenderGBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, VkGuid& levelId, const float deltaTime)
{
	levelSystem.RenderGBuffer(commandBuffer, renderPassId, levelId, deltaTime);
}

void LevelSystem_RenderGaussianBlurPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId, uint blurDirection)
{
	levelSystem.RenderGaussianBlurPass(commandBuffer, renderPassId, blurDirection);
}

void LevelSystem_RenderBloomPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId)
{
	levelSystem.RenderBloomPass(commandBuffer, renderPassId);
}

void LevelSystem_RenderHdrPass(VkCommandBuffer& commandBuffer, VkGuid& renderPassId)
{
	levelSystem.RenderHdrPass(commandBuffer, renderPassId);
}

void LevelSystem_RenderFrameBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId)
{
	levelSystem.RenderFrameBuffer(commandBuffer, renderPassId);
}

void LevelSystem_Draw(VkCommandBuffer& commandBuffer, const float& deltaTime)
{
	levelSystem.Draw(commandBuffer, deltaTime);
}
