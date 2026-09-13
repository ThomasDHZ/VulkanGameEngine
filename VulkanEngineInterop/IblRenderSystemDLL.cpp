#include "IblRenderSystemDLL.h"
#include <LevelSystem.h>
#include "LevelSystemDLL.h"

void IblRenderSystem_StartUp(const char* texturePath)
{
	iblRenderSystem.StartUp(texturePath);
}

RenderPassNodeDLL* IblRenderSystem_CreateDrawCommands(VkCommandBuffer& commandBuffer, const float& deltaTime, size_t* renderPassNodeCount)
{
    return LevelSystem_CreateDrawCommands(commandBuffer, deltaTime, renderPassNodeCount);
}

void IblRenderSystem_SetEnvironmentMap(const char* texturePath)
{
	iblRenderSystem.SetEnvironmentMap(texturePath);
}
