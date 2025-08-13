#pragma once
#include "Typedef.h"
#include "JsonStruct.h"
#include "JsonLoader.h"
#include "CoreVulkanRenderer.h"
#include "VulkanPipeline.h"
#include "Texture.h"

struct VulkanRenderPassDLL
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
	bool UseFrameBufferResolution = true;
};

struct VulkanRenderPass
{
	RenderPassGuid RenderPassId;
	VkSampleCountFlagBits SampleCount;
	VkRect2D RenderArea;
	VkRenderPass RenderPass = VK_NULL_HANDLE;
	Vector<VkFramebuffer> FrameBufferList;
	Vector<VkClearValue> ClearValueList;
	VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
	bool UseFrameBufferResolution = true;
};

#ifdef __cplusplus
extern "C" {
#endif
DLL_EXPORT VulkanRenderPassDLL* VulkanRenderPass_ConvertToVulkanRenderPassDLL(VulkanRenderPass& renderPass);
DLL_EXPORT VulkanRenderPass VulkanRenderPass_ConvertToVulkanRenderPass(VulkanRenderPassDLL* renderPassDLL);

DLL_EXPORT VulkanRenderPassDLL* VulkanRenderPass_CreateVulkanRenderPassCS(const RendererStateCS& renderStateCS, const char* renderPassLoader, ivec2& renderPassResolution, int ConstBuffer, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);
DLL_EXPORT VulkanRenderPassDLL* VulkanRenderPass_CreateVulkanRenderPass(const RendererState& renderState, const char* renderPassLoader, ivec2& renderPassResolution, int ConstBuffer, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture);
DLL_EXPORT void VulkanRenderPass_DestroyRenderPass(RendererState& renderState, VulkanRenderPassDLL& renderPass);

VkResult RenderPass_CreateCommandBuffers(const RendererState& renderState, VkCommandBuffer* commandBufferList, uint32 commandBufferCount);
VkRenderPass RenderPass_BuildRenderPass(const RendererState& renderState, VulkanRenderPass& vulkanRenderPass, const RenderPassLoader& renderPassBuildInfo, Vector<Texture>& renderedTextureList, Texture& depthTexture);
Vector<VkFramebuffer> RenderPass_BuildFrameBuffer(const RendererState& renderState, const VulkanRenderPass& vulkanRenderPass, const RenderPassLoader& renderPassBuildInfo, Vector<Texture>& renderedTextureList, Texture& depthTexture, ivec2& renderPassResolution);
void VulkanRenderPass_DeleteVulkanRenderPassDLL(VulkanRenderPassDLL* vulkanRenderPassDLL);
#ifdef __cplusplus
}
#endif