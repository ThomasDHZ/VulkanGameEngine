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
        public static Guid levelRenderPass2DId { get; private set; }
        public static Guid spriteRenderPass2DId { get; private set; }
        public static Guid frameBufferId { get; private set; }
        public static LevelLayout levelLayout { get; private set; }
        public static List<LevelLayer> LevelLayerList { get; private set; } = new List<LevelLayer>();
        public static List<ListPtr<uint>> LevelTileMapList { get; private set; } = new List<ListPtr<uint>>();
        public static Dictionary<Guid, LevelTileSet> LevelTileSetMap { get; private set; } = new Dictionary<Guid, LevelTileSet>();
        public static Dictionary<uint, List<SpriteInstanceStruct>> SpriteInstanceListMap { get; private set; } = new Dictionary<uint, List<SpriteInstanceStruct>>();
        public static Dictionary<uint, int> SpriteInstanceBufferMap { get; private set; } = new Dictionary<uint, int>();
        public static Dictionary<uint, List<uint>> SpriteBatchLayerObjectListMap { get; private set; } = new Dictionary<uint, List<uint>>();
        public static Dictionary<uint, Animation2D> AnimationMap { get; private set; } = new Dictionary<uint, Animation2D>();
        public static Dictionary<Guid, ListPtr<vec2>> AnimationFrameListMap { get; private set; } = new Dictionary<Guid, ListPtr<vec2>>();


        public static void LoadLevel(string levelPath)
        {
            var res = new vec2(RenderSystem.renderer.SwapChainResolution.width, RenderSystem.renderer.SwapChainResolution.height);
            var pos = new vec2(0.0f, 0.0f);
            OrthographicCamera = new OrthographicCamera2D(res, pos);

            string levelDirectory = Path.GetDirectoryName(levelPath);
            string jsonContent = File.ReadAllText(levelPath);
            LevelLoader levelLoader = JsonConvert.DeserializeObject<LevelLoader>(jsonContent);

            string renderPassJsonContent = File.ReadAllText(Path.Combine(levelDirectory, "../RenderPass/LevelShader2DRenderPass.json"));
            RenderPassLoaderModel renderPassLoaderModel = JsonConvert.DeserializeObject<RenderPassLoaderModel>(renderPassJsonContent);

            string pipelineJsonContent = File.ReadAllText(Path.Combine(levelDirectory, renderPassLoaderModel.RenderPipelineList[0]));
            PipelineShader pipelineLoaderModel = JsonConvert.DeserializeObject<PipelineShader>(pipelineJsonContent);
            ShaderSystem.LoadShaderPipelineStructPrototypes(pipelineLoaderModel.ShaderList);

            Guid tileSetId = new Guid();
            foreach (var texturePath in levelLoader.LoadTextures)
            {
                string fullTexturePath = Path.GetFullPath(Path.Combine(levelDirectory, texturePath));
                TextureSystem.LoadTexture(fullTexturePath);
            }
            foreach (var materialPath in levelLoader.LoadMaterials)
            {
                string fullMaterialPath = Path.GetFullPath(Path.Combine(levelDirectory, materialPath));
                MaterialSystem.LoadMaterial(fullMaterialPath);
            }
            //foreach (var spriteVRAMPath in levelLoader.LoadSpriteVRAM)
            //{
            //    string fullSpriteVRAMPath = Path.GetFullPath(Path.Combine(levelDirectory, spriteVRAMPath));
            //    SpriteSystem.LoadSpriteVRAM(fullSpriteVRAMPath);
            //}
            foreach (var levelLayoutPath in levelLoader.LoadTileSetVRAM)
            {
                string fullLevelLayoutPath = Path.GetFullPath(Path.Combine(levelDirectory, levelLayoutPath));
                tileSetId = LoadTileSetVRAM(fullLevelLayoutPath);
            }
            foreach (var gameObjectLoader in levelLoader.GameObjectList)
            {
                string gameObjectPath = Path.GetFullPath(Path.Combine(levelDirectory, gameObjectLoader.GameObjectPath));
                GameObjectSystem.CreateGameObject(gameObjectPath, new vec2((float)gameObjectLoader.GameObjectPositionOverride[0], (float)gameObjectLoader.GameObjectPositionOverride[1]));
            }
            {
                string fullLevelLayoutPath = Path.GetFullPath(Path.Combine(levelDirectory, levelLoader.LoadLevelLayout));
                LoadLevelLayout(fullLevelLayoutPath);
                LoadLevelMesh(tileSetId);
            }
            {
                Guid dummyGuid = new Guid();
                string fullRenderPassPath = @$"{ConstConfig.BaseDirectoryPath}RenderPass/LevelShader2DRenderPass.json";
                string gameObjectPath = Path.GetFullPath(Path.Combine(levelDirectory, fullRenderPassPath));

                string jsonContent2 = File.ReadAllText(gameObjectPath);
                RenderPassLoaderModel renderPassId = JsonConvert.DeserializeObject<RenderPassLoaderModel>(jsonContent);
                spriteRenderPass2DId = new Guid("aa18e942-497b-4981-b917-d93a5b1de6eb");

                SpriteSystem.AddSpriteBatchLayer(spriteRenderPass2DId);
                ivec2 renderPassResultion = new ivec2((int)RenderSystem.renderer.SwapChainResolution.width, (int)RenderSystem.renderer.SwapChainResolution.height);
                spriteRenderPass2DId = RenderSystem.LoadRenderPass(levelLayout.LevelLayoutId, @$"{ConstConfig.BaseDirectoryPath}RenderPass/LevelShader2DRenderPass.json", renderPassResultion);
                frameBufferId = RenderSystem.LoadRenderPass(dummyGuid, @$"{ConstConfig.BaseDirectoryPath}RenderPass/FrameBufferRenderPass.json", renderPassResultion);
            }
        }

        public static void Update(float deltaTime)
        {
            OrthographicCamera.Update(ShaderSystem.GetGlobalShaderPushConstant("sceneData"));
          //  SpriteSystem.Update(deltaTime);
            foreach (var levelLayer in LevelLayerList)
            {
                // levelLayer.Update(deltaTime);
            }
        }

        public static void Draw(ListPtr<VkCommandBuffer> commandBufferList, float deltaTime)
        {
            commandBufferList.Add(RenderSystem.RenderLevel(spriteRenderPass2DId, levelLayout.LevelLayoutId, deltaTime, ShaderSystem.GetGlobalShaderPushConstant("sceneData")));
            commandBufferList.Add(RenderSystem.RenderFrameBuffer(frameBufferId));
        }

        private static Guid LoadTileSetVRAM(string levelTileSetPath)
        {
            if (levelTileSetPath.IsEmpty())
            {
                return new Guid();
            }

            string jsonContent = File.ReadAllText(levelTileSetPath);
            LevelTileSet levelTileSetJson = JsonConvert.DeserializeObject<LevelTileSet>(jsonContent);

            Guid tileSetId = levelTileSetJson.TileSetId;
            if (LevelTileSetMap.ContainsKey(levelTileSetJson.TileSetId))
            {
                return tileSetId;
            }

            Material material = MaterialSystem.MaterialMap[levelTileSetJson.MaterialId];
            Texture tileSetTexture = TextureSystem.TextureList[material.AlbedoMapId];
            LevelTileSetMap[tileSetId] = VRAM_LoadTileSetVRAM(levelTileSetPath, material, tileSetTexture);
            var levelTileSet = LevelTileSetMap[tileSetId];

            VRAM_LoadTileSets(levelTileSetPath, &levelTileSet);
            LevelTileSetMap[tileSetId] = levelTileSet;
            return tileSetId;
        }

        public static Guid LoadLevelLayout(string levelLayoutPath)
        {
            levelLayout = VRAM_LoadLevelInfo(levelLayoutPath);
            VkQueue levelLayerList = VRAM_LoadLevelLayout(levelLayoutPath, out size_t levelLayerCount, out size_t levelLayerMapCount);
            if (levelLayerList != VkQueue.Zero &&
                levelLayerCount > 0)
            {
                using (var layerPointers = new ListPtr<VkQueue>((VkQueue*)levelLayerList, levelLayerCount))
                {
                    for (uint x = 0; x < layerPointers.UCount; x++)
                    {
                        VkQueue layerPtr = layerPointers[(int)x];
                        if (layerPtr != VkQueue.Zero)
                        {
                            using (var layerData = new ListPtr<uint>((uint*)layerPtr, levelLayerMapCount))
                            {
                                ListPtr<uint> managedLayer = new ListPtr<uint>(layerData.ToList());
                                LevelTileMapList.Add(managedLayer);
                            }

                            VRAM_DeleteLevelLayerMapPtr((uint*)layerPtr);
                        }
                    }
                }
                VRAM_DeleteLevelLayerPtr((uint**)levelLayerList);
            }

            return levelLayout.LevelLayoutId;
        }

        public static void LoadLevelMesh(Guid tileSetId)
        {
            for (int x = 0; x < LevelTileMapList.Count(); x++)
            {
                LevelTileSet levelTileSet = LevelTileSetMap[tileSetId];
                var levelBounds = levelLayout.LevelBounds;
                LevelLayerList.Add(Level2D_LoadLevelInfo(levelLayout.LevelLayoutId, levelTileSet, LevelTileMapList[x].Ptr, LevelTileMapList[x].Count(), ref levelBounds, x));

                ListPtr<Vertex2D> vertexList = new ListPtr<Vertex2D>(LevelLayerList[x].VertexList, LevelLayerList[x].VertexListCount);
                ListPtr<uint> indexList = new ListPtr<uint>(LevelLayerList[x].IndexList, LevelLayerList[x].IndexListCount);
                MeshSystem.CreateLevelLayerMesh(levelLayout.LevelLayoutId, vertexList, indexList);
            }
        }

        public static void Destory()
        {
            foreach (var tileMap in LevelTileSetMap)
            {
                VRAM_DeleteLevelVRAM(tileMap.Value.LevelTileListPtr);
            }
            foreach (var levelLayer in LevelLayerList)
            {
                Level2D_DeleteLevel(levelLayer.TileIdMap, levelLayer.TileMap, levelLayer.VertexList, levelLayer.IndexList);
            }
        }

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern LevelLayer Level2D_LoadLevelInfo(Guid levelId, LevelTileSet tileSet, uint* tileIdMapList, size_t tileIdMapCount, ref ivec2 levelBounds, int levelLayerIndex);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern LevelTileSet VRAM_LoadTileSetVRAM([MarshalAs(UnmanagedType.LPStr)] string tileSetPath, Material material, Texture tileVramTexture);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void VRAM_LoadTileSets([MarshalAs(UnmanagedType.LPStr)] string tileSetPath, LevelTileSet* levelTileSet);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern LevelLayout VRAM_LoadLevelInfo([MarshalAs(UnmanagedType.LPStr)] string levelLayoutPath);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern VkQueue VRAM_LoadLevelLayout([MarshalAs(UnmanagedType.LPStr)] string levelLayoutPath, out size_t levelLayerCount, out size_t levelLayerMapCount);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void VRAM_DeleteSpriteVRAM(Animation2D* animationListPtr, vec2* animationFrameListPtr);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void VRAM_DeleteLevelVRAM(Tile* levelTileList);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void VRAM_DeleteLevelLayerPtr(uint** levelLayerPtr);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void VRAM_DeleteLevelLayerMapPtr(uint* levelLayerMapPtr);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] private static extern void Level2D_DeleteLevel(uint* TileIdMap, Tile* TileMap, Vertex2D* VertexList, uint* IndexList);
    }
}
