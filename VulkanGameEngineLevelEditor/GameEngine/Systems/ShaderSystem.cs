using CSScripting;
using GlmSharp;
using Newtonsoft.Json;
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

        public static ShaderPipelineData LoadShaderPipelineData(List<string> shaderPathList)
        {
            IntPtr[] cShaderPathList = CHelper.VectorToConstCharPtrPtr(shaderPathList);
            IntPtr shaderPathsPtr = IntPtr.Zero;
            if (cShaderPathList != null && cShaderPathList.Length > 0)
            {
                shaderPathsPtr = Marshal.AllocHGlobal(cShaderPathList.Length * IntPtr.Size);
                for (int x = 0; x < cShaderPathList.Length; x++)
                {
                    Marshal.WriteIntPtr(shaderPathsPtr, x * IntPtr.Size, cShaderPathList[x]);
                }
            }

            try
            {
                ShaderPipelineData pipelineData = Shader_LoadPipelineShaderData(shaderPathsPtr, (size_t)shaderPathList.Count);
                if (pipelineData.PushConstantList != null && pipelineData.PushConstantCount > 0)
                {
                    ShaderPushConstant* pushConstantsPtr = pipelineData.PushConstantList;
                    for (size_t x = 0; x < pipelineData.PushConstantCount; x++)
                    {
                        ShaderPushConstant pushConstant = pushConstantsPtr[(int)x];
                        string name = pushConstant.GetName();
                        if (!ShaderPushConstantExists(name))
                        {
                            ShaderVariable* newVariableList = (ShaderVariable*)Marshal.AllocHGlobal((int)pushConstant.PushConstantVariableListCount * sizeof(ShaderVariable));
                            if (pushConstant.PushConstantVariableList != null)
                            {
                                for (nuint y = 0; y < pushConstant.PushConstantVariableListCount; y++)
                                {
                                    ShaderVariable srcVar = pushConstant.PushConstantVariableList[(int)y];
                                    string varName = srcVar.GetName();
                                    newVariableList[(int)y] = new ShaderVariable
                                    {
                                        Name = Marshal.StringToHGlobalAnsi(varName),
                                        Size = srcVar.Size,
                                        ByteAlignment = srcVar.ByteAlignment,
                                        MemberTypeEnum = srcVar.MemberTypeEnum,
                                        Value = null 
                                    };
                                }
                            }

                            ShaderPushConstantMap[name] = new ShaderPushConstant
                            {
                                PushConstantName = Marshal.StringToHGlobalAnsi(name),
                                PushConstantSize = pushConstant.PushConstantSize,
                                PushConstantVariableListCount = pushConstant.PushConstantVariableListCount,
                                ShaderStageFlags = pushConstant.ShaderStageFlags,
                                PushConstantVariableList = newVariableList,
                                PushConstantBuffer = MemorySystem.AddPtrBuffer<byte>((byte)pushConstant.PushConstantSize, name),
                                GlobalPushContant = pushConstant.GlobalPushContant
                            };

                            for (nuint z = 0; z < ShaderPushConstantMap[name].PushConstantVariableListCount; z++)
                            {
                                ref ShaderVariable var = ref ShaderPushConstantMap[name].PushConstantVariableList[(int)z];
                                var.Value = MemorySystem.AddPtrBuffer<byte>((byte)var.Size, string.Empty, string.Empty, 0, string.Empty);
                                Shader_SetVariableDefaults(var);  
                            }
                        }
                    }
                }
                return pipelineData;
            }
            finally
            {
                CHelper.CHelper_DestroyConstCharPtrPtr(cShaderPathList);
                if (shaderPathsPtr != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(shaderPathsPtr);
                }
            }
        }

        public static unsafe ShaderVariable* SearchGlobalShaderConstantVar(ShaderPushConstant* pushConstant, string varName)
        {
            return Shader_SearchShaderConstStructVar(pushConstant, varName);
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
            VulkanBuffer vulkanBuffer = BufferSystem.FindVulkanBuffer((uint)vulkanBufferId);
            Shader_UpdateShaderBuffer(RenderSystem.renderer, vulkanBuffer, &shaderStruct, 1);
        }

        public static ShaderPushConstant GetGlobalShaderPushConstant(string pushConstantName)
        {
            return ShaderPushConstantExists(pushConstantName) ? ShaderPushConstantMap[pushConstantName] : new ShaderPushConstant();
        }

        public static void LoadShaderPipelineStructPrototypes(List<String> shaderList)
        {
            for (int x = 0; x < shaderList.Count(); x++)
            {
                var shaderDirectory = Path.GetDirectoryName((@$"{ConstConfig.BaseDirectoryPath}Shaders/"));
                List<String> fullPathString = new List<string> { Path.Combine(shaderDirectory, shaderList[0]), Path.Combine(shaderDirectory, shaderList[1]) };
                IntPtr[] cShaderPathList = CHelper.VectorToConstCharPtrPtr(fullPathString);
                IntPtr shaderPathsPtr = IntPtr.Zero;
                if (cShaderPathList != null && cShaderPathList.Length > 0)
                {
                    shaderPathsPtr = Marshal.AllocHGlobal(cShaderPathList.Length * IntPtr.Size);
                    for (int y = 0; y < cShaderPathList.Length; y++)
                    {
                        Marshal.WriteIntPtr(shaderPathsPtr, y * IntPtr.Size, cShaderPathList[y]);
                    }
                }
                try
                {
                    ShaderStruct* shaderStructArray = Shader_LoadProtoTypeStructs(cShaderPathList, cShaderPathList.Count(), out size_t protoTypeStructCount);
                    ListPtr<ShaderStruct> shaderStructList = new ListPtr<ShaderStruct>(shaderStructArray, protoTypeStructCount);
                    foreach (var shaderStruct in shaderStructList)
                    {
                        string name = shaderStruct.GetName();
                        if (!ShaderStructPrototypeExists(name))
                        {
                            PipelineShaderStructPrototypeMap[name] = new ShaderStruct
                            {
                                Name = shaderStruct.Name,
                                ShaderBufferSize = shaderStruct.ShaderBufferSize,
                                ShaderBufferVariableListCount = shaderStruct.ShaderBufferVariableListCount,
                                ShaderBufferVariableList = MemorySystem.AddPtrBuffer<ShaderVariable>(shaderStruct.ShaderBufferVariableList, (size_t)shaderStruct.ShaderBufferVariableListCount),
                                ShaderStructBufferId = shaderStruct.ShaderStructBufferId,
                                ShaderStructBuffer = MemorySystem.AddPtrBuffer<byte>((byte)shaderStruct.ShaderBufferSize)
                            };
                        }
                        MemorySystem.RemovePtrBuffer(shaderStruct.ShaderBufferVariableList);
                    }
                    MemorySystem.RemovePtrBuffer<ShaderStruct>(shaderStructArray);
                }
                finally
                {
                    CHelper.CHelper_DestroyConstCharPtrPtr(cShaderPathList);
                    if (shaderPathsPtr != IntPtr.Zero)
                    {
                        Marshal.FreeHGlobal(shaderPathsPtr);
                    }
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

        public static ShaderStruct CopyShaderStructProtoType(string structName, int bufferId)
        {
            ShaderStruct shaderStructCopy = FindShaderProtoTypeStruct(structName);
            ShaderStruct shaderStruct = Shader_CopyShaderStructPrototype(shaderStructCopy);
            shaderStruct.ShaderStructBufferId = bufferId;
            return shaderStruct;
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

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_StartUp();
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_ShaderDestroy(ShaderPipelineData shader);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_DestroyShaderStructData(ShaderStruct* shaderStruct);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_DestroyPushConstantBufferData(ShaderPushConstant* pushConstant);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_SetVariableDefaults(ShaderVariable shaderVariable);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern bool Shader_BuildGLSLShaders([MarshalAs(UnmanagedType.LPStr)] string command);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern VkShaderModule Shader_BuildGLSLShaderFile(VkDevice device, [MarshalAs(UnmanagedType.LPStr)] string path);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern VkPipelineShaderStageCreateInfo Shader_CreateShader(VkShaderModule shaderModule, VkShaderStageFlagBits shaderStages);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderPipelineData Shader_LoadPipelineShaderData(IntPtr pipelineShaderPaths, size_t pipelineShaderCount);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern VkPipelineShaderStageCreateInfo Shader_CreateShader(VkDevice device, [MarshalAs(UnmanagedType.LPStr)] string path, VkShaderStageFlagBits shaderStages);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_UpdateShaderBuffer(GraphicsRenderer renderer, VulkanBuffer vulkanBuffer, ShaderStruct* shaderStruct, size_t shaderCount);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_UpdatePushConstantBuffer(GraphicsRenderer renderer, ShaderPushConstant pushConstantStruct);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderStruct* Shader_LoadProtoTypeStructs(IntPtr[] pipelineShaderPaths, size_t pipelineShaderCount, out size_t outProtoTypeStructCount);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderStruct Shader_CopyShaderStructPrototype(ShaderStruct shaderStructToCopy);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderVariable* Shader_SearchShaderConstStructVar(ShaderPushConstant* pushConstant, [MarshalAs(UnmanagedType.LPStr)] string varName);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderVariable* Shader_SearchShaderStructVar(ShaderStruct shaderStruct, [MarshalAs(UnmanagedType.LPStr)] string varName);
    }
}
