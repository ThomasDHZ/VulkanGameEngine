#pragma once
#include "DLL.h"
#include "VulkanError.h"
#include "VulkanRenderer.h"
#include "VulkanRenderPass.h"
#include "VulkanPipeline.h"

#ifdef __cplusplus
    extern "C"
    {
        #endif
        DLL_EXPORT GraphicsRenderer RenderSystem_StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface, VkDebugUtilsMessengerEXT& debugMessenger);
        DLL_EXPORT void RenderSystem_Update(void* windowHandle, VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime);
        DLL_EXPORT VkGuid RenderSystem_LoadRenderPass(VkGuid& levelId, const String& jsonPath, ivec2 renderPassResolution);
        DLL_EXPORT void RenderSystem_RecreateSwapChain(void* windowHandle, VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime);
        DLL_EXPORT VulkanRenderPass& RenderSystem_FindRenderPass(const RenderPassGuid& guid);
        DLL_EXPORT Vector<VulkanPipeline>& RenderSystem_FindRenderPipelineList(const RenderPassGuid& guid);
        DLL_EXPORT VkResult RenderSystem_StartFrame();
        DLL_EXPORT VkResult RenderSystem_EndFrame(VkCommandBuffer* commandBufferListPtr, size_t commandBufferCount);
        DLL_EXPORT void RenderSystem_DestroyRenderPasses();
        DLL_EXPORT void RenderSystem_DestroyRenderPipelines();
        DLL_EXPORT void RenderSystem_Destroy();
        #ifdef __cplusplus
    }
#endif

const Vector<VkDescriptorBufferInfo> Renderer_GetVertexPropertiesBuffer();
const Vector<VkDescriptorBufferInfo> Renderer_GetIndexPropertiesBuffer();
const Vector<VkDescriptorBufferInfo> Renderer_GetGameObjectTransformBuffer();
const Vector<VkDescriptorBufferInfo> Renderer_GetMeshPropertiesBuffer(const VkGuid& levelLayerId);
const Vector<VkDescriptorImageInfo>  Renderer_GetTexturePropertiesBuffer(const VkGuid& renderPassId);

class RenderSystem
{
    friend class JsonRenderPass;

private:

    void RecreateSwapchain(void* windowHandle, VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime) { RecreateSwapchain(windowHandle, spriteRenderPass2DId, levelId, deltaTime); }

    const Vector<VkDescriptorBufferInfo> GetVertexPropertiesBuffer() { Renderer_GetVertexPropertiesBuffer(); };
    const Vector<VkDescriptorBufferInfo> GetIndexPropertiesBuffer() { Renderer_GetIndexPropertiesBuffer(); };
    const Vector<VkDescriptorBufferInfo> GetGameObjectTransformBuffer() { Renderer_GetGameObjectTransformBuffer(); };
    const Vector<VkDescriptorBufferInfo> GetMeshPropertiesBuffer(const VkGuid& levelLayerId) { Renderer_GetMeshPropertiesBuffer(levelLayerId); };
    const Vector<VkDescriptorImageInfo>  GetTexturePropertiesBuffer(const VkGuid& renderPassId) { Renderer_GetTexturePropertiesBuffer(renderPassId); };

public:
    UnorderedMap<RenderPassGuid, VulkanRenderPass>                RenderPassMap;
    UnorderedMap<RenderPassGuid, Vector<VulkanPipeline>>          RenderPipelineMap;
    UnorderedMap<RenderPassGuid, String>                          RenderPassLoaderJsonMap;
    VkCommandBufferBeginInfo                                      CommandBufferBeginInfo;

    RenderSystem() {}
    ~RenderSystem() {}

    void StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface, VkDebugUtilsMessengerEXT& debugMessenger)
    {
        Renderer_RendererSetUp(windowHandle, instance, surface, debugMessenger);
        shaderSystem.StartUp();
    }

    void Update(void* windowHandle, VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime)
    {
        if (renderer.RebuildRendererFlag)
        {
            RecreateSwapchain(windowHandle, spriteRenderPass2DId, levelId, deltaTime);
            renderer.RebuildRendererFlag = false;
        }
    }
    VkGuid LoadRenderPass(VkGuid& levelId, const String& jsonPath, ivec2 renderPassResolution) 
    { 
        return RenderSystem_LoadRenderPass(levelId, jsonPath, renderPassResolution); 
    }

    const VulkanRenderPass& FindRenderPass(const RenderPassGuid& guid) 
    { 
        return RenderPassMap.at(guid); 
    }

    const Vector<VulkanPipeline>& FindRenderPipelineList(const RenderPassGuid& guid) 
    { 
        return RenderPipelineMap.at(guid); 
    }

    void DestroyRenderPasses() 
    { 
        RenderSystem_DestroyRenderPasses(); 
    }

    void DestroyRenderPipelines() 
    { 
        RenderSystem_DestroyRenderPipelines(); 
    }

    void Destroy() 
    { 
        RenderSystem_Destroy(); 
    }

    void DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList) 
    { 
        Renderer_DestroyFrameBuffers(renderer.Device, frameBufferList.data(), frameBufferList.size()); 
    }

    void DestroyCommandBuffers(VkCommandBuffer& commandBuffer) 
    { 
        Renderer_DestroyCommandBuffers(renderer.Device, &renderer.CommandPool, &commandBuffer, 1); 
    }

    void DestroyBuffer(VkBuffer& buffer) 
    { 
        Renderer_DestroyBuffer(renderer.Device, &buffer); 
    }

    VkCommandBuffer BeginSingleTimeCommands() 
    { 
        return Renderer_BeginSingleTimeCommands(renderer.Device, renderer.CommandPool); 
    }

    VkCommandBuffer BeginSingleTimeCommands(VkCommandPool& commandPool) 
    { 
        return Renderer_BeginSingleTimeCommands(renderer.Device, renderer.CommandPool); 
    }

    VkResult EndSingleTimeCommands(VkCommandBuffer commandBuffer) 
    { 
        return Renderer_EndSingleTimeCommands(renderer.Device, renderer.CommandPool, renderer.GraphicsQueue, commandBuffer); 
    }

    VkResult EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool& commandPool) 
    { 
        return Renderer_EndSingleTimeCommands(renderer.Device, commandPool, renderer.GraphicsQueue, commandBuffer); 
    }

    VkResult StartFrame()
    {
        return RenderSystem_StartFrame();
    }

    VkResult EndFrame(Vector<VkCommandBuffer> commandBufferSubmitList)
    {
        return RenderSystem_EndFrame(commandBufferSubmitList.data(), commandBufferSubmitList.size());
    }
};
DLL_EXPORT RenderSystem renderSystem;