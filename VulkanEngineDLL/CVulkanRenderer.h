#pragma once
#include <windows.h>
#include <stdbool.h>

#include "DLL.h"
#include "Macro.h"
#include "CTypedef.h"
#include "VulkanError.h"

#ifdef __cplusplus
extern "C" {
#endif

	static const int MAX_FRAMES_IN_FLIGHT = 3;
	typedef void (*RichTextBoxCallback)(const char*);

	typedef struct
	{
		VkRenderPass* pRenderPass;
		const VkAttachmentDescription* pAttachmentList;
		const VkSubpassDescription* pSubpassDescriptionList;
		const VkSubpassDependency* pSubpassDependencyList;
		uint32						AttachmentCount;
		uint32						SubpassCount;
		uint32						DependencyCount;
		uint32						Width;
		uint32						Height;
	}RenderPassCreateInfoStruct;

	DLL_EXPORT VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	DLL_EXPORT void DestroyDebugUtilsMessengerEXT(VkInstance instance, const VkAllocationCallbacks* pAllocator);
	DLL_EXPORT VkResult Renderer_CreateRenderPass(VkDevice device, RenderPassCreateInfoStruct* renderPassCreateInfo);
	DLL_EXPORT VkResult Renderer_CreateDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool, VkDescriptorPoolCreateInfo* descriptorPoolCreateInfo);
	DLL_EXPORT VkResult Renderer_CreateDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout, VkDescriptorSetLayoutCreateInfo* descriptorSetLayoutCreateInfo);
	DLL_EXPORT VkResult Renderer_CreatePipelineLayout(VkDevice device, VkPipelineLayout* pipelineLayout, VkPipelineLayoutCreateInfo* pipelineLayoutCreateInfo);
	DLL_EXPORT VkResult Renderer_AllocateDescriptorSets(VkDevice device, VkDescriptorSet* descriptorSet, VkDescriptorSetAllocateInfo* descriptorSetAllocateInfo);
	DLL_EXPORT VkResult Renderer_AllocateCommandBuffers(VkDevice device, VkCommandBuffer* commandBuffer, VkCommandBufferAllocateInfo* commandBufferAllocateInfo);
	DLL_EXPORT VkResult Renderer_CreateGraphicsPipelines(VkDevice device, VkPipeline* graphicPipeline, VkGraphicsPipelineCreateInfo* createGraphicPipelines, uint32 createGraphicPipelinesCount);
	DLL_EXPORT VkResult Renderer_CreateCommandPool(VkDevice device, VkCommandPool* commandPool, VkCommandPoolCreateInfo* commandPoolInfo);
	DLL_EXPORT VkResult Renderer_CreateSemaphores(VkDevice device, VkFence* inFlightFences, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, int maxFramesInFlight);
	DLL_EXPORT void Renderer_UpdateDescriptorSet(VkDevice device, VkWriteDescriptorSet* writeDescriptorSet, uint32 count);
	DLL_EXPORT VkResult Renderer_StartFrame(VkDevice device, VkSwapchainKHR swapChain, VkFence* fenceList, VkSemaphore* acquireImageSemaphoreList, size_t* pImageIndex, size_t* pCommandIndex, bool* pRebuildRendererFlag);
	DLL_EXPORT VkResult Renderer_EndFrame(VkSwapchainKHR swapChain, VkSemaphore* acquireImageSemaphoreList, VkSemaphore* presentImageSemaphoreList, VkFence* fenceList, VkQueue graphicsQueue, VkQueue presentQueue, size_t imageIndex, size_t commandIndex, VkCommandBuffer* pCommandBufferSubmitList, uint32_t commandBufferCount, bool* rebuildRendererFlag);
	DLL_EXPORT uint32 Renderer_GetMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	DLL_EXPORT VkResult Renderer_BeginCommandBuffer(VkCommandBuffer* pCommandBufferList, VkCommandBufferBeginInfo* commandBufferBeginInfo);
	DLL_EXPORT VkResult Renderer_EndCommandBuffer(VkCommandBuffer* pCommandBufferList);
	DLL_EXPORT VkCommandBuffer Renderer_BeginSingleUseCommandBuffer(VkDevice device, VkCommandPool commandPool);
	DLL_EXPORT VkResult Renderer_EndSingleUseCommandBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);
	DLL_EXPORT void Renderer_DestroyRenderPass(VkDevice device, VkRenderPass* renderPass);
	DLL_EXPORT void Renderer_DestroyFrameBuffers(VkDevice device, VkFramebuffer* frameBufferList, uint32 count);
	DLL_EXPORT void Renderer_DestroyDescriptorPool(VkDevice device, VkDescriptorPool* descriptorPool);
	DLL_EXPORT void Renderer_DestroyCommandBuffers(VkDevice device, VkCommandPool* commandPool, VkCommandBuffer* commandBufferList, uint32 count);
	DLL_EXPORT void Renderer_DestroyBuffer(VkDevice device, VkBuffer* buffer);

	 void Renderer_DestroyFences(VkDevice device, VkSemaphore* acquireImageSemaphores, VkSemaphore* presentImageSemaphores, VkFence* fences, size_t semaphoreCount);
	 void Renderer_DestroyCommandPool(VkDevice device, VkCommandPool* commandPool);
	 void Renderer_DestroyDevice(VkDevice device);
	 void Renderer_DestroySurface(VkInstance instance, VkSurfaceKHR* surface);
	 void Renderer_DestroyDebugger(VkInstance* instance, VkDebugUtilsMessengerEXT debugUtilsMessengerEXT);
	 void Renderer_DestroyInstance(VkInstance* instance);
	 void Renderer_DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout);
	 void Renderer_FreeDeviceMemory(VkDevice device, VkDeviceMemory* memory);
	 void Renderer_DestroySwapChainImageView(VkDevice device, VkSurfaceKHR surface, VkImageView* pSwapChainImageViewList, uint32 count);
	 void Renderer_DestroySwapChain(VkDevice device, VkSwapchainKHR* swapChain);
	 void Renderer_DestroyImageView(VkDevice device, VkImageView* imageView);
	 void Renderer_DestroyImage(VkDevice device, VkImage* image);
	 void Renderer_DestroySampler(VkDevice device, VkSampler* sampler);
	 void Renderer_DestroyPipeline(VkDevice device, VkPipeline* pipeline);
	 void Renderer_DestroyPipelineLayout(VkDevice device, VkPipelineLayout* pipelineLayout);
	 void Renderer_DestroyPipelineCache(VkDevice device, VkPipelineCache* pipelineCache);

#ifdef __cplusplus
}
#endif