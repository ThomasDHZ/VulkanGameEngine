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
            VkSurfaceKHR surface = Renderer_CreateVulkanSurface(renderAreaHandle, instance);
            DLLSystem.CallDLLFunc(() => RenderSystem_StartUp(renderAreaHandle, ref instance, ref surface, ref debugMessenger));
        }

        public static unsafe void LoadRenderPass(Guid levelId, string jsonPath, ivec2 renderPassResolution)
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_LoadRenderPass(levelId, jsonPath, renderPassResolution));
        }

        public static void Update(Guid spriteRenderPass2DId, Guid levelId, float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_Update(spriteRenderPass2DId, levelId, deltaTime));
        }

        public unsafe static void UpdateRenderPasses(Guid levelId, [MarshalAs(UnmanagedType.LPStr)] String jsonPath, ivec2 renderPassResolution)
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_LoadRenderPass(levelId, jsonPath, renderPassResolution));
        }

        public static void RecreateSwapchain(Guid spriteRenderPass2DId, Guid levelId, float deltaTime)
        {
            DLLSystem.CallDLLFunc(() => RenderSystem_RecreateSwapChain(RenderAreaHandle, spriteRenderPass2DId, levelId, deltaTime));
        }

        public static VkCommandBuffer RenderFrameBuffer(Guid renderPassId)
        {
            VulkanRenderPass renderPass = RenderPassMap[renderPassId];
            VulkanPipeline pipeline = RenderPipelineMap[renderPassId][0];
            VkCommandBuffer commandBuffer = renderPass.CommandBuffer;
            //ListPtr<Texture> renderPassTexture = TextureSystem.FindRenderedTextureList(LevelSystem.spriteRenderPass2DId);

            //VkViewport viewport = new VkViewport
            //{
            //    x = 0.0f,
            //    y = 0.0f,
            //    width = renderer.SwapChainResolution.width,
            //    height = renderer.SwapChainResolution.height,
            //    minDepth = 0.0f,
            //    maxDepth = 1.0f
            //};

            //VkRect2D scissor = new VkRect2D
            //{
            //    offset = new VkOffset2D
            //    {
            //        x = 0,
            //        y = 0
            //    },
            //    extent = new VkExtent2D
            //    {
            //        width = renderer.SwapChainResolution.width,
            //        height = renderer.SwapChainResolution.height
            //    }
            //};

            //VkRenderPassBeginInfo renderPassBeginInfo = new VkRenderPassBeginInfo
            //{
            //    sType = VkStructureType.VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            //    renderPass = renderPass.RenderPass,
            //    framebuffer = renderPass.FrameBufferList[(uint)renderer.ImageIndex],
            //    renderArea = scissor,
            //    clearValueCount = renderPass.ClearValueCount,
            //    pClearValues = renderPass.ClearValueList
            //};

            //var beginCommandbufferindo = CommandBufferBeginInfo;
            //VkFunc.vkBeginCommandBuffer(commandBuffer, &beginCommandbufferindo);
            //TextureSystem.UpdateTextureLayout(renderPassTexture[0], commandBuffer, VkImageLayout.VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            //VkFunc.vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
            //VkFunc.vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
            //VkFunc.vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VkSubpassContents.VK_SUBPASS_CONTENTS_INLINE);
            //VkFunc.vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
            //VkFunc.vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, (uint)pipeline.DescriptorSetCount, pipeline.DescriptorSetList, 0, null);
            //VkFunc.vkCmdDraw(commandBuffer, 6, 1, 0, 0);
            //VkFunc.vkCmdEndRenderPass(commandBuffer);
            //VkFunc.vkEndCommandBuffer(commandBuffer);
            return commandBuffer;
        }

        public static VkCommandBuffer RenderLevel(Guid renderPassId, Guid levelId, float deltaTime, ShaderPushConstant sceneDataBuffer)
        {
            VulkanRenderPass renderPass = RenderPassMap[renderPassId];
            VulkanPipeline spritePipeline = RenderPipelineMap[renderPassId][0];
            VulkanPipeline levelPipeline = RenderPipelineMap[renderPassId][1];
            ListPtr<SpriteBatchLayer> spriteLayerList = SpriteSystem.FindSpriteBatchLayer(renderPassId);
            ListPtr<Mesh> levelLayerList = MeshSystem.FindLevelLayerMeshList(levelId);
            ListPtr<VkClearValue> clearColorValues = new ListPtr<VkClearValue>(renderPass.ClearValueList, renderPass.ClearValueCount);
            VkCommandBuffer commandBuffer = renderPass.CommandBuffer;

            VkRenderPassBeginInfo renderPassBeginInfo = new VkRenderPassBeginInfo
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                renderPass = renderPass.RenderPass,
                framebuffer = renderPass.FrameBufferList[(int)renderer.ImageIndex],
                renderArea = renderPass.RenderArea,
                clearValueCount = clearColorValues.Count,
                pClearValues = clearColorValues.Ptr
            };

            var beginCommandbufferinfo = CommandBufferBeginInfo;
            VkFunc.vkBeginCommandBuffer(commandBuffer, &beginCommandbufferinfo);
            VkFunc.vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VkSubpassContents.VK_SUBPASS_CONTENTS_INLINE);
            foreach (var levelLayer in levelLayerList)
            {
                VkBuffer meshVertexBuffer = BufferSystem.VulkanBufferMap[(uint)levelLayer.MeshVertexBufferId].Buffer;
                VkBuffer meshIndexBuffer = BufferSystem.VulkanBufferMap[(uint)levelLayer.MeshIndexBufferId].Buffer;

                ListPtr<VkDeviceSize> offsets = new ListPtr<ulong> { 0 };
                VkFunc.vkCmdPushConstants(commandBuffer, levelPipeline.PipelineLayout, VkShaderStageFlagBits.VK_SHADER_STAGE_VERTEX_BIT | VkShaderStageFlagBits.VK_SHADER_STAGE_FRAGMENT_BIT, 0, (uint)sceneDataBuffer.PushConstantSize, sceneDataBuffer.PushConstantBuffer);
                VkFunc.vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.Pipeline);
                VkFunc.vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.PipelineLayout, 0, (uint)levelPipeline.DescriptorSetCount, levelPipeline.DescriptorSetList, 0, null);
                VkFunc.vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets.Ptr);
                VkFunc.vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VkIndexType.VK_INDEX_TYPE_UINT32);
                VkFunc.vkCmdDrawIndexed(commandBuffer, (uint)levelLayer.IndexCount, 1, 0, 0, 0);
            }
            //foreach (var spriteLayer in spriteLayerList)
            //{
            //    if (spriteLayerList.Ptr != null)
            //    {
            //        ListPtr<SpriteInstanceStruct> spriteInstanceList = SpriteSystem.FindSpriteInstanceList(spriteLayer.SpriteBatchLayerId);
            //        Mesh spriteMesh = MeshSystem.SpriteMeshMap[(int)spriteLayer.SpriteLayerMeshId];
            //        VkBuffer meshVertexBuffer = BufferSystem.VulkanBufferMap[(uint)spriteMesh.MeshVertexBufferId].Buffer;
            //        VkBuffer meshIndexBuffer = BufferSystem.VulkanBufferMap[(uint)spriteMesh.MeshIndexBufferId].Buffer;
            //        VkBuffer spriteInstanceBuffer = BufferSystem.VulkanBufferMap[(uint)SpriteSystem.FindSpriteInstanceBufferId(spriteLayer.SpriteBatchLayerId)].Buffer;

            //        ListPtr<VkDeviceSize> offsets = new ListPtr<ulong> { 0 };
            //        VkFunc.vkCmdPushConstants(commandBuffer, spritePipeline.PipelineLayout, VkShaderStageFlagBits.VK_SHADER_STAGE_VERTEX_BIT | VkShaderStageFlagBits.VK_SHADER_STAGE_FRAGMENT_BIT, 0, (uint)sceneDataBuffer.PushConstantSize, sceneDataBuffer.PushConstantBuffer);
            //        VkFunc.vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.Pipeline);
            //        VkFunc.vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.PipelineLayout, 0, (uint)spritePipeline.DescriptorSetCount, spritePipeline.DescriptorSetList, 0, null);
            //        VkFunc.vkCmdBindVertexBuffers(commandBuffer, 0, 1, &spriteInstanceBuffer, offsets.Ptr);
            //        VkFunc.vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VkIndexType.VK_INDEX_TYPE_UINT32);
            //        VkFunc.vkCmdDrawIndexed(commandBuffer, (uint)GameObjectSystem.SpriteIndexList.ToList().Count(), (uint)spriteInstanceList.Count, 0, 0, 0);
            //    }
            //}
            VkFunc.vkCmdEndRenderPass(commandBuffer);
            VkFunc.vkEndCommandBuffer(commandBuffer);
            return commandBuffer;
        }

        public static VkResult StartFrame()
        {

            var imageIndex = renderer.ImageIndex;
            var commandIndex = renderer.CommandIndex;
            var rebuildRendererFlag = renderer.RebuildRendererFlag;
            var result = Renderer_StartFrame(renderer.Device, renderer.Swapchain, renderer.InFlightFences, renderer.AcquireImageSemaphores, &imageIndex, &commandIndex, &rebuildRendererFlag);

            var rendererTemp = renderer;
            rendererTemp.ImageIndex = imageIndex;
            rendererTemp.CommandIndex = commandIndex;
            rendererTemp.RebuildRendererFlag = rebuildRendererFlag;
            renderer = rendererTemp;
            return result;
        }

        public static unsafe VkResult EndFrame(ListPtr<VkCommandBuffer> commandBufferSubmitList)
        {
            VkSwapchainKHR swapChain = renderer.Swapchain;
            VkSemaphore* acquireImageSemaphoreList = renderer.AcquireImageSemaphores;
            VkSemaphore* presentImageSemaphoreList = renderer.PresentImageSemaphores;
            VkFence* fenceList = renderer.InFlightFences;
            VkQueue graphicsQueue = renderer.GraphicsQueue;
            VkQueue presentQueue = renderer.PresentQueue;
            size_t commandIndex = renderer.CommandIndex;
            size_t imageIndex = renderer.ImageIndex;
            VkCommandBuffer* pCommandBufferSubmitList = commandBufferSubmitList.Ptr;
            size_t commandBufferCount = commandBufferSubmitList.Count;
            bool rebuildRendererFlag = renderer.RebuildRendererFlag;
            var result = Renderer_EndFrame(swapChain, acquireImageSemaphoreList, presentImageSemaphoreList, fenceList, graphicsQueue, presentQueue, commandIndex, imageIndex, pCommandBufferSubmitList, commandBufferCount, &rebuildRendererFlag);
            return result;
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
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern VkResult Renderer_StartFrame(VkDevice device, VkSwapchainKHR swapChain, VkFence* fenceList, VkSemaphore* acquireImageSemaphoreList, size_t* pImageIndex, size_t* pCommandIndex, bool* pRebuildRendererFlag);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern VkResult Renderer_EndFrame(VkSwapchainKHR swapChain, VkSemaphore* acquireImageSemaphoreList, VkSemaphore* presentImageSemaphoreList, VkFence* fenceList, VkQueue graphicsQueue, VkQueue presentQueue, size_t commandIndex, size_t imageIndex, VkCommandBuffer* pCommandBufferSubmitList, size_t commandBufferCount, bool* rebuildRendererFlag);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern VkCommandBuffer Renderer_BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern VkResult Renderer_EndSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);


        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern void RenderSystem_StartUp(void* windowHandle, ref VkInstance instance, ref VkSurfaceKHR surface, ref VkDebugUtilsMessengerEXT debugMessenger);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern void RenderSystem_Update(Guid spriteRenderPass2DId, Guid levelId, float deltaTime);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern Guid RenderSystem_LoadRenderPass(Guid levelId, [MarshalAs(UnmanagedType.LPStr)] String jsonPath, ivec2 renderPassResolution);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern void RenderSystem_RecreateSwapChain(void* windowHandle, Guid spriteRenderPass2DId, Guid levelId, float deltaTime);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern VulkanRenderPass RenderSystem_FindRenderPass(Guid guid);
        //[DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern Vector<VulkanPipeline>& RenderSystem_FindRenderPipelineList(const RenderPassGuid& guid);
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern void RenderSystem_DestroyRenderPasses();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern void RenderSystem_DestroyRenderPipelines();
        [DllImport(DLLSystem.GameEngineDLL, CallingConvention = CallingConvention.StdCall)] public static extern void RenderSystem_Destroy();
    }
}
