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
            renderer = Renderer_RendererSetUp(windowType, renderAreaHandle, debuggerHandle);
        }

        public static unsafe Guid LoadRenderPass(Guid levelId, string jsonPath, ivec2 renderPassResolution)
        {
            string jsonContent = File.ReadAllText(jsonPath);
            RenderPassLoaderModel renderPassLoader = JsonConvert.DeserializeObject<RenderPassLoaderModel>(jsonContent);
            if (renderPassLoader.RenderArea.UseDefaultRenderArea)
            {
                renderPassLoader.RenderArea = new RenderAreaModel
                {
                    RenderArea = new VkRect2D
                    {
                        extent = new VkExtent2D
                        {
                            width = (uint)renderPassResolution.x,
                            height = (uint)renderPassResolution.y
                        }
                    }
                };
                foreach (var renderTexture in renderPassLoader.RenderedTextureInfoModelList)
                {
                    renderTexture.ImageCreateInfo.Extent = new VkExtent3DModel()
                    {
                        _width = (uint)renderPassResolution.x,
                        _height = (uint)renderPassResolution.y,
                        _depth = 1
                    };
                }
            }

            VulkanRenderPass vulkanRenderPass = VulkanRenderPass_CreateVulkanRenderPass(renderer, jsonPath, out RenderPassAttachementTextures renderPassAttachments, ref renderPassResolution);
            RenderPassMap[vulkanRenderPass.RenderPassId] = vulkanRenderPass;
            RenderPassEditor_RenderPass[renderPassLoader.RenderPassId] = renderPassLoader;

            ListPtr<Texture> renderTextureList = new ListPtr<Texture>(renderPassAttachments.RenderPassTexture, renderPassAttachments.RenderPassTextureCount);
            TextureSystem.AddRenderedTexture(vulkanRenderPass.RenderPassId, renderTextureList);
            if(renderPassAttachments.DepthTexture != null)
            {
                TextureSystem.AddDepthTexture(vulkanRenderPass.RenderPassId, *renderPassAttachments.DepthTexture);
            }
            MemorySystem.RemovePtrBuffer(renderPassAttachments.RenderPassTexture);
            MemorySystem.RemovePtrBuffer(renderPassAttachments.DepthTexture);

            ListPtr<VkDescriptorBufferInfo> vertexPropertiesList = GetVertexPropertiesBuffer();
            ListPtr<VkDescriptorBufferInfo> indexPropertiesList = GetIndexPropertiesBuffer();
            ListPtr<VkDescriptorBufferInfo> transformPropertiesList = GetGameObjectTransformBuffer();
            ListPtr<VkDescriptorBufferInfo> meshPropertiesList = GetMeshPropertiesBuffer(levelId);
            ListPtr<VkDescriptorImageInfo> texturePropertiesList = GetTexturePropertiesBuffer(vulkanRenderPass.RenderPassId);
            ListPtr<VkDescriptorBufferInfo> materialPropertiesList = MaterialSystem.GetMaterialPropertiesBuffer();
            GPUIncludes gpuIncludes = new GPUIncludes
            {
                VertexPropertiesCount = vertexPropertiesList.Count,
                IndexPropertiesCount = indexPropertiesList.Count,
                TransformPropertiesCount = transformPropertiesList.Count,
                MeshPropertiesCount = meshPropertiesList.Count,
                TexturePropertiesCount = texturePropertiesList.Count,
                MaterialPropertiesCount = materialPropertiesList.Count,
                VertexProperties = vertexPropertiesList.Ptr,
                IndexProperties = indexPropertiesList.Ptr,
                TransformProperties = transformPropertiesList.Ptr,
                MeshProperties = meshPropertiesList.Ptr,
                TextureProperties = texturePropertiesList.Ptr,
                MaterialProperties = materialPropertiesList.Ptr,
            };


            ListPtr<VulkanPipeline> vulkanPipelineList = new ListPtr<VulkanPipeline>();
            for (int x = 0; x < renderPassLoader.RenderPipelineList.Count(); x++)
            {
                List<String> ShaderList = new List<string>();
                string pipelineJsonContent = File.ReadAllText(@$"{ConstConfig.BaseDirectoryPath}RenderPass/{renderPassLoader.RenderPipelineList[x]}");

                JObject jObject = JObject.Parse(pipelineJsonContent);
                string pipelineJson = renderPassLoader.RenderPipelineList[x];
                string shader1 = @$"{ConstConfig.BaseDirectoryPath}RenderPass/{jObject["ShaderList"][0].ToString()}";
                string shader2 = @$"{ConstConfig.BaseDirectoryPath}RenderPass/{jObject["ShaderList"][1].ToString()}";
                ShaderPipelineData shaderPiplineInfo = ShaderSystem.LoadShaderPipelineData(new List<string> { shader1, shader2 });
                var a = VulkanPipeline_CreateRenderPipelineCS(renderer.Device, vulkanRenderPass, @$"{ConstConfig.BaseDirectoryPath}RenderPass/{renderPassLoader.RenderPipelineList[x]}", gpuIncludes, shaderPiplineInfo);
            }
            return vulkanRenderPass.RenderPassId;
        }

        public static void Update(Guid spriteRenderPass2DId, Guid levelId, float deltaTime)
        {
            if (RebuildRendererFlag)
            {
                uint width = renderer.SwapChainResolution.width;
                uint height = renderer.SwapChainResolution.height;
                // RecreateSwapchain(spriteRenderPass2DId, levelId, deltaTime);
                RebuildRendererFlag = false;
            }
        }

        public unsafe static void UpdateRenderPasses(Dictionary<Guid, string> renderPassLoaderJsonMap, Dictionary<Guid, List<string>> pipelineLoaderJsonMap, ShaderPushConstant sceneDataBuffer)
        {
            try
            {
                VkFunc.vkDeviceWaitIdle(renderer.Device);
                GraphicsRenderer rendererPtr = renderer;
                Renderer_RebuildSwapChain(windowType, RenderAreaHandle, &rendererPtr);
                renderer = rendererPtr;

                int x = 0;
                foreach (var renderPassJson in renderPassLoaderJsonMap)
                {
                    Texture depthTexture = new Texture();
                    RenderPassLoaderJsonMap[renderPassJson.Key] = renderPassJson.Value;
                    size_t size = TextureSystem.RenderedTextureListMap[renderPassJson.Key].Count();
                    ListPtr<Texture> renderedTextureList = TextureSystem.RenderedTextureListMap[renderPassJson.Key];
                    if (TextureSystem.DepthTextureExists(renderPassJson.Key))
                    {
                        depthTexture = TextureSystem.DepthTextureList[renderPassJson.Key];
                    }

                    ivec2 renderPassResolution = new ivec2((int)renderer.SwapChainResolution.width, (int)renderer.SwapChainResolution.height);
                    if (RenderPassMap.ContainsKey(renderPassJson.Key))
                    {
                        var renderPass = RenderPassMap[renderPassJson.Key];
                     //   RenderPassMap[renderPassJson.Key] = VulkanRenderPass_RebuildSwapChain(renderer, renderPass, renderPassJson.Value, ref renderPassResolution, renderedTextureList, size, ref depthTexture);
                        UpdateRenderPipelines(renderPassJson.Key, pipelineLoaderJsonMap, sceneDataBuffer);
                    }
                    else
                    {
                        LoadRenderPass(LevelSystem.levelLayout.LevelLayoutId, renderPassJson.Value, renderPassResolution);
                    }

                    TextureSystem.RenderedTextureListMap[renderPassJson.Key] = new ListPtr<Texture>();
                    for (int y = 0; y < size; y++)
                    {
                        TextureSystem.RenderedTextureListMap[renderPassJson.Key].Add(renderedTextureList[y]);
                    }
                    if (depthTexture.textureView != VulkanCSConst.VK_NULL_HANDLE)
                    {
                        TextureSystem.DepthTextureList[renderPassJson.Key] = depthTexture;
                    }

                    x++;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error: {ex.Message}");
            }
        }

        public static void UpdateRenderPipelines(Guid renderPassId, Dictionary<Guid, List<string>> updatePipelineLoaderJsonMap, ShaderPushConstant sceneDataBuffer)
        {
            ivec2 renderPassResolution = new ivec2((int)renderer.SwapChainResolution.width, (int)renderer.SwapChainResolution.height);
            foreach (var pipelineLoaderJson in updatePipelineLoaderJsonMap[renderPassId])
            {
                RenderPipelineLoaderModel model = JsonConvert.DeserializeObject<RenderPipelineLoaderModel>(pipelineLoaderJson);
                var pipelineIndex = RenderPipelineMap[renderPassId].ToList().FindIndex(x => x.PipelineId == model.PipelineId);

                size_t pipelineConsts = (size_t)sceneDataBuffer.PushConstantSize;
                VulkanRenderPass renderPass = RenderPassMap[renderPassId];
                List<string> pipelineUpdateJson = new List<string> { pipelineLoaderJson };
                GPUIncludes includes = GetGPUIncludes(renderPassId, LevelSystem.levelLayout.LevelLayoutId);
                VulkanPipeline oldVulkanPipeline = RenderPipelineMap[renderPassId][pipelineIndex];
                if (RenderPipelineMap.ContainsKey(renderPassId))
                {
                    //RenderPipelineMap[renderPassId][pipelineIndex] = VulkanPipeline_RebuildSwapChain(RenderSystem.renderer.Device, renderPassId, oldVulkanPipeline, pipelineLoaderJson, renderPass.RenderPass, pipelineConsts, ref renderPassResolution, includes);
                }
                else
                {
                    //RenderPipelineMap[renderPassId].Add(LoadVulkanPipeline(LevelSystem.levelLayout.LevelLayoutId, renderPassId, new List<string> { pipelineLoaderJson }, renderPassResolution).First());
                }
            }
        }

        public static void RecreateSwapchain(Guid spriteRenderPass2DId, Guid levelId, float deltaTime, ivec2 swapChainResolution)
        {
            VkFunc.vkDeviceWaitIdle(renderer.Device);
            GraphicsRenderer rendererPtr = renderer;
            Renderer_RebuildSwapChain(windowType, RenderAreaHandle, &rendererPtr);
            renderer = rendererPtr;

            foreach (var renderPass in RenderPassMap)
            {
                Texture depthTexture = new Texture();
                ListPtr<Texture> renderedTextureList = TextureSystem.RenderedTextureListMap[renderPass.Value.RenderPassId];
                size_t size = renderedTextureList.Count();
                if (TextureSystem.DepthTextureExists(renderPass.Value.RenderPassId))
                {
                    depthTexture = TextureSystem.DepthTextureList[renderPass.Value.RenderPassId];
                }

               // RenderPassMap[renderPass.Key] = VulkanRenderPass_RebuildSwapChain(renderer, renderPass.Value, RenderPassLoaderJsonMap[renderPass.Value.RenderPassId], ref swapChainResolution, renderedTextureList.Ptr, &size, ref depthTexture);
            }
        }

        public static VkCommandBuffer RenderFrameBuffer(Guid renderPassId)
        {
            VulkanRenderPass renderPass = RenderPassMap[renderPassId];
            VulkanPipeline pipeline = RenderPipelineMap[renderPassId][0];
            VkCommandBuffer commandBuffer = renderPass.CommandBuffer;
            ListPtr<Texture> renderPassTexture = TextureSystem.FindRenderedTextureList(LevelSystem.spriteRenderPass2DId);

            VkViewport viewport = new VkViewport
            {
                x = 0.0f,
                y = 0.0f,
                width = renderer.SwapChainResolution.width,
                height = renderer.SwapChainResolution.height,
                minDepth = 0.0f,
                maxDepth = 1.0f
            };

            VkRect2D scissor = new VkRect2D
            {
                offset = new VkOffset2D
                {
                    x = 0,
                    y = 0
                },
                extent = new VkExtent2D
                {
                    width = renderer.SwapChainResolution.width,
                    height = renderer.SwapChainResolution.height
                }
            };

            VkRenderPassBeginInfo renderPassBeginInfo = new VkRenderPassBeginInfo
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                renderPass = renderPass.RenderPass,
                framebuffer = renderPass.FrameBufferList[(uint)renderer.ImageIndex],
                renderArea = scissor,
                clearValueCount = renderPass.ClearValueCount,
                pClearValues = renderPass.ClearValueList
            };

            var beginCommandbufferindo = CommandBufferBeginInfo;
            VkFunc.vkBeginCommandBuffer(commandBuffer, &beginCommandbufferindo);
            TextureSystem.UpdateTextureLayout(renderPassTexture[0], commandBuffer, VkImageLayout.VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            VkFunc.vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
            VkFunc.vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
            VkFunc.vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VkSubpassContents.VK_SUBPASS_CONTENTS_INLINE);
            VkFunc.vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
            VkFunc.vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, (uint)pipeline.DescriptorSetCount, pipeline.DescriptorSetList, 0, null);
            VkFunc.vkCmdDraw(commandBuffer, 6, 1, 0, 0);
            VkFunc.vkCmdEndRenderPass(commandBuffer);
            VkFunc.vkEndCommandBuffer(commandBuffer);
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

        public unsafe static ListPtr<VkDescriptorImageInfo> GetTexturePropertiesBuffer(Guid renderPassId)
        {
            ListPtr<Texture> textureList = new ListPtr<Texture>();
            VulkanRenderPass renderPass = RenderPassMap[renderPassId];
            if (renderPass.InputTextureIdListCount > 0)
            {
                ListPtr<Guid> inputTextureList = new ListPtr<Guid>(renderPass.InputTextureIdList, (size_t)renderPass.InputTextureIdListCount);
                foreach (var inputTexture in inputTextureList)
                {
                    textureList.Add(TextureSystem.FindRenderedTexture(inputTexture));
                }
            }
            else
            {
                foreach (var texture in TextureSystem.TextureList)
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
                    anisotropyEnable = 1,
                    maxAnisotropy = 16.0f,
                    compareEnable = 0,
                    compareOp = VkCompareOp.VK_COMPARE_OP_ALWAYS,
                    minLod = 0,
                    maxLod = 0,
                    borderColor = VkBorderColor.VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                    unnormalizedCoordinates = 0,
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

        private static GPUIncludes GetGPUIncludes(Guid renderPassId, Guid levelId)
        {
            ListPtr<VkDescriptorBufferInfo> vertexPropertiesList = GetVertexPropertiesBuffer();
            ListPtr<VkDescriptorBufferInfo> indexPropertiesList = GetIndexPropertiesBuffer();
            ListPtr<VkDescriptorBufferInfo> transformPropertiesList = GetGameObjectTransformBuffer();
            ListPtr<VkDescriptorBufferInfo> meshPropertiesList = GetMeshPropertiesBuffer(levelId);
            ListPtr<VkDescriptorImageInfo> texturePropertiesList = GetTexturePropertiesBuffer(renderPassId);
            ListPtr<VkDescriptorBufferInfo> materialPropertiesList = MaterialSystem.GetMaterialPropertiesBuffer();
            GPUIncludes gpuIncludes = new GPUIncludes
            {
                VertexPropertiesCount = vertexPropertiesList.Count,
                IndexPropertiesCount = indexPropertiesList.Count,
                TransformPropertiesCount = transformPropertiesList.Count,
                MeshPropertiesCount = meshPropertiesList.Count,
                TexturePropertiesCount = texturePropertiesList.Count,
                MaterialPropertiesCount = materialPropertiesList.Count,
                VertexProperties = vertexPropertiesList.Ptr,
                IndexProperties = indexPropertiesList.Ptr,
                TransformProperties = transformPropertiesList.Ptr,
                MeshProperties = meshPropertiesList.Ptr,
                TextureProperties = texturePropertiesList.Ptr,
                MaterialProperties = materialPropertiesList.Ptr,
            };

            return gpuIncludes;
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
            foreach (var renderPass in RenderPassMap)
            {
                VulkanRenderPass_DestroyRenderPass(renderer, renderPass.Value);
            }
            RenderPassMap.Clear();
        }

        public static void DestroyRenderPipelines()
        {
            foreach (var renderPipelineList in RenderPipelineMap)
            {
                foreach (var renderPipeline in renderPipelineList.Value)
                {
                    VulkanPipeline_Destroy(renderer.Device, renderPipeline);
                }
            }
            RenderPipelineMap.Clear();
        }

        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern GraphicsRenderer Renderer_RendererSetUp(WindowType windowType, void* windowHandle, void* debuggerHandle);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern GraphicsRenderer Renderer_RebuildSwapChain(WindowType windowType, void* windowHandle, GraphicsRenderer* renderer);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VkResult Renderer_StartFrame(VkDevice device, VkSwapchainKHR swapChain, VkFence* fenceList, VkSemaphore* acquireImageSemaphoreList, size_t* pImageIndex, size_t* pCommandIndex, bool* pRebuildRendererFlag);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VkResult Renderer_EndFrame(VkSwapchainKHR swapChain, VkSemaphore* acquireImageSemaphoreList, VkSemaphore* presentImageSemaphoreList, VkFence* fenceList, VkQueue graphicsQueue, VkQueue presentQueue, size_t commandIndex, size_t imageIndex, VkCommandBuffer* pCommandBufferSubmitList, size_t commandBufferCount, bool* rebuildRendererFlag);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VkCommandBuffer Renderer_BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VkResult Renderer_EndSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkCommandBuffer commandBuffer);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VulkanRenderPass VulkanRenderPass_CreateVulkanRenderPass(GraphicsRenderer renderer, [MarshalAs(UnmanagedType.LPStr)] string jsonPath, out RenderPassAttachementTextures renderPassAttachments, ref ivec2 renderPassResolution);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VulkanRenderPass VulkanRenderPass_RebuildSwapChain(GraphicsRenderer renderer, VulkanRenderPass vulkanRenderPass, [MarshalAs(UnmanagedType.LPStr)] string renderPassJsonFilePath, ref ivec2 renderPassResolution, Texture renderedTextureListPtr, size_t renderedTextureCount, Texture depthTexture);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void VulkanRenderPass_DestroyRenderPass(GraphicsRenderer renderer, VulkanRenderPass renderPass);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VulkanPipeline VulkanPipeline_CreateRenderPipelineCS(VkDevice device, VulkanRenderPass vulkanRenderPass, [MarshalAs(UnmanagedType.LPStr)] string pipelineJsonFilePath, GPUIncludes gpuIncludes, ShaderPipelineData shaderPipelineData);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VulkanPipeline VulkanPipeline_RebuildSwapChain(VkDevice device, VulkanPipeline oldPipeline, VulkanRenderPass vulkanRenderPass, [MarshalAs(UnmanagedType.LPStr)] string pipelineJsonFilePath, GPUIncludes gpuIncludes, ShaderPipelineData shaderPipelineData);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void VulkanPipeline_Destroy(VkDevice device, VulkanPipeline vulkanPipelineDLL);
    }
}
