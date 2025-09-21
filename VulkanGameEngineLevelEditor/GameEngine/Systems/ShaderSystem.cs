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
using System.Xml.Linq;
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
                                   // string varName = srcVar.GetName();
                                    newVariableList[(int)y] = new ShaderVariable
                                    {
                                    //    Name = Marshal.StringToHGlobalAnsi(varName),
                                        Size = srcVar.Size,
                                        ByteAlignment = srcVar.ByteAlignment,
                                        MemberTypeEnum = srcVar.MemberTypeEnum,
                                      //  Value = IntPtr.Zero 
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
                            //    var.Value = (nint)MemorySystem.AddPtrBuffer<byte>((byte)var.Size, string.Empty, string.Empty, 0, string.Empty);
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

        //public static void LoadShaderPipelineStructPrototypes(List<string> shaderList)
        //{
        //    if (shaderList == null || shaderList.Count == 0)
        //    {
        //        Console.WriteLine("LoadShaderPipelineStructPrototypes: Shader list is null or empty.");
        //        throw new ArgumentException("Shader list cannot be null or empty.");
        //    }

        //    var shaderDirectory = Path.GetDirectoryName($"{ConstConfig.BaseDirectoryPath}Shaders/");
        //    if (string.IsNullOrEmpty(shaderDirectory))
        //    {
        //        Console.WriteLine("LoadShaderPipelineStructPrototypes: Invalid shader directory.");
        //        throw new InvalidOperationException("Shader directory is invalid.");
        //    }

        //    var fullPathString = shaderList.Select(path =>
        //    {
        //        if (string.IsNullOrWhiteSpace(path))
        //        {
        //            Console.WriteLine($"LoadShaderPipelineStructPrototypes: Invalid shader path: {path}");
        //            throw new ArgumentException($"Invalid shader path: {path}");
        //        }
        //        string fullPath = Path.GetFullPath(Path.Combine(shaderDirectory, path));
        //        if (!File.Exists(fullPath))
        //        {
        //            Console.WriteLine($"LoadShaderPipelineStructPrototypes: Shader file not found: {fullPath}");
        //            throw new FileNotFoundException($"Shader file not found: {fullPath}");
        //        }
        //        return fullPath;
        //    }).ToList();

        //    Console.WriteLine($"LoadShaderPipelineStructPrototypes: Full paths: {string.Join(", ", fullPathString)}");

        //    IntPtr[] cShaderPathList = CHelper.VectorToConstCharPtrPtr(fullPathString);
        //    if (cShaderPathList == null || cShaderPathList.Length == 0)
        //    {
        //        Console.WriteLine("LoadShaderPipelineStructPrototypes: Failed to convert shader paths to native pointers.");
        //        throw new InvalidOperationException("Failed to convert shader paths to native pointers.");
        //    }

        //    Console.WriteLine($"LoadShaderPipelineStructPrototypes: cShaderPathList length: {cShaderPathList.Length}");

        //    try
        //    {
        //        ShaderStructDLL shaderStructDLL = Shader_LoadProtoTypeStructsCS(cShaderPathList, (nuint)cShaderPathList.Length, out nuint protoTypeStructCount);
        //        // Now construct as before
        //        ShaderStruct shaderStruct = new ShaderStruct(shaderStructDLL);

        //        Console.WriteLine($"LoadShaderPipelineStructPrototypes: protoTypeStructCount: {protoTypeStructCount}");

        //        if (protoTypeStructCount == 0)
        //        {
        //            Console.WriteLine("LoadShaderPipelineStructPrototypes: No prototype structs were loaded.");
        //            throw new InvalidOperationException("No prototype structs were loaded.");
        //        }

        //        shaderStruct.ShaderStructBuffer = MemorySystem.AddPtrBuffer((byte)shaderStruct.ShaderBufferSize);
        //    }
        //    finally
        //    {
        //        CHelper.CHelper_DestroyConstCharPtrPtr(cShaderPathList);
        //    }
        //}

        public static void LoadShaderPipelineStructPrototypes(List<string> shaderList)
        {
            var shaderDirectory = Path.GetDirectoryName(Path.Combine(ConstConfig.BaseDirectoryPath, "Assets\\Shaders"));
            List<string> fullPathString = shaderList.Select(path => Path.Combine("C:\\Users\\dotha\\Documents\\GitHub\\VulkanGameEngine\\VulkanGameEngine\\", path)).ToList();
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

            ShaderStructDLL* shaderStructPtr = null;
            try
            {
                shaderStructPtr = Shader_LoadProtoTypeStructsCS(cShaderPathList, (nuint)cShaderPathList.Length, out nuint protoTypeStructCount);
                 for (nuint x = 0; x < protoTypeStructCount; x++)
                {
                    ShaderStructDLL* currentPtr = shaderStructPtr + x;  
                    ShaderStruct shaderStruct = new ShaderStruct(currentPtr);
                    PipelineShaderStructPrototypeMap[shaderStruct.Name] = shaderStruct;
                }
            }
            finally
            {
                CHelper.CHelper_DestroyConstCharPtrPtr(cShaderPathList);
                if (shaderPathsPtr != IntPtr.Zero)
                {
                    Marshal.FreeHGlobal(shaderPathsPtr);
                }
            }
            var a = 234;
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
            shaderStructCopy.ShaderStructBuffer = MemorySystem.AddPtrBuffer(shaderStructCopy.ShaderBufferSize);
            shaderStructCopy.ShaderStructBufferId = bufferId;
            return shaderStructCopy;
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
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)]
        private static extern ShaderStructDLL* Shader_LoadProtoTypeStructsCS(IntPtr[] pipelineShaderPaths, nuint pipelineShaderCount, out nuint outProtoTypeStructCount);

        // Add this for freeing (implement in C++)
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)]
        private static extern void Shader_FreeProtoTypeStructsCS(ShaderStructDLL* ptr, nuint count);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderStruct Shader_CopyShaderStructPrototype(ShaderStruct shaderStructToCopy);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderVariable* Shader_SearchShaderConstStructVar(ShaderPushConstant* pushConstant, [MarshalAs(UnmanagedType.LPStr)] string varName);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderVariable* Shader_SearchShaderStructVar(ShaderStruct shaderStruct, [MarshalAs(UnmanagedType.LPStr)] string varName);
    }
}

