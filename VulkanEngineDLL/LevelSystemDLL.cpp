#include "LevelSystemDLL.h"

void LevelSystem_LoadLevel(const char* levelPath)
{
	levelSystem.LoadLevel(levelPath);
}

void LevelSystem_Update(const float& deltaTime)
{
	levelSystem.Update(deltaTime);
}

void LevelSystem_RenderFrameBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId)
{
	levelSystem.RenderFrameBuffer(commandBuffer, renderPassId);
}
