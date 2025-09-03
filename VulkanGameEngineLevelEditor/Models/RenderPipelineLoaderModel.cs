using GlmSharp;
using Silk.NET.SDL;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;


namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public unsafe class RenderPipelineLoaderModel
    {
        public string Name;
        public Guid PipelineId;
        public Guid RenderPassId;
        public List<String> ShaderList;
        public VkRenderPass RenderPass;
        public GPUIncludes gpuIncludes;
        public ShaderPushConstant PushConstant;
        public ShaderPipelineData ShaderPiplineInfo;
        public size_t ViewportCount = 0;
        public size_t ScissorCount = 0;
        public size_t PipelineColorBlendAttachmentStateCount = 0;
        public VkPipelineColorBlendAttachmentState* PipelineColorBlendAttachmentStateList = null;
        public VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo;
        public VkViewport* ViewportList = null;
        public VkRect2D* ScissorList = null;
        public VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo;
        public VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo;
        public VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo;
        public VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfoModel;
        public ivec2 RenderPassResolution;

        public RenderPipelineLoaderModel()
        {
        }
    }
}
