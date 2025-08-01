using Microsoft.CodeAnalysis;
using Microsoft.VisualBasic.Devices;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.Attributes;

namespace VulkanGameEngineLevelEditor.Models
{
    public struct VkVertexInputAttributeDescriptionModel
    {
        [Tooltip("Name of the vertex attribute for display or identification purposes.")]
        public string Name { get; set; }

        [Tooltip("Specifies the binding number from which the attribute data is sourced.")]
        public uint Binding { get; set; }

        [Tooltip("Specifies the shader location for the vertex attribute.")]
        public uint Location { get; set; }

        [Tooltip("Defines the data type of the vertex attribute for display or configuration.")]
        public string Type { get; set; }

        [Tooltip("Specifies the format of the vertex attribute data.")]
        public VkFormat Format { get; set; }

        [Tooltip("Sets the byte offset of the attribute within the vertex data.")]
        public uint Offset { get; set; }
    }
}
