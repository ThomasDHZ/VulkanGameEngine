﻿using GlmSharp;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public class RenderPassBuildInfoModel : RenderPassEditorBaseModel
    {
        public ivec2 SwapChainResuloution { get; set; } = new ivec2();
        public List<RenderPipeline> RenderPipelineList { get; set; } = new List<RenderPipeline>();

        public List<RenderedTextureInfoModel> RenderedTextureInfoModelList = new List<RenderedTextureInfoModel>();
        public List<SubpassDependencyModel> SubpassDependencyList { get; set; } = new List<SubpassDependencyModel>();

        public RenderPassBuildInfoModel() 
        {
        }

        public RenderPassBuildInfoModel(string jsonFilePath) : base()
        {
            LoadJsonComponent(jsonFilePath);
        }

        public RenderPassBuildInfoModel(string name, string jsonFilePath) : base(name)
        {
           // LoadJsonComponent(@"C:\Users\dotha\Documents\GitHub\VulkanGameEngine\RenderPass\RenderPass\DefaultSubpassDependency.json");
        }

        public void ConvertToVulkan()
        {
            throw new NotImplementedException();
        }

        public void LoadJsonComponent(string jsonPath)
        {
            var obj = base.LoadJsonComponent<RenderPassBuildInfoModel>(jsonPath);
            foreach (PropertyInfo property in typeof(RenderPassBuildInfoModel).GetProperties())
            {
                if (property.CanWrite)
                {
                    property.SetValue(this, property.GetValue(obj));
                }
            }
        }
    }
}
