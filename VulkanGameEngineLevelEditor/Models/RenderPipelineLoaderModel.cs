using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;


namespace VulkanGameEngineLevelEditor.Models
{
    public unsafe class RenderPipelineLoaderModel : RenderPassEditorBaseModel
    {

        public string Name { get; set; } = string.Empty;

        [IgnoreProperty()]
        public Guid RenderPipelineId { get; set; } = Guid.Empty;

        [DisplayName("Vertex Shader")]
        [ControlTypeAttribute(typeof(TypeOfFileLoader))]
        public String VertexShader { get; set; }

        [DisplayName("Pixel Shader")]
        [ControlTypeAttribute(typeof(TypeOfFileLoader))]
        public String FragmentShader { get; set; }
 
        [DisplayName("Vertex Shader Source")]
        [ControlTypeAttribute(typeof(TypeOfFileLoader))]
        public String VertexShaderSrc { get; set; }

        [DisplayName("Pixel Shader Source")]
        [ControlTypeAttribute(typeof(TypeOfFileLoader))]
        public String FragmentShaderSrc { get; set; }

        [DisplayName("Vertex Type")]
        public VertexTypeEnum VertexType { get; set; }

        [UIList]
        [DisplayName("Viewports")]
        public List<VkViewport> ViewportList { get; set; } = new List<VkViewport>();

        [UIList]
        [DisplayName("Scissors")]
        public List<VkRect2D> ScissorList { get; set; } = new List<VkRect2D>();

        [UIList]
        [DisplayName("Color Blend Attachments")]
        public List<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStateList { get; set; } = new List<VkPipelineColorBlendAttachmentState>();

        [DisplayName("Color State Attachment")]
        public VkPipelineColorBlendStateCreateInfoModel PipelineColorBlendStateCreateInfoModel { get; set; } = new VkPipelineColorBlendStateCreateInfoModel();

        [DisplayName("Rasterization State")]
        public VkPipelineRasterizationStateCreateInfoModel PipelineRasterizationStateCreateInfo { get; set; } = new VkPipelineRasterizationStateCreateInfoModel();

        [DisplayName("Multisample State")]
        public VkPipelineMultisampleStateCreateInfoModel PipelineMultisampleStateCreateInfo { get; set; } = new VkPipelineMultisampleStateCreateInfoModel();

        [DisplayName("Depth Stencil State")]
        public VkPipelineDepthStencilStateCreateInfoModel PipelineDepthStencilStateCreateInfo { get; set; }

        [DisplayName("Input Assembly State")]
        public VkPipelineInputAssemblyStateCreateInfoModel PipelineInputAssemblyStateCreateInfo { get; set; } = new VkPipelineInputAssemblyStateCreateInfoModel();

        [UIList]
        [DisplayName("Layout Bindings")]
        public List<VkDescriptorSetLayoutBindingModel> LayoutBindingList { get; set; } = new List<VkDescriptorSetLayoutBindingModel>();

        [UIList]
        [DisplayName("Pipeline Descriptors")]
        public List<PipelineDescriptorModel> PipelineDescriptorModelsList { get; set; } = new List<PipelineDescriptorModel>();

        [UIList]
        [DisplayName("Vertex Bindings")]
        public List<VkVertexInputBindingDescription> VertexInputBindingDescriptionList { get; set; } = new List<VkVertexInputBindingDescription>();

        [UIList]
        [DisplayName("Vertex Attributes")]
        public List<VkVertexInputAttributeDescriptionModel> VertexInputAttributeDescriptionList { get; set; } = new List<VkVertexInputAttributeDescriptionModel>();

        public RenderPipelineLoaderModel()
        {
        }

        public RenderPipelineLoaderModel(string name, string vertexShader, string pixelShader) : base()
        {
            Name = name;
        }
    }
}
