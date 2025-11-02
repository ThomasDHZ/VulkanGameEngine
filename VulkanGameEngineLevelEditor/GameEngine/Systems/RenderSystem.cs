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


namespace VulkanGameEngineLevelEditor.GameEngine.Systems
{
    public unsafe struct RenderPassAttachementTextures
    {
        public size_t RenderPassTextureCount {  get; set; }
        public Texture* RenderPassTexture {  get; set; }
        public Texture* DepthTexture {  get; set; }
    };

    public unsafe static class RenderSystem
    {
        public static readonly WindowType windowType = WindowType.Win32;
        public static void* RenderAreaHandle = null;
        public static GraphicsRenderer renderer { get; set; }
        public static Dictionary<Guid, VulkanRenderPass> RenderPassMap { get; set; } = new Dictionary<Guid, VulkanRenderPass>();
        public static Dictionary<Guid, ListPtr<VulkanPipeline>> RenderPipelineMap { get; set; } = new Dictionary<Guid, ListPtr<VulkanPipeline>>();
        public static Dictionary<Guid, RenderPassLoaderModel> RenderPassEditor_RenderPass = new Dictionary<Guid, RenderPassLoaderModel>();
        public static Dictionary<Guid, string> RenderPassLoaderJsonMap = new Dictionary<Guid, string>();
        public static Dictionary<Guid, List<string>> RenderPipelineLoaderJsonMap = new Dictionary<Guid, List<string>>();
        public static VkCommandBufferBeginInfo CommandBufferBeginInfo { get; set; } = new VkCommandBufferBeginInfo();
        public static bool RebuildRendererFlag { get; set; }

        public static void CreateVulkanRenderer(WindowType windowType, void* renderAreaHandle, void* debuggerHandle)
        {
            RenderAreaHandle = renderAreaHandle;
            VkInstance instance = Renderer_CreateVulkanInstance();
            VkDebugUtilsMessengerEXT debugMessenger = Renderer_SetupDebugMessenger(instance);
            VkSurfaceKHR surface = Renderer_CreateVulkanSurface(RenderAreaHandle, instance);
            renderer = DLLSystem.CallDLLFunc(() => RenderSystem_StartUp(RenderAreaHandle, ref instance, ref surface, ref debugMessenger));
        }

        public static void LoadRenderPass(Guid levelId, string jsonPath, ivec2 renderPassResolution)
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_LoadRenderPass(levelId, jsonPath, renderPassResolution));
        }

        public static void Update(Guid spriteRenderPass2DId, Guid levelId, float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_Update(RenderAreaHandle, spriteRenderPass2DId, levelId, deltaTime));
        }

        public static void RecreateSwapchain(Guid spriteRenderPass2DId, Guid levelId, float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_RecreateSwapChain(RenderAreaHandle, spriteRenderPass2DId, levelId, deltaTime));
        }

        public static VkResult StartFrame()
        {
            return DLLSystem.CallDLLFunc(() => RenderSystem_StartFrame());
        }

        public static unsafe VkResult EndFrame(ListPtr<VkCommandBuffer> commandBufferSubmitList)
        {
            return DLLSystem.CallDLLFunc(() => RenderSystem_EndFrame(commandBufferSubmitList.Ptr, commandBufferSubmitList.Count));
        }

        public static VkCommandBuffer BeginSingleTimeCommands()
        {
            return Renderer_BeginSingleTimeCommands(renderer.Device, renderer.CommandPool);
        }

        public static VkCommandBuffer BeginSingleTimeCommands(VkCommandPool commandPool)
        {
            return Renderer_BeginSingleTimeCommands(renderer.Device, renderer.CommandPool);
        }

        public static VkResult EndSingleTimeCommands(VkCommandBuffer commandBuffer)
        {
            return Renderer_EndSingleTimeCommands(renderer.Device, renderer.CommandPool, renderer.GraphicsQueue, commandBuffer);
        }

        public static VkResult EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool commandPool)
        {
            return Renderer_EndSingleTimeCommands(renderer.Device, commandPool, renderer.GraphicsQueue, commandBuffer);
        }

        public static void DestroyRenderPasses()
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_DestroyRenderPasses());
        }

        public static void DestroyRenderPipelines()
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_DestroyRenderPipelines());
        }

        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern VkInstance Renderer_CreateVulkanInstance();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern VkDebugUtilsMessengerEXT Renderer_SetupDebugMessenger(VkInstance instance);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern VkSurfaceKHR Renderer_CreateVulkanSurface(void* windowHandle, VkInstance instance);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern VkCommandBuffer Renderer_BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern VkResult Renderer_EndSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);


        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern GraphicsRenderer RenderSystem_StartUp(void* windowHandle, ref VkInstance instance, ref VkSurfaceKHR surface, ref VkDebugUtilsMessengerEXT debugMessenger);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_Update(void* windowHandle, Guid spriteRenderPass2DId, Guid levelId, float deltaTime);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Guid RenderSystem_LoadRenderPass(Guid levelId, [MarshalAs(UnmanagedType.LPStr)] String jsonPath, ivec2 renderPassResolution);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_RecreateSwapChain(void* windowHandle, Guid spriteRenderPass2DId, Guid levelId, float deltaTime);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern VulkanRenderPass RenderSystem_FindRenderPass(Guid guid);
        //[DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern Vector<VulkanPipeline>& RenderSystem_FindRenderPipelineList(const RenderPassGuid& guid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern VkResult RenderSystem_StartFrame();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern VkResult RenderSystem_EndFrame(VkCommandBuffer* commandBufferListPtr, size_t commandBufferCount);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_DestroyRenderPasses();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_DestroyRenderPipelines();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] private static extern void RenderSystem_Destroy();
    }
}
