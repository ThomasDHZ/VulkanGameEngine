#pragma once
#include "Typedef.h"
#include "JsonStruct.h"
#include "JsonLoader.h"
#include "MemorySystem.h"
#include "VulkanRenderer.h"
#include "VulkanPipeline.h"
#include "Texture.h"

struct VulkanRenderPass
{
	RenderPassGuid RenderPassId;
	VkSampleCountFlagBits SampleCount;
	VkRect2D RenderArea;
	VkRenderPass RenderPass = VK_NULL_HANDLE;
	VkFramebuffer* FrameBufferList;
	VkClearValue* ClearValueList;
	size_t FrameBufferCount;
	size_t ClearValueCount;
	VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
	ivec2 RenderPassResolution;
	bool IsRenderedToSwapchain = false;
};

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT VulkanRenderPass VulkanRenderPass_CreateVulkanRenderPass(GraphicsRenderer& renderer, const char* renderPassLoader, ivec2& renderPassResolution, int ConstBuffer, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);
	DLL_EXPORT VulkanRenderPass VulkanRenderPass_RebuildSwapChain(GraphicsRenderer& renderer, VulkanRenderPass& vulkanRenderPass, const char* renderPassJsonLoader, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);
	DLL_EXPORT void VulkanRenderPass_DestroyRenderPass(GraphicsRenderer& renderer, VulkanRenderPass& renderPass, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);

	VkResult RenderPass_CreateCommandBuffers(const GraphicsRenderer& renderer, VkCommandBuffer* commandBufferList, size_t commandBufferCount);
	VkRenderPass RenderPass_BuildRenderPass(const GraphicsRenderer& renderer, const RenderPassLoader& renderPassJsonLoader, Vector<Texture>& renderedTextureList, Texture& depthTexture);
	void RenderPass_BuildRenderPassAttachments(const GraphicsRenderer& renderer, const RenderPassLoader& renderPassJsonLoader, Vector<Texture>& renderedTextureList, Texture& depthTexture);
#ifdef __cplusplus
}
#endif

void VulkanRenderPass_DestoryRenderPassSwapChainTextures(GraphicsRenderer& renderer, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);
Vector<VkFramebuffer> RenderPass_BuildFrameBuffer(const GraphicsRenderer& renderer, VulkanRenderPass& renderPass, Vector<Texture>& renderedTextureList, Texture& depthTexture);
