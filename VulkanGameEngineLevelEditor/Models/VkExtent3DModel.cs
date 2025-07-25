using Silk.NET.Vulkan;
using System;
using System.ComponentModel;
using System.Globalization;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using Vulkan;

namespace VulkanGameEngineLevelEditor.Models
{
    public unsafe class VkExtent3DModel : RenderPassEditorBaseModel
    {
        public uint _width { get; set; }
        public uint _height { get; set; }
        public uint _depth { get; set; }

        public VkExtent3DModel()
        {
        }
    }
}
