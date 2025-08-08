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
    [Serializable]
    public unsafe class RenderPipelineLoaderModel : RenderPassEditorBaseModel
    {
        [Tooltip("Specifies the name of the render pipeline for identification.")]
        public string Name { get; set; } = string.Empty;

        [IgnoreProperty]
        [Tooltip("Unique identifier for the render pipeline.")]
        public Guid PipelineId { get; set; } = Guid.Empty;

        [DisplayName("Vertex Shader")]
        [ControlTypeAttribute(typeof(TypeOfFileLoader))]
        [Tooltip("Path to the vertex shader file.")]
        public string VertexShader { get; set; }

        [DisplayName("Pixel Shader")]
        [ControlTypeAttribute(typeof(TypeOfFileLoader))]
        [Tooltip("Path to the pixel (fragment) shader file.")]
        public string FragmentShader { get; set; }

        [DisplayName("Vertex Type")]
        [Tooltip("Defines the type of vertex data used by the pipeline.")]
        public VertexTypeEnum VertexType { get; set; }

        [DisplayName("Viewports")]
        [Tooltip("List of viewport configurations for the pipeline.")]
        public List<VkViewport> ViewportList { get; set; } = new List<VkViewport>();

        [DisplayName("Scissors")]
        [Tooltip("List of scissor rectangle configurations for the pipeline.")]
        public List<VkRect2D> ScissorList { get; set; } = new List<VkRect2D>();

        [DisplayName("Color Blend Attachments")]
        [Tooltip("List of color blend attachment states for the pipeline.")]
        public List<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStateList { get; set; } = new List<VkPipelineColorBlendAttachmentState>();

        [DisplayName("Color State Attachment")]
        [Tooltip("Configures the color blending state for the pipeline.")]
        public VkPipelineColorBlendStateCreateInfoModel PipelineColorBlendStateCreateInfoModel { get; set; } = new VkPipelineColorBlendStateCreateInfoModel();

        [DisplayName("Rasterization State")]
        [Tooltip("Configures the rasterization state for the pipeline.")]
        public VkPipelineRasterizationStateCreateInfoModel PipelineRasterizationStateCreateInfo { get; set; } = new VkPipelineRasterizationStateCreateInfoModel();

        [DisplayName("Multisample State")]
        [Tooltip("Configures the multisampling state for the pipeline.")]
        public VkPipelineMultisampleStateCreateInfoModel PipelineMultisampleStateCreateInfo { get; set; } = new VkPipelineMultisampleStateCreateInfoModel();

        [DisplayName("Depth Stencil State")]
        [Tooltip("Configures the depth and stencil state for the pipeline.")]
        public VkPipelineDepthStencilStateCreateInfoModel PipelineDepthStencilStateCreateInfo { get; set; }

        [DisplayName("Input Assembly State")]
        [Tooltip("Configures the input assembly state for vertex data in the pipeline.")]
        public VkPipelineInputAssemblyStateCreateInfoModel PipelineInputAssemblyStateCreateInfo { get; set; } = new VkPipelineInputAssemblyStateCreateInfoModel();

        [DisplayName("Pipeline Descriptors")]
        [Tooltip("List of descriptor configurations for the pipeline.")]
        public List<PipelineDescriptorModel> PipelineDescriptorModelsList { get; set; } = new List<PipelineDescriptorModel>();

        [DisplayName("Vertex Bindings")]
        [Tooltip("List of vertex input binding descriptions for the pipeline.")]
        public List<VkVertexInputBindingDescription> VertexInputBindingDescriptionList { get; set; } = new List<VkVertexInputBindingDescription>();

        [DisplayName("Vertex Attributes")]
        [Tooltip("List of vertex input attribute descriptions for the pipeline.")]
        public List<VkVertexInputAttributeDescription> VertexInputAttributeDescriptionList { get; set; } = new List<VkVertexInputAttributeDescription>();

        public RenderPipelineLoaderModel()
        {
        }
    }
}
