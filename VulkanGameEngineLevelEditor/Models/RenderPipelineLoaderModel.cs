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
                if (model.PipelineColorBlendAttachmentStateList != null && model.PipelineColorBlendAttachmentStateCount > 0)
                {
                    PipelineColorBlendAttachmentStateCount = (size_t)model.PipelineColorBlendAttachmentStateCount;
                    colorBlendPtr = Marshal.AllocHGlobal((int)(PipelineColorBlendAttachmentStateCount * sizeof(VkPipelineColorBlendAttachmentState)));
                    for (int i = 0; i < model.PipelineColorBlendAttachmentStateCount; i++)
                    {
                        Marshal.StructureToPtr(model.PipelineColorBlendAttachmentStateList[i], colorBlendPtr + i * sizeof(VkPipelineColorBlendAttachmentState), false);
                    }
                    PipelineColorBlendAttachmentStateList = (VkPipelineColorBlendAttachmentState*)colorBlendPtr;
                }

                if (model.ViewportList != null && model.ViewportCount > 0)
                {
                    ViewportCount = (size_t)model.ViewportCount;
                    viewportPtr = Marshal.AllocHGlobal((int)(ViewportCount * sizeof(VkViewport)));
                    for (int i = 0; i < model.ViewportCount; i++)
                    {
                        Marshal.StructureToPtr(model.ViewportList[i], viewportPtr + i * sizeof(VkViewport), false);
                    }
                    ViewportList = (VkViewport*)viewportPtr;
                }

                if (model.ScissorList != null && model.ScissorCount > 0)
                {
                    ScissorCount = (size_t)model.ScissorCount;
                    scissorPtr = Marshal.AllocHGlobal((int)(ScissorCount * sizeof(VkRect2D)));
                    for (int i = 0; i < model.ScissorCount; i++)
                    {
                        Marshal.StructureToPtr(model.ScissorCount, scissorPtr + i * sizeof(VkRect2D), false);
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

    public unsafe class RenderPipelineJsonLoaderModel
    {
        public Guid PipelineId { get; set; } = Guid.Empty;
        public Guid RenderPassId { get; set; } = Guid.Empty;
        public VkRenderPass RenderPass { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public ivec2 RenderPassResolution { get; set; } = new ivec2();
        public GPUIncludes gpuIncludes { get; set; } = new GPUIncludes();
        public ShaderPipelineData ShaderPiplineInfo { get; set; } = new ShaderPipelineData();
        public size_t ViewportCount { get; set; } = 0;
        public size_t ScissorCount { get; set; } = 0;
        public size_t PipelineColorBlendAttachmentStateCount { get; set; } = 0;
        public VkPipelineColorBlendAttachmentState* PipelineColorBlendAttachmentStateList { get; set; } = null;
        public VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo { get; set; } = new VkPipelineInputAssemblyStateCreateInfo();
        public VkViewport* ViewportList { get; set; } = null;
        public VkRect2D* ScissorList { get; set; } = null;
        public VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo { get; set; } = new VkPipelineRasterizationStateCreateInfo();
        public VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo { get; set; } = new VkPipelineMultisampleStateCreateInfo();
        public VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo { get; set; } = new VkPipelineDepthStencilStateCreateInfo();
        public VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfoModel { get; set; } = new VkPipelineColorBlendStateCreateInfo();
    }
}
