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
        public static LevelGuid LevelId { get; set; } = new LevelGuid("a4ff0086-0069-4265-9a81-b38fb9e6b5be");
        public static Guid SpriteRenderPass2DId { get; set; } = new Guid("aa18e942-497b-4981-b917-d93a5b1de6eb");
        public static Guid GaussianBlurRenderPassId { get; set; } = new Guid("44d22120-d385-4cbb-86d0-55afd870ce50");
        public static Guid FrameBufferId { get; set; } = new Guid("76db13ab-75ab-4fb1-ae02-0c6dfb447856");

        public static void LoadLevel(string levelPath)
        {
            LevelSystem_LoadLevel(levelPath);
        }

        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_Update(deltaTime));
        }

        public static void Draw(ListPtr<VkCommandBuffer> commandBufferList, float deltaTime)
        {
            commandBufferList.Add(DLLSystem.CallDLLFunc(() => LevelSystem_RenderLevel(SpriteRenderPass2DId, SpriteRenderPass2DId, deltaTime)));
            commandBufferList.Add(DLLSystem.CallDLLFunc(() => LevelSystem_RenderFrameBuffer(FrameBufferId)));
        }

        public static void DestroyLevel()
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_DestroyLevel());
        }

        public static LevelLayout GetLevelLayout()
        {
            return LevelSystem_GetLevelLayout();
        }
        public static List<LevelLayer> GetLevelLayerList()
        {
            LevelLayer* gameObjectListPtr = LevelSystem_GetLevelLayerList(out int outCount);
            List<LevelLayer> gameObjectList = new ListPtr<LevelLayer>(gameObjectListPtr, outCount).ToList();
            return gameObjectList;
        }

        //public static void List<List<uint>> GetLevelTileMapList()
        //{
        //    [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern uint** LevelSystem_GetLevelTileMapList(out int outCount);
        //}

        public static List<LevelTileSet> GetLevelTileSetList()
        {
            LevelTileSet* levelTileSetPtr = LevelSystem_GetLevelTileSetList(out int outCount);
            List<LevelTileSet> levelTileSetList = new ListPtr<LevelTileSet>(levelTileSetPtr, outCount).ToList();
            return levelTileSetList;
        }

        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern VkCommandBuffer LevelSystem_RenderFrameBuffer(Guid renderPassId);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern VkCommandBuffer LevelSystem_RenderLevel(Guid renderPassId, Guid levelId, float deltaTime);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_LoadLevel([MarshalAs(UnmanagedType.LPStr)] string levelPath);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_Update(float deltaTime);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_DestroyLevel();
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern LevelLayout LevelSystem_GetLevelLayout();
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern LevelLayer* LevelSystem_GetLevelLayerList(out int outCount);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern uint** LevelSystem_GetLevelTileMapList(out int outCount);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern LevelTileSet* LevelSystem_GetLevelTileSetList(out int outCount);
    }
}
