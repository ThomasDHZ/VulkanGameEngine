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
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.Window;

namespace VulkanGameEngineLevelEditor.GameEngine
{
    public struct SpriteLayer
    {
        public uint InstanceCount { get; set; } = 0;
        public uint StartInstanceIndex { get; set; } = 0;
        public uint SpriteDrawLayer { get; set; } = uint.MaxValue;
        public SpriteLayer()
        {
        }
    };

    struct SpriteVram
    {
        public Guid VramSpriteID { get; set; } = Guid.Empty;
        public Guid SpriteMaterialID { get; set; } = Guid.Empty;
        public uint SpriteLayer { get; set; } = 0;
        public vec4 SpriteColor { get; set; } = new vec4(0.0f, 0.0f, 0.0f, 1.0f);
        public ivec2 SpritePixelSize { get; set; } = new ivec2();
        public vec2 SpriteScale { get; set; } = new vec2(1.0f, 1.0f);
        public ivec2 SpriteCells { get; set; } = new ivec2(0, 0);
        public vec2 SpriteUVSize { get; set; } = new vec2();
        public vec2 SpriteSize { get; set; } = new vec2(50.0f);
        public uint AnimationListID { get; set; } = 0;
        public SpriteVram()
        {
        }
    };

    public struct Sprite
    {
        public uint GameObjectId { get; set; } = uint.MaxValue;
        public uint SpriteInstanceId { get; set; } = 0;
        public uint CurrentAnimationId { get; set; } = 0;
        public uint CurrentFrame { get; set; } = 0;
        public uint SpriteLayer { get; set; } = 0;
        public ivec2 FlipSprite { get; set; } = new ivec2(0);
        public Guid SpriteVramId { get; set; } = Guid.Empty;
        public float CurrentFrameTime { get; set; } = 0.0f;
        public Sprite()
        {
        }
    };

    public struct SpriteComponent
    {

        public Guid spriteVramId { get; set; }
        public uint currentAnimationId { get; set; } = 0;
        public uint currentFrame { get; set; } = 0;
        public float frameTimeAccumulator { get; set; } = 0.0f;
        public bool flipX { get; set; } = false;
        public bool flipY { get; set; } = false;
        public int layer { get; set; } = 0;
        public vec4 tint { get; set; } = new vec4(1.0f);
        public SpriteComponent()
        {
        }
    };

    public struct Animation2D
    {
        public uint AnimationId { get; set; }
        public List<ivec2> FrameList { get; set; }
        public float FrameHoldTime { get; set; }
    };

    public enum TileColliderTypeEnum
    {
        kTileColliderNone,
        kTileCollidable
    };

    public static unsafe class SpriteSystem
    {
        public static void CreateSprite(uint gameObjectId, Guid spriteVramId)
        {
            DLLSystem.CallDLLFunc(() => SpriteSystem_CreateSprite(gameObjectId, spriteVramId));
        }

        public static void LoadSpriteVRAM(string spriteVramPath)
        {
            DLLSystem.CallDLLFunc(() => SpriteSystem_LoadSpriteVRAM(spriteVramPath));
        }

        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => SpriteSystem_Update(deltaTime));
        }

        public static void SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum)
        {
            DLLSystem.CallDLLFunc(() => SpriteSystem_SetSpriteAnimation(sprite, spriteAnimationEnum));
        }

        public static void FindSpriteVram(Guid vramSpriteId)
        {
            DLLSystem.CallDLLFunc(() => SpriteSystem_FindSpriteVram(vramSpriteId));
        }

        public static void FindSpriteAnimation(Guid vramId, AnimationListId animationId)
        {
            DLLSystem.CallDLLFunc(() => SpriteSystem_FindSpriteAnimation(vramId, animationId));
        }

        public static void SpriteVramExists(Guid vramId)
        {
            DLLSystem.CallDLLFunc(() => SpriteSystem_SpriteVramExists(vramId));
        }

        public static void Destroy(Sprite* sprite)
        {
            DLLSystem.CallDLLFunc(() => SpriteSystem_Destroy(sprite));
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void SpriteSystem_CreateSprite(uint gameObjectId, Guid spriteVramId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Guid SpriteSystem_LoadSpriteVRAM([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string spriteVramPath);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void SpriteSystem_Update(float deltaTime);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void SpriteSystem_SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern SpriteVram SpriteSystem_FindSpriteVram(Guid vramSpriteId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Animation2D SpriteSystem_FindSpriteAnimation(Guid vramId, AnimationListId animationId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern bool SpriteSystem_SpriteVramExists(Guid vramId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void SpriteSystem_Destroy(Sprite* sprite);
    }
}