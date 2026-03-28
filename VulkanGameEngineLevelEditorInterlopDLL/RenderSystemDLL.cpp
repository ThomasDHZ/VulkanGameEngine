#include "RenderSystemDLL.h"
#include "../VulkanGameEngine/GameSystem.h"

 RenderPassGuid RenderSystem_LoadRenderPass(LevelGuid& levelGuid, const char* jsonPath)
{
	 return  renderSystem.LoadRenderPass(levelGuid, jsonPath);
}

 void RenderSystem_RebuildSwapChain(VulkanRenderPass& vulkanRenderPass)
{
	 renderSystem.RebuildSwapChain(vulkanRenderPass);
}

 void RenderSystem_Update(void* windowHandle, const float deltaTime)
{
	 renderSystem.Update(windowHandle, deltaTime);
}

 VulkanRenderPass RenderSystem_FindRenderPass(RenderPassGuid renderPassGuid)
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

 void RenderSystem_RenderTest(float deltaTime)
 {
	 vulkanSystem.StartFrame();
	 VkCommandBuffer commandBuffer = vulkanSystem.CommandBuffers[vulkanSystem.CommandIndex];
	 // materialBakerSystem.Draw(commandBuffer);
	 levelSystem.Draw(commandBuffer, deltaTime);
	// ImGui_Draw(commandBuffer, imGuiRenderer);
	 vulkanSystem.EndFrame(commandBuffer);
 }

 const vec4 RenderSystem_SampleRenderPassPixel(const TextureGuid& textureGuid, ivec2 mousePosition)
 {
	 return renderSystem.SampleRenderPassPixel(textureGuid, mousePosition);
 }
