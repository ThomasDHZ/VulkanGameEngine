#pragma once
#include "VulkanRenderer.h"
#include "ShaderCompiler.h"

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

struct RenderPipelineLoader
{
    VkGuid PipelineId; 
    String VertexShaderPath;
    String FragmentShaderPath;
    Vector<VkViewport> ViewportList;
    Vector<VkRect2D> ScissorList;
    VertexTypeEnum VertexType;
    VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfoModel;
    VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo;
    VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo;
    VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo;
    VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo;
    Vector<PipelineDescriptorModel> PipelineDescriptorModelsList;
    Vector<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStateList;
    Vector<VkVertexInputBindingDescription> VertexInputBindingDescriptionList;
    Vector<VkVertexInputAttributeDescription> VertexInputAttributeDescriptionList;
};

#ifdef __cplusplus
extern "C" {
#endif
DLL_EXPORT VulkanPipeline VulkanPipeline_CreateRenderPipeline(VkDevice device, VkGuid& renderPassId, const char* pipelineJson, VkRenderPass renderPass, ShaderPushConstant& pushConstant, ivec2& renderPassResolution, const GPUIncludes& includes);
DLL_EXPORT VulkanPipeline VulkanPipeline_RebuildSwapChain(VkDevice device, VkGuid& renderPassId, VulkanPipeline& oldVulkanPipeline, const char* pipelineJson, VkRenderPass renderPass, ShaderPushConstant& pushConstant, ivec2& renderPassResolution, const GPUIncludes& includes);
DLL_EXPORT void VulkanPipeline_Destroy(VkDevice device, VulkanPipeline& vulkanPipelineDLL);
#ifdef __cplusplus
}
#endif

VkDescriptorPool Pipeline_CreatePipelineDescriptorPool(VkDevice device, const RenderPipelineLoader& model, const GPUIncludes& includes);
Vector<VkDescriptorSetLayout> Pipeline_CreatePipelineDescriptorSetLayout(VkDevice device, const RenderPipelineLoader& model, const GPUIncludes& includes);
Vector<VkDescriptorSet> Pipeline_AllocatePipelineDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, const RenderPipelineLoader& model, const Vector<VkDescriptorSetLayout>& descriptorSetLayoutList);
void Pipeline_UpdatePipelineDescriptorSets(VkDevice device, const Vector<VkDescriptorSet>& descriptorSetList, const RenderPipelineLoader& model, const GPUIncludes& includes);
VkPipelineLayout Pipeline_CreatePipelineLayout(VkDevice device, const Vector<VkDescriptorSetLayout>& descriptorSetLayoutList, ShaderPushConstant& pushConstant);
VkPipeline Pipeline_CreatePipeline(VkDevice device, VkRenderPass renderpass, VkPipelineLayout pipelineLayout, VkPipelineCache pipelineCache, const RenderPipelineLoader& model, ivec2& extent);
