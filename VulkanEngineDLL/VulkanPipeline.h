#pragma once
#include "VulkanRenderer.h"
#include "JsonStruct.h"
#include "ShaderSystem.h"


struct VulkanPipeline
{
    VkGuid RenderPipelineId;
    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
    Vector<VkDescriptorSetLayout> DescriptorSetLayoutList;
    Vector<VkDescriptorSet> DescriptorSetList;
    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipelineCache PipelineCache = VK_NULL_HANDLE;
};

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT VulkanPipeline VulkanPipeline_CreateRenderPipeline(VulkanRenderPass& vulkanRenderPass, const char* pipelineJsonFilePath, ShaderPipelineDataDLL& shaderPipelineData);
    DLL_EXPORT VulkanPipeline VulkanPipeline_RebuildSwapChain(VulkanPipeline& oldPipeline, VulkanRenderPass& vulkanRenderPass, const char* pipelineJsonFilePath, ShaderPipelineDataDLL& shaderPipelineData);
    DLL_EXPORT void VulkanPipeline_Destroy(VulkanPipeline& vulkanPipelineDLL);
#ifdef __cplusplus
}
#endif

VkDescriptorPool Pipeline_CreatePipelineDescriptorPool(RenderPipelineLoader& renderPipelineLoader);
Vector<VkDescriptorSetLayout> Pipeline_CreatePipelineDescriptorSetLayout(RenderPipelineLoader& renderPipelineLoader);
Vector<VkDescriptorSet> Pipeline_AllocatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader, const VkDescriptorPool& descriptorPool, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount);
void Pipeline_UpdatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount);
VkPipelineLayout Pipeline_CreatePipelineLayout(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount);
VkPipeline Pipeline_CreatePipeline(RenderPipelineLoader& renderPipelineLoader, VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount);
void Pipeline_PipelineBindingData(RenderPipelineLoader& renderPipelineLoader);