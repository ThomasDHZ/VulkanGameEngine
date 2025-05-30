#pragma once
#include "CoreVulkanRenderer.h"

struct VulkanPipeline
{
    uint RenderPipelineId;
    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
    Vector<VkDescriptorSetLayout> DescriptorSetLayoutList;
    Vector<VkDescriptorSet> DescriptorSetList;
    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipelineCache PipelineCache = VK_NULL_HANDLE;
};

DLL_EXPORT VulkanPipeline Pipeline_CreateRenderPipeline(VkDevice device, VkGuid& renderPassId, uint renderPipelineId, RenderPipelineModel& model, VkRenderPass renderPass, uint constBufferSize, ivec2& renderPassResolution, const GPUIncludes& includes, Vector<VkPipelineShaderStageCreateInfo>& pipelineShaders);
DLL_EXPORT void Pipeline_RecreateSwapchain(VkRenderPass renderPass, uint constBufferSize, int newWidth, int newHeight);
DLL_EXPORT void Pipeline_Destroy(VkDevice device, VulkanPipeline& vulkanPipeline);

VkDescriptorPool Pipeline_CreatePipelineDescriptorPool(VkDevice device, const RenderPipelineModel& model, const GPUIncludes& includes);
Vector<VkDescriptorSetLayout> Pipeline_CreatePipelineDescriptorSetLayout(VkDevice device, const RenderPipelineModel& model, const GPUIncludes& includes);
Vector<VkDescriptorSet> Pipeline_AllocatePipelineDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, const RenderPipelineModel& model, const Vector<VkDescriptorSetLayout>& descriptorSetLayoutList);
void Pipeline_UpdatePipelineDescriptorSets(VkDevice device, const Vector<VkDescriptorSet>& descriptorSetList, const RenderPipelineModel& model, const GPUIncludes& includes);
VkPipelineLayout Pipeline_CreatePipelineLayout(VkDevice device, const Vector<VkDescriptorSetLayout>& descriptorSetLayoutList, uint constBufferSize);
VkPipeline Pipeline_CreatePipeline(VkDevice device, VkRenderPass renderpass, VkPipelineLayout pipelineLayout, VkPipelineCache pipelineCache, const RenderPipelineModel& model, ivec2& extent, Vector<VkPipelineShaderStageCreateInfo>& pipelineShaders);