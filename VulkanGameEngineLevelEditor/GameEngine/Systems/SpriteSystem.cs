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
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void SpriteSystem_AddSprite(GameObject gameObject, VramSpriteGuid spriteVramId);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern VramSpriteGuid SpriteSystem_LoadSpriteVRAM(const char* spriteVramPath);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void SpriteSystem_Update(float deltaTime);
    [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void SpriteSystem_SetSpriteAnimation(Sprite* sprite, uint spriteAnimationEnum);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern Sprite* SpriteSystem_FindSprite(uint gameObjectId);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern SpriteVram SpriteSystem_FindSpriteVram(VramSpriteGuid VramSpriteID);
        [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern Animation2D SpriteSystem_FindSpriteAnimation(VramSpriteGuid vramId, const UM_AnimationListID& animationId);
    [DllImport(DLLSystem.Game2DDLL, CallingConvention = CallingConvention.StdCall)] private static extern void SpriteSystem_Destroy();
    }
}