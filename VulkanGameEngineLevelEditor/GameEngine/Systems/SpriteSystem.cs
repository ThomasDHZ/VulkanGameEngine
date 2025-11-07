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
    public struct Sprite
    {
        public enum SpriteAnimationEnum
        {
            kStanding,
            kWalking
        };

        public uint GameObjectId { get; set; } = uint.MaxValue;
        public uint SpriteID { get; set; } = uint.MaxValue;
        public uint CurrentAnimationID { get; set; } = uint.MaxValue;
        public uint CurrentFrame { get; set; } = uint.MaxValue;
        public uint SpriteLayer { get; set; } = uint.MaxValue;
        public uint SpriteInstance { get; set; } = uint.MaxValue;
        public Guid SpriteVramId = Guid.Empty;
        public float CurrentFrameTime { get; set; } = 0.0f;
        public bool SpriteAlive = true;
        public ivec2 FlipSprite { get; set; } = new ivec2(0);
        public vec2 LastSpritePosition { get; set; } = new vec2(0.0f);
        public vec2 LastSpriteRotation { get; set; } = new vec2(0.0f);
        public vec2 LastSpriteScale { get; set; } = new vec2(1.0f);
        public vec2 SpritePosition { get; set; } = new vec2(0.0f);
        public vec2 SpriteRotation { get; set; } = new vec2(0.0f);
        public vec2 SpriteScale { get; set; } = new vec2(1.0f);

        public Sprite()
        {
        }
    };

    public static unsafe class SpriteSystem
    {
        public static void AddSprite(GameObject gameObject, VramSpriteGuid spriteVramId)
        {
            DLLSystem.CallDLLFunc(() => SpriteSystem_AddSprite(gameObject, spriteVramId));
        }

        public static VramSpriteGuid LoadSpriteVRAM(string spriteVramPath)
        {
            return DLLSystem.CallDLLFunc(() => SpriteSystem_LoadSpriteVRAM(spriteVramPath));
        }

        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => SpriteSystem_Update(deltaTime));
        }

        public static void SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum)
        {
            DLLSystem.CallDLLFunc(() => SpriteSystem_SetSpriteAnimation(sprite, spriteAnimationEnum));
        }

        public static Sprite FindSprite(uint gameObjectId)
        {
            return DLLSystem.CallDLLFunc(() => SpriteSystem_FindSprite(gameObjectId));
        }

        public static SpriteVram FindSpriteVram(VramSpriteGuid VramSpriteID)
        {
            return DLLSystem.CallDLLFunc(() => SpriteSystem_FindSpriteVram(VramSpriteID));
        }

        public static Animation2D FindSpriteAnimation(VramSpriteGuid vramId, AnimationListId animationId)
        {
            return DLLSystem.CallDLLFunc(() => SpriteSystem_FindSpriteAnimation(vramId, animationId));
        }

        public static void Destroy()
        {
            DLLSystem.CallDLLFunc(() => SpriteSystem_Destroy());
        }

        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void SpriteSystem_AddSprite(GameObject gameObject, VramSpriteGuid spriteVramId);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern VramSpriteGuid SpriteSystem_LoadSpriteVRAM([MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] string spriteVramPath);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void SpriteSystem_Update(float deltaTime);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void SpriteSystem_SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern Sprite SpriteSystem_FindSprite(uint gameObjectId);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern SpriteVram SpriteSystem_FindSpriteVram(VramSpriteGuid VramSpriteID);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern Animation2D SpriteSystem_FindSpriteAnimation(VramSpriteGuid vramId, AnimationListId animationId);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void SpriteSystem_Destroy();
    }
}