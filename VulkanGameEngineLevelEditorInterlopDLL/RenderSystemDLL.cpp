#include "RenderSystemDLL.h"
#include <VulkanSystem.h>
#include "../VulkanGameEngine/GameSystem.h"

 RenderPassGuid RenderSystem_LoadRenderPass(const char* jsonPath)
{
	 return  renderSystem.LoadRenderPass(jsonPath);
}

 void RenderSystem_Update(void* windowHandle, const float deltaTime)
{
	 renderSystem.Update(windowHandle, deltaTime);
}

 VulkanRenderPass RenderSystem_FindRenderPass(RenderPassGuid renderPassGuid)
{
	 return renderSystem.FindRenderPass(renderPassGuid);
}

 void RenderSystem_DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList)
{
	 renderSystem.DestroyFrameBuffers(frameBufferList);
}

 void RenderSystem_DestroyCommandBuffers(Vector<VkCommandBuffer>& commandBuffer)
{
	 renderSystem.DestroyCommandBuffers(commandBuffer);
}

 void RenderSystem_DestroyBuffer(VkBuffer& buffer)
{
	 renderSystem.DestroyBuffer(buffer);
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
