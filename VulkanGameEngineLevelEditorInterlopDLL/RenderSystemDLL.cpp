#include "RenderSystemDLL.h"

GraphicsRenderer RenderSystem_StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface)
{
    return renderSystem.StartUp(windowHandle, instance, surface);
}

void RenderSystem_Update(void* windowHandle, RenderPassGuid& spriteRenderPassGuidId, LevelGuid& levelGuid, const float& deltaTime)
{
    return renderSystem.Update(windowHandle, spriteRenderPassGuidId, levelGuid, deltaTime);
}

RenderPassGuid RenderSystem_LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath, ivec2 renderPassResolution)
{
    return renderSystem.LoadRenderPass(levelGuid, jsonPath, renderPassResolution);
}

void RenderSystem_StartFrame()
{
    renderSystem.StartFrame();
}

void RenderSystem_EndFrame(VkCommandBuffer* commandBufferListPtr, size_t commandBufferCount)
{
    Vector<VkCommandBuffer> commandBufferList = Vector<VkCommandBuffer>(commandBufferListPtr, commandBufferListPtr + commandBufferCount);
    renderSystem.EndFrame(commandBufferList);
}

GraphicsRenderer RenderSystem_StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface, VkDebugUtilsMessengerEXT& debugMessenger)
{
    return renderSystem.StartUp(windowHandle, instance, surface);
}

void RenderSystem_RecreateSwapChain(void* windowHandle, VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime)
{
    throw std::runtime_error("Not Implimented Yet");
}

VulkanRenderPass RenderSystem_FindRenderPass(const RenderPassGuid& renderPassGuid)
{
    return renderSystem.FindRenderPass(renderPassGuid);
}

Vector<VulkanPipeline> RenderSystem_FindRenderPipelineList(const RenderPassGuid& renderPassGuid)
{
    return renderSystem.FindRenderPipelineList(renderPassGuid);
}

void RenderSystem_DestroyRenderPasses()
{
    return renderSystem.DestroyRenderPasses();
}

void RenderSystem_DestroyRenderPipelines()
{
    return renderSystem.DestroyRenderPipelines();
}

void RenderSystem_Destroy()
{
    return renderSystem.Destroy();
}