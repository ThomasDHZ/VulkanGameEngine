using GlmSharp;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;


namespace VulkanGameEngineLevelEditor.Models
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct RenderPipelineLoaderModel
    {
        public Guid PipelineId { get; set; }
        public Guid RenderPassId { get; set; }
        public IntPtr RenderPass { get; set; } // VkRenderPass is an opaque handle
        public ivec2 RenderPassResolution { get; set; }
        public GPUIncludes gpuIncludes { get; set; }
        public ShaderPipelineData ShaderPiplineInfo { get; set; }
        public size_t ViewportCount { get; set; }
        public size_t ScissorCount { get; set; }
        public size_t PipelineColorBlendAttachmentStateCount { get; set; }
        public VkPipelineColorBlendAttachmentState* PipelineColorBlendAttachmentStateList { get; set; }
        public VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo { get; set; }
        public VkViewport* ViewportList { get; set; }
        public VkRect2D* ScissorList { get; set; }
        public VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo { get; set; }
        public VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo { get; set; }
        public VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo { get; set; }
        public VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfoModel { get; set; }

        public RenderPipelineLoaderModel()
        {
        }

        public RenderPipelineLoaderModel(RenderPipelineJsonLoaderModel model)
        {
            // Allocate native memory for arrays
            IntPtr colorBlendPtr = IntPtr.Zero;
            IntPtr viewportPtr = IntPtr.Zero;
            IntPtr scissorPtr = IntPtr.Zero;

            try
            {
                // Convert lists to native arrays
                if (model.PipelineColorBlendAttachmentStateList != null && model.PipelineColorBlendAttachmentStateList.Count > 0)
                {
                    PipelineColorBlendAttachmentStateCount = (size_t)model.PipelineColorBlendAttachmentStateList.Count;
                    colorBlendPtr = Marshal.AllocHGlobal((int)(PipelineColorBlendAttachmentStateCount * sizeof(VkPipelineColorBlendAttachmentState)));
                    for (int i = 0; i < model.PipelineColorBlendAttachmentStateList.Count; i++)
                    {
                        Marshal.StructureToPtr(model.PipelineColorBlendAttachmentStateList[i], colorBlendPtr + i * sizeof(VkPipelineColorBlendAttachmentState), false);
                    }
                    PipelineColorBlendAttachmentStateList = (VkPipelineColorBlendAttachmentState*)colorBlendPtr;
                }

                if (model.ViewportList != null && model.ViewportList.Count > 0)
                {
                    ViewportCount = (size_t)model.ViewportList.Count;
                    viewportPtr = Marshal.AllocHGlobal((int)(ViewportCount * sizeof(VkViewport)));
                    for (int i = 0; i < model.ViewportList.Count; i++)
                    {
                        Marshal.StructureToPtr(model.ViewportList[i], viewportPtr + i * sizeof(VkViewport), false);
                    }
                    ViewportList = (VkViewport*)viewportPtr;
                }

                if (model.ScissorList != null && model.ScissorList.Count > 0)
                {
                    ScissorCount = (size_t)model.ScissorList.Count;
                    scissorPtr = Marshal.AllocHGlobal((int)(ScissorCount * sizeof(VkRect2D)));
                    for (int i = 0; i < model.ScissorList.Count; i++)
                    {
                        Marshal.StructureToPtr(model.ScissorList[i], scissorPtr + i * sizeof(VkRect2D), false);
                    }
                    ScissorList = (VkRect2D*)scissorPtr;
                }

                PipelineId = model.PipelineId;
                PipelineInputAssemblyStateCreateInfo = model.PipelineInputAssemblyStateCreateInfo;
                PipelineColorBlendStateCreateInfoModel = model.PipelineColorBlendStateCreateInfoModel;
                PipelineDepthStencilStateCreateInfo = model.PipelineDepthStencilStateCreateInfo;
                PipelineRasterizationStateCreateInfo = model.PipelineRasterizationStateCreateInfo;
                PipelineMultisampleStateCreateInfo = model.PipelineMultisampleStateCreateInfo;
            }
            catch
            {
                // Clean up on failure
                if (colorBlendPtr != IntPtr.Zero) Marshal.FreeHGlobal(colorBlendPtr);
                if (viewportPtr != IntPtr.Zero) Marshal.FreeHGlobal(viewportPtr);
                if (scissorPtr != IntPtr.Zero) Marshal.FreeHGlobal(scissorPtr);
                throw;
            }
        }

        // Clean up native memory
        public void Dispose()
        {
            if (PipelineColorBlendAttachmentStateList != null)
            {
                Marshal.FreeHGlobal((IntPtr)PipelineColorBlendAttachmentStateList);
                PipelineColorBlendAttachmentStateList = null;
            }
            if (ViewportList != null)
            {
                Marshal.FreeHGlobal((IntPtr)ViewportList);
                ViewportList = null;
            }
            if (ScissorList != null)
            {
                Marshal.FreeHGlobal((IntPtr)ScissorList);
                ScissorList = null;
            }
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
