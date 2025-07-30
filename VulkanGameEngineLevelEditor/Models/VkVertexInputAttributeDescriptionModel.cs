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

namespace VulkanGameEngineLevelEditor.Models
{
    public struct VkVertexInputAttributeDescriptionModel
    {
        public string Name { get; set; }
        public uint Binding { get; set; }
        public uint Location { get; set; }
        public string Type { get; set; }
        public VkFormat Format { get; set; }
        public uint Offset { get; set; }
    }

    //public class VkVertexInputAttributeDescriptionModelLoader
    //{
    //    public List<VkVertexInputAttributeDescriptionModel> vertexInputList { get; set; } = new List<VkVertexInputAttributeDescriptionModel>();
    //    public VkVertexInputAttributeDescriptionModelLoader()
    //    {

    //    }

    //    public void LoadVertexLayout(string vertexShaderFile)
    //    {
    //        var attributes = new List<VkVertexInputAttributeDescriptionModel>();
    //        var regex = new Regex(@"layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s*in\s+([^\s]+)\s+([^\s;]+)\s*;", RegexOptions.Compiled);

    //        try
    //        {
    //            var vertexShaderTextLines = File.ReadLines(vertexShaderFile);
    //            foreach (var line in vertexShaderTextLines)
    //            {
    //                var trimmedLine = line.Trim();
    //                if (string.IsNullOrEmpty(trimmedLine) || 
    //                    trimmedLine.StartsWith("//") || 
    //                    trimmedLine.StartsWith("/*"))
    //                {
    //                    continue;
    //                }

    //                var match = regex.Match(trimmedLine);
    //                if (match.Success)
    //                {
    //                    uint location = uint.Parse(match.Groups[1].Value);
    //                    string shaderType = match.Groups[2].Value;
    //                    string varName = match.Groups[3].Value;

    //                    var (format, size, locationCount) = MapShaderTypeToVulkanFormat(shaderType);
    //                    for (uint x = 0; x < locationCount; x++)
    //                    {
    //                        attributes.Add(new VkVertexInputAttributeDescriptionModel
    //                        {
    //                            Location = location + x,
    //                            Type = shaderType,
    //                            Name = varName,
    //                            Format = format,
    //                            Size = size / locationCount 
    //                        });
    //                    }
    //                }
    //            }
  
    //            //uint stride = 0;
    //            //foreach (var attribute in attributes)
    //            //{
    //            //    stride += attribute.Size;
    //            //}

    //            //var bindingDesc = new VkVertexInputBindingDescription
    //            //{
    //            //    binding = 0, 
    //            //    stride = stride,
    //            //    inputRate = VkVertexInputRate.VK_VERTEX_INPUT_RATE_VERTEX
    //            //};

    //            //var attribDescs = new VkVertexInputAttributeDescription[attributes.Count];
    //            //uint offset = 0;
    //            //for (int i = 0; i < attributes.Count; i++)
    //            //{
    //            //    attribDescs[i] = new VkVertexInputAttributeDescription
    //            //    {
    //            //        binding = 0,
    //            //        location = attributes[i].Location,
    //            //        format = attributes[i].Format,
    //            //        offset = offset
    //            //    };
    //            //    offset += attributes[i].Size;
    //            //}

    //            //return (bindingDesc, attribDescs);
    //        }
    //        catch (Exception ex)
    //        {
    //            MessageBox.Show($"Error loading vertex layout from {vertexShaderFile}: {ex.Message}");
    //            throw;
    //        }
    //    }


    //    private (VkFormat Format, uint Size, uint LocationCount) MapShaderTypeToVulkanFormat(string shaderType)
    //    {
    //        switch (shaderType)
    //        {
    //            case "float": return (VkFormat.VK_FORMAT_R32_SFLOAT, 4, 1);
    //            case "vec2": return (VkFormat.VK_FORMAT_R32G32_SFLOAT, 8, 1);
    //            case "vec3": return (VkFormat.VK_FORMAT_R32G32B32_SFLOAT, 12, 1);
    //            case "vec4": return (VkFormat.VK_FORMAT_R32G32B32A32_SFLOAT, 16, 1);
    //            case "int": return (VkFormat.VK_FORMAT_R32_SINT, 4, 1);
    //            case "ivec2": return (VkFormat.VK_FORMAT_R32G32_SINT, 8, 1);
    //            case "uint": return (VkFormat.VK_FORMAT_R32_UINT, 4, 1);
    //            case "mat4": return (VkFormat.VK_FORMAT_R32G32B32A32_SFLOAT, 64, 4);
    //            default: throw new NotSupportedException($"Unsupported GLSL type: {shaderType}");
    //        }
    //    }
    //}
}
