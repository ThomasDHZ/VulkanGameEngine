using GlmSharp;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
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

    public unsafe static class GameSystem
    {
        public static ListPtr<VkCommandBuffer> CommandBufferSubmitList { get; set; } = new ListPtr<VkCommandBuffer>();
        public static unsafe void StartUp(void* renderAreaHandle, void* debuggerHandle)
        {
            VulkanSystem.StartUpVulkan(WindowType.Win32, renderAreaHandle, debuggerHandle);
            MemoryPoolSystem.StartUp();
            LevelSystem.LoadLevel(@$"Levels/TestLevel.json");
        }

        public static void Update(float deltaTime)
        {
            LevelSystem.Update(deltaTime);
            GameObjectSystem.Update(deltaTime);
            LevelSystem.Update(deltaTime);
            SpriteSystem.Update(deltaTime);
            MeshSystem.Update(deltaTime);
            RenderSystem.Update(deltaTime);
        }

        public static void Draw(float deltaTime)
        {
            //RenderSystem.StartFrame();
            LevelSystem.Draw(CommandBufferSubmitList.First(), deltaTime);
            ////  CommandBufferSubmitList.Add(ImGui_Draw(RenderSystem.renderer, RenderSystem.imGuiRenderer));
            //RenderSystem.EndFrame(CommandBufferSubmitList);
            //CommandBufferSubmitList.Clear();
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
