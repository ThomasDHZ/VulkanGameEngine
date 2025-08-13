#include "DLL.h"
#include "Typedef.h"
#include <vulkan/vulkan_core.h>

DLL_EXPORT VkAttachmentDescription Json_LoadAttachmentDescription(nlohmann::json json);
DLL_EXPORT VkImageCreateInfo Json_LoadImageCreateInfo(nlohmann::json json, ivec2 textureResolution);
DLL_EXPORT VkSamplerCreateInfo Json_LoadVulkanSamplerCreateInfo(nlohmann::json json);
DLL_EXPORT VkClearValue Json_LoadClearValue(nlohmann::json json);
DLL_EXPORT VkSubpassDependency Json_LoadSubpassDependency(nlohmann::json json);
DLL_EXPORT VkViewport Json_LoadViewPort(nlohmann::json json);
DLL_EXPORT VkOffset2D Json_LoadOffset2D(nlohmann::json json);
DLL_EXPORT VkExtent2D Json_LoadExtent2D(nlohmann::json json);
DLL_EXPORT VkRect2D Json_LoadRect2D(nlohmann::json json);
DLL_EXPORT VkVertexInputBindingDescription Json_LoadVertexInputBindingDescription(nlohmann::json json);
DLL_EXPORT VkVertexInputAttributeDescription Json_LoadVertexInputAttributeDescription(nlohmann::json json);
DLL_EXPORT VkPipelineColorBlendAttachmentState Json_LoadPipelineColorBlendAttachmentState(nlohmann::json json);
DLL_EXPORT VkPipelineColorBlendStateCreateInfo Json_LoadPipelineColorBlendStateCreateInfo(nlohmann::json json);
DLL_EXPORT VkPipelineRasterizationStateCreateInfo Json_LoadPipelineRasterizationStateCreateInfo(nlohmann::json json);
DLL_EXPORT VkPipelineMultisampleStateCreateInfo Json_LoadPipelineMultisampleStateCreateInfo(nlohmann::json json);
DLL_EXPORT VkPipelineDepthStencilStateCreateInfo Json_LoadPipelineDepthStencilStateCreateInfo(nlohmann::json json);
DLL_EXPORT VkPipelineInputAssemblyStateCreateInfo Json_LoadPipelineInputAssemblyStateCreateInfo(nlohmann::json json);
DLL_EXPORT VkDescriptorSetLayoutBinding Json_LoadLayoutBinding(nlohmann::json json);