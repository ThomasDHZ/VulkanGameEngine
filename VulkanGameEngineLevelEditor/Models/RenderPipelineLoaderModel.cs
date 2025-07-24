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
        public int RenderPipelineId = 0;
        [ControlTypeAttribute(typeof(TypeOfFileLoader))]
        public String VertexShader { get; set; }
        [ControlTypeAttribute(typeof(TypeOfFileLoader))]
        public String FragmentShader { get; set; }
        public size_t DescriptorSetCount { get; set; }
        public size_t DescriptorSetLayoutCount { get; set; }
        public VertexTypeEnum VertexType { get; set; }
        public List<VkViewport> ViewportList { get; set; } = new List<VkViewport>();
        public List<VkRect2D> ScissorList { get; set; } = new List<VkRect2D>();
        public List<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStateList { get; set; } = new List<VkPipelineColorBlendAttachmentState>();
        public VkPipelineColorBlendStateCreateInfoModel PipelineColorBlendStateCreateInfoModel { get; set; } = new VkPipelineColorBlendStateCreateInfoModel();
        public VkPipelineRasterizationStateCreateInfoModel PipelineRasterizationStateCreateInfo { get; set; } = new VkPipelineRasterizationStateCreateInfoModel();
        public VkPipelineMultisampleStateCreateInfoModel PipelineMultisampleStateCreateInfo { get; set; } = new VkPipelineMultisampleStateCreateInfoModel();
        public VkPipelineDepthStencilStateCreateInfoModel PipelineDepthStencilStateCreateInfo { get; set; }
        public VkPipelineInputAssemblyStateCreateInfoModel PipelineInputAssemblyStateCreateInfo { get; set; } = new VkPipelineInputAssemblyStateCreateInfoModel();
        public List<VkDescriptorSetLayoutBindingModel> LayoutBindingList { get; set; } = new List<VkDescriptorSetLayoutBindingModel>();
        public List<PipelineDescriptorModel> PipelineDescriptorModelsList { get; set; } = new List<PipelineDescriptorModel>();
        public List<VkVertexInputBindingDescription> VertexInputBindingDescriptionList { get; set; } = new List<VkVertexInputBindingDescription>();
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
