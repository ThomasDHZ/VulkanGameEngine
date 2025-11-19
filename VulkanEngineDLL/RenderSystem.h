#pragma once
#include "Platform.h"
#include "VulkanRenderer.h"
#include "VulkanRenderPass.h"
#include "VulkanPipeline.h"

class RenderSystem
{
    friend class JsonRenderPass;
private:
    UnorderedMap<RenderPassGuid, VulkanRenderPass>                RenderPassMap;
    UnorderedMap<RenderPassGuid, Vector<VulkanPipeline>>          RenderPipelineMap;
    UnorderedMap<RenderPassGuid, String>                          RenderPassLoaderJsonMap;

    void RecreateSwapchain(void* windowHandle, RenderPassGuid& renderPassGuid, LevelGuid& levelGuid, const float& deltaTime);


public:
    VkCommandBufferBeginInfo                                      CommandBufferBeginInfo;

    RenderSystem();
    ~RenderSystem();

    DLL_EXPORT GraphicsRenderer              StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface, VkDebugUtilsMessengerEXT& debugMessenger);
    DLL_EXPORT RenderPassGuid                LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath, ivec2 renderPassResolution);
    DLL_EXPORT void                          Update(void* windowHandle, RenderPassGuid& spriteRenderPass2DGuid, LevelGuid& levelGuid, const float& deltaTime);
    DLL_EXPORT VkCommandBuffer               BeginSingleTimeCommands();
    DLL_EXPORT VkCommandBuffer               BeginSingleTimeCommands(VkCommandPool& commandPool);
    DLL_EXPORT VkResult                      EndSingleTimeCommands(VkCommandBuffer commandBuffer);
    DLL_EXPORT VkResult                      EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool& commandPool);
    DLL_EXPORT VkResult                      StartFrame();
    DLL_EXPORT VkResult                      EndFrame(Vector<VkCommandBuffer> commandBufferSubmitList);
    DLL_EXPORT const VulkanRenderPass        FindRenderPass(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT const Vector<VulkanPipeline>  FindRenderPipelineList(const RenderPassGuid& renderPassGuid);
    DLL_EXPORT void                          Destroy();
    DLL_EXPORT void                          DestroyRenderPasses();
    DLL_EXPORT void                          DestroyRenderPipelines();
    DLL_EXPORT void                          DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList);
    DLL_EXPORT void                          DestroyCommandBuffers(VkCommandBuffer& commandBuffer);
    DLL_EXPORT void                          DestroyBuffer(VkBuffer& buffer);

    Vector<VkDescriptorBufferInfo>    GetVertexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo>    GetIndexPropertiesBuffer();
    Vector<VkDescriptorBufferInfo>    GetGameObjectTransformBuffer();
    Vector<VkDescriptorBufferInfo>    GetMeshPropertiesBuffer(const  LevelGuid& levelGuid);
    Vector<VkDescriptorImageInfo>     GetTexturePropertiesBuffer(const RenderPassGuid& renderPassGuid);
};
extern DLL_EXPORT RenderSystem renderSystem;

#ifdef __cplusplus
    extern "C"
    {
        #endif
        DLL_EXPORT GraphicsRenderer       RenderSystem_StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface, VkDebugUtilsMessengerEXT& debugMessenger);
        DLL_EXPORT void                   RenderSystem_Update(void* windowHandle, VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime);
        DLL_EXPORT RenderPassGuid         RenderSystem_LoadRenderPass(VkGuid& levelGuid, const String& jsonPath, ivec2 renderPassResolution);
        DLL_EXPORT void                   RenderSystem_RecreateSwapChain(void* windowHandle, VkGuid& spriteRenderPass2DId, VkGuid& levelId, const float& deltaTime);
        DLL_EXPORT VulkanRenderPass       RenderSystem_FindRenderPass(const RenderPassGuid& guid);
        //DLL_EXPORT Vector<VulkanPipeline> RenderSystem_FindRenderPipelineList(const RenderPassGuid& renderPassGuid);
        DLL_EXPORT VkResult               RenderSystem_StartFrame();
        DLL_EXPORT VkResult               RenderSystem_EndFrame(VkCommandBuffer* commandBufferListPtr, size_t commandBufferCount);
        DLL_EXPORT void                   RenderSystem_DestroyRenderPasses();
        DLL_EXPORT void                   RenderSystem_DestroyRenderPipelines();
        DLL_EXPORT void                   RenderSystem_Destroy();
        #ifdef __cplusplus
    }
#endif