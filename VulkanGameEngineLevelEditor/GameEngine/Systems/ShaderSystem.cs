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


        public static unsafe ShaderVariable* SearchGlobalShaderConstantVar(ShaderPushConstant pushConstant, string varName)
        {
            ListPtr<ShaderVariableDLL> shaderVariableList = new ListPtr<ShaderVariableDLL>();
            for(int x = 0; x < pushConstant.PushConstantVariableList.Count(); x++)
            {
                shaderVariableList.Add(new ShaderVariableDLL
                {
                    ByteAlignment = pushConstant.PushConstantVariableList[(int)x].ByteAlignment,
                    MemberTypeEnum = pushConstant.PushConstantVariableList[(int)x].MemberTypeEnum,
                    Name = Marshal.StringToHGlobalAnsi(pushConstant.PushConstantVariableList[(int)x].Name),
                    Size = pushConstant.PushConstantVariableList[(int)x].Size,
                    Value = (nint)pushConstant.PushConstantVariableList[(int)x].Value
                });
            }

            ShaderPushConstantDLL shaderPushConstantDLL = new ShaderPushConstantDLL
            {
                GlobalPushContant = pushConstant.GlobalPushContant,
                PushConstantBuffer = pushConstant.PushConstantBuffer,
                PushConstantName = Marshal.StringToHGlobalAnsi(pushConstant.PushConstantName),
                PushConstantSize = pushConstant.PushConstantSize,
                PushConstantVariableList = shaderVariableList.Ptr,
                PushConstantVariableListCount = (nuint)shaderVariableList.Count,
                ShaderStageFlags = pushConstant.ShaderStageFlags
            };
            return Shader_SearchGlobalShaderConstantVar(&shaderPushConstantDLL, varName);
        }

        public static ShaderVariable* SearchShaderStruct(ShaderStruct shaderStruct, string varName)
        {
            ListPtr<ShaderVariableDLL> shaderVariableList = new ListPtr<ShaderVariableDLL>();
            for (int x = 0; x < shaderStruct.ShaderBufferVariableList.Count(); x++)
            {
                shaderVariableList.Add(new ShaderVariableDLL
                {
                    ByteAlignment = shaderStruct.ShaderBufferVariableList[(int)x].ByteAlignment,
                    MemberTypeEnum = shaderStruct.ShaderBufferVariableList[(int)x].MemberTypeEnum,
                    Name = Marshal.StringToHGlobalAnsi(shaderStruct.ShaderBufferVariableList[(int)x].Name),
                    Size = shaderStruct.ShaderBufferVariableList[(int)x].Size,
                    Value = (nint)shaderStruct.ShaderBufferVariableList[(int)x].Value
                });
            }

            ShaderStructDLL shaderStructDLL = new ShaderStructDLL
            {
                Name = Marshal.StringToHGlobalAnsi(shaderStruct.Name),
                ShaderBufferSize = shaderStruct.ShaderBufferSize,
                ShaderBufferVariableCount = (nuint)shaderVariableList.Count,
                ShaderBufferVariableList = shaderVariableList.Ptr,
                ShaderStructBuffer = shaderStruct.ShaderStructBuffer,
                ShaderStructBufferId = shaderStruct.ShaderStructBufferId
            };
            return Shader_SearchShaderStructVarCS(&shaderStructDLL, varName);
        }

        public static void UpdateShaderBuffer(int vulkanBufferId)
        {
            if (!ShaderStructExists(vulkanBufferId))
            {
                return;
            }

            ShaderStruct shaderStruct = PipelineShaderStructMap[vulkanBufferId];
            VulkanBuffer vulkanBuffer = BufferSystem.FindVulkanBuffer((uint)vulkanBufferId);
           // Shader_UpdateShaderBuffer(RenderSystem.renderer, vulkanBuffer, &shaderStruct, 1);
        }

        public static ShaderPushConstant GetGlobalShaderPushConstant(string pushConstantName)
        {
            return ShaderPushConstantExists(pushConstantName) ? ShaderPushConstantMap[pushConstantName] : new ShaderPushConstant();
        }

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
                Shader_LoadShaderPipelineStructPrototypes(cShaderPathList, (nuint)cShaderPathList.Length);
         
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

        public static bool ShaderStructExists(int vulkanBufferKey)
        {
            return PipelineShaderStructMap.ContainsKey(vulkanBufferKey);
        }

      

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Shader_LoadShaderPipelineStructPrototypes(IntPtr[] pipelineShaderPaths, nuint pipelineShaderCount);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderVariable* Shader_SearchShaderPushConstStructVarCS(ShaderPushConstantDLL* pushConstant, [MarshalAs(UnmanagedType.LPStr)] string varName);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderVariable* Shader_SearchShaderStructVarCS(ShaderStructDLL* shaderStruct, [MarshalAs(UnmanagedType.LPStr)] string varName);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern ShaderVariable* Shader_SearchGlobalShaderConstantVar(ShaderPushConstantDLL* pushConstant,[MarshalAs(UnmanagedType.LPStr)] string pushConstantName);
    }
}

