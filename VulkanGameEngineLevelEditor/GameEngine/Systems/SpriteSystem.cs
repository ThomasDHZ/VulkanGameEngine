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