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
        public static LevelGuid LevelId { get; set; }
        public static Guid SpriteRenderPass2DId { get; set; } = new Guid("aa18e942-497b-4981-b917-d93a5b1de6eb");
        public static Guid GaussianBlurRenderPassId { get; set; } = new Guid("44d22120-d385-4cbb-86d0-55afd870ce50");
        public static Guid FrameBufferId { get; set; } = new Guid("76db13ab-75ab-4fb1-ae02-0c6dfb447856");

        public static void LoadLevel(string levelPath)
        {
            LevelId = DLLSystem.CallDLLFunc(() => LevelSystem_LoadLevel(levelPath));
        }

        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_Update(deltaTime));
        }

        public static void Draw(ListPtr<VkCommandBuffer> commandBufferList, float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_Draw(commandBufferList.Ptr, commandBufferList.Count, deltaTime));
        }

        public static void DestroyLevel()
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_DestroyLevel());
        }

        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern LevelGuid LevelSystem_LoadLevel([MarshalAs(UnmanagedType.LPStr)] string levelPath);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_Update(float deltaTime);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_Draw(VkCommandBuffer* commandBufferListPtr, size_t commandBufferCount, float deltaTime);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_DestroyLevel();
    }
}
