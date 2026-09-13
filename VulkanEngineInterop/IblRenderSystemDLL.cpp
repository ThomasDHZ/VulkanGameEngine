#include "IblRenderSystemDLL.h"
#include <LevelSystem.h>

void IblRenderSystem_StartUp(const char* texturePath)
{
	iblRenderSystem.StartUp(texturePath);
}

RenderPassNodeDLL* IblRenderSystem_CreateDrawCommands(VkCommandBuffer& commandBuffer, const float& deltaTime, size_t* renderPassNodeCount)
{
    Vector<RenderPassNode> renderPassNodeList = levelSystem.CreateDrawCommands(commandBuffer, deltaTime);
    if (renderPassNodeList.empty())
    {
        if (renderPassNodeCount) *renderPassNodeCount = 0;
        return nullptr;
    }
    return ToDLL_RenderPassNodeDLL(renderPassNodeList, commandBuffer, deltaTime, renderPassNodeCount);
}

void IblRenderSystem_SetEnvironmentMap(const char* texturePath)
{
	iblRenderSystem.SetEnvironmentMap(texturePath);
}
