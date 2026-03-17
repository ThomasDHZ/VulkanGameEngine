#include "RenderSystemDLL.h"

LogVulkanMessageCallback g_logVulkanMessageCallback = nullptr;

void RenderSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback)
{
	g_logVulkanMessageCallback = callback;
}

GraphicsSystem RenderSystem_StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface)
{
    renderSystem.StartUp(windowHandle, instance, surface);
    return GraphicsSystem
    {
        .ApiVersion = vulkanSystem.ApiVersion,
        .Instance = vulkanSystem.Instance,
        .Device = vulkanSystem.Device,
        .PhysicalDevice = vulkanSystem.PhysicalDevice,
        .Surface = vulkanSystem.Surface,
        .CommandPool = vulkanSystem.CommandPool,
        .DebugMessenger = vulkanSystem.DebugMessenger,
        .Swapchain = vulkanSystem.Swapchain,
        .GraphicsQueue = vulkanSystem.GraphicsQueue,
        .PresentQueue = vulkanSystem.PresentQueue,

        .InFlightFencesPtr = vulkanSystem.InFlightFences.data(),
        .SwapChainImagesPtr = vulkanSystem.SwapChainImages.data(),
        .CommandBuffersPtr = vulkanSystem.CommandBuffers.data(),
        .SwapChainImageViewPtr = vulkanSystem.SwapChainImageViews.data(),
        .AcquireImageSemaphoresPtr = vulkanSystem.AcquireImageSemaphores.data(),
        .PresentImageSemaphoresPtr = vulkanSystem.PresentImageSemaphores.data(),
        .InFlightFencesCount = vulkanSystem.InFlightFences.size(),
        .SwapChainImagesCount = vulkanSystem.SwapChainImages.size(),
        .CommandBuffersCount = vulkanSystem.CommandBuffers.size(),
        .SwapChainImageViewCount = vulkanSystem.SwapChainImageViews.size(),
        .AcquireImageSemaphoresCount = vulkanSystem.AcquireImageSemaphores.size(),
        .PresentImageSemaphoresCount = vulkanSystem.PresentImageSemaphores.size()
    };
}

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
