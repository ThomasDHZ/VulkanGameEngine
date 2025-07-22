using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Vulkan;

namespace VulkanGameEngineLevelEditor.GameEngine.Structs
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct VulkanRenderPass
    {

        public Guid RenderPassId { get; set; } = Guid.Empty;
        public VkSampleCountFlagBits SampleCount { get; set; } = VkSampleCountFlagBits.VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
        public VkRect2D RenderArea { get; set; } = new VkRect2D();
        public VkRenderPass RenderPass { get; set; } = new VkRenderPass();
        public VkFramebuffer* FrameBufferList { get; set; } = null;
        public VkClearValue* ClearValueList { get; set; } = null;
        public size_t FrameBufferCount { get; set; } = 0;
        public size_t ClearValueCount { get; set; } = 0;
        public VkCommandBuffer CommandBuffer { get; set; }
        public bool IsRenderedToSwapchain { get; set; }
        public VulkanRenderPass()
        {
        }
    };
}
