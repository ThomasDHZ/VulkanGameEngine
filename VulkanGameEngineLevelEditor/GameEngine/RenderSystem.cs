using AutoMapper.Features;
using GlmSharp;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net.Http.Json;
using System.Numerics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;
using Vulkan;
using VulkanGameEngineLevelEditor.GameEngine.Structs;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.LevelEditor;
using VulkanGameEngineLevelEditor.Models;
using static System.Windows.Forms.DataFormats;


namespace VulkanGameEngineLevelEditor.GameEngine
{
    public unsafe struct RenderPassAttachementTextures
    {
        public size_t RenderPassTextureCount { get; set; }
        public Texture* RenderPassTexture { get; set; }
        public Texture* DepthTexture { get; set; }
    };

    public unsafe static class RenderSystem
    {
        public static readonly WindowType windowType = WindowType.Win32;
        public static void* RenderAreaHandle = null;
        public static bool RebuildRendererFlag = true;
        public static Guid LevelId { get; set; } = new Guid("a4ff0086-0069-4265-9a81-b38fb9e6b5be");
        public static void CreateVulkanRenderer(WindowType windowType, void* renderAreaHandle, void* debuggerHandle)
        {
            RenderAreaHandle = renderAreaHandle;
            VulkanSystem.CreateVulkanInstance();
            VulkanSystem.CreateVulkanSurface();
            VulkanSystem.StartUpVulkan(windowType, renderAreaHandle, debuggerHandle);
        }

        public static RenderPassGuid LoadRenderPass(LevelGuid levelGuid, String jsonPath) 
        {
            return DLLSystem.CallDLLFunc(() => RenderSystem_LoadRenderPass(levelGuid, jsonPath));
        }

        public static void RebuildSwapChain(VulkanRenderPass vulkanRenderPass) 
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_RebuildSwapChain(vulkanRenderPass));
        }

        public static void Update(float deltaTime) 
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_Update(VulkanSystem.RenderAreaHandle, LevelId, deltaTime));
        }

        public static VulkanRenderPass FindRenderPass(RenderPassGuid renderPassGuid) 
        {
            return DLLSystem.CallDLLFunc(() => RenderSystem_FindRenderPass(renderPassGuid));
        }

        public static void Destroy() 
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_Destroy());
        }

        public static void DestroyRenderPass(VulkanRenderPass renderPass) 
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_DestroyRenderPass(renderPass));
        }

        public static void DestroyRenderPasses() 
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_DestroyRenderPasses());
        }

        public static void DestroyRenderPipelines() 
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_DestroyRenderPipelines());
        }

        public static void DestroyPipeline(VulkanPipeline vulkanPipelineDLL) 
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_DestroyPipeline(vulkanPipelineDLL));
        }

        public static void DestroyFrameBuffers(Vector<VkFramebuffer> frameBufferList) 
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_DestroyFrameBuffers(frameBufferList));
        }

        public static void DestroyCommandBuffers(VkCommandBuffer commandBuffer) 
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_DestroyCommandBuffers(commandBuffer));
        }

        public static void DestroyBuffer(VkBuffer buffer) 
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_DestroyBuffer(buffer));
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern RenderPassGuid RenderSystem_LoadRenderPass(LevelGuid levelGuid, [MarshalAs(UnmanagedType.LPStr)] String jsonPath);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_RebuildSwapChain(VulkanRenderPass vulkanRenderPass);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_Update(void* windowHandle, LevelGuid levelGuid, float deltaTime);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern VulkanRenderPass RenderSystem_FindRenderPass(RenderPassGuid renderPassGuid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_Destroy();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_DestroyRenderPass(VulkanRenderPass renderPass);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_DestroyRenderPasses();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_DestroyRenderPipelines();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_DestroyPipeline(VulkanPipeline vulkanPipelineDLL);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_DestroyFrameBuffers(Vector<VkFramebuffer> frameBufferList);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_DestroyCommandBuffers(VkCommandBuffer commandBuffer);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_DestroyBuffer(VkBuffer buffer);
    }
}
