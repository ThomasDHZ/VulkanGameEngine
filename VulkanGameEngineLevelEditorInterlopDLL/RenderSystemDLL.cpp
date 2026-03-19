#include "RenderSystemDLL.h"

 RenderPassGuid RenderSystem_LoadRenderPass(LevelGuid& levelGuid, const char* jsonPath)
{
	 return  renderSystem.LoadRenderPass(levelGuid, jsonPath);
}

 void RenderSystem_RebuildSwapChain(VulkanRenderPass& vulkanRenderPass)
{
	 renderSystem.RebuildSwapChain(vulkanRenderPass);
}

 void RenderSystem_Update(void* windowHandle, const float& deltaTime)
{
	 renderSystem.Update(windowHandle, deltaTime);
}

 VulkanRenderPass RenderSystem_FindRenderPass(const RenderPassGuid& renderPassGuid)
{
	 return renderSystem.FindRenderPass(renderPassGuid);
}

 void RenderSystem_Destroy()
{
	 renderSystem.Destroy();
}

 void RenderSystem_DestroyRenderPass(VulkanRenderPass& renderPass)
{
	 renderSystem.DestroyRenderPass(renderPass);
}

 void RenderSystem_DestroyRenderPasses()
{
	 renderSystem.DestroyRenderPasses();
}

 void RenderSystem_DestroyRenderPipelines()
{
	 renderSystem.DestroyRenderPipelines();
}

 void RenderSystem_DestroyPipeline(VulkanPipeline& vulkanPipelineDLL)
{
	 renderSystem.DestroyPipeline(vulkanPipelineDLL);
}

 void RenderSystem_DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList)
{
	 renderSystem.DestroyFrameBuffers(frameBufferList);
}

 void RenderSystem_DestroyCommandBuffers(VkCommandBuffer& commandBuffer)
{
	 renderSystem.DestroyCommandBuffers(commandBuffer);
}

 void RenderSystem_DestroyBuffer(VkBuffer& buffer)
{
	 renderSystem.DestroyBuffer(buffer);
}
