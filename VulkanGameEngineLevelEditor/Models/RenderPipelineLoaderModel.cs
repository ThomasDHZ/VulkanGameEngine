using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Reflection;
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
        public int RenderPipelineId { get; set; } = 0;
        [DisplayName("Vertex Shader")]
        [ControlTypeAttribute(typeof(FileLoaderForm))]
        public String VertexShader { get; set; }
        [DisplayName("Pixel Shader")]
        [ControlTypeAttribute(typeof(FileLoaderForm))]
        public String FragmentShader { get; set; }
        [DisplayName("Descriptor Set Count")]
        public size_t DescriptorSetCount { get; set; }
        [DisplayName("Descriptor Set Layout Count")]
        public size_t DescriptorSetLayoutCount { get; set; }
        [DisplayName("Vertex Type")]
        public VertexTypeEnum VertexType { get; set; }
        [DisplayName("Viewports")]
        public List<VkViewport> ViewportList { get; set; } = new List<VkViewport>();
        [DisplayName("Scissors")]
        public List<VkRect2D> ScissorList { get; set; } = new List<VkRect2D>();
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
        [DisplayName("Layout Bindings")]
        public List<VkDescriptorSetLayoutBindingModel> LayoutBindingList { get; set; } = new List<VkDescriptorSetLayoutBindingModel>();
        [DisplayName("Pipeline Descriptors")]
        public List<PipelineDescriptorModel> PipelineDescriptorModelsList { get; set; } = new List<PipelineDescriptorModel>();
        [DisplayName("Vertex Bindings")]
        public List<VkVertexInputBindingDescription> VertexInputBindingDescriptionList { get; set; } = new List<VkVertexInputBindingDescription>();
        [DisplayName("Vertex Attributes")]
        public List<VkVertexInputAttributeDescription> VertexInputAttributeDescriptionList { get; set; } = new List<VkVertexInputAttributeDescription>();

        public RenderPipelineLoaderModel()
        {
        }

        public RenderPipelineLoaderModel(string name) : base()
        {
            Name = name;
        }

        public int UiPropertiesControls(object obj, int xPosition, int yOffset, int width)
        {
            foreach (var prop in obj.GetType().GetProperties())
            {
                if (prop.GetCustomAttributes(typeof(IgnorePropertyAttribute), true).FirstOrDefault() as IgnorePropertyAttribute != null)
                {
                    continue;
                }
                var readOnlyAttribute = prop.GetCustomAttributes(typeof(ReadOnlyAttribute), true).FirstOrDefault() as ReadOnlyAttribute;
                bool readOnly = readOnlyAttribute?.IsReadOnly ?? false;

                System.Windows.Forms.Label label = new System.Windows.Forms.Label { Text = prop.Name, Location = new Point(5, yOffset), AutoSize = true, ForeColor = Color.White };
            }
            return yOffset;
        }
    }
}
