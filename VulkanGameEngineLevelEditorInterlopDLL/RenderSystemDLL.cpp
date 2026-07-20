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
	 Vector<RenderPassNode> renderNodes = levelSystem.Draw(commandBuffer, deltaTime);
	 renderSystem.Draw(commandBuffer, renderNodes);
	 levelSystem.RenderFrameBuffer(commandBuffer, levelSystem.frameBufferId);
	 //ImGui_Draw(commandBuffer, imGuiRenderer);
	 vulkan.Swapchain().EndFrame(commandBuffer);
 }
