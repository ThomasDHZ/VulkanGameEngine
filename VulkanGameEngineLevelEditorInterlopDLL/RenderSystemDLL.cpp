#include "RenderSystemDLL.h"
#include "../VulkanGameEngine/GameSystem.h"

VkGuid RenderSystem_LoadRenderPass(VkGuid& levelGuid, const char* jsonPath)
{
	 return  renderSystem.LoadRenderPass(levelGuid, jsonPath);
}

 void RenderSystem_Update(void* windowHandle, const float deltaTime)
{
	 renderSystem.Update(windowHandle, deltaTime);
}

 VulkanRenderPass RenderSystem_FindRenderPass(VkGuid renderPassGuid)
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
	// renderSystem.DestroyPipeline(vulkanPipelineDLL);
}

 void RenderSystem_DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList)
{
	// renderSystem.DestroyFrameBuffers(frameBufferList);
}

 void RenderSystem_DestroyCommandBuffers(Vector<VkCommandBuffer>& commandBuffer)
{
	// renderSystem.DestroyCommandBuffers(commandBuffer);
}

 void RenderSystem_DestroyBuffer(VkBuffer& buffer)
{
	// renderSystem.DestroyBuffer(buffer);
}

 void RenderSystem_RenderTest(float deltaTime)
 {
	 vulkan.Swapchain().StartFrame();
	 VkCommandBuffer commandBuffer = vulkan.CommandBufferList()[vulkan.Swapchain().CommandIndex()];
	 // materialBakerSystem.Draw(commandBuffer);
	 levelSystem.Draw(commandBuffer, deltaTime);
	 levelSystem.RenderFrameBuffer(commandBuffer, levelSystem.frameBufferId);
	 renderSystem.Draw(commandBuffer);
	 //ImGui_Draw(commandBuffer, imGuiRenderer);
	 vulkan.Swapchain().EndFrame(commandBuffer);
 }
