using GlmSharp;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor.GameEngineAPI;

namespace VulkanGameEngineLevelEditor.Vulkan
{
    public struct LevelLoader
    {
        public Guid LevelId;
        public List<string> TextureList;
    }

    public unsafe struct VulkanRenderPassDLL
    {
        public Guid RenderPassId;
        public VkSampleCountFlagBits SampleCount;
        public VkRect2D RenderArea;
        public VkRenderPass RenderPass;
        public VkFramebuffer* FrameBufferList;
        public VkClearValue* ClearValueList;
        public size_t FrameBufferCount;
        public size_t ClearValueCount;
        public VkCommandBuffer CommandBuffer;
        public bool UseFrameBufferResolution;
    };

    public static class GameSystem
    {
        public static OrthographicCamera2D OrthographicCamera { get; set; }
        public static Guid TileSetId { get; set; }
        public static Guid LevelRendererId { get; set; }
        public static Guid SpriteRenderPass2DId { get; set; }
        public static Guid FrameBufferId { get; set; }

        /*   public static extern IntPtr VulkanRenderPass_CreateVulkanRenderPassCS(
        ref RendererStateCS renderStateCS,

        [MarshalAs(UnmanagedType.LPStr)] string renderPassLoader,
        VkExtent2D renderPassResolution,
            int ConstBuffer,
            TextureStruct* renderedTextureListPtr,
            ulong* renderedTextureCount,
            TextureStruct* depthTexture
        );*/

        public static unsafe void StartUp(IntPtr window, IntPtr renderAreaHandle)
        {
            RenderSystem.CreateVulkanRenderer(window, renderAreaHandle);

            ivec2 renderResolution = new ivec2((int)RenderSystem.SwapChainResolution.width, (int)RenderSystem.SwapChainResolution.height);
            ListPtr<TextureStruct> textureList = new ListPtr<TextureStruct>(32);
            TextureStruct depthTexture = new TextureStruct();
            var count = (ulong)textureList.Count;
            var renderSystemStruct = RenderSystem.ToStruct();
            var jsonText = "C:/Users/dotha/Documents/GitHub/VulkanGameEngine/RenderPass/LevelShader2DRenderPass.json";
            var asdf = RenderSystem.SwapChainResolution;
            var sf = textureList[0];
            VulkanRenderPassDLL* a = GameEngineImport.VulkanRenderPass_CreateVulkanRenderPassCS(ref renderSystemStruct, jsonText, ref asdf, sizeof(SceneDataBuffer), ref sf, ref count, ref depthTexture);
            LoadLevel("C:/Users/dotha/Documents/GitHub/VulkanGameEngine/Levels/TestLevel.json");                                                                                                  
        }
        public static void LoadLevel(String levelPath)
        {
            var res = new vec2((float)RenderSystem.SwapChainResolution.width, (float)RenderSystem.SwapChainResolution.height);
            var pos = new vec2(0.0f, 0.0f);
            OrthographicCamera = new OrthographicCamera2D(res, pos);

            string jsonContent = File.ReadAllText(levelPath);
            LevelLoader levelLoader = JsonConvert.DeserializeObject<LevelLoader>(levelPath);

            foreach (var texturePath in levelLoader.TextureList)
            {
                TextureSystem.LoadTexture(texturePath);
            }
            foreach (var texturePath in levelLoader.TextureList)
            {
                TextureSystem.LoadTexture(texturePath);
            }
            foreach (var texturePath in levelLoader.TextureList)
            {
                TextureSystem.LoadTexture(texturePath);
            }
            foreach (var texturePath in levelLoader.TextureList)
            {
                TextureSystem.LoadTexture(texturePath);
            }
        }
    }
}
