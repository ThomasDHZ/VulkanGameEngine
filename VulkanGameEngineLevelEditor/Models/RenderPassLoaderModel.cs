using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Xml.Linq;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngineAPI;


namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public unsafe class RenderPassLoaderModel : RenderPassEditorBaseModel
    {
        public string Name { get; set; } = string.Empty;
        [IgnoreProperty]
        public Guid RenderPassId { get; set; } = Guid.NewGuid();
        [IgnoreProperty]
        [DisplayName("Pipeline List")]
        public List<string> RenderPipelineList { get; set; } = new List<string>();
        [IgnoreProperty]
        [DisplayName("Input Texture List")]
        public List<Guid> InputTextureList { get; set; } = new List<Guid>();
        [DisplayName("Pipeline List")]
        public List<RenderPipelineLoaderModel> renderPipelineModelList { get; set; } = new List<RenderPipelineLoaderModel>();
        [DisplayName("Render Pass Output Images")]
        public List<RenderedTextureInfoModel> RenderedTextureInfoModelList { get; set; } = new List<RenderedTextureInfoModel>();
        [DisplayName("Subpass Dependencies")]
        public List<VkSubpassDependencyModel> SubpassDependencyList { get; set; } = new List<VkSubpassDependencyModel>();
        [DisplayName("Clear Value List")]
        public List<VkClearValue> ClearValueList { get; set; } = new List<VkClearValue>();
        [DisplayName("Render Area Size")]
        public RenderAreaModel RenderArea { get; set; } = new RenderAreaModel();
        [DisplayName("Render to SwapChain")]
        public bool IsRenderedToSwapchain { get; set; } = false;

        public RenderPassLoaderModel()
        {
        }

        public RenderPassLoaderModel(string jsonFilePath) : base()
        {
            LoadJsonComponent(jsonFilePath);
        }

        public RenderPassLoaderModel(string name, string jsonFilePath) : base() 
        {
            Name = name;
            LoadJsonComponent(@"C:\Users\dotha\Documents\GitHub\VulkanGameEngine\RenderPass\RenderPass\DefaultSubpassDependency.json");
        }

        public void LoadJsonComponent(string jsonPath)
        {
            var obj = base.LoadJsonComponent<RenderPassLoaderModel>(jsonPath);
            RenderPipelineList = obj.RenderPipelineList;
            RenderedTextureInfoModelList = obj.RenderedTextureInfoModelList;
            SubpassDependencyList = obj.SubpassDependencyList;
            IsRenderedToSwapchain = obj.IsRenderedToSwapchain;
        }
    }
}
