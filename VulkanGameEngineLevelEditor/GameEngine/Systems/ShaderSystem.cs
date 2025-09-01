using CSScripting;
using GlmSharp;
using Silk.NET.GLFW;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public unsafe static class ShaderSystem
    {
        public static SystemMessenger systemMessenger { get; set; }
        public static Dictionary<string, ShaderPipelineData> ShaderModuleMap { get; private set; } = new Dictionary<string, ShaderPipelineData>();
        public static Dictionary<string, ShaderPushConstant> ShaderPushConstantMap { get; private set; } = new Dictionary<string, ShaderPushConstant>();
        public static Dictionary<string, ShaderStruct> PipelineShaderStructPrototypeMap { get; private set; } = new Dictionary<string, ShaderStruct>();
        public static Dictionary<int, ShaderStruct> PipelineShaderStructMap { get; private set; } = new Dictionary<int, ShaderStruct>();

        static void StartUp()
        {
            Shader_StartUp();
        }

        static void Destroy()
        {
            List<String> pushConstantKeys = new List<string>();
            foreach (var pair in ShaderPushConstantMap)
            {
                pushConstantKeys.Add(pair.Key);
            }
            foreach (var key in pushConstantKeys)
            {
                var pushConstant = ShaderPushConstantMap[key];
                Shader_DestroyPushConstantBufferData(&pushConstant);
            }
            ShaderPushConstantMap.Clear();

            List<String> shaderStructProtoKeys = new List<string>();
            foreach (var pair in PipelineShaderStructPrototypeMap)
            {
                shaderStructProtoKeys.Add(pair.Key);
            }
            foreach (var key in shaderStructProtoKeys)
            {
                var shaderStruct = PipelineShaderStructPrototypeMap[key];
                Shader_DestroyShaderStructData(&shaderStruct);
            }
            PipelineShaderStructPrototypeMap.Clear();

            List<int> shaderStructKeys = new List<int>();
            foreach (var pair in PipelineShaderStructMap)
            {
                shaderStructKeys.Add(pair.Key);
            }
            foreach (var key in shaderStructKeys)
            {
                var shaderStruct = PipelineShaderStructMap[key];
                Shader_DestroyShaderStructData(&shaderStruct);
            }
            PipelineShaderStructMap.Clear();

            List<String> shaderModuleKeys = new List<string>();
            foreach (var pair in ShaderModuleMap)
            {
                shaderModuleKeys.Add(pair.Key);
            }
            foreach (var key in shaderModuleKeys)
            {
                var pipelineData = ShaderModuleMap[key];
                Shader_ShaderDestroy(pipelineData);
            }
            ShaderModuleMap.Clear();
        }

        public static VkPipelineShaderStageCreateInfo CreateShader(VkDevice device, string filename, VkShaderStageFlagBits shaderStages)
        {
            VkShaderModule shaderModule = VulkanCSConst.VK_NULL_HANDLE;
            shaderModule = Shader_BuildGLSLShaderFile(device, filename);

            return Shader_CreateShader(shaderModule, shaderStages);
        }

        public static ShaderPipelineData LoadShaderPipelineData(List<String> shaderPathList)
        {
            IntPtr[] cShaderPathList = CHelper.VectorToConstCharPtrPtr(shaderPathList);
            ShaderPipelineData pipelineData = Shader_GetShaderData(cShaderPathList, shaderPathList.Count);
            //ListPtr<ShaderPushConstant> pushConstantList = new ListPtr<ShaderPushConstant>(pipelineData.PushConstantList, pipelineData.PushConstantCount);
            //foreach (var pushConstant in pushConstantList)
            //{
            //    if (!ShaderPushConstantExists(pushConstant.PushConstantName))
            //    {
            //        ShaderPushConstantMap[pushConstant.PushConstantName] = new ShaderPushConstant
            //        {
            //            .PushConstantName = pushConstant.PushConstantName,
            //            .PushConstantSize = pushConstant.PushConstantSize,
            //            .PushConstantVariableListCount = pushConstant.PushConstantVariableListCount,
            //            .ShaderStageFlags = pushConstant.ShaderStageFlags,
            //            .PushConstantVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(pushConstant.PushConstantVariableList, pushConstant.PushConstantVariableListCount, __FILE__, __LINE__, __func__, pushConstant.PushConstantName.c_str()),
            //            .PushConstantBuffer = memorySystem.AddPtrBuffer<byte>(pushConstant.PushConstantSize, __FILE__, __LINE__, __func__, pushConstant.PushConstantName.c_str()),
            //            .GlobalPushContant = pushConstant.GlobalPushContant
            //        };

            //        for (int x = 0; x < ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableListCount; x++)
            //        {
            //            ShaderVariable* variablePtr = &pushConstant.PushConstantVariableList[x];
            //            ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Value = memorySystem.AddPtrBuffer<byte>(ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Size, __FILE__, __LINE__, __func__, ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Name.c_str());
            //            Shader_SetVariableDefaults(ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x]);
            //            memorySystem.RemovePtrBuffer<ShaderVariable>(variablePtr);
            //        }
            //    }
            //    memorySystem.RemovePtrBuffer<ShaderVariable>(pushConstant.PushConstantVariableList);
            //    memorySystem.RemovePtrBuffer(pushConstant.PushConstantBuffer);
            //}
            //CHelper.CHelper_DestroyConstCharPtrPtr(cShaderPathList);
            //ShaderModuleMap[pipelineData.ShaderList[0]] = pipelineData;
            return pipelineData;
        }

        public static ShaderVariable* SearchGlobalShaderConstantVar(ShaderPushConstant* pushConstant, string varName)
        {
            if (pushConstant == null)
            {
                return null;
            }

            ListPtr<ShaderVariable> pushConstantShaderVariables = new ListPtr<ShaderVariable>(pushConstant->PushConstantVariableList, pushConstant->PushConstantVariableListCount);
            var matchingVar = pushConstantShaderVariables.FirstOrDefault(var => Marshal.PtrToStringAnsi((IntPtr)var.Name) == varName);
            return &matchingVar;
        }

        public static ShaderVariable* SearchShaderStruct(ShaderStruct shaderStruct, string varName)
        {
            return Shader_SearchShaderStructVar(shaderStruct, varName);
        }

        public static void UpdateGlobalShaderBuffer(string pushConstantName)
        {
            if (!ShaderPushConstantExists(pushConstantName))
            {
               // std::cerr << "Error: Push constant '" << pushConstantName << "' does not exist!" << std::endl;
                return;
            }
            Shader_UpdatePushConstantBuffer(RenderSystem.renderer, ShaderPushConstantMap[pushConstantName]);
        }

        public static void UpdateShaderBuffer(int vulkanBufferId)
        {
            if (!ShaderStructExists(vulkanBufferId))
            {
                return;
            }

            ShaderStruct shaderStruct = PipelineShaderStructMap[vulkanBufferId];
            VulkanBuffer vulkanBuffer = BufferSystem.FindVulkanBuffer(vulkanBufferId);
            Shader_UpdateShaderBuffer(RenderSystem.renderer, vulkanBuffer, &shaderStruct, 1);
        }

        public static ShaderPushConstant* GetGlobalShaderPushConstant(string pushConstantName)
        {
            return ShaderPushConstantExists(pushConstantName) ? &ShaderPushConstantMap[pushConstantName] : nullptr;
        }

        public static void LoadShaderPipelineStructPrototypes(List<String> renderPassJsonList)
        {
            size_t protoTypeStructCount = 0;
            for (int x = 0; x < renderPassJsonList.size(); x++)
            {
                nlohmann::json renderPassJson = Json::ReadJson(renderPassJsonList[x]);
                for (int x = 0; x < renderPassJson["RenderPipelineList"].size(); x++)
                {
                    nlohmann::json pipelineJson = Json::ReadJson(renderPassJson["RenderPipelineList"][x]);
                    Vector<String> shaderJsonList = Vector<String>{ pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1]
                };
                    const char** cShaderList = CHelper_VectorToConstCharPtrPtr(shaderJsonList);
                    ShaderStruct* shaderStructArray = Shader_LoadProtoTypeStructs(cShaderList, shaderJsonList.size(), protoTypeStructCount);
                    Span<ShaderStruct> shaderStructList(shaderStructArray, protoTypeStructCount);
                    for (auto & shaderStruct : shaderStructList)
                    {
                        if (!ShaderStructPrototypeExists(shaderStruct.Name))
                        {
                            PipelineShaderStructPrototypeMap[shaderStruct.Name] = ShaderStruct
                            {
                                .Name = shaderStruct.Name,
                                .ShaderBufferSize = shaderStruct.ShaderBufferSize,
                                .ShaderBufferVariableListCount = shaderStruct.ShaderBufferVariableListCount,
                                .ShaderBufferVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(shaderStruct.ShaderBufferVariableList, shaderStruct.ShaderBufferVariableListCount, __FILE__, __LINE__, __func__, shaderStruct.Name.c_str()),
                                .ShaderStructBufferId = shaderStruct.ShaderStructBufferId,
                                .ShaderStructBuffer = memorySystem.AddPtrBuffer<byte>(shaderStruct.ShaderBufferSize, __FILE__, __LINE__, __func__, shaderStruct.Name.c_str())
                            };
                        }
                        memorySystem.RemovePtrBuffer(shaderStruct.ShaderBufferVariableList);
                    }
                    memorySystem.RemovePtrBuffer<ShaderStruct>(shaderStructArray);
                    CHelper_DestroyConstCharPtrPtr(cShaderList);
                }
            }
        }

        public static ShaderPipelineData FindShaderModule(string shaderFile)
        {
            return ShaderModuleMap[shaderFile];
        }

        public static ShaderPushConstant FindShaderPushConstant(string shaderFile)
        {
            return ShaderPushConstantMap[shaderFile];
        }

        public static ShaderStruct FindShaderProtoTypeStruct(string shaderKey)
        {
            return PipelineShaderStructPrototypeMap[shaderKey];
        }

        public static ShaderStruct FindShaderStruct(int vulkanBufferId)
        {
            return PipelineShaderStructMap[vulkanBufferId];
        }

        public static ShaderStruct CopyShaderStructProtoType(string structName)
        {
            ShaderStruct shaderStructCopy = FindShaderProtoTypeStruct(structName);
        return Shader_CopyShaderStructPrototype(shaderStructCopy);
        }

        public static bool ShaderPushConstantExists(string pushConstantName)
        {
            return ShaderPushConstantMap.ContainsKey(pushConstantName);
        }

        public static bool ShaderModuleExists(string shaderFile)
        {
            return ShaderModuleMap.ContainsKey(shaderFile);
        }

        public static bool ShaderStructPrototypeExists(string structKey) 
        {
            return PipelineShaderStructPrototypeMap.ContainsKey(structKey);
        }

        public static bool ShaderStructExists(int vulkanBufferKey)
        {
            return PipelineShaderStructMap.ContainsKey(vulkanBufferKey);
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

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern VkShaderModule Shader_BuildGLSLShaderFile(VkDevice device, [MarshalAs(UnmanagedType.LPStr)] string path);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern bool Shader_BuildGLSLShaders([MarshalAs(UnmanagedType.LPStr)] string command);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern VkPipelineShaderStageCreateInfo Shader_CreateShader(VkShaderModule shaderModule, VkShaderStageFlagBits shaderStages);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_StartUp();
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderPipelineData Shader_GetShaderData(IntPtr[] pipelineShaderPaths, size_t pipelineShaderCount);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern VkPipelineShaderStageCreateInfo Shader_CreateShader(VkDevice device, [MarshalAs(UnmanagedType.LPStr)] string path, VkShaderStageFlagBits shaderStages);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_UpdateShaderBuffer(GraphicsRenderer renderer, VulkanBuffer vulkanBuffer, ShaderStruct* shaderStruct, size_t shaderCount);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_UpdatePushConstantBuffer(GraphicsRenderer renderer, ShaderPushConstant pushConstantStruct);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderStruct* Shader_LoadProtoTypeStructs(IntPtr[] pipelineShaderPaths, size_t pipelineShaderCount, size_t outProtoTypeStructCount);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderStruct Shader_CopyShaderStructPrototype(ShaderStruct shaderStructToCopy);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderVariable* Shader_SearchShaderStructVar(ShaderStruct shaderStruct, [MarshalAs(UnmanagedType.LPStr)] string varName);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_ShaderDestroy(ShaderPipelineData shader);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_DestroyShaderStructData(ShaderStruct* shaderStruct);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_DestroyPushConstantBufferData(ShaderPushConstant* pushConstant);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_SetVariableDefaults(ShaderVariable shaderVariable);
    }
}
