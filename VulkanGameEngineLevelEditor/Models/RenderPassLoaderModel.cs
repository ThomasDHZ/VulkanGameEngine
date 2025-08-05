using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.InteropServices;
using System.Xml.Linq;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;


namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public unsafe class RenderPassLoaderModel : RenderPassEditorBaseModel, INotifyPropertyChanged
    {
        [Tooltip("Specifies the name of the render pass for identification.")]
        public string Name { get; set; } = string.Empty;

        [IgnoreProperty]
        [Tooltip("Unique identifier for the render pass.")]
        public Guid RenderPassId { get; set; } = Guid.NewGuid();

        [IgnoreProperty]
        [DisplayName("Pipeline List")]
        [Tooltip("List of pipeline names associated with the render pass.")]
        public List<string> RenderPipelineList { get; set; } = new List<string>();

        [IgnoreProperty]
        [DisplayName("Input Texture List")]
        [Tooltip("List of unique identifiers for input textures used by the render pass.")]
        public List<Guid> InputTextureList { get; set; } = new List<Guid>();

        [JsonIgnore]
        [DisplayName("Pipeline List")]
        [Tooltip("List of pipeline configuration models for the render pass.")]
        public List<RenderPipelineLoaderModel> renderPipelineModelList { get; set; } = new List<RenderPipelineLoaderModel>();

        [DisplayName("Render Pass Output Images")]
        [Tooltip("List of texture information models defining output images for the render pass.")]
        public List<RenderedTextureInfoModel> RenderedTextureInfoModelList { get; set; } = new List<RenderedTextureInfoModel>();

        [DisplayName("Subpass Dependencies")]
        [Tooltip("List of subpass dependency configurations for the render pass.")]
        public List<VkSubpassDependencyModel> SubpassDependencyList { get; set; } = new List<VkSubpassDependencyModel>();

        [UIList]
        [DisplayName("Clear Value List")]
        [Tooltip("List of clear values for attachments in the render pass.")]
        public List<VkClearValue> ClearValueList { get; set; } = new List<VkClearValue>();

        [DisplayName("Render Area Size")]
        [Tooltip("Defines the render area dimensions for the render pass.")]
        public RenderAreaModel RenderArea { get; set; } = new RenderAreaModel();

        [DisplayName("Render to SwapChain")]
        [Tooltip("Indicates whether the render pass outputs to the swapchain.")]
        public bool IsRenderedToSwapchain { get; set; } = false;

        public RenderPassLoaderModel()
        {
        }
    }
}
