#pragma once
#include "Platform.h"
#include "JsonStruct.h"
#include "JsonLoader.h"
#include "MemorySystem.h"
#include "VulkanRenderer.h"
#include "TextureSystem.h"

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT VulkanRenderPass VulkanRenderPass_CreateVulkanRenderPass(GraphicsRenderer& renderer, const char* renderPassJsonFilePath, RenderPassAttachementTextures& vulkanRenderPass, ivec2& renderPassResolution);
	DLL_EXPORT VulkanRenderPass VulkanRenderPass_RebuildSwapChain(GraphicsRenderer& renderer, VulkanRenderPass& vulkanRenderPass, const char* renderPassJsonFilePath, ivec2& renderPassResolution, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);
	DLL_EXPORT void VulkanRenderPass_DestroyRenderPass(GraphicsRenderer& renderer, VulkanRenderPass& renderPass);

	void RenderPass_CreateCommandBuffers(const GraphicsRenderer& renderer, VkCommandBuffer* commandBufferList, size_t commandBufferCount);
	VkRenderPass RenderPass_BuildRenderPass(const GraphicsRenderer& renderer, const RenderPassLoader& renderPassJsonLoader, Vector<Texture>& renderedTextureList, Texture& depthTexture);
	void RenderPass_BuildRenderPassAttachments(const GraphicsRenderer& renderer, const RenderPassLoader& renderPassJsonLoader, Vector<Texture>& renderedTextureList, Texture& depthTexture);
#ifdef __cplusplus
}
#endif

void VulkanRenderPass_DestoryRenderPassSwapChainTextures(GraphicsRenderer& renderer, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);
Vector<VkFramebuffer> RenderPass_BuildFrameBuffer(const GraphicsRenderer& renderer, VulkanRenderPass& renderPass, Vector<Texture>& renderedTextureList, Texture& depthTexture);
