#pragma once
#include "DLL.h"
#include <VulkanRenderPass.h>

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT VulkanRenderPass*				   VulkanRenderPass_Create();
	DLL_EXPORT void								   VulkanRenderPass_LoadRenderPass(VulkanRenderPass* renderPass, RenderPassLoader& renderPassLoader);
	DLL_EXPORT void                                VulkanRenderPass_BeginRenderPass(VulkanRenderPass* renderPass, VkCommandBuffer& commandBuffer, uint mipLevel = 0);
	DLL_EXPORT void                                VulkanRenderPass_NextSubpass(VulkanRenderPass* renderPass, VkCommandBuffer& commandBuffer);
	DLL_EXPORT void                                VulkanRenderPass_BindViewPort(VulkanRenderPass* renderPass, VkCommandBuffer& commandBuffer, uint drawMipLevel = 0);
	DLL_EXPORT void                                VulkanRenderPass_BindRenderPassPipeline(VulkanRenderPass* renderPass, VkCommandBuffer& commandBuffer, const VulkanPipeline& pipeline, uint32 firstSet);
	DLL_EXPORT void                                VulkanRenderPass_DrawMesh(VulkanRenderPass* renderPass, VkCommandBuffer cmd, MeshDrawMessage& mesh);
	DLL_EXPORT void                                VulkanRenderPass_EndRenderPass(VulkanRenderPass* renderPass, VkCommandBuffer& commandBuffer);
	DLL_EXPORT void                                VulkanRenderPass_Destroy(VulkanRenderPass* renderPass);

	DLL_EXPORT [[nodiscard]] VkGuid                VulkanRenderPass_RenderPassId(VulkanRenderPass* renderPass);
	DLL_EXPORT [[nodiscard]] ivec2                 VulkanRenderPass_RenderPassResolution(VulkanRenderPass* renderPass);
	DLL_EXPORT [[nodiscard]] VulkanTexture*		   VulkanRenderPass_AttachmentList(VulkanRenderPass* renderPass);
	DLL_EXPORT [[nodiscard]] VulkanPipeline*	   VulkanRenderPass_PipelineList(VulkanRenderPass* renderPass, size_t& outCount);
	DLL_EXPORT [[nodiscard]] VulkanSubPass**	   VulkanRenderPass_SubPassList(VulkanRenderPass* renderPass, size_t* renderPassOutCount, size_t** subPassOutCount); //renderPassOutCount = number of subpasses (outer size); subPassOutCount = array of sizes for each subpass;
	DLL_EXPORT [[nodiscard]] VkSampleCountFlagBits VulkanRenderPass_SampleCount(VulkanRenderPass* renderPass, size_t& outCount);
#ifdef __cplusplus
}
#endif
