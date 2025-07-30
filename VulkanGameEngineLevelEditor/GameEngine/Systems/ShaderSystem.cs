using CSScripting;
using GlmSharp;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;
using Vulkan;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public unsafe static class ShaderSystem
    {
        public static SystemMessenger systemMessenger { get; set; }


        public static VkPipelineShaderStageCreateInfo CreateShader(VkDevice device, string filename, VkShaderStageFlagBits shaderStages)
        {
            VkShaderModule shaderModule = VulkanCSConst.VK_NULL_HANDLE;
            shaderModule = Shader_BuildGLSLShaderFile(device, filename);

            return Shader_CreateShader(shaderModule, shaderStages);
        }

        public static bool CompileShader(string shaderFilePath, string shaderFile)
        {
            string ext = shaderFile.GetExtension();
            string args = $"--target-env=vulkan1.4 --target-spv=spv1.6 {shaderFilePath}/{shaderFile} -o {shaderFilePath}{shaderFile.GetFileName()}{ext}.spv";

            if (systemMessenger == null)
            {
                MessageBox.Show($"Messager must be set.");
                return false;
            }

            if (!File.Exists($"{shaderFilePath}/{shaderFile}"))
            {
                MessageBox.Show($"Shader file not found: {shaderFilePath}/{shaderFile}");
                return false;
            }

            try
            {
                systemMessenger.WriteLine($@"Building {shaderFile}:");

                ProcessStartInfo startInfo = new ProcessStartInfo
                {
                    FileName = ConstConfig.ShaderCompilerPath,
                    Arguments = args,
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    CreateNoWindow = true
                };

                using (Process process = new Process { StartInfo = startInfo })
                {
                    process.Start();
                    string output = process.StandardOutput.ReadToEnd();
                    string error = process.StandardError.ReadToEnd();
                    process.WaitForExit();

                    string result = output + error;
                    if (process.ExitCode != 0)
                    {
                        systemMessenger.CompilerWriteLine(result, "Error");
                        MessageBox.Show($"Compilation failed with error:\n{result}");
                        return false;
                    }
                    else
                    {
                        systemMessenger.CompilerWriteLine(result, "Success");
                        return true;
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error: {ex.Message}");
                return false;
            }
        }

        public static bool CompileAllShaders(string shaderFileDirectory)
        {
            if (systemMessenger == null)
            {
                MessageBox.Show($"Messager must be set.\n");
                return false;
            }

            string[] fileList = Directory.GetFiles(shaderFileDirectory);
            systemMessenger.WriteLine($@"Building on directory: {shaderFileDirectory}:");
            foreach (string file in fileList)
            {
                if (file.GetExtension() == ".vert" ||
                    file.GetExtension() == ".frag")
                {
                    CompileShader(shaderFileDirectory, file.GetFileName());
                }
            }
            return true;
        }

        public static ListPtr<VkVertexInputBindingDescription> LoadVertexBindingLayout(string vertexShaderFile)
        {
            ListPtr<VkVertexInputBindingDescription> vertexInputBindingList = new ListPtr<VkVertexInputBindingDescription>();
            var vertexShaderTextLines = File.ReadLines(vertexShaderFile).ToList();
            try
            {
                var vertexRegex = new Regex(@"layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s*in\s+([^\s]+)\s+([^\s;]+)\s*;", RegexOptions.IgnorePatternWhitespace | RegexOptions.Compiled);
                var vertexInputLineList = vertexShaderTextLines.Where(x => vertexRegex.IsMatch(x)).ToList();

                var paramsRegex = new Regex(@"//\s*InputParams\s*\(\s*binding\s*=\s*(\d+)\s*,\s*VertexType\s*=\s*(Vertex|Instance)\s*\)", RegexOptions.Compiled);
                var paramLineList = vertexShaderTextLines.Where(x => paramsRegex.IsMatch(x)).ToList();

                if (paramLineList.Any())
                {
                    foreach (var vertexInputLine in paramLineList)
                    {
                        var paramsMatch = paramsRegex.Match(vertexInputLine);
                        vertexInputBindingList.Add(new VkVertexInputBindingDescription
                        {
                            binding = uint.Parse(paramsMatch.Groups[1].Value),
                            inputRate = paramsMatch.Groups[2].Value == "Instance" ? VkVertexInputRate.VK_VERTEX_INPUT_RATE_INSTANCE : VkVertexInputRate.VK_VERTEX_INPUT_RATE_VERTEX
                        });
                    }
                }
                else
                {
                    foreach (var vertexInputLine in vertexInputLineList)
                    {
                        vertexInputBindingList.Add(new VkVertexInputBindingDescription
                        {
                            binding = 0,
                            inputRate = VkVertexInputRate.VK_VERTEX_INPUT_RATE_VERTEX
                        });
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error loading vertex binding layout from {vertexShaderFile}: {ex.Message}");
                throw;
            }
            return vertexInputBindingList;
        }

        public static List<VkVertexInputAttributeDescriptionModel> LoadVertexAttributesLayout(string vertexShaderFile)
        {
            List<VkVertexInputAttributeDescriptionModel> vertexInputAttributeList = new List<VkVertexInputAttributeDescriptionModel>();
            var vertexShaderTextLines = File.ReadLines(vertexShaderFile).ToList();
            try
            {
                var vertexRegex = new Regex(@"layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s*in\s+([^\s]+)\s+([^\s;]+)\s*;", RegexOptions.IgnorePatternWhitespace | RegexOptions.Compiled);
                var vertexInputLineList = vertexShaderTextLines.Where(x => vertexRegex.IsMatch(x)).ToList();

                var paramsRegex = new Regex(@"//\s*InputParams\s*\(\s*binding\s*=\s*(\d+)\s*,\s*VertexType\s*=\s*(Vertex|Instance)\s*\)", RegexOptions.IgnorePatternWhitespace | RegexOptions.Compiled);
                var paramLineList = vertexShaderTextLines.Where(x => paramsRegex.IsMatch(x)).ToList();

                int x = 0;
                uint vertexOffset = 0;
                foreach (var vertexInputLine in vertexInputLineList)
                {
                    int size = 0;
                    VkFormat format = VkFormat.VK_FORMAT_UNDEFINED;
                    Match vertexMatchData = vertexRegex.Match(vertexInputLine);
                    switch (vertexMatchData.Groups[2].Value)
                    {
                        case "float": format = VkFormat.VK_FORMAT_R32_SFLOAT; size = sizeof(float); break;
                        case "vec2": format = VkFormat.VK_FORMAT_R32G32_SFLOAT; size = sizeof(vec2); break;
                        case "vec3": format = VkFormat.VK_FORMAT_R32G32B32_SFLOAT; size = sizeof(vec3); break;
                        case "vec4": format = VkFormat.VK_FORMAT_R32G32B32A32_SFLOAT; size = sizeof(vec4); break;
                        case "int": format = VkFormat.VK_FORMAT_R32_SINT; size = sizeof(int); break;
                        case "ivec2": format = VkFormat.VK_FORMAT_R32G32_SINT; size = sizeof(ivec2); break;
                        case "ivec3": format = VkFormat.VK_FORMAT_R32G32B32_SINT; size = sizeof(ivec3); break;
                        case "ivec4": format = VkFormat.VK_FORMAT_R32G32B32_SINT; size = sizeof(ivec4); break;
                        case "uint": format = VkFormat.VK_FORMAT_R32_UINT; size = sizeof(uint); break;
                        case "mat2": format = VkFormat.VK_FORMAT_R32G32_SFLOAT; size = sizeof(mat2); break;
                        case "mat3": format = VkFormat.VK_FORMAT_R32G32B32_SFLOAT; size = sizeof(mat3); break;
                        case "mat4": format = VkFormat.VK_FORMAT_R32G32B32A32_SFLOAT; size = sizeof(mat4); break;
                        default: throw new NotSupportedException($"Unsupported GLSL type: {vertexMatchData.Groups[2].Value}");
                    }

                    vertexInputAttributeList.Add(new VkVertexInputAttributeDescriptionModel
                    {
                        Name = vertexMatchData.Groups[3].Value,
                        Binding = paramLineList.Any() ? uint.Parse(paramsRegex.Match(paramLineList[x]).Groups[1].Value) : 0,
                        Location = uint.Parse(vertexMatchData.Groups[1].Value),
                        Type = vertexMatchData.Groups[2].Value,
                        Format = format,
                        Offset = vertexOffset
                    });
                    vertexOffset += (uint)size;
                    x++;
                }
                return vertexInputAttributeList;
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error loading vertex attribute layout from {vertexShaderFile}: {ex.Message}");
                throw;
            }
        }

        public static List<PipelineDescriptorModel> LoadDescriptorSetBindings(string vertexShaderFile)
        {
            List<PipelineDescriptorModel> pipelineDescriptorList = new List<PipelineDescriptorModel>();
            var vertexShaderTextLines = File.ReadLines(vertexShaderFile).ToList();
            try
            {
                var descriptorBindingPropertiesRegex = new Regex(@"//\s*InputParams\s*\(\s*DescriptorBindingPropertiesEnum\s*=\s*([^\s\)]+)\s*\)", RegexOptions.Compiled);
                var descriptorBindingPropertiesRegexLineList = vertexShaderTextLines.Where(x => descriptorBindingPropertiesRegex.IsMatch(x)).ToList();

                var descriptorTypeRegex = new Regex(@"layout\s*\(\s*binding\s*=\s*(\d+)\s*\)\s*(?:readonly\s+)?(readonly\s+buffer|buffer|uniform\s+sampler2D|uniform\s+samplerCube|uniform|[^\s;]+)", RegexOptions.Compiled);
                var descriptorTypeLineList = vertexShaderTextLines.Where(x => descriptorTypeRegex.IsMatch(x)).ToList();

                int x = 0;
                foreach (var descriptorTypeLine in descriptorTypeLineList)
                {
                    Match descriptorBindingPropertiesMatch = descriptorBindingPropertiesRegex.Match(descriptorBindingPropertiesRegexLineList[x]);
                    DescriptorBindingPropertiesEnum descriptorBinding = new DescriptorBindingPropertiesEnum();
                    if (descriptorBindingPropertiesMatch.Success)
                    {
                        string enumName = descriptorBindingPropertiesMatch.Groups[1].Value;
                        if (!Enum.TryParse(enumName, ignoreCase: true, out descriptorBinding))
                        {
                            throw new InvalidOperationException($"Invalid DescriptorBindingPropertiesEnum value: {enumName}");

                        }
                    }

                    Match descriptorTypeMatch = descriptorTypeRegex.Match(descriptorTypeLine);
                    VkDescriptorType descriptorType = VkDescriptorType.VK_DESCRIPTOR_TYPE_MAX_ENUM;
                    switch (descriptorTypeMatch.Groups[2].Value)
                    {
                        case "buffer": descriptorType = VkDescriptorType.VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
                        case "uniform": descriptorType = VkDescriptorType.VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
                        case "uniform sampler2D": descriptorType = VkDescriptorType.VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; break;
                        case "uniform samplerCube": descriptorType = VkDescriptorType.VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; break;
                        default: throw new NotSupportedException($"Unsupported descriptor type: {descriptorTypeMatch.Groups[2].Value}");
                    }

                    pipelineDescriptorList.Add(new PipelineDescriptorModel
                    {
                        BindingNumber = uint.Parse(descriptorTypeMatch.Groups[1].Value),
                        BindingPropertiesList = descriptorBinding,
                        DescriptorType = descriptorType,
                    });
                    x++;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error loading descriptor set bindings from {vertexShaderFile}: {ex.Message}");
                throw;
            }
            return pipelineDescriptorList;
        }

        public static List<VkDescriptorSetLayoutBindingModel> LoadDescriptorSetLayoutBindings(string vertexShaderFile)
        {
            List<VkDescriptorSetLayoutBindingModel> descriptorSetLayoutBindingList = new List<VkDescriptorSetLayoutBindingModel>();
            var vertexShaderTextLines = File.ReadLines(vertexShaderFile).ToList();
            try
            {
                var descriptorTypeRegex = new Regex(@"layout\s*\(\s*binding\s*=\s*(\d+)\s*\)\s*(?:readonly\s+)?(readonly\s+buffer|buffer|uniform\s+sampler2D|uniform\s+samplerCube|uniform|[^\s;]+)", RegexOptions.Compiled);
                var descriptorTypeLineList = vertexShaderTextLines.Where(x => descriptorTypeRegex.IsMatch(x)).ToList();

                int x = 0;
                foreach (var descriptorTypeLine in descriptorTypeLineList)
                {
                    var match = descriptorTypeRegex.Match(descriptorTypeLine);
                    VkDescriptorType descriptorType = VkDescriptorType.VK_DESCRIPTOR_TYPE_MAX_ENUM;
                    switch (match.Groups[2].Value)
                    {
                        case "readonly buffer":
                        case "buffer": descriptorType = VkDescriptorType.VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
                        case "uniform": descriptorType = VkDescriptorType.VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
                        case "uniform sampler2D": descriptorType = VkDescriptorType.VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; break;
                        default: throw new NotSupportedException($"Unsupported descriptor type: {match.Groups[2].Value}");
                    }

                    descriptorSetLayoutBindingList.Add(new VkDescriptorSetLayoutBindingModel
                    {
                        binding = uint.Parse(match.Groups[1].Value),
                        descriptorType = descriptorType
                    });
                    x++;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error loading descriptor set layout bindings from {vertexShaderFile}: {ex.Message}");
                throw;
            }

            return descriptorSetLayoutBindingList;
        }

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern VkShaderModule Shader_BuildGLSLShaderFile(VkDevice device, [MarshalAs(UnmanagedType.LPStr)] string path);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern bool Shader_BuildGLSLShaders([MarshalAs(UnmanagedType.LPStr)] string command);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern VkPipelineShaderStageCreateInfo Shader_CreateShader(VkShaderModule shaderModule, VkShaderStageFlagBits shaderStages);
    }
}
