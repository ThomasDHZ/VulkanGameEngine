using GlmSharp;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanCS;
using VulkanEngineCoreCS.Models;

namespace VulkanEngineCS.Models
{
    [Serializable]
    public class RenderPassLoader
    {
        public string Name = string.Empty;
        public Guid RenderPassId { get; set; } = new Guid();
        public ivec2 RenderPassResolution { get; set; } = new ivec2(0);
        public List<RenderPassAttachmentLoader> AttachmentList { get; set; } = new List<RenderPassAttachmentLoader>();
        public List<VkSubpassDependency> SubpassDependencyList { get; set; } = new List<VkSubpassDependency>();
        List<string> PipelineList { get; set; } = new List<string>();
        public List<VulkanPipelinePackageLoader> PipelinePackageList { get; set; } = new List<VulkanPipelinePackageLoader>();
        public List<List<VulkanSubPassLoader>> SubPassList { get; set; } = new List<List<VulkanSubPassLoader>>();
        public List<ShaderLoader> ShaderList { get; set; } = new List<ShaderLoader>();
        public List<VkClearValue> ClearValueList { get; set; } = new List<VkClearValue>();
        public VkSampleCountFlagBits SampleCount { get; set; } = VkSampleCountFlagBits.VK_SAMPLE_COUNT_1_BIT;
        public bool UseDefaultRenderResolution { get; set; } = true;
        public bool UseGlobalBindlessSet { get; set; } = false;
        public bool UseVkMultiview { get; set; } = false;
        public bool RenderAsCubemap { get; set; } = false;
        public bool UseFrameBufferResolution { get; set; } = true;
        public bool OneTimeDraw = false;
    }
}
