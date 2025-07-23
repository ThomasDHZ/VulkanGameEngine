using GlmSharp;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.Models;


namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public enum WindowType
    {
        Win32,
        SDL,
        GLFW
    }

    public static class GameSystem
    {
        public static Guid TileSetId { get; set; }
        public static Guid LevelRendererId { get; set; }
        public static Guid SpriteRenderPass2DId { get; set; }
        public static Guid FrameBufferId { get; set; }
        public static ListPtr<VkCommandBuffer> CommandBufferSubmitList { get; set; } = new ListPtr<VkCommandBuffer>();
        public static unsafe void StartUp(void* renderAreaHandle, void* debuggerHandle)
        {
            RenderSystem.CreateVulkanRenderer(WindowType.Win32, renderAreaHandle, debuggerHandle);
            LevelSystem.LoadLevel(@$"{ConstConfig.BaseDirectoryPath}Levels/TestLevel.json");
        }

        public static void Update(float deltaTime)
        {
            LevelSystem.Update(deltaTime);
            TextureSystem.Update(deltaTime);
            MaterialSystem.Update(deltaTime);
            RenderSystem.Update(SpriteRenderPass2DId, LevelRendererId, deltaTime);

            VkCommandBuffer commandBuffer = RenderSystem.BeginSingleTimeCommands();
            MeshSystem.Update(deltaTime);
            RenderSystem.EndSingleTimeCommands(commandBuffer);
        }

        public static void Draw(float deltaTime)
        {
            RenderSystem.StartFrame();
            LevelSystem.Draw(CommandBufferSubmitList, deltaTime);
            //  CommandBufferSubmitList.Add(ImGui_Draw(RenderSystem.renderer, RenderSystem.imGuiRenderer));
            RenderSystem.EndFrame(CommandBufferSubmitList);
            CommandBufferSubmitList.Clear();
        }

        public static void Destroy()
        {
            ////gameObjectSystem.DestroyGameObjects();
            ////meshSystem.DestroyAllGameObjects();
            //TextureSystem.DestroyAllTextures();
            //MaterialSystem.DestroyAllMaterials();
            //LevelSystem.DestoryLevel();
            //MeshSystem.DestroyAllGameObjects();
            //BufferSystem.DestroyAllBuffers();
            //RenderSystem.Destroy();
            //MemorySystem.ReportLeaks();
        }
    }
}
