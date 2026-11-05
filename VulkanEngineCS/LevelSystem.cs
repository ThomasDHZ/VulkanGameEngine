using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http.Json;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanEngineCoreCS;
using VulkanEngineCoreCS.Models;
using VulkanEngineCoreCS.Vulkan;
using VulkanEngineCS.Models;

namespace VulkanEngineCS
{
    public unsafe class LevelSystem
    {
        public static Guid LevelId { get; set; } = new Guid();
        public static string Skybox = string.Empty;
        public static List<MaterialLoader> MaterialLoaderList { get; set; } = new List<MaterialLoader>();
        public static List<RenderPassLoader> MainRenderPassList { get; set; } = new List<RenderPassLoader>();
        public static List<RenderPassLoader> LevelRenderPassList { get; set; } = new List<RenderPassLoader>();
        public static List<SpriteVram> SpriteVramList { get; set; } = new List<SpriteVram>();
        public static List<GameObjectBehaviors> GameObjectBehaviorList { get; set; } = new List<GameObjectBehaviors>();
        public static LevelTileSet levelTileSet { get; set; } = new LevelTileSet();

        public static void LoadLevel([MarshalAs(UnmanagedType.LPStr)] string levelPath)
        {
            string jsonContent = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{levelPath}");
            LevelLoader level = JsonConvert.DeserializeObject<LevelLoader>(jsonContent);
            foreach (var renderPass in level.MainRenderPassList)
            {
                LoadRenderPass(renderPass);
            }
            foreach (var renderPass in level.LevelEditorRenderPass)
            {
                LoadLevelEditorRenderPass(renderPass);
            }
            foreach (var material in level.Materials)
            {
                LoadMaterial(material);
            }
            foreach (var spriteVram in level.SpriteVram)
            {
                LoadSpriteVram(spriteVram);
            }
            foreach (var grameObjectBehavior in level.GameObjectBehaviors)
            {
                LoadGameObjectBehavior(grameObjectBehavior);
            }
            LoadTileSet(level.TileSetVram);
            DLLSystem.CallDLLFunc(() => LevelSystem_LoadLevel(levelPath));
        }

        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_Update(deltaTime));
        }
        public static void LevelEditorRenderPass(string levelPath)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_LevelEditorRenderPass(levelPath));
        }

        public static List<RenderPassNode> CreateDrawCommands(VkCommandBuffer commandBuffer, float deltaTime)
        {
            List<RenderPassNode> renderPassNodeList = new List<RenderPassNode>();
            RenderPassNodeDLL* nodes = LevelSystem_CreateDrawCommands(ref commandBuffer, deltaTime, out size_t count);
            if (nodes == null || count == 0) return new List<RenderPassNode>();

            for (size_t x = 0; x < count; x++)
            {
                List<List<VulkanDrawMessage>> subPassDrawList = new List<List<VulkanDrawMessage>>();
                ref RenderPassNodeDLL node = ref nodes[x];
                for (size_t y = 0; y < node.SubPassDrawMessage_RenderPassCount; y++)
                {
                    size_t drawCount = node.SubPassDrawMessage_SubPassCounts[y];
                    VulkanDrawMessageDLL* draws = node.SubPassDrawMessage[y];

                    List<VulkanDrawMessage> drawMeshMessage = new List<VulkanDrawMessage>();
                    for (size_t z = 0; z < drawCount; z++)
                    {
                        drawMeshMessage.Add(VulkanDrawMessage.FromDLL(draws[z]));
                    }
                    subPassDrawList.Add(drawMeshMessage);
                }
                renderPassNodeList.Add(new RenderPassNode
                {
                    RenderPassGuid = node.RenderPassGuid,
                    MipCount = node.MipCount,
                    PostRenderPassCmd = node.PostRenderPassCmd,
                    PreRenderPassCmd = node.PreRenderPassCmd,
                    SubPassDrawMessage = subPassDrawList,
                });
            }
            return renderPassNodeList;
        }

        private static void LoadMaterial(string filePath)
        {
            string json = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{filePath}");
            MaterialLoaderList.Add(JsonConvert.DeserializeObject<MaterialLoader>(json));
        }

        private static void LoadTileSet(string filePath)
        {
            string json = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{filePath}");
            levelTileSet = JsonConvert.DeserializeObject<LevelTileSet>(json);
        }

        private static void LoadRenderPass(string filePath)
        {
            string json = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{filePath}");
            MainRenderPassList.Add(JsonConvert.DeserializeObject<RenderPassLoader>(json));
        }

        private static void LoadLevelEditorRenderPass(string filePath)
        {
            string json = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{filePath}");
            LevelRenderPassList.Add(JsonConvert.DeserializeObject<RenderPassLoader>(json));
        }

        private static void LoadSpriteVram(string filePath)
        {
            string json = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{filePath}");
            SpriteVramList.Add(JsonConvert.DeserializeObject<SpriteVram>(json));
        }

        private static void LoadGameObjectBehavior(string filePath)
        {
            string json = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{filePath}");
            GameObjectBehaviorList.Add(JsonConvert.DeserializeObject<GameObjectBehaviors>(json));
        }

        public static void SaveLevelData(string filePath)
        {
            foreach(var material in MaterialLoaderList)
            {
                string a = JsonConvert.SerializeObject(material);
                int ad = 34;
            }
        }

        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_LoadLevel([MarshalAs(UnmanagedType.LPStr)] string levelPath);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_Update(float deltaTime); 
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_LevelEditorRenderPass([MarshalAs(UnmanagedType.LPStr)] string levelPath);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern RenderPassNodeDLL* LevelSystem_CreateDrawCommands(ref VkCommandBuffer commandBuffer, float deltaTime, out size_t renderPassNodeDllCount);
    }
}
