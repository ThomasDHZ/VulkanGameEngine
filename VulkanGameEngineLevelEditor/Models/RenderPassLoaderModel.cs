using System;
using System.Collections.Generic;
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
        public Guid RenderPassId { get; set; } = Guid.NewGuid();
        [IgnoreProperty]
        public List<string> RenderPipelineList { get; set; } = new List<string>();
        [IgnoreProperty]
        public List<Guid> InputTextureList { get; set; } = new List<Guid>();
        [IgnoreProperty]
        public List<RenderPipelineLoaderModel> renderPipelineModelList { get; set; } = new List<RenderPipelineLoaderModel>();
        [IgnoreProperty]
        public List<RenderedTextureInfoModel> RenderedTextureInfoModelList { get; set; } = new List<RenderedTextureInfoModel>();
        [IgnoreProperty]
        public List<VkSubpassDependencyModel> SubpassDependencyList { get; set; } = new List<VkSubpassDependencyModel>();
        [IgnoreProperty]
        public List<VkClearValue> ClearValueList { get; set; } = new List<VkClearValue>();
        [IgnoreProperty]
        public RenderAreaModel RenderArea { get; set; } = new RenderAreaModel();
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
