﻿using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using VulkanGameEngineLevelEditor.GameEngineAPI;


namespace VulkanGameEngineLevelEditor.Models
{
    [Serializable]
    public unsafe class RenderPassBuildInfoModel : RenderPassEditorBaseModel
    {
        public List<string> RenderPipelineList { get; set; } = new List<string>();

        public List<RenderedTextureInfoModel> RenderedTextureInfoModelList = new List<RenderedTextureInfoModel>();
        public List<VkSubpassDependencyModel> SubpassDependencyList { get; set; } = new List<VkSubpassDependencyModel>();
        public List<VkClearValue> ClearValueList { get; set; } = new List<VkClearValue>();
        public RenderAreaModel RenderArea { get; set; }
        public bool IsRenderedToSwapchain { get; set; }

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

        //public RenderPassBuildInfoDLL ToDLL()
        //{
        //    var renderPipelineArray = new IntPtr[RenderPipelineList.Count];
        //    for (int x = 0; x < RenderPipelineList.Count; x++)
        //    {
        //        renderPipelineArray[x] = Marshal.StringToHGlobalAnsi(RenderPipelineList[x]);
        //    }

        //    var renderedTextureInfoDLLList = new List<RenderedTextureInfoDLL>();
        //    foreach (var renderedTextureInfoModel in RenderedTextureInfoModelList)
        //    {
        //        renderedTextureInfoDLLList.Add(renderedTextureInfoModel.ToDLL());
        //    }
        //    var renderedTextureInfoArray = renderedTextureInfoDLLList.ToArray();

        //    var subpassDependencyDLLList = new List<VkSubpassDependency>();
        //    foreach (var subpassDependencyModel in SubpassDependencyList)
        //    {
        //        subpassDependencyDLLList.Add(subpassDependencyModel.Convert());
        //    }
        //    VkSubpassDependency[] subpassArray = subpassDependencyDLLList.ToArray();

        //    GCHandle renderPipelineHandle = GCHandle.Alloc(renderPipelineArray, GCHandleType.Pinned);
        //    GCHandle textureInfoHandle = GCHandle.Alloc(renderedTextureInfoArray, GCHandleType.Pinned);
        //    GCHandle subpassHandle = GCHandle.Alloc(subpassArray, GCHandleType.Pinned);
        //    fixed (byte* namePtr = System.Text.Encoding.UTF8.GetBytes(_name + "\0"))
        //    fixed (VkClearValue* clearColorPtr = ClearValueList.ToArray())
        //    {
        //        return new RenderPassBuildInfoDLL
        //        {
        //            // Name = (IntPtr)namePtr,
        //            IsRenderedToSwapchain = IsRenderedToSwapchain,
        //            RenderedTextureInfoModelList = (RenderedTextureInfoDLL*)textureInfoHandle.AddrOfPinnedObject(),
        //            RenderedTextureInfoModeCount = RenderedTextureInfoModelList.UCount(),
        //            ClearValueCount = ClearValueList.UCount(),
        //            // RenderPipelineList = (IntPtr*)renderPipelineHandle.AddrOfPinnedObject(),
        //            SubpassDependencyCount = SubpassDependencyList.UCount(),
        //            SubpassDependencyList = (VkSubpassDependency*)subpassHandle.AddrOfPinnedObject(),
        //            ClearValueList = clearColorPtr,
        //            RenderArea = RenderArea
        //        };
        //    }
        //}

        public void LoadJsonComponent(string jsonPath)
        {
            var obj = base.LoadJsonComponent<RenderPassBuildInfoModel>(jsonPath);
            RenderPipelineList = obj.RenderPipelineList;
            RenderedTextureInfoModelList = obj.RenderedTextureInfoModelList;
            SubpassDependencyList = obj.SubpassDependencyList;
            IsRenderedToSwapchain = obj.IsRenderedToSwapchain;
        }
    }
}
