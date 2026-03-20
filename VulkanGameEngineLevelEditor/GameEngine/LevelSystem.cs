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
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.LevelEditor.EditorEnhancements;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.GameEngine
{
    public static unsafe class LevelSystem
    {
        public static Guid TileSetId { get; set; }
        public static Guid LevelRendererId { get; set; }
        public static Guid SpriteRenderPass2DId { get; set; }
        public static Guid FrameBufferId { get; set; }
        public static void LoadLevel([MarshalAs(UnmanagedType.LPStr)] String levelPath)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_LoadLevel(levelPath));
        }

        public static void Update(float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_Update(deltaTime));
        }

        public static void RenderIrradianceMapRenderPass(VkCommandBuffer commandBuffer, Guid renderPassId, float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_RenderIrradianceMapRenderPass(commandBuffer, renderPassId, deltaTime));
        }

        public static void RenderPrefilterMapRenderPass(VkCommandBuffer commandBuffer, Guid renderPassId, float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_RenderPrefilterMapRenderPass(commandBuffer, renderPassId, deltaTime));
        }

        public static void RenderGBuffer(VkCommandBuffer commandBuffer, Guid renderPassId, Guid levelId, float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_RenderGBuffer(commandBuffer, renderPassId, levelId, deltaTime));
        }

        public static void RenderGaussianBlurPass(VkCommandBuffer commandBuffer, Guid renderPassId, uint blurDirection)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_RenderBloomPass(commandBuffer, renderPassId));
        }

        public static void RenderBloomPass(VkCommandBuffer commandBuffer, Guid renderPassId)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_RenderBloomPass(commandBuffer, renderPassId));
        }

        public static void RenderHdrPass(VkCommandBuffer commandBuffer, Guid renderPassId)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_RenderHdrPass(commandBuffer, renderPassId));
        }

        public static void RenderFrameBuffer(VkCommandBuffer commandBuffer, Guid renderPassId)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_RenderFrameBuffer(commandBuffer, renderPassId));
        }

        public static void Draw(VkCommandBuffer commandBuffer, float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => LevelSystem_Draw(commandBuffer, deltaTime));
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_LoadLevel([MarshalAs(UnmanagedType.LPStr)] String levelPath);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_Update(float deltaTime);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_RenderIrradianceMapRenderPass(VkCommandBuffer commandBuffer, Guid renderPassId, float deltaTime);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_RenderPrefilterMapRenderPass(VkCommandBuffer commandBuffer, Guid renderPassId, float deltaTime);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_RenderGBuffer(VkCommandBuffer commandBuffer, Guid renderPassId, Guid levelId, float deltaTime);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_RenderGaussianBlurPass(VkCommandBuffer commandBuffer, Guid renderPassId, uint blurDirection);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_RenderBloomPass(VkCommandBuffer commandBuffer, Guid renderPassId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_RenderHdrPass(VkCommandBuffer commandBuffer, Guid renderPassId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_RenderFrameBuffer(VkCommandBuffer commandBuffer, Guid renderPassId);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void LevelSystem_Draw(VkCommandBuffer commandBuffer, float deltaTime);
    }
}
