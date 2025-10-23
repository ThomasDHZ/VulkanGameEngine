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
using VulkanGameEngineLevelEditor.GameEngine.GameObjectComponents;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.Window;

namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public static unsafe class SpriteSystem
    {
        private static uint NextSpriteBatchLayerID = 0;

        public static ListPtr<Sprite> SpriteList { get; set; } = new ListPtr<Sprite>();
        public static ListPtr<SpriteInstanceStruct> SpriteInstanceList { get; set; } = new ListPtr<SpriteInstanceStruct>();
        public static ListPtr<SpriteBatchLayer> SpriteBatchLayerList { get; set; } = new ListPtr<SpriteBatchLayer>();
        public static ListPtr<SpriteVram> SpriteVramList { get; set; } = new ListPtr<SpriteVram>();
        public static Dictionary<uint, size_t> SpriteIdToListIndexMap { get; set; } = new Dictionary<uint, size_t>();
        public static Dictionary<uint, uint> SpriteInstanceBufferIdMap { get; set; } = new Dictionary<uint, uint>();
        public static Dictionary<Guid, ListPtr<Animation2D>> SpriteAnimationMap { get; set; } = new Dictionary<Guid, ListPtr<Animation2D>>();
        public static Dictionary<Guid, ListPtr<vec2>> SpriteAnimationFrameListMap { get; set; } = new Dictionary<Guid, ListPtr<vec2>>();
        public static Dictionary<uint, ListPtr<SpriteInstanceStruct>> SpriteInstanceListMap { get; set; } = new Dictionary<uint, ListPtr<SpriteInstanceStruct>>();
        public static Dictionary<uint, ListPtr<uint>> SpriteBatchObjectListMap { get; set; } = new Dictionary<uint, ListPtr<uint>>();

        public static void Update(float deltaTime)
        {
            if (SpriteList.Count > 100)
            {
                UpdateBatchSprites(deltaTime);
            }
            else
            {
                UpdateSprites(deltaTime);
            }

            VkCommandBuffer commandBuffer = RenderSystem.BeginSingleTimeCommands();
            UpdateSpriteBatchLayers(deltaTime);
            RenderSystem.EndSingleTimeCommands(commandBuffer);
        }

        private static void UpdateSpriteBatchLayers(float deltaTime)
        {
            foreach (var spriteBatchLayer in SpriteBatchLayerList)
            {
                var spriteInstanceList = FindSpriteInstanceList(spriteBatchLayer.SpriteBatchLayerId);
                var spriteObjects = FindSpriteBatchObjectListMap((int)spriteBatchLayer.SpriteBatchLayerId);
                var spriteLayer = FindSpriteInstanceList(spriteBatchLayer.SpriteBatchLayerId);
                spriteLayer = new ListPtr<SpriteInstanceStruct>(spriteObjects.Count());

                for (var x = 0; x < spriteLayer.Count; x++)
                {
                    spriteLayer[x] = FindSpriteInstance(spriteObjects[x]);
                }

                if (spriteObjects.Any())
                {
                    uint bufferId = FindSpriteInstanceBufferId(spriteBatchLayer.SpriteBatchLayerId);
                    BufferSystem.UpdateBufferMemory(RenderSystem.renderer, bufferId, spriteLayer.ToList());
                }
            }
        }

        private static void UpdateBatchSprites(float deltaTime)
        {
            int count = (int)SpriteInstanceList.Count;
            var transform2D = new ListPtr<Transform2DComponent>(count);
            var vram = new ListPtr<SpriteVram>(count);
            var animation = new ListPtr<Animation2D>(count);
            var frameList = new ListPtr<vec2>(count);
            var material = new ListPtr<Material>(count);

            for (int x = 0; x < count; ++x)
            {
                var instance = SpriteInstanceList[x];
                var sprite = SpriteList[x];

                transform2D[x] = GameObjectSystem.Transform2DComponentMap[(int)sprite.GameObjectId];
                vram[x] = FindVramSprite(sprite.SpriteVramId);
                animation[x] = FindSpriteAnimation(vram[x].VramSpriteId, sprite.CurrentAnimationID);
                material[x] = MaterialSystem.FindMaterial(vram[x].VramSpriteId);
            }

            Sprite_UpdateBatchSprites(
                SpriteInstanceList.Ptr, SpriteList.Ptr,
                transform2D.Ptr, vram.Ptr, animation.Ptr,
                frameList.Ptr, material.Ptr, SpriteInstanceList.Count, deltaTime
            );
        }

        private static void UpdateSprites(float deltaTime)
        {
            for (int x = 0; x < SpriteInstanceList.Count; x++)
            {
                var sprite = SpriteList[x];
                var transform2D = GameObjectSystem.Transform2DComponentMap[(int)sprite.GameObjectId];
                var vram = FindVramSprite(sprite.SpriteVramId);
                var animation = FindSpriteAnimation(vram.VramSpriteId, sprite.CurrentAnimationID);
                var material = MaterialSystem.MaterialMap[vram.MaterialId];
                var currentFrame = animation.FrameList[sprite.CurrentFrame];
                SpriteInstanceList[x] = Sprite_UpdateSprites(ref transform2D, ref vram, ref animation, ref material, currentFrame, ref sprite, animation.FrameCount, deltaTime);
                SpriteList[x] = sprite;
            }
        }

        // Public methods
        public static void AddSprite(uint gameObjectId, Guid spriteVramId)
        {
            Sprite sprite = new Sprite
            {
                GameObjectId = gameObjectId,
                SpriteVramId = spriteVramId
            };
            SpriteList.Add(sprite);
            SpriteInstanceList.Add(new SpriteInstanceStruct());
            SpriteIdToListIndexMap[gameObjectId] = SpriteList.Count();
        }

        public static void AddSpriteInstanceBufferId(uint spriteInstanceBufferId, uint BufferId)
        {
            SpriteInstanceBufferIdMap[spriteInstanceBufferId] = BufferId;
        }

        public static void AddSpriteInstanceLayerList(uint spriteBatchId, ListPtr<SpriteInstanceStruct> spriteInstanceList)
        {
            SpriteInstanceListMap[spriteBatchId] = spriteInstanceList;
        }

        public static void AddSpriteBatchObjectList(uint spriteBatchId, ListPtr<uint> spriteBatchObject)
        {
            SpriteBatchObjectListMap.Add(spriteBatchId, spriteBatchObject);
        }

        public static Guid LoadSpriteVRAM(string spriteVramPath)
        {
            return Sprite_LoadSpriteVRAM(spriteVramPath);
        }

        // Find functions
        public static Sprite FindSprite(int gameObjectId)
        {
            return SpriteList.Where(x => x.GameObjectId == gameObjectId).First();
        }

        public static SpriteVram FindVramSprite(Guid vramSpriteId)
        {
            return SpriteVramList.Where(x => x.VramSpriteId == vramSpriteId).First();
        }

        public static Animation2D FindSpriteAnimation(Guid vramId, uint animationId)
        {
            return SpriteAnimationMap.Where(x => x.Key == vramId).First().Value[(int)animationId];
        }

        public static SpriteInstanceStruct FindSpriteInstance(uint gameObjectId)
        {
            if (SpriteInstanceList.Count() <= 200)
            {
                uint spriteInstanceIndex = FindSpriteIndex(gameObjectId);
                return SpriteInstanceList[(int)spriteInstanceIndex];
            }
            else
            {
                var spriteInstanceIndex = SpriteIdToListIndexMap.Where(x => x.Key == gameObjectId).First().Value;
                return SpriteInstanceList[(int)spriteInstanceIndex];
            }
        }

        public static uint FindSpriteInstanceBufferId(uint spriteInstanceBufferId)
        {
            return SpriteInstanceBufferIdMap.Where(x => x.Key == spriteInstanceBufferId).First().Value;
        }

        public static ListPtr<SpriteInstanceStruct> FindSpriteInstanceList(uint spriteBatchId)
        {
            return SpriteInstanceListMap.Where(x => x.Key == spriteBatchId).First().Value;
        }

        public static ListPtr<uint> FindSpriteBatchObjectListMap(int spriteBatchObjectListId)
        {
            return SpriteBatchObjectListMap.Where(x => x.Key == spriteBatchObjectListId).First().Value;
        }

        public static ListPtr<SpriteBatchLayer> FindSpriteBatchLayer(Guid renderPassId)
        {
            return new ListPtr<SpriteBatchLayer>(SpriteBatchLayerList.Where(x => x.RenderPassId == renderPassId).ToList());
        }

        public static uint FindSpriteIndex(uint gameObjectId)
        {
            var sprite = SpriteList.Where(x => x.GameObjectId == gameObjectId).First();
            return (uint)SpriteList.ToList().IndexOf(sprite);
        }

        [DllImport(GameEngineImport.Game2DPath, CallingConvention = CallingConvention.StdCall)] private static extern Guid Sprite_LoadSpriteVRAM([MarshalAs(UnmanagedType.LPStr)] string spritePath);
        [DllImport(GameEngineImport.Game2DPath, CallingConvention = CallingConvention.StdCall)] public static extern void Sprite_UpdateBatchSprites(SpriteInstanceStruct* spriteInstanceList, Sprite* spriteList, Transform2DComponent* transform2DList, SpriteVram* vramList, Animation2D* animationList, vec2* frameList, Material* materialList, size_t spriteCount, float deltaTime);
        [DllImport(GameEngineImport.Game2DPath, CallingConvention = CallingConvention.StdCall)]public static extern SpriteInstanceStruct Sprite_UpdateSprites(ref Transform2DComponent transform2D, ref SpriteVram vram, ref Animation2D animation, ref Material material, ivec2 currentFrame, ref Sprite sprite, size_t frameCount, float deltaTime);
        [DllImport(GameEngineImport.Game2DPath, CallingConvention = CallingConvention.StdCall)] public static extern void Sprite_SetSpriteAnimation(Sprite* sprite, Sprite.SpriteAnimationEnum spriteAnimation);
    }
}