using GlmSharp;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
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
        public Guid PipelineId { get; set; }
        public Guid RenderPassId { get; set; }
        public VkRenderPass RenderPass { get; set; }
        public GPUIncludes gpuIncludes { get; set; }
        public ShaderPushConstant PushConstant { get; set; }
        public ShaderPipelineData ShaderPiplineInfo { get; set; }
        public size_t ViewportCount { get; set; } = 0;
        public size_t ScissorCount { get; set; } = 0;
        public size_t PipelineColorBlendAttachmentStateCount { get; set; } = 0;
        public VkPipelineColorBlendAttachmentState* PipelineColorBlendAttachmentStateList { get; set; } = null;
        public VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo { get; set; }
        public VkViewport* ViewportList { get; set; } = null;
        public VkRect2D* ScissorList { get; set; } = null;
        public VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo { get; set; }
        public VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo { get; set; }
        public VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo { get; set; }
        public VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfoModel { get; set; }
        public ivec2 RenderPassResolution { get; set; }

        public RenderPipelineLoaderModel()
        {
        }

        public RenderPipelineLoaderModel(RenderPipelineJsonLoaderModel model)
        {
            ListPtr<VkViewport> ViewportJsonList = new ListPtr<VkViewport>(model.ViewportList);
            ListPtr<VkRect2D> ScissorJsonList = new ListPtr<VkRect2D>(model.ScissorList);
            ListPtr<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStateJsonList = new ListPtr<VkPipelineColorBlendAttachmentState>(model.PipelineColorBlendAttachmentStateList);

            PipelineColorBlendAttachmentStateCount = PipelineColorBlendAttachmentStateJsonList.Count;
            PipelineColorBlendAttachmentStateList = PipelineColorBlendAttachmentStateJsonList.Ptr;
            PipelineInputAssemblyStateCreateInfo = model.PipelineInputAssemblyStateCreateInfo;
            PipelineColorBlendStateCreateInfoModel = model.PipelineColorBlendStateCreateInfoModel;
            PipelineDepthStencilStateCreateInfo = model.PipelineDepthStencilStateCreateInfo;
            PipelineRasterizationStateCreateInfo = model.PipelineRasterizationStateCreateInfo;
            PipelineId = model.PipelineId;
            PipelineMultisampleStateCreateInfo = model.PipelineMultisampleStateCreateInfo;
            ViewportCount = ViewportJsonList.Count;
            ViewportList = ViewportJsonList.Ptr;
            ScissorCount = ScissorJsonList.Count;
            ScissorList = ScissorJsonList.Ptr;
        }
    }

    public class RenderPipelineJsonLoaderModel
    {
        public Guid PipelineId { get; set; }
        public List<string> ShaderList { get; set; } = new List<string>();
        public List<VkViewport> ViewportList { get; set; } = new List<VkViewport>();
        public List<VkRect2D> ScissorList { get; set; } = new List<VkRect2D>();
        public List<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStateList { get; set; }
        public VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfoModel { get; set; }
        public VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo { get; set; }
        public VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo { get; set; }
        public VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo { get; set; }
        public VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo { get; set; }
    }
}
