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
        public static List<JsonAssetWriter<MaterialLoader>> MaterialLoaderList { get; set; } = new List<JsonAssetWriter<MaterialLoader>>();
        public static List<JsonAssetWriter<RenderPassLoader>> MainRenderPassList { get; set; } = new List<JsonAssetWriter<RenderPassLoader>>();
        public static List<JsonAssetWriter<RenderPassLoader>> LevelEditorRenderPassList { get; set; } = new List<JsonAssetWriter<RenderPassLoader>>();
        public static List<JsonAssetWriter<SpriteVramLoader>> SpriteVramList { get; set; } = new List<JsonAssetWriter<SpriteVramLoader>>();
        public static List<JsonAssetWriter<GameObjectBehaviors>> GameObjectBehaviorList { get; set; } = new List<JsonAssetWriter<GameObjectBehaviors>>();
        public static LevelTileSet levelTileSet { get; set; } = new LevelTileSet();

        public static void LoadLevel([MarshalAs(UnmanagedType.LPStr)] string levelPath)
        {
            string jsonContent = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{levelPath}");
            LevelLoader level = JsonConvert.DeserializeObject<LevelLoader>(jsonContent);
            foreach (var filePath in level.MainRenderPassList)
            {
                LoadRenderPass(filePath);
            }
            foreach (var filePath in level.LevelEditorRenderPass)
            {
                LoadLevelEditorRenderPass(filePath);
            }
            foreach (var filePath in level.Materials)
            {
                LoadMaterial(filePath);
            }
            foreach (var filePath in level.SpriteVram)
            {
                LoadSpriteVram(filePath);
            }
            foreach (var filePath in level.GameObjectBehaviors)
            {
                LoadGameObjectBehavior(filePath);
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
            MaterialLoaderList.Add(new JsonAssetWriter<MaterialLoader>
            {
                JsonWriteLocation = filePath,
                JsonWriteObject = JsonConvert.DeserializeObject<MaterialLoader>(json)
            }); 
        }

        private static void LoadTileSet(string filePath)
        {
            //string json = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{filePath}");
            //levelTileSet.Add(new JsonAssetWriter<LevelTileSet>
            //{
            //    JsonWriteLocation = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{filePath}"),
            //    JsonWriteObject = JsonConvert.DeserializeObject<LevelTileSet>(json)
            //});
        }

        private static void LoadRenderPass(string filePath)
        {
            string json = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{filePath}");
            MainRenderPassList.Add(new JsonAssetWriter<RenderPassLoader>
            {
                JsonWriteLocation = filePath,
                JsonWriteObject = JsonConvert.DeserializeObject<RenderPassLoader>(json)
            });
        }

        private static void LoadLevelEditorRenderPass(string filePath)
        {
            string json = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{filePath}");
            LevelEditorRenderPassList.Add(new JsonAssetWriter<RenderPassLoader>
            {
                JsonWriteLocation = filePath,
                JsonWriteObject = JsonConvert.DeserializeObject<RenderPassLoader>(json)
            });
        }

        private static void LoadSpriteVram(string filePath)
        {
            string json = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{filePath}");
            SpriteVramList.Add(new JsonAssetWriter<SpriteVramLoader>
            {
                JsonWriteLocation = filePath,
                JsonWriteObject = JsonConvert.DeserializeObject<SpriteVramLoader>(json)
            });
        }

        private static void LoadGameObjectBehavior(string filePath)
        {
            string json = File.ReadAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{filePath}");
            GameObjectBehaviorList.Add(new JsonAssetWriter<GameObjectBehaviors>
            {
                JsonWriteLocation = filePath,
                JsonWriteObject = JsonConvert.DeserializeObject<GameObjectBehaviors>(json)
            });
        }

        public static void SaveLevelData(string levelFilePath)
        {
            foreach (var material in MaterialLoaderList)
            {
                string json = JsonConvert.SerializeObject(material.JsonWriteObject, Formatting.Indented);
                File.WriteAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{material.JsonWriteLocation}", json);
            }
            foreach (var renderPass in MainRenderPassList)
            {
                string json = JsonConvert.SerializeObject(renderPass.JsonWriteObject, Formatting.Indented);
                File.WriteAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{renderPass.JsonWriteLocation}", json);
            }
            foreach (var renderPass in LevelEditorRenderPassList)
            {
                string json = JsonConvert.SerializeObject(renderPass.JsonWriteObject, Formatting.Indented);
                File.WriteAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{renderPass.JsonWriteLocation}", json);
            }
            foreach (var renderPass in SpriteVramList)
            {
                string json = JsonConvert.SerializeObject(renderPass.JsonWriteObject, Formatting.Indented);
                File.WriteAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{renderPass.JsonWriteLocation}", json);
            }
            foreach (var renderPass in GameObjectBehaviorList)
            {
                string json = JsonConvert.SerializeObject(renderPass.JsonWriteObject, Formatting.Indented);
                File.WriteAllText($@"C:\Users\DHZ\Documents\GitHub\VulkanGameEngine\Assets\{renderPass.JsonWriteLocation}", json);
            }
            int a = 234;
            //string a = JsonConvert.SerializeObject(renderPass);
        }

        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_LoadLevel([MarshalAs(UnmanagedType.LPStr)] string levelPath);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_Update(float deltaTime); 
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern void LevelSystem_LevelEditorRenderPass([MarshalAs(UnmanagedType.LPStr)] string levelPath);
        [DllImport("VulkanEngineInterop.dll", CallingConvention = CallingConvention.Cdecl)] private static extern RenderPassNodeDLL* LevelSystem_CreateDrawCommands(ref VkCommandBuffer commandBuffer, float deltaTime, out size_t renderPassNodeDllCount);
    }
}
