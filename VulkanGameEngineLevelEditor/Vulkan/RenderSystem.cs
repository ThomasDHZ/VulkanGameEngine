using GlmSharp;
using Newtonsoft.Json;
using Silk.NET.SDL;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http.Json;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.Models;

namespace VulkanGameEngineLevelEditor.Vulkan
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public struct TextureJsonLoader
    {
        public string TextureFilePath { get; set; }
        public Guid TextureId { get; set; }
        public VkFormat TextureByteFormat { get; set; }
        public VkImageAspectFlagBits ImageType { get; set; }
        public TextureTypeEnum TextureType { get; set; }
        public bool UseMipMaps { get; set; }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public struct MaterialJsonLoader
    {
        Guid MaterialId;
        uint MaterialBufferIndex;

        vec3 Albedo;
        float Metallic;
        float Roughness;
        float AmbientOcclusion;
        vec3 Emission;
        float Alpha;

        Guid AlbedoMapId;
        Guid MetallicRoughnessMapId;
        Guid MetallicMapId;
        Guid RoughnessMapId;
        Guid AmbientOcclusionMapId;
        Guid NormalMapId;
        Guid DepthMapId;
        Guid AlphaMapId;
        Guid EmissionMapId;
        Guid HeightMapId;

        VulkanBuffer<MaterialProperitiesBuffer> MaterialBuffer;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct RendererStateCS
    {
        public VkInstance Instance { get; set; }
        public VkDevice Device { get; set; }
        public VkPhysicalDevice PhysicalDevice { get; set; }
        public VkSurfaceKHR Surface { get; set; }
        public VkCommandPool CommandPool { get; set; }
        public VkDebugUtilsMessengerEXT DebugMessenger { get; set; }

        public VkFence* InFlightFences { get; set; }
        public VkSemaphore* AcquireImageSemaphores { get; set; }
        public VkSemaphore* PresentImageSemaphores { get; set; }
        public VkImage* SwapChainImages { get; set; }
        public VkImageView* SwapChainImageViews { get; set; }
        public VkExtent2D SwapChainResolution { get; set; }
        public VkSwapchainKHR Swapchain { get; set; }

        public uint SwapChainImageCount { get; set; }
        public uint InFlightFencesCount { get; set; }
        public uint AcquireImageSemaphoresCount { get; set; }
        public uint PresentImageSemaphoresCount { get; set; }
        public uint SwapChainImagesCount { get; set; }
        public uint SwapChainImageViewsCount { get; set; }

        public uint ImageIndex { get; set; }
        public uint CommandIndex { get; set; }
        public uint GraphicsFamily { get; set; }
        public uint PresentFamily { get; set; }

        public VkQueue GraphicsQueue { get; set; }
        public VkQueue PresentQueue { get; set; }
        public VkFormat Format { get; set; }
        public VkColorSpaceKHR ColorSpace { get; set; }
        public VkPresentModeKHR PresentMode { get; set; }

        public bool RebuildRendererFlag { get; set; }
    }

    public struct VulkanRenderPassStruct
    {
        public Guid RenderPassId { get; set; }
        public VkSampleCountFlagBits SampleCount { get; set; }
        public VkRect2D RenderArea { get; set; }
        public VkRenderPass RenderPass { get; set; }
        public List<VkFramebuffer> FrameBufferList { get; set; }
        public List<VkClearValue> ClearValueList { get; set; }
        public VkCommandBuffer CommandBuffer { get; set; }
        public bool UseFrameBufferResolution { get; set; }
    };

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public struct VulkanRenderPassCS
    {
        public Guid RenderPassId { get; set; }
        public VkSampleCountFlagBits SampleCount { get; set; }
        public VkRect2D RenderArea { get; set; }
        public VkRenderPass RenderPass { get; set; }
        public VkFramebuffer FrameBufferList { get; set; }
        public VkClearValue ClearValueList { get; set; }
        public int FrameBufferCount { get; set; }
        public int ClearValueCount { get; set; }
        public VkCommandBuffer CommandBuffer { get; set; }
        public bool UseFrameBufferResolution { get; set; }
    };

    public unsafe static class RenderSystem
    {
        public static uint SwapChainImageCount  { get; set; } 
        public static uint GraphicsFamily { get; set; }
        public static uint PresentFamily { get; set; }
        public static uint ImageIndex { get; set; }
        public static uint CommandIndex { get; set; }

        public static VkInstance Instance { get; set; } = VulkanConst.VK_NULL_HANDLE;
        public static VkDevice Device { get; set; } = VulkanConst.VK_NULL_HANDLE;
        public static VkPhysicalDevice PhysicalDevice { get; set; } = VulkanConst.VK_NULL_HANDLE;
        public static VkSurfaceKHR Surface { get; set; } = VulkanConst.VK_NULL_HANDLE;
        public static VkCommandPool CommandPool { get; set; } = VulkanConst.VK_NULL_HANDLE;
        public static VkDebugUtilsMessengerEXT DebugMessenger { get; set; } = VulkanConst.VK_NULL_HANDLE;
        public static VkExtent2D SwapChainResolution { get; set; }
        public static VkSwapchainKHR Swapchain { get; set; } = VulkanConst.VK_NULL_HANDLE;
        public static VkQueue GraphicsQueue { get; set; } = VulkanConst.VK_NULL_HANDLE;
        public static VkQueue PresentQueue { get; set; } = VulkanConst.VK_NULL_HANDLE;
        public static ListPtr<VkFence> InFlightFences { get; set; } = new ListPtr<VkFence>();
        public static ListPtr<VkSemaphore> AcquireImageSemaphores { get; set; } = new ListPtr<VkSemaphore>();
        public static ListPtr<VkSemaphore> PresentImageSemaphores { get; set; } = new ListPtr<VkSemaphore>();
        public static ListPtr<VkImage> SwapChainImages { get; set; } = new ListPtr<VkImage>();
        public static ListPtr<VkImageView> SwapChainImageViews { get; set; } = new ListPtr<VkImageView>();

        public static Dictionary<Guid, VulkanRenderPassStruct> RenderPassList { get; set; }
        public static bool RebuildRendererFlag { get; set; }

        public static void CreateVulkanRenderer(IntPtr window, IntPtr renderAreaHandle)
        {
            RendererStateCS renderStateCS = GameEngineImport.Renderer_RendererSetUp_CS(window.ToPointer());

            AcquireImageSemaphores = new ListPtr<VkSemaphore>(renderStateCS.AcquireImageSemaphores, renderStateCS.AcquireImageSemaphoresCount);
            CommandIndex = renderStateCS.CommandIndex;
            CommandPool = renderStateCS.CommandPool;
            DebugMessenger = renderStateCS.DebugMessenger;
            Device = renderStateCS.Device;
            GraphicsFamily = renderStateCS.GraphicsFamily;
            GraphicsQueue = renderStateCS.GraphicsQueue;
            ImageIndex = renderStateCS.ImageIndex;
            InFlightFences = new ListPtr<VkFence>(renderStateCS.InFlightFences, renderStateCS.InFlightFencesCount);
            Instance = renderStateCS.Instance;
            PhysicalDevice = renderStateCS.PhysicalDevice;
            PresentFamily = renderStateCS.PresentFamily;
            PresentImageSemaphores = new ListPtr<VkSemaphore>(renderStateCS.PresentImageSemaphores, renderStateCS.PresentImageSemaphoresCount);
            PresentQueue = renderStateCS.PresentQueue;
            Surface = renderStateCS.Surface;
            Swapchain = renderStateCS.Swapchain;
            SwapChainImageCount = renderStateCS.SwapChainImageCount;
            SwapChainImages = new ListPtr<VkImage>(renderStateCS.SwapChainImages, renderStateCS.SwapChainImagesCount);
            SwapChainImageViews = new ListPtr<VkImageView>(renderStateCS.SwapChainImageViews, renderStateCS.SwapChainImageViewsCount);
            SwapChainResolution = renderStateCS.SwapChainResolution;

       //     GameEngineImport.Texture_CreateTexture_DLL(RenderSystem.ToStruct(), "C:\\Users\\dotha\\Documents\\GitHub\\VulkanGameEngine\\Textures\\SparkManTexture.json", VkImageAspectFlagBits.VK_IMAGE_ASPECT_COLOR_BIT, VkCreate);

        }

        public static Guid AddRenderPass(Guid levelId, string jsonPath, ivec2 renderPassResolution)
        {
            string jsonContent = File.ReadAllText(jsonPath);
            RenderPassBuildInfoModel model = JsonConvert.DeserializeObject<RenderPassBuildInfoModel>(jsonContent);
            //  RenderPassList[model.RenderPassId] = RenderPass_CreateVulkanRenderPass(cRenderer, model, renderPassResolution, sizeof(SceneDataBuffer), textureSystem.RenderedTextureList[model.RenderPassId], textureSystem.DepthTextureList[model.RenderPassId]);

            return new Guid();
        }

        public static VkResult StartFrame()
        {
            CommandIndex = (CommandIndex + 1) % VulkanConst.MAX_FRAMES_IN_FLIGHT;

            var fence = InFlightFences[(int)CommandIndex];
            var imageSemaphore = AcquireImageSemaphores[(int)CommandIndex];

            VkFunc.vkWaitForFences(Device, 1, &fence, true, ulong.MaxValue);
            VkFunc.vkResetFences(Device, 1, &fence);

            VkResult result = VkFunc.vkAcquireNextImageKHR(Device, Swapchain, ulong.MaxValue, imageSemaphore, fence, out var imageIndex);
            ImageIndex = imageIndex;

            if (result == VkResult.VK_ERROR_OUT_OF_DATE_KHR)
            {
                RebuildRendererFlag = true;
                return result;
            }

            return result;
        }

        public static unsafe VkResult EndFrame(ListPtr<VkCommandBuffer> commandBufferSubmitList)
        {
            var fence = InFlightFences[(int)CommandIndex];
            var presentSemaphore = PresentImageSemaphores[(int)CommandIndex];
            var imageSemaphore = AcquireImageSemaphores[(int)CommandIndex];

            VkFunc.vkWaitForFences(Device, 1, &fence, true, ulong.MaxValue);
            VkFunc.vkResetFences(Device, 1, &fence);
            InFlightFences[(int)CommandIndex] = fence;

            VkPipelineStageFlagBits[] waitStages = new VkPipelineStageFlagBits[]
            {
                VkPipelineStageFlagBits.VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
            };

            fixed (VkPipelineStageFlagBits* pWaitStages = waitStages)
            {
                VkSubmitInfo submitInfo = new VkSubmitInfo()
                {
                    sType = VkStructureType.VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    waitSemaphoreCount = 1,
                    pWaitSemaphores = &imageSemaphore,
                    pWaitDstStageMask = pWaitStages,
                    commandBufferCount = commandBufferSubmitList.UCount,
                    pCommandBuffers = commandBufferSubmitList.Ptr,
                    signalSemaphoreCount = 1,
                    pSignalSemaphores = &presentSemaphore
                };

                VkResult submitResult = VkFunc.vkQueueSubmit(GraphicsQueue, 1, &submitInfo, fence);
                if (submitResult != VkResult.VK_SUCCESS)
                {
                    return submitResult;
                }

                var imageIndex = ImageIndex;
                var swapchain = Swapchain;
                VkPresentInfoKHR presentInfo = new VkPresentInfoKHR()
                {
                    sType = VkStructureType.VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    waitSemaphoreCount = 1,
                    pWaitSemaphores = &presentSemaphore,
                    swapchainCount = 1,
                    pSwapchains = &swapchain,
                    pImageIndices = &imageIndex
                };

                VkResult result = VkFunc.vkQueuePresentKHR(PresentQueue, in presentInfo);
                if (result == VkResult.VK_ERROR_OUT_OF_DATE_KHR || result == VkResult.VK_SUBOPTIMAL_KHR)
                {
                    RebuildRendererFlag = true;
                }

                return result;
            }
        }

        public static uint GetMemoryType(uint typeFilter, VkMemoryPropertyFlagBits properties)
        {
            return GameEngineImport.DLL_Tools_GetMemoryType(RenderSystem.PhysicalDevice, typeFilter, properties);
        }

        public static VkCommandBuffer BeginSingleUseCommandBuffer()
        {
            VkCommandBuffer commandBuffer = new VkCommandBuffer();
            VkCommandBufferAllocateInfo allocInfo = new VkCommandBufferAllocateInfo()
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                level = VkCommandBufferLevel.VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                commandPool = CommandPool,
                commandBufferCount = 1
            };
            VkFunc.vkAllocateCommandBuffers(Device, in allocInfo, out commandBuffer);

            VkCommandBufferBeginInfo beginInfo = new VkCommandBufferBeginInfo()
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                flags = VkCommandBufferUsageFlagBits.VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            };
            VkFunc.vkBeginCommandBuffer(commandBuffer, &beginInfo);
            return commandBuffer;
        }

        public static VkResult EndSingleUseCommandBuffer(VkCommandBuffer commandBuffer)
        {
            VkSubmitInfo submitInfo = new VkSubmitInfo()
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_SUBMIT_INFO,
                commandBufferCount = 1,
                pCommandBuffers = &commandBuffer
            };

            VkFence fence = new VkFence();
            VkFunc.vkEndCommandBuffer(commandBuffer);
            VkFunc.vkQueueSubmit(GraphicsQueue, 1, &submitInfo, fence);
            VkFunc.vkQueueWaitIdle(GraphicsQueue);
            VkFunc.vkFreeCommandBuffers(Device, CommandPool, 1, &commandBuffer);
            return VkResult.VK_SUCCESS;
        }

        public static void CreateCommandBuffers(ListPtr<VkCommandBuffer> commandBufferList)
        {
            for (int x = 0; x < SwapChainImageViews.Count; x++)
            {
                VkCommandBufferAllocateInfo commandBufferAllocateInfo = new VkCommandBufferAllocateInfo()
                {
                    sType = VkStructureType.VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    commandPool = CommandPool,
                    level = VkCommandBufferLevel.VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    commandBufferCount = 1
                };

                VkCommandBuffer commandBuffer = new VkCommandBuffer();
                VkFunc.vkAllocateCommandBuffers(Device, in commandBufferAllocateInfo, out commandBuffer);
                commandBufferList.Add(commandBuffer);
            }
        }

        public static RendererStateCS ToStruct()
        {
            return new RendererStateCS()
            {
                AcquireImageSemaphores = AcquireImageSemaphores.Ptr,
                AcquireImageSemaphoresCount = AcquireImageSemaphores.UCount,
                CommandIndex = CommandIndex,
                CommandPool = CommandPool,
                DebugMessenger = DebugMessenger,
                Device = Device,
                GraphicsFamily = GraphicsFamily,
                GraphicsQueue = GraphicsQueue,
                ImageIndex = ImageIndex,
                InFlightFences = InFlightFences.Ptr,
                InFlightFencesCount = InFlightFences.UCount,
                Instance = Instance,
                PhysicalDevice = PhysicalDevice,
                PresentFamily = PresentFamily,
                PresentImageSemaphores = PresentImageSemaphores.Ptr,
                PresentImageSemaphoresCount = PresentImageSemaphores.UCount,
                PresentQueue = PresentQueue,
                RebuildRendererFlag = RebuildRendererFlag,
                Surface = Surface,
                Swapchain = Swapchain,
                SwapChainImageCount = SwapChainImageCount,
                SwapChainImages = SwapChainImages.Ptr,
                SwapChainImagesCount = SwapChainImages.UCount,
                SwapChainImageViews = SwapChainImageViews.Ptr,
                SwapChainImageViewsCount = SwapChainImageViews.UCount,
                SwapChainResolution = SwapChainResolution
            };
        }
    }
}
