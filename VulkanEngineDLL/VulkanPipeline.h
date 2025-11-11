#pragma once
#include "VulkanRenderer.h"
#include "JsonStruct.h"
#include "ShaderSystem.h"


struct VulkanPipeline
{
    VkGuid RenderPipelineId;
    size_t DescriptorSetLayoutCount;
    size_t DescriptorSetCount;
    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout* DescriptorSetLayoutList = nullptr;
    VkDescriptorSet* DescriptorSetList = nullptr;
    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipelineCache PipelineCache = VK_NULL_HANDLE;
};

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT VulkanPipeline VulkanPipeline_CreateRenderPipeline(VkDevice device, VulkanRenderPass& vulkanRenderPass, const char* pipelineJsonFilePath, GPUIncludes& gpuIncludes, ShaderPipelineData& shaderPipelineData);
    DLL_EXPORT VulkanPipeline VulkanPipeline_RebuildSwapChain(VkDevice device, VulkanPipeline& oldPipeline, VulkanRenderPass& vulkanRenderPass, const char* pipelineJsonFilePath, GPUIncludes& gpuIncludes, ShaderPipelineData& shaderPipelineData);
    DLL_EXPORT void VulkanPipeline_Destroy(VkDevice device, VulkanPipeline& vulkanPipelineDLL);
#ifdef __cplusplus
}
#endif

VkDescriptorPool Pipeline_CreatePipelineDescriptorPool(VkDevice device, RenderPipelineLoader& renderPipelineLoader);
Vector<VkDescriptorSetLayout> Pipeline_CreatePipelineDescriptorSetLayout(VkDevice device, RenderPipelineLoader& renderPipelineLoader);
Vector<VkDescriptorSet> Pipeline_AllocatePipelineDescriptorSets(VkDevice device, RenderPipelineLoader& renderPipelineLoader, const VkDescriptorPool& descriptorPool, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount);
void Pipeline_UpdatePipelineDescriptorSets(VkDevice device, RenderPipelineLoader& renderPipelineLoader, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount);
VkPipelineLayout Pipeline_CreatePipelineLayout(VkDevice device, RenderPipelineLoader& renderPipelineLoader, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount);
VkPipeline Pipeline_CreatePipeline(VkDevice device, RenderPipelineLoader& renderPipelineLoader, VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount);
void Pipeline_PipelineBindingData(RenderPipelineLoader& renderPipelineLoader);