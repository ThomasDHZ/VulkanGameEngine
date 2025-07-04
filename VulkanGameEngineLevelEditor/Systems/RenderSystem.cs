﻿using AutoMapper.Features;
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
using System.Windows.Forms;
using VulkanCS;
using VulkanGameEngineLevelEditor.GameEngineAPI;
using VulkanGameEngineLevelEditor.Models;
using VulkanGameEngineLevelEditor.Systems;


namespace VulkanGameEngineLevelEditor.Systems
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct RenderPipelineJsonLoader
    {
        public String VertexShader { get; set; }
        public String FragmentShader { get; set; }
        public size_t DescriptorSetCount { get; set; }
        public size_t DescriptorSetLayoutCount { get; set; }
        public VertexTypeEnum VertexType { get; set; }
        public ListPtr<VkViewport> ViewportList { get; set; } = new ListPtr<VkViewport>();
        public ListPtr<VkRect2D> ScissorList { get; set; } = new ListPtr<VkRect2D>();
        public ListPtr<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStateList { get; set; } = new ListPtr<VkPipelineColorBlendAttachmentState>();
        public VkPipelineColorBlendStateCreateInfoModel PipelineColorBlendStateCreateInfoModel { get; set; } = new VkPipelineColorBlendStateCreateInfoModel();
        public VkPipelineRasterizationStateCreateInfoModel PipelineRasterizationStateCreateInfo { get; set; } = new VkPipelineRasterizationStateCreateInfoModel();
        public VkPipelineMultisampleStateCreateInfoModel PipelineMultisampleStateCreateInfo { get; set; } = new VkPipelineMultisampleStateCreateInfoModel();
        public VkPipelineDepthStencilStateCreateInfoModel PipelineDepthStencilStateCreateInfo { get; set; }
        public VkPipelineInputAssemblyStateCreateInfoModel PipelineInputAssemblyStateCreateInfo { get; set; } = new VkPipelineInputAssemblyStateCreateInfoModel();
        public ListPtr<VkDescriptorSetLayoutBindingModel> LayoutBindingList { get; set; } = new ListPtr<VkDescriptorSetLayoutBindingModel>();
        public ListPtr<PipelineDescriptorModel> PipelineDescriptorModelsList { get; set; } = new ListPtr<PipelineDescriptorModel>();
        public ListPtr<VkVertexInputBindingDescription> VertexInputBindingDescriptionList { get; set; } = new ListPtr<VkVertexInputBindingDescription>();
        public ListPtr<VkVertexInputAttributeDescription> VertexInputAttributeDescriptionList { get; set; } = new ListPtr<VkVertexInputAttributeDescription>();
        public ListPtr<VkClearValue> ClearValueList { get; set; } = new ListPtr<VkClearValue>();

        public RenderPipelineJsonLoader()
        {
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct RenderPipelineLoader
    {
        public string VertexShaderPath;
        public string FragmentShaderPath;
        public VertexTypeEnum VertexType;
        public size_t ViewportListCount;
        public size_t ScissorListCount;
        public size_t DescriptorSetCount;
        public size_t DescriptorSetLayoutCount;
        public size_t LayoutBindingListCount;
        public size_t PipelineDescriptorModelsListCount;
        public size_t PipelineColorBlendAttachmentStateListCount;
        public size_t VertexInputBindingDescriptionListCount;
        public size_t VertexInputAttributeDescriptionListCount;
        public VkViewport* ViewportList;
        public VkRect2D* ScissorList;
        public VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfoModel;
        public VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo;
        public VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo;
        public VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo;
        public VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo;
        public VkDescriptorSetLayoutBinding* LayoutBindingList;
        public PipelineDescriptorModel* PipelineDescriptorModelsList;
        public VkPipelineColorBlendAttachmentState* PipelineColorBlendAttachmentStateList;
        public VkVertexInputBindingDescription* VertexInputBindingDescriptionList;
        public VkVertexInputAttributeDescription* VertexInputAttributeDescriptionList;
    };

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
    public unsafe struct GraphicsRenderer
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

        public size_t SwapChainImageCount { get; set; }
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


    public unsafe struct RenderPipelineDLL
    {

        public String VertexShader { get; set; }
        public String FragmentShader { get; set; }
        public size_t DescriptorSetCount { get; set; }
        public size_t DescriptorSetLayoutCount { get; set; }
        public VertexTypeEnum VertexType { get; set; }
        public List<VkViewport> ViewportList { get; set; } = new List<VkViewport>();
        public List<VkRect2D> ScissorList { get; set; } = new List<VkRect2D>();
        public List<VkPipelineColorBlendAttachmentState> PipelineColorBlendAttachmentStateList { get; set; } = new List<VkPipelineColorBlendAttachmentState>();
        public VkPipelineColorBlendStateCreateInfoModel PipelineColorBlendStateCreateInfoModel { get; set; } = new VkPipelineColorBlendStateCreateInfoModel();
        public VkPipelineRasterizationStateCreateInfoModel PipelineRasterizationStateCreateInfo { get; set; } = new VkPipelineRasterizationStateCreateInfoModel();
        public VkPipelineMultisampleStateCreateInfoModel PipelineMultisampleStateCreateInfo { get; set; } = new VkPipelineMultisampleStateCreateInfoModel();
        public VkPipelineDepthStencilStateCreateInfoModel PipelineDepthStencilStateCreateInfo { get; set; }
        public VkPipelineInputAssemblyStateCreateInfoModel PipelineInputAssemblyStateCreateInfo { get; set; } = new VkPipelineInputAssemblyStateCreateInfoModel();
        public List<VkDescriptorSetLayoutBindingModel> LayoutBindingList { get; set; } = new List<VkDescriptorSetLayoutBindingModel>();
        //  public List<PipelineDescriptorModel> PipelineDescriptorModelsList { get; set; } = new List<PipelineDescriptorModel>();
        public List<VkVertexInputBindingDescription> VertexInputBindingDescriptionList { get; set; } = new List<VkVertexInputBindingDescription>();
        public List<VkVertexInputAttributeDescription> VertexInputAttributeDescriptionList { get; set; } = new List<VkVertexInputAttributeDescription>();
        public List<VkClearValue> ClearValueList { get; set; } = new List<VkClearValue>();
        public RenderPipelineDLL()
        {
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct VulkanPipeline
    {
        public uint RenderPipelineId { get; set; } = 0;
        public size_t DescriptorSetLayoutCount { get; set; } = 0;
        public size_t DescriptorSetCount { get; set; } = 0;
        public VkDescriptorPool DescriptorPool { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkDescriptorSetLayout* DescriptorSetLayoutList { get; set; } = null;
        public VkDescriptorSet* DescriptorSetList { get; set; } = null;
        public VkPipeline Pipeline { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkPipelineLayout PipelineLayout { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VkPipelineCache PipelineCache { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public VulkanPipeline()
        {
        }

    };

    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    public unsafe struct VulkanRenderPass
    {

        public Guid RenderPassId { get; set; } = Guid.Empty;
        public VkSampleCountFlagBits SampleCount { get; set; } = VkSampleCountFlagBits.VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
        public VkRect2D RenderArea { get; set; } = new VkRect2D();
        public VkRenderPass RenderPass { get; set; } = new VkRenderPass();
        public VkFramebuffer* FrameBufferList { get; set; } = null;
        public VkClearValue* ClearValueList { get; set; } = null;
        public size_t FrameBufferCount { get; set; } = 0;
        public size_t ClearValueCount { get; set; } = 0;
        public VkCommandBuffer CommandBuffer { get; set; }
        public bool UseFrameBufferResolution { get; set; }
        public VulkanRenderPass()
        {
        }
    };

    public unsafe static class RenderSystem
    {
        public static GraphicsRenderer renderer { get; set; }
        public static size_t SwapChainImageCount { get; set; }
        public static uint GraphicsFamily { get; set; }
        public static uint PresentFamily { get; set; }
        public static uint ImageIndex { get; set; }
        public static uint CommandIndex { get; set; }

        public static VkInstance Instance { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkDevice Device { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkPhysicalDevice PhysicalDevice { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkSurfaceKHR Surface { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkCommandPool CommandPool { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkDebugUtilsMessengerEXT DebugMessenger { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkExtent2D SwapChainResolution { get; set; }
        public static VkSwapchainKHR Swapchain { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkQueue GraphicsQueue { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static VkQueue PresentQueue { get; set; } = VulkanCSConst.VK_NULL_HANDLE;
        public static ListPtr<VkFence> InFlightFences { get; set; } = new ListPtr<VkFence>();
        public static ListPtr<VkSemaphore> AcquireImageSemaphores { get; set; } = new ListPtr<VkSemaphore>();
        public static ListPtr<VkSemaphore> PresentImageSemaphores { get; set; } = new ListPtr<VkSemaphore>();
        public static ListPtr<VkImage> SwapChainImages { get; set; } = new ListPtr<VkImage>();
        public static ListPtr<VkImageView> SwapChainImageViews { get; set; } = new ListPtr<VkImageView>();

        public static Dictionary<Guid, VulkanRenderPass> RenderPassList { get; set; } = new Dictionary<Guid, VulkanRenderPass>();
        public static Dictionary<Guid, ListPtr<VulkanPipeline>> RenderPipelineMap { get; set; } = new Dictionary<Guid, ListPtr<VulkanPipeline>>();
        public static VkCommandBufferBeginInfo CommandBufferBeginInfo { get; set; } = new VkCommandBufferBeginInfo();
        public static bool RebuildRendererFlag { get; set; }

        public static void CreateVulkanRenderer(VkQueue window, VkQueue renderAreaHandle)
        {
            renderer = Renderer_RendererSetUp_CS(window.ToPointer());

            AcquireImageSemaphores = new ListPtr<VkSemaphore>(renderer.AcquireImageSemaphores, renderer.SwapChainImageCount);
            CommandIndex = renderer.CommandIndex;
            CommandPool = renderer.CommandPool;
            DebugMessenger = renderer.DebugMessenger;
            Device = renderer.Device;
            GraphicsFamily = renderer.GraphicsFamily;
            GraphicsQueue = renderer.GraphicsQueue;
            ImageIndex = renderer.ImageIndex;
            InFlightFences = new ListPtr<VkFence>(renderer.InFlightFences, renderer.SwapChainImageCount);
            Instance = renderer.Instance;
            PhysicalDevice = renderer.PhysicalDevice;
            PresentFamily = renderer.PresentFamily;
            PresentImageSemaphores = new ListPtr<VkSemaphore>(renderer.PresentImageSemaphores, renderer.SwapChainImageCount);
            PresentQueue = renderer.PresentQueue;
            Surface = renderer.Surface;
            Swapchain = renderer.Swapchain;
            SwapChainImageCount = renderer.SwapChainImageCount;
            SwapChainImages = new ListPtr<VkImage>(renderer.SwapChainImages, renderer.SwapChainImageCount);
            SwapChainImageViews = new ListPtr<VkImageView>(renderer.SwapChainImageViews, renderer.SwapChainImageCount);
            SwapChainResolution = renderer.SwapChainResolution;
        }

        public static void Update(float deltaTime)
        {
            if (RebuildRendererFlag)
            {
                uint width = SwapChainResolution.width;
                uint height = SwapChainResolution.height;
                RecreateSwapchain();
                RebuildRendererFlag = false;
            }
        }

        public static void RecreateSwapchain()
        {
            /*  int width = 0;
              int height = 0;

              vkDeviceWaitIdle(*Device.get());

              vulkanWindow->GetFrameBufferSize(vulkanWindow, &width, &height);
              renderer.DestroySwapChainImageView();
              renderer.DestroySwapChain();
              renderer.SetUpSwapChain();

              RenderPassID id;
              id.id = 2;

              RenderPassList[id].RecreateSwapchain(width, height);*/
        }

        public static VkCommandBuffer RenderFrameBuffer(Guid renderPassId)
        {
             VulkanRenderPass renderPass = RenderPassList[renderPassId];
             VulkanPipeline pipeline = RenderPipelineMap[renderPassId][0];
             VkCommandBuffer commandBuffer = renderPass.CommandBuffer;

            VkRenderPassBeginInfo renderPassBeginInfo = new VkRenderPassBeginInfo
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                renderPass = renderPass.RenderPass,
                framebuffer = renderPass.FrameBufferList[(uint)renderer.ImageIndex],
                renderArea = renderPass.RenderArea,
                clearValueCount = renderPass.ClearValueCount,
                pClearValues = renderPass.ClearValueList
            };

            var beginCommandbufferindo = CommandBufferBeginInfo;
            VkFunc.vkResetCommandBuffer(commandBuffer, 0);
            VkFunc.vkBeginCommandBuffer(commandBuffer, &beginCommandbufferindo);
            VkFunc.vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VkSubpassContents.VK_SUBPASS_CONTENTS_INLINE);
            VkFunc.vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
            VkFunc.vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, (uint)pipeline.DescriptorSetCount, pipeline.DescriptorSetList, 0, null);
            VkFunc.vkCmdDraw(commandBuffer, 6, 1, 0, 0);
            VkFunc.vkCmdEndRenderPass(commandBuffer);
            VkFunc.vkEndCommandBuffer(commandBuffer);
            return commandBuffer;
        }

        public static VkCommandBuffer RenderLevel(Guid renderPassId, Guid levelId, float deltaTime, SceneDataBuffer sceneDataBuffer)
        {
            VulkanRenderPass renderPass = RenderPassList[renderPassId];
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
            VkFunc.vkResetCommandBuffer(commandBuffer, 0);
            VkFunc.vkBeginCommandBuffer(commandBuffer, &beginCommandbufferinfo);
            VkFunc.vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VkSubpassContents.VK_SUBPASS_CONTENTS_INLINE);

            foreach (var levelLayer in levelLayerList)
            {
                VkBuffer meshVertexBuffer = BufferSystem.VulkanBufferMap[(uint)levelLayer.MeshVertexBufferId].Buffer;
                VkBuffer meshIndexBuffer = BufferSystem.VulkanBufferMap[(uint)levelLayer.MeshIndexBufferId].Buffer;

                ListPtr<VkDeviceSize> offsets = new ListPtr<ulong> { 0 };
                VkFunc.vkCmdPushConstants(commandBuffer, levelPipeline.PipelineLayout, VkShaderStageFlagBits.VK_SHADER_STAGE_VERTEX_BIT | VkShaderStageFlagBits.VK_SHADER_STAGE_FRAGMENT_BIT, 0, (uint)sizeof(SceneDataBuffer), &sceneDataBuffer);
                VkFunc.vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.Pipeline);
                VkFunc.vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.PipelineLayout, 0, (uint)levelPipeline.DescriptorSetCount, levelPipeline.DescriptorSetList, 0, null);
                VkFunc.vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets.Ptr);
                VkFunc.vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VkIndexType.VK_INDEX_TYPE_UINT32);
                VkFunc.vkCmdDrawIndexed(commandBuffer, (uint)levelLayer.IndexCount, 1, 0, 0, 0);
            }
            foreach (var spriteLayer in spriteLayerList)
            {
                ListPtr<SpriteInstanceStruct> spriteInstanceList = SpriteSystem.FindSpriteInstanceList(spriteLayer.SpriteBatchLayerId);
                Mesh spriteMesh = MeshSystem.SpriteMeshMap[(int)spriteLayer.SpriteLayerMeshId];
                VkBuffer meshVertexBuffer = BufferSystem.VulkanBufferMap[(uint)spriteMesh.MeshVertexBufferId].Buffer;
                VkBuffer meshIndexBuffer = BufferSystem.VulkanBufferMap[(uint)spriteMesh.MeshIndexBufferId].Buffer;
                VkBuffer spriteInstanceBuffer = BufferSystem.VulkanBufferMap[(uint)SpriteSystem.FindSpriteInstanceBufferId(spriteLayer.SpriteBatchLayerId)].Buffer;

                ListPtr<VkDeviceSize> offsets = new ListPtr<ulong>{ 0 };
                VkFunc.vkCmdPushConstants(commandBuffer, spritePipeline.PipelineLayout, VkShaderStageFlagBits.VK_SHADER_STAGE_VERTEX_BIT | VkShaderStageFlagBits.VK_SHADER_STAGE_FRAGMENT_BIT, 0, (uint)sizeof(SceneDataBuffer), &sceneDataBuffer);
                VkFunc.vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.Pipeline);
                VkFunc.vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.PipelineLayout, 0, (uint)spritePipeline.DescriptorSetCount, spritePipeline.DescriptorSetList, 0, null);
                VkFunc.vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets.Ptr);
                VkFunc.vkCmdBindVertexBuffers(commandBuffer, 1, 1, &spriteInstanceBuffer, offsets.Ptr);
                VkFunc.vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VkIndexType.VK_INDEX_TYPE_UINT32);
                VkFunc.vkCmdDrawIndexed(commandBuffer, (uint)MeshSystem.IndexListMap.Count(), (uint)spriteInstanceList.Count, 0, 0, 0);
            }
            VkFunc.vkCmdEndRenderPass(commandBuffer);
            VkFunc.vkEndCommandBuffer(commandBuffer);
            return commandBuffer;
        }

        public static Guid LoadRenderPass(Guid levelId, string jsonPath, ivec2 renderPassResolution)
        {
            string jsonContent = File.ReadAllText(jsonPath);
            RenderPassBuildInfoModel model = JsonConvert.DeserializeObject<RenderPassBuildInfoModel>(jsonContent);

            VkExtent2D extent2D = new VkExtent2D
            {
                width = (uint)renderPassResolution.x,
                height = (uint)renderPassResolution.y
            };

            size_t renderedTextureCount = model.RenderedTextureInfoModelList.Where(x => x.TextureType == RenderedTextureType.ColorRenderedTexture).Count();
            ListPtr<Texture> renderedTextureListPtr = new ListPtr<Texture>(renderedTextureCount);
            VulkanRenderPass vulkanRenderPass = VulkanRenderPass_CreateVulkanRenderPass(RenderSystem.renderer, jsonPath, ref extent2D, sizeof(SceneDataBuffer), renderedTextureListPtr.Ptr, ref renderedTextureCount, out Texture depthTexture);
            RenderPassList[vulkanRenderPass.RenderPassId] = vulkanRenderPass;

            TextureSystem.RenderedTextureList[vulkanRenderPass.RenderPassId] = new ListPtr<Texture>();
            for (int x = 0; x < renderedTextureCount; x++)
            {
                TextureSystem.RenderedTextureList[vulkanRenderPass.RenderPassId].Add(renderedTextureListPtr[x]);
            }
            if (depthTexture.textureView != VulkanCSConst.VK_NULL_HANDLE)
            {
                TextureSystem.DepthTextureList[vulkanRenderPass.RenderPassId] = depthTexture;
            }

            ListPtr<VulkanPipeline> vulkanPipelineList = new ListPtr<VulkanPipeline>();
            foreach (var pipelineId in model.RenderPipelineList)
            {
                string fulRenderPassPath = Path.GetFullPath("C:\\Users\\dotha\\Documents\\GitHub\\VulkanGameEngine\\pipelines\\" + pipelineId);

                ListPtr<VkDescriptorBufferInfo> vertexPropertiesList = RenderSystem.GetVertexPropertiesBuffer();
                ListPtr<VkDescriptorBufferInfo> indexPropertiesList = RenderSystem.GetIndexPropertiesBuffer();
                ListPtr<VkDescriptorBufferInfo> transformPropertiesList = RenderSystem.GetGameObjectTransformBuffer();
                ListPtr<VkDescriptorBufferInfo> meshPropertiesList = RenderSystem.GetMeshPropertiesBuffer(levelId);
                //  ListPtr<VkDescriptorBufferInfo> levelLayerMeshPropertiesList = Vector<VkDescriptorBufferInfo>(includes.LevelLayerMeshProperties, includes.LevelLayerMeshProperties + includes.LevelLayerMeshPropertiesCount);
                ListPtr<VkDescriptorImageInfo> texturePropertiesList = RenderSystem.GetTexturePropertiesBuffer(vulkanRenderPass.RenderPassId, null);
                ListPtr<VkDescriptorBufferInfo> materialPropertiesList = MaterialSystem.GetMaterialPropertiesBuffer();
               
                GPUIncludes include = new GPUIncludes
                {
                    vertexPropertiesList = vertexPropertiesList.Ptr,
                    indexPropertiesList = indexPropertiesList.Ptr,
                    transformPropertiesList = transformPropertiesList.Ptr,
                    meshPropertiesList = meshPropertiesList.Ptr,
                    texturePropertiesList = texturePropertiesList.Ptr,
                    materialPropertiesList = materialPropertiesList.Ptr,
                    vertexPropertiesListCount = vertexPropertiesList.Count,
                    indexPropertiesListCount = indexPropertiesList.Count,
                    transformPropertiesListCount = transformPropertiesList.Count,
                    meshPropertiesListCount = meshPropertiesList.Count,
                    texturePropertiesListCount = texturePropertiesList.Count,
                    materialPropertiesListCount = materialPropertiesList.Count
                };

                var pipeLineId = (uint)RenderPipelineMap.Count;
                var renderPassId = vulkanRenderPass.RenderPassId;
                VulkanPipeline vulkanPipelineDLL = VulkanPipeline_CreateRenderPipeline(renderer.Device, ref renderPassId, pipeLineId, fulRenderPassPath, RenderPassList[vulkanRenderPass.RenderPassId].RenderPass, (uint)sizeof(SceneDataBuffer), ref renderPassResolution, include);
                vulkanPipelineList.Add(vulkanPipelineDLL);
            }
            RenderPipelineMap[vulkanRenderPass.RenderPassId] = vulkanPipelineList;
            return vulkanRenderPass.RenderPassId;
        }

        public static Guid LoadRenderPass(Guid levelId, string jsonPath, Texture inputTexture, ivec2 renderPassResolution)
        {
            string jsonContent = File.ReadAllText(jsonPath);
            RenderPassBuildInfoModel model = JsonConvert.DeserializeObject<RenderPassBuildInfoModel>(jsonContent);

            VkExtent2D extent2D = new VkExtent2D
            {
                width = (uint)renderPassResolution.x,
                height = (uint)renderPassResolution.y
            };

            size_t renderedTextureCount = model.RenderedTextureInfoModelList.Where(x => x.TextureType == RenderedTextureType.ColorRenderedTexture).Count();
            ListPtr<Texture> renderedTextureListPtr = new ListPtr<Texture>(renderedTextureCount);
            VulkanRenderPass vulkanRenderPass = VulkanRenderPass_CreateVulkanRenderPass(RenderSystem.renderer, jsonPath, ref extent2D, sizeof(SceneDataBuffer), renderedTextureListPtr.Ptr, ref renderedTextureCount, out Texture depthTexture);
            RenderPassList[vulkanRenderPass.RenderPassId] = vulkanRenderPass;

            TextureSystem.RenderedTextureList[vulkanRenderPass.RenderPassId] = new ListPtr<Texture>();
            for (int x = 0; x < renderedTextureCount; x++)
            {
                TextureSystem.RenderedTextureList[vulkanRenderPass.RenderPassId].Add(renderedTextureListPtr[x]);
            }
            if (depthTexture.textureView != VulkanCSConst.VK_NULL_HANDLE)
            {
                TextureSystem.DepthTextureList[vulkanRenderPass.RenderPassId] = depthTexture;
            }

            ListPtr<VulkanPipeline> vulkanPipelineList = new ListPtr<VulkanPipeline>();
            foreach (var pipelineId in model.RenderPipelineList)
            {
                string fulRenderPassPath = Path.GetFullPath("C:\\Users\\dotha\\Documents\\GitHub\\VulkanGameEngine\\pipelines\\" + pipelineId);

                ListPtr<VkDescriptorBufferInfo> vertexPropertiesList = RenderSystem.GetVertexPropertiesBuffer();
                ListPtr<VkDescriptorBufferInfo> indexPropertiesList = RenderSystem.GetIndexPropertiesBuffer();
                ListPtr<VkDescriptorBufferInfo> transformPropertiesList = RenderSystem.GetGameObjectTransformBuffer();
                ListPtr<VkDescriptorBufferInfo> meshPropertiesList = RenderSystem.GetMeshPropertiesBuffer(levelId);
                //  ListPtr<VkDescriptorBufferInfo> levelLayerMeshPropertiesList = Vector<VkDescriptorBufferInfo>(includes.LevelLayerMeshProperties, includes.LevelLayerMeshProperties + includes.LevelLayerMeshPropertiesCount);
                ListPtr<VkDescriptorImageInfo> texturePropertiesList = RenderSystem.GetTexturePropertiesBuffer(vulkanRenderPass.RenderPassId, &inputTexture);
                ListPtr<VkDescriptorBufferInfo> materialPropertiesList = MaterialSystem.GetMaterialPropertiesBuffer();

                GPUIncludes include = new GPUIncludes
                {
                    vertexPropertiesList = vertexPropertiesList.Ptr,
                    indexPropertiesList = indexPropertiesList.Ptr,
                    transformPropertiesList = transformPropertiesList.Ptr,
                    meshPropertiesList = meshPropertiesList.Ptr,
                    texturePropertiesList = texturePropertiesList.Ptr,
                    materialPropertiesList = materialPropertiesList.Ptr,
                    vertexPropertiesListCount = vertexPropertiesList.Count,
                    indexPropertiesListCount = indexPropertiesList.Count,
                    transformPropertiesListCount = transformPropertiesList.Count,
                    meshPropertiesListCount = meshPropertiesList.Count,
                    texturePropertiesListCount = texturePropertiesList.Count,
                    materialPropertiesListCount = materialPropertiesList.Count
                };

                var pipeLineId = (uint)RenderPipelineMap.Count;
                var renderPassId = vulkanRenderPass.RenderPassId;
                VulkanPipeline vulkanPipelineDLL = VulkanPipeline_CreateRenderPipeline(renderer.Device, ref renderPassId, pipeLineId, fulRenderPassPath, RenderPassList[vulkanRenderPass.RenderPassId].RenderPass, (uint)sizeof(SceneDataBuffer), ref renderPassResolution, include);
                vulkanPipelineList.Add(vulkanPipelineDLL);
            }
            RenderPipelineMap[vulkanRenderPass.RenderPassId] = vulkanPipelineList;
            return vulkanRenderPass.RenderPassId;
        }

        public static VkResult StartFrame()
        {
            CommandIndex = (CommandIndex + 1) % VulkanCSConst.MAX_FRAMES_IN_FLIGHT;

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
                    commandBufferCount = (uint)commandBufferSubmitList.Count,
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

        public static ListPtr<VkDescriptorBufferInfo> GetVertexPropertiesBuffer()
        {
            //Vector<MeshStruct> meshList;
            //meshList.reserve(meshSystem.SpriteMeshList.size());
            //std::transform(meshSystem.SpriteMeshList.begin(), meshSystem.SpriteMeshList.end(),
            //    std::back_inserter(meshList),
            //    [](const auto& pair) { return pair.second; });


            ListPtr<VkDescriptorBufferInfo> vertexPropertiesBuffer = new ListPtr<VkDescriptorBufferInfo>();
            //if (meshList.empty())
            //{
            //    vertexPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
            //        {
            //            .buffer = VK_NULL_HANDLE,
            //            .offset = 0,
            //            .range = VK_WHOLE_SIZE
            //        });
            //}
            //else
            //{
            //    for (auto& mesh : meshList)
            //    {
            //        const VulkanBufferStruct& vertexProperties = bufferSystem.VulkanBuffer[mesh.MeshVertexBufferId];
            //        vertexPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
            //            {
            //                .buffer = vertexProperties.Buffer,
            //                .offset = 0,
            //                .range = VK_WHOLE_SIZE
            //            });
            //    }
            //}

            return vertexPropertiesBuffer;
        }

        public static ListPtr<VkDescriptorBufferInfo> GetIndexPropertiesBuffer()
        {
            //Vector<MeshStruct> meshList;
            //meshList.reserve(meshSystem.SpriteMeshList.size());
            //std::transform(meshSystem.SpriteMeshList.begin(), meshSystem.SpriteMeshList.end(),
            //    std::back_inserter(meshList),
            //    [](const auto& pair) { return pair.second; });

            ListPtr<VkDescriptorBufferInfo> indexPropertiesBuffer = new ListPtr<VkDescriptorBufferInfo>();
            //if (meshList.empty())
            //{
            //    indexPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
            //        {
            //            .buffer = VK_NULL_HANDLE,
            //            .offset = 0,
            //            .range = VK_WHOLE_SIZE
            //        });
            //}
            //else
            //{
            //    for (auto& mesh : meshList)
            //    {
            //        const VulkanBufferStruct& indexProperties = bufferSystem.VulkanBuffer[mesh.MeshIndexBufferId];
            //        indexPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
            //            {
            //                .buffer = indexProperties.Buffer,
            //                .offset = 0,
            //                .range = VK_WHOLE_SIZE
            //            });
            //    }
            //}
            return indexPropertiesBuffer;
        }

        public static ListPtr<VkDescriptorBufferInfo> GetGameObjectTransformBuffer()
        {
            //Vector<MeshStruct> meshList;
            //meshList.reserve(meshSystem.SpriteMeshList.size());
            //std::transform(meshSystem.SpriteMeshList.begin(), meshSystem.SpriteMeshList.end(),
            //    std::back_inserter(meshList),
            //    [](const auto& pair) { return pair.second; });

            ListPtr<VkDescriptorBufferInfo> transformPropertiesBuffer = new ListPtr<VkDescriptorBufferInfo>();
            //if (meshList.empty())
            //{
            //    transformPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
            //        {
            //            .buffer = VK_NULL_HANDLE,
            //            .offset = 0,
            //            .range = VK_WHOLE_SIZE
            //        });
            //}
            //else
            //{
            //    for (auto& mesh : meshList)
            //    {
            //        const VulkanBufferStruct& transformBuffer = bufferSystem.VulkanBuffer[mesh.MeshTransformBufferId];
            //        transformPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
            //            {
            //                .buffer = transformBuffer.Buffer,
            //                .offset = 0,
            //                .range = VK_WHOLE_SIZE
            //            });
            //    }
            //}

            return transformPropertiesBuffer;
        }

        public static ListPtr<VkDescriptorBufferInfo> GetMeshPropertiesBuffer(Guid levelLayerId)
        {
            ListPtr<Mesh> meshList = new ListPtr<Mesh>();
            if (levelLayerId == Guid.Empty)
            {
                foreach (var sprite in MeshSystem.SpriteMeshMap)
                {
                    meshList.Add(sprite.Value);

                }
            }
            else
            {
                foreach (var layer in MeshSystem.LevelLayerMeshListMap[levelLayerId])
                {
                    meshList.Add(layer);
                }
            }

            ListPtr<VkDescriptorBufferInfo> meshPropertiesBuffer = new ListPtr<VkDescriptorBufferInfo>();
            if (!meshList.Any())
            {
                meshPropertiesBuffer.Add(new VkDescriptorBufferInfo
                    {
                        buffer = VulkanCSConst.VK_NULL_HANDLE,
                        offset = 0,
                        range = VulkanCSConst.VK_WHOLE_SIZE
                });
            }
            else
            {
                foreach (var mesh in meshList)
                {
                    VulkanBuffer meshProperties = BufferSystem.VulkanBufferMap[(uint)mesh.PropertiesBufferId];
                    meshPropertiesBuffer.Add(new VkDescriptorBufferInfo
                        {
                            buffer = meshProperties.Buffer,
                            offset = 0,
                            range = VulkanCSConst.VK_WHOLE_SIZE
                        });
                }
            }

            return meshPropertiesBuffer;
        }


        public unsafe static ListPtr<VkDescriptorImageInfo> GetTexturePropertiesBuffer(Guid renderPassId, Texture* renderedTexture)
        {
            ListPtr<Texture> textureList = new ListPtr<Texture>();
            if ((IntPtr)renderedTexture != IntPtr.Zero)
            {
                var texture = *renderedTexture;
                textureList.Add(texture);
            }
            else
            {
                foreach(var texture in TextureSystem.TextureList)
                {
                    textureList.Add(texture.Value);
                }
            }

            ListPtr<VkDescriptorImageInfo> texturePropertiesBuffer = new ListPtr<VkDescriptorImageInfo>();
            if (!textureList.Any())
            {
                VkSamplerCreateInfo NullSamplerInfo = new VkSamplerCreateInfo()
                {
                    sType = VkStructureType.VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                    magFilter = VkFilter.VK_FILTER_NEAREST,
                    minFilter = VkFilter.VK_FILTER_NEAREST,
                    mipmapMode = VkSamplerMipmapMode.VK_SAMPLER_MIPMAP_MODE_LINEAR,
                    addressModeU = VkSamplerAddressMode.VK_SAMPLER_ADDRESS_MODE_REPEAT,
                    addressModeV = VkSamplerAddressMode.VK_SAMPLER_ADDRESS_MODE_REPEAT,
                    addressModeW = VkSamplerAddressMode.VK_SAMPLER_ADDRESS_MODE_REPEAT,
                    mipLodBias = 0,
                    anisotropyEnable = true,
                    maxAnisotropy = 16.0f,
                    compareEnable = false,
                    compareOp = VkCompareOp.VK_COMPARE_OP_ALWAYS,
                    minLod = 0,
                    maxLod = 0,
                    borderColor = VkBorderColor.VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                    unnormalizedCoordinates = false,
                };

                VkFunc.vkCreateSampler(renderer.Device, &NullSamplerInfo, null, out VkSampler nullSampler);
                VkDescriptorImageInfo nullBuffer = new VkDescriptorImageInfo()
                {
                    sampler = nullSampler,
                    imageView = VulkanCSConst.VK_NULL_HANDLE,
                    imageLayout = VkImageLayout.VK_IMAGE_LAYOUT_UNDEFINED,
                };
                texturePropertiesBuffer.Add(nullBuffer);
            }
            else
            {
                foreach (var texture in textureList)
                {
                    TextureSystem.GetTexturePropertiesBuffer(texture, ref texturePropertiesBuffer);
                }
            }

            return texturePropertiesBuffer;
        }

        public static VkCommandBuffer BeginSingleTimeCommands()
        {
            return Renderer_BeginSingleTimeCommands(RenderSystem.Device, RenderSystem.CommandPool);
        }

        public static VkCommandBuffer BeginSingleTimeCommands(VkCommandPool commandPool)
        {
            return Renderer_BeginSingleTimeCommands(RenderSystem.Device, RenderSystem.CommandPool);
        }

        public static VkResult EndSingleTimeCommands(VkCommandBuffer commandBuffer)
        {
            return Renderer_EndSingleTimeCommands(RenderSystem.Device, RenderSystem.CommandPool, RenderSystem.GraphicsQueue, commandBuffer);
        }

        public static VkResult EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool commandPool)
        {
            return Renderer_EndSingleTimeCommands(RenderSystem.Device, commandPool, RenderSystem.GraphicsQueue, commandBuffer);
        }

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern GraphicsRenderer Renderer_RendererSetUp_CS(void* windowHandle);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VkCommandBuffer Renderer_BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VkResult Renderer_EndSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VulkanRenderPass VulkanRenderPass_CreateVulkanRenderPass(GraphicsRenderer renderer, [MarshalAs(UnmanagedType.LPStr)] string renderPassLoader, ref VkExtent2D renderPassResolution, int ConstBuffer, Texture* renderedTextureListPtr, ref size_t renderedTextureCount, out Texture depthTexture);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void VulkanRenderPass_DestroyRenderPass(GraphicsRenderer renderer, VulkanRenderPass renderPass);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VulkanPipeline VulkanPipeline_CreateRenderPipeline(IntPtr device, ref Guid renderPassId, uint renderPipelineId, [MarshalAs(UnmanagedType.LPStr)] string pipelineJson, IntPtr renderPass, uint constBufferSize, ref ivec2 renderPassResolution, GPUIncludes includes);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void VulkanPipeline_Destroy(VkDevice device, VulkanPipeline vulkanPipelineDLL);
    }
}
