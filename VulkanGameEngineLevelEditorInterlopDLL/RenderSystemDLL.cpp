#include "RenderSystemDLL.h"

 RenderPassGuid RenderSystem_LoadRenderPass(LevelGuid& levelGuid, const char* jsonPath, bool useGlobalDescriptorSet)
{
	 return  renderSystem.LoadRenderPass(levelGuid, jsonPath, useGlobalDescriptorSet);
}

 RenderPassGuid RenderSystem_LoadRenderPass2(LevelGuid& levelGuid, RenderPassLoader& renderPassLoader, bool useGlobalDescriptorSet)
{
	 return  renderSystem.LoadRenderPass(levelGuid, renderPassLoader, useGlobalDescriptorSet);
}

 void RenderSystem_RebuildSwapChain(VulkanRenderPass& vulkanRenderPass)
{
	 renderSystem.RebuildSwapChain(vulkanRenderPass);
}

 void RenderSystem_Update(void* windowHandle, LevelGuid& levelGuid, const float& deltaTime)
{
	 renderSystem.Update(windowHandle, levelGuid, deltaTime);
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
