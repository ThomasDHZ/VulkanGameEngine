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
        public List<VkVertexInputAttributeDescriptionModel> VertexInputAttributeDescriptionList { get; set; } = new List<VkVertexInputAttributeDescriptionModel>();

        public RenderPipelineLoaderModel()
        {
        }

        public RenderPipelineLoaderModel(string name, string vertexShader, string pixelShader) : base()
        {
            Name = name;
        }

        private void VertexAttrbutes(string vertexShader)
        {
        //    var attributes = new List<VkVertexInputAttributeDescriptionModel>();
        //    var regex = new Regex(@"layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s*in\s+([^\s]+)\s+([^\s;]+)\s*;", RegexOptions.Compiled);

        //    try
        //    {
        //        var vertexShaderTextLines = File.ReadLines(vertexShaderFile);
        //        foreach (var line in vertexShaderTextLines)
        //        {
        //            var trimmedLine = line.Trim();
        //            if (string.IsNullOrEmpty(trimmedLine) ||
        //                trimmedLine.StartsWith("//") ||
        //                trimmedLine.StartsWith("/*"))
        //            {
        //                continue;
        //            }

        //            var match = regex.Match(trimmedLine);
        //            if (match.Success)
        //            {
        //                uint location = uint.Parse(match.Groups[1].Value);
        //                string shaderType = match.Groups[2].Value;
        //                string varName = match.Groups[3].Value;

        //                var (format, size, locationCount) = MapShaderTypeToVulkanFormat(shaderType);
        //                for (uint x = 0; x < locationCount; x++)
        //                {
        //                    attributes.Add(new VkVertexInputAttributeDescriptionModel
        //                    {
        //                        Location = location + x,
        //                        Type = shaderType,
        //                        Name = varName,
        //                        Format = format,
        //                        Size = size / locationCount
        //                    });
        //                }
        //            }
        //        }
        //    }
        //    catch (Exception ex)
        //    {
        //        MessageBox.Show($"Error loading vertex layout from {vertexShaderFile}: {ex.Message}");
        //        throw;
        //    }
        }
    }
}
