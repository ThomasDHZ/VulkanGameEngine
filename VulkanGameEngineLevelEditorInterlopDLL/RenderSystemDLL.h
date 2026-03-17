#pragma once
#include <VulkanSystem.h>
#include <RenderSystem.h>

struct GraphicsSystem
{
    uint32								   ApiVersion = VK_API_VERSION_1_1;
    VkInstance							   Instance = VK_NULL_HANDLE;
    VkDevice							   Device = VK_NULL_HANDLE;
    VkPhysicalDevice					   PhysicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR						   Surface = VK_NULL_HANDLE;
    VkCommandPool						   CommandPool = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT			   DebugMessenger = VK_NULL_HANDLE;
    VkSwapchainKHR						   Swapchain = VK_NULL_HANDLE;
    VkQueue								   GraphicsQueue = VK_NULL_HANDLE;
    VkQueue								   PresentQueue = VK_NULL_HANDLE;

    VkFence*						      InFlightFencesPtr = nullptr;
    VkImage*						      SwapChainImagesPtr = nullptr;
    VkCommandBuffer*				      CommandBuffersPtr = nullptr;
    VkImageView*					      SwapChainImageViewPtr = nullptr;
    VkSemaphore*					      AcquireImageSemaphoresPtr = nullptr;
    VkSemaphore*					      PresentImageSemaphoresPtr = nullptr;
    size_t                                InFlightFencesCount = MAXSIZE_T;
    size_t                                SwapChainImagesCount = MAXSIZE_T;
    size_t                                CommandBuffersCount = MAXSIZE_T;
    size_t                                SwapChainImageViewCount = MAXSIZE_T;
    size_t                                AcquireImageSemaphoresCount = MAXSIZE_T;
    size_t                                PresentImageSemaphoresCount = MAXSIZE_T;
};

typedef void (*LogVulkanMessageCallback)(const char* message, int severity);
#ifdef __cplusplus
extern "C"
{
#endif
    DLL_EXPORT void			                                           RenderSystem_CreateLogMessageCallback(LogVulkanMessageCallback callback);
    DLL_EXPORT GraphicsSystem                                          RenderSystem_StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface);
    DLL_EXPORT RenderPassGuid                                          RenderSystem_LoadRenderPass(LevelGuid& levelGuid, const char* jsonPath, bool useGlobalDescriptorSet);
    DLL_EXPORT RenderPassGuid                                          RenderSystem_LoadRenderPass2(LevelGuid& levelGuid, RenderPassLoader& renderPassLoader, bool useGlobalDescriptorSet);
    DLL_EXPORT void                                                    RenderSystem_RebuildSwapChain(VulkanRenderPass& vulkanRenderPass);
    DLL_EXPORT void                                                    RenderSystem_Update(void* windowHandle, LevelGuid& levelGuid, const float& deltaTime);
    DLL_EXPORT VulkanRenderPass                                        RenderSystem_FindRenderPass(const RenderPassGuid& renderPassGuid);

    DLL_EXPORT void                                                    RenderSystem_Destroy();
    DLL_EXPORT void                                                    RenderSystem_DestroyRenderPass(VulkanRenderPass& renderPass);
    DLL_EXPORT void                                                    RenderSystem_DestroyRenderPasses();
    DLL_EXPORT void                                                    RenderSystem_DestroyRenderPipelines();
    DLL_EXPORT void                                                    RenderSystem_DestroyPipeline(VulkanPipeline& vulkanPipelineDLL);
    DLL_EXPORT void                                                    RenderSystem_DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList);
    DLL_EXPORT void                                                    RenderSystem_DestroyCommandBuffers(VkCommandBuffer& commandBuffer);
    DLL_EXPORT void                                                    RenderSystem_DestroyBuffer(VkBuffer& buffer);
#ifdef __cplusplus
}
#endif