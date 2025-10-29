using CSScripting;
using GlmSharp;
using Newtonsoft.Json;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
   public struct PipelineShader
    {
        public List<string> ShaderList = new List<string>();

        public PipelineShader()
        {
        }
    };

    public static unsafe class LevelSystem
    {
        public static OrthographicCamera2D OrthographicCamera { get; set; } = new OrthographicCamera2D();
        public static Guid spriteRenderPass2DId { get; private set; }
        public static Guid frameBufferId { get; private set; }
        public static LevelLayout levelLayout { get; private set; }
        public static List<LevelLayer> LevelLayerList { get; private set; } = new List<LevelLayer>();


        public static void LoadLevel(string levelPath)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_LoadLevel(levelPath));
        }

        public static void Update(float deltaTime)
        {
            LevelSystem_Update(deltaTime);
        }

        public static void Draw(ListPtr<VkCommandBuffer> commandBufferList, float deltaTime)
        {
            LevelSystem_Draw(commandBufferList.Ptr, commandBufferList.Count, deltaTime);
        }

        public static void DestroyLevel()
        {
            LevelSystem_DestroyLevel();
        }

        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_LoadLevel([MarshalAs(UnmanagedType.LPStr)] string levelPath);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_Update(float deltaTime);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_Draw(VkCommandBuffer* commandBufferListPtr, size_t commandBufferCount, float deltaTime);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_DestroyLevel();
    }
}
