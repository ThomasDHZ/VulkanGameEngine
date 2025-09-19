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
                ShaderPipelineData shaderPiplineInfo = ShaderSystem.LoadShaderPipelineData(new List<string> { jObject["ShaderList"][0].ToString(), jObject["ShaderList"][1].ToString() });
                var a = VulkanPipeline_CreateRenderPipeline(renderer.Device, vulkanRenderPass, pipelineJson, gpuIncludes, shaderPiplineInfo);
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
            foreach (var spriteLayer in spriteLayerList)
            {
                if (spriteLayerList.Ptr != null)
                {
                    ListPtr<SpriteInstanceStruct> spriteInstanceList = SpriteSystem.FindSpriteInstanceList(spriteLayer.SpriteBatchLayerId);
                    Mesh spriteMesh = MeshSystem.SpriteMeshMap[(int)spriteLayer.SpriteLayerMeshId];
                    VkBuffer meshVertexBuffer = BufferSystem.VulkanBufferMap[(uint)spriteMesh.MeshVertexBufferId].Buffer;
                    VkBuffer meshIndexBuffer = BufferSystem.VulkanBufferMap[(uint)spriteMesh.MeshIndexBufferId].Buffer;
                    VkBuffer spriteInstanceBuffer = BufferSystem.VulkanBufferMap[(uint)SpriteSystem.FindSpriteInstanceBufferId(spriteLayer.SpriteBatchLayerId)].Buffer;

                    ListPtr<VkDeviceSize> offsets = new ListPtr<ulong> { 0 };
                    VkFunc.vkCmdPushConstants(commandBuffer, spritePipeline.PipelineLayout, VkShaderStageFlagBits.VK_SHADER_STAGE_VERTEX_BIT | VkShaderStageFlagBits.VK_SHADER_STAGE_FRAGMENT_BIT, 0, (uint)sceneDataBuffer.PushConstantSize, sceneDataBuffer.PushConstantBuffer);
                    VkFunc.vkCmdBindPipeline(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.Pipeline);
                    VkFunc.vkCmdBindDescriptorSets(commandBuffer, VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.PipelineLayout, 0, (uint)spritePipeline.DescriptorSetCount, spritePipeline.DescriptorSetList, 0, null);
                    VkFunc.vkCmdBindVertexBuffers(commandBuffer, 0, 1, &spriteInstanceBuffer, offsets.Ptr);
                    VkFunc.vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VkIndexType.VK_INDEX_TYPE_UINT32);
                    VkFunc.vkCmdDrawIndexed(commandBuffer, (uint)GameObjectSystem.SpriteIndexList.ToList().Count(), (uint)spriteInstanceList.Count, 0, 0, 0);
                }
            }
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

        public static void DebugRenderPipelineLoaderModel(RenderPipelineLoaderModel model, string indent = "")
        {
            Console.WriteLine($"{indent}=== RenderPipelineLoaderModel ===");
            Console.WriteLine($"{indent}PipelineId: {model.PipelineId}");
            Console.WriteLine($"{indent}RenderPassId: {model.RenderPassId}");
            Console.WriteLine($"{indent}RenderPass: 0x{model.RenderPass.ToInt64():X16}");
            Console.WriteLine($"{indent}RenderPassResolution: ({model.RenderPassResolution.x}, {model.RenderPassResolution.y})");

            Console.WriteLine($"{indent}--- GPUIncludes ---");
            DebugGPUIncludes(model.gpuIncludes, indent + "  ");

            Console.WriteLine($"{indent}--- ShaderPipelineData ---");
            DebugShaderPipelineData(model.ShaderPiplineInfo, indent + "  ");

            Console.WriteLine($"{indent}ViewportCount: {model.ViewportCount}");
            DebugViewportList(model.ViewportList, model.ViewportCount, indent + "  ");

            Console.WriteLine($"{indent}ScissorCount: {model.ScissorCount}");
            DebugScissorList(model.ScissorList, model.ScissorCount, indent + "  ");

            Console.WriteLine($"{indent}PipelineColorBlendAttachmentStateCount: {model.PipelineColorBlendAttachmentStateCount}");
            DebugPipelineColorBlendAttachmentStateList(model.PipelineColorBlendAttachmentStateList, model.PipelineColorBlendAttachmentStateCount, indent + "  ");

            Console.WriteLine($"{indent}PipelineInputAssemblyStateCreateInfo:");
            DebugPipelineInputAssemblyState(model.PipelineInputAssemblyStateCreateInfo, indent + "  ");

            Console.WriteLine($"{indent}PipelineRasterizationStateCreateInfo:");
            DebugPipelineRasterizationState(model.PipelineRasterizationStateCreateInfo, indent + "  ");

            Console.WriteLine($"{indent}PipelineMultisampleStateCreateInfo:");
            DebugPipelineMultisampleState(model.PipelineMultisampleStateCreateInfo, indent + "  ");

            Console.WriteLine($"{indent}PipelineDepthStencilStateCreateInfo:");
            DebugPipelineDepthStencilState(model.PipelineDepthStencilStateCreateInfo, indent + "  ");

            Console.WriteLine($"{indent}PipelineColorBlendStateCreateInfo:");
            DebugPipelineColorBlendState(model.PipelineColorBlendStateCreateInfoModel, indent + "  ");
        }

        private static void DebugGPUIncludes(GPUIncludes includes, string indent)
        {
            Console.WriteLine($"{indent}VertexPropertiesCount: {includes.VertexPropertiesCount}");
            DebugDescriptorBufferInfoList(includes.VertexProperties, includes.VertexPropertiesCount, "VertexProperties", indent + "  ");

            Console.WriteLine($"{indent}IndexPropertiesCount: {includes.IndexPropertiesCount}");
            DebugDescriptorBufferInfoList(includes.IndexProperties, includes.IndexPropertiesCount, "IndexProperties", indent + "  ");

            Console.WriteLine($"{indent}TransformPropertiesCount: {includes.TransformPropertiesCount}");
            DebugDescriptorBufferInfoList(includes.TransformProperties, includes.TransformPropertiesCount, "TransformProperties", indent + "  ");

            Console.WriteLine($"{indent}MeshPropertiesCount: {includes.MeshPropertiesCount}");
            DebugDescriptorBufferInfoList(includes.MeshProperties, includes.MeshPropertiesCount, "MeshProperties", indent + "  ");

            Console.WriteLine($"{indent}TexturePropertiesCount: {includes.TexturePropertiesCount}");
            DebugDescriptorImageInfoList(includes.TextureProperties, includes.TexturePropertiesCount, indent + "  ");

            Console.WriteLine($"{indent}MaterialPropertiesCount: {includes.MaterialPropertiesCount}");
            DebugDescriptorBufferInfoList(includes.MaterialProperties, includes.MaterialPropertiesCount, "MaterialProperties", indent + "  ");
        }

        private static void DebugShaderPipelineData(ShaderPipelineData data, string indent)
        {
            Console.WriteLine($"{indent}ShaderCount: {data.ShaderCount}");
            DebugShaderList(data.ShaderList, data.ShaderCount, indent + "  ");

            Console.WriteLine($"{indent}DescriptorBindingCount: {data.DescriptorBindingCount}");
            DebugDescriptorBindingsList(data.DescriptorBindingsList, data.DescriptorBindingCount, indent + "  ");

            Console.WriteLine($"{indent}ShaderStructCount: {data.ShaderStructCount}");
            DebugShaderStructList(data.ShaderStructList, data.ShaderStructCount, indent + "  ");

            Console.WriteLine($"{indent}VertexInputBindingCount: {data.VertexInputBindingCount}");
            DebugVertexInputBindingList(data.VertexInputBindingList, data.VertexInputBindingCount, indent + "  ");

            Console.WriteLine($"{indent}VertexInputAttributeListCount: {data.VertexInputAttributeListCount}");
            DebugVertexInputAttributeList(data.VertexInputAttributeList, data.VertexInputAttributeListCount, indent + "  ");

            Console.WriteLine($"{indent}ShaderOutputCount: {data.ShaderOutputCount}");
            DebugShaderVariableList(data.ShaderOutputList, data.ShaderOutputCount, "ShaderOutput", indent + "  ");

            Console.WriteLine($"{indent}PushConstantCount: {data.PushConstantCount}");
            DebugPushConstantList(data.PushConstantList, data.PushConstantCount, indent + "  ");
        }

        private static void DebugShaderList(IntPtr shaderList, size_t count, string indent)
        {
            if (shaderList == IntPtr.Zero || count == 0)
            {
                Console.WriteLine($"{indent}ShaderList: (empty)");
                return;
            }

            for (size_t i = 0; i < count; i++)
            {
                IntPtr stringPtr = Marshal.ReadIntPtr(shaderList, (int)(i * IntPtr.Size));
                string shaderName = Marshal.PtrToStringAnsi(stringPtr) ?? "(null)";
                Console.WriteLine($"{indent}[{i}] Shader: {shaderName}");
            }
        }

        private static void DebugDescriptorBindingsList(ShaderDescriptorBinding* bindings, size_t count, string indent)
        {
            if (bindings == null || count == 0)
            {
                Console.WriteLine($"{indent}DescriptorBindingsList: (empty)");
                return;
            }

            for (size_t i = 0; i < count; i++)
            {
                ShaderDescriptorBinding binding = bindings[i];
                Console.WriteLine($"{indent}[{i}] DescriptorBinding:");
                Console.WriteLine($"{indent}  Name: {binding.GetName()}");
                Console.WriteLine($"{indent}  Binding: {binding.Binding}");
                Console.WriteLine($"{indent}  DescriptorCount: {binding.DescriptorCount}");
                Console.WriteLine($"{indent}  ShaderStageFlags: {binding.ShaderStageFlags}");
                Console.WriteLine($"{indent}  DescriptorBindingType: {binding.DescriptorBindingType}");
                Console.WriteLine($"{indent}  DescripterType: {binding.DescripterType}");
                Console.WriteLine($"{indent}  DescriptorImageInfo: 0x{(binding.DescriptorImageInfo != null ? (long)binding.DescriptorImageInfo : 0):X16}");
                Console.WriteLine($"{indent}  DescriptorBufferInfo: 0x{(binding.DescriptorBufferInfo != null ? (long)binding.DescriptorBufferInfo : 0):X16}");
            }
        }

        private static void DebugShaderStructList(ShaderStruct* structs, size_t count, string indent)
        {
            if (structs == null || count == 0)
            {
                Console.WriteLine($"{indent}ShaderStructList: (empty)");
                return;
            }

            for (size_t i = 0; i < count; i++)
            {
                ShaderStruct shaderStruct = structs[i];
                Console.WriteLine($"{indent}[{i}] ShaderStruct:");
                Console.WriteLine($"{indent}  Name: {shaderStruct.GetName()}");
                Console.WriteLine($"{indent}  ShaderBufferSize: {shaderStruct.ShaderBufferSize}");
                Console.WriteLine($"{indent}  ShaderBufferVariableListCount: {shaderStruct.ShaderBufferVariableListCount}");
                Console.WriteLine($"{indent}  ShaderStructBufferId: {shaderStruct.ShaderStructBufferId}");
                Console.WriteLine($"{indent}  ShaderStructBuffer: 0x{(shaderStruct.ShaderStructBuffer != null ? (long)shaderStruct.ShaderStructBuffer : 0):X16}");
                DebugShaderVariableList(shaderStruct.ShaderBufferVariableList, (size_t)shaderStruct.ShaderBufferVariableListCount, "ShaderBufferVariable", indent + "    ");
            }
        }

        private static void DebugShaderVariableList(ShaderVariable* variables, size_t count, string listName, string indent)
        {
            if (variables == null || count == 0)
            {
                Console.WriteLine($"{indent}{listName}List: (empty)");
                return;
            }

            for (size_t i = 0; i < count; i++)
            {
                ShaderVariable variable = variables[i];
                Console.WriteLine($"{indent}[{i}] {listName}:");
               // Console.WriteLine($"{indent}  Name: {variable.GetName()}");
                Console.WriteLine($"{indent}  Size: {variable.Size}");
                Console.WriteLine($"{indent}  ByteAlignment: {variable.ByteAlignment}");
                Console.WriteLine($"{indent}  Value: 0x{(variable.Value != null ? (long)variable.Value : 0):X16}");
                Console.WriteLine($"{indent}  MemberTypeEnum: {variable.MemberTypeEnum}");
            }
        }

        private static void DebugPushConstantList(ShaderPushConstant* pushConstants, size_t count, string indent)
        {
            if (pushConstants == null || count == 0)
            {
                Console.WriteLine($"{indent}PushConstantList: (empty)");
                return;
            }

            for (size_t i = 0; i < count; i++)
            {
                ShaderPushConstant pushConstant = pushConstants[i];
                Console.WriteLine($"{indent}[{i}] PushConstant:");
                Console.WriteLine($"{indent}  Name: {pushConstant.GetName()}");
                Console.WriteLine($"{indent}  PushConstantSize: {pushConstant.PushConstantSize}");
                Console.WriteLine($"{indent}  PushConstantVariableListCount: {pushConstant.PushConstantVariableListCount}");
                Console.WriteLine($"{indent}  ShaderStageFlags: {pushConstant.ShaderStageFlags}");
                Console.WriteLine($"{indent}  PushConstantBuffer: 0x{(pushConstant.PushConstantBuffer != null ? (long)pushConstant.PushConstantBuffer : 0):X16}");
                Console.WriteLine($"{indent}  GlobalPushContant: {pushConstant.GlobalPushContant}");
                DebugShaderVariableList(pushConstant.PushConstantVariableList, (size_t)pushConstant.PushConstantVariableListCount, "PushConstantVariable", indent + "    ");
            }
        }

        private static void DebugVertexInputBindingList(VkVertexInputBindingDescription* bindings, size_t count, string indent)
        {
            if (bindings == null || count == 0)
            {
                Console.WriteLine($"{indent}VertexInputBindingList: (empty)");
                return;
            }

            for (size_t i = 0; i < count; i++)
            {
                VkVertexInputBindingDescription binding = bindings[i];
                Console.WriteLine($"{indent}[{i}] VertexInputBinding:");
                Console.WriteLine($"{indent}  Binding: {binding.binding}");
                Console.WriteLine($"{indent}  Stride: {binding.stride}");
                Console.WriteLine($"{indent}  InputRate: {binding.inputRate}");
            }
        }

        private static void DebugVertexInputAttributeList(VkVertexInputAttributeDescription* attributes, size_t count, string indent)
        {
            if (attributes == null || count == 0)
            {
                Console.WriteLine($"{indent}VertexInputAttributeList: (empty)");
                return;
            }

            for (size_t i = 0; i < count; i++)
            {
                VkVertexInputAttributeDescription attribute = attributes[i];
                Console.WriteLine($"{indent}[{i}] VertexInputAttribute:");
                Console.WriteLine($"{indent}  Location: {attribute.location}");
                Console.WriteLine($"{indent}  Binding: {attribute.binding}");
                Console.WriteLine($"{indent}  Format: {attribute.format}");
                Console.WriteLine($"{indent}  Offset: {attribute.offset}");
            }
        }

        private static void DebugDescriptorBufferInfoList(VkDescriptorBufferInfo* infos, size_t count, string name, string indent)
        {
            if (infos == null || count == 0)
            {
                Console.WriteLine($"{indent}{name}List: (empty)");
                return;
            }

            for (size_t i = 0; i < count; i++)
            {
                VkDescriptorBufferInfo info = infos[i];
                Console.WriteLine($"{indent}[{i}] {name}:");
                Console.WriteLine($"{indent}  Buffer: 0x{(long)info.buffer:X16}");
                Console.WriteLine($"{indent}  Offset: {info.offset}");
                Console.WriteLine($"{indent}  Range: {info.range}");
            }
        }

        private static void DebugDescriptorImageInfoList(VkDescriptorImageInfo* infos, size_t count, string indent)
        {
            if (infos == null || count == 0)
            {
                Console.WriteLine($"{indent}TexturePropertiesList: (empty)");
                return;
            }

            for (size_t i = 0; i < count; i++)
            {
                VkDescriptorImageInfo info = infos[i];
                Console.WriteLine($"{indent}[{i}] TextureProperties:");
                Console.WriteLine($"{indent}  Sampler: 0x{(long)info.sampler:X16}");
                Console.WriteLine($"{indent}  ImageView: 0x{(long)info.imageView:X16}");
                Console.WriteLine($"{indent}  ImageLayout: {info.imageLayout}");
            }
        }

        private static void DebugViewportList(VkViewport* viewports, size_t count, string indent)
        {
            if (viewports == null || count == 0)
            {
                Console.WriteLine($"{indent}ViewportList: (empty)");
                return;
            }

            for (size_t i = 0; i < count; i++)
            {
                VkViewport viewport = viewports[i];
                Console.WriteLine($"{indent}[{i}] Viewport:");
                Console.WriteLine($"{indent}  X: {viewport.x}");
                Console.WriteLine($"{indent}  Y: {viewport.y}");
                Console.WriteLine($"{indent}  Width: {viewport.width}");
                Console.WriteLine($"{indent}  Height: {viewport.height}");
                Console.WriteLine($"{indent}  MinDepth: {viewport.minDepth}");
                Console.WriteLine($"{indent}  MaxDepth: {viewport.maxDepth}");
            }
        }

        private static void DebugScissorList(VkRect2D* scissors, size_t count, string indent)
        {
            if (scissors == null || count == 0)
            {
                Console.WriteLine($"{indent}ScissorList: (empty)");
                return;
            }

            for (size_t i = 0; i < count; i++)
            {
                VkRect2D scissor = scissors[i];
                Console.WriteLine($"{indent}[{i}] Scissor:");
                Console.WriteLine($"{indent}  Offset: ({scissor.offset.x}, {scissor.offset.y})");
                Console.WriteLine($"{indent}  Extent: ({scissor.extent.width}, {scissor.extent.height})");
            }
        }

        private static void DebugPipelineColorBlendAttachmentStateList(VkPipelineColorBlendAttachmentState* states, size_t count, string indent)
        {
            if (states == null || count == 0)
            {
                Console.WriteLine($"{indent}PipelineColorBlendAttachmentStateList: (empty)");
                return;
            }

            for (size_t i = 0; i < count; i++)
            {
                VkPipelineColorBlendAttachmentState state = states[i];
                Console.WriteLine($"{indent}[{i}] PipelineColorBlendAttachmentState:");
                Console.WriteLine($"{indent}  BlendEnable: {state.blendEnable}");
                Console.WriteLine($"{indent}  SrcColorBlendFactor: {state.srcColorBlendFactor}");
                Console.WriteLine($"{indent}  DstColorBlendFactor: {state.dstColorBlendFactor}");
                Console.WriteLine($"{indent}  ColorBlendOp: {state.colorBlendOp}");
                Console.WriteLine($"{indent}  SrcAlphaBlendFactor: {state.srcAlphaBlendFactor}");
                Console.WriteLine($"{indent}  DstAlphaBlendFactor: {state.dstAlphaBlendFactor}");
                Console.WriteLine($"{indent}  AlphaBlendOp: {state.alphaBlendOp}");
                Console.WriteLine($"{indent}  ColorWriteMask: {state.colorWriteMask}");
            }
        }

        private static void DebugPipelineInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo state, string indent)
        {
            Console.WriteLine($"{indent}Topology: {state.topology}");
            Console.WriteLine($"{indent}PrimitiveRestartEnable: {state.primitiveRestartEnable}");
        }

        private static void DebugPipelineRasterizationState(VkPipelineRasterizationStateCreateInfo state, string indent)
        {
            Console.WriteLine($"{indent}DepthClampEnable: {state.depthClampEnable}");
            Console.WriteLine($"{indent}RasterizerDiscardEnable: {state.rasterizerDiscardEnable}");
            Console.WriteLine($"{indent}PolygonMode: {state.polygonMode}");
            Console.WriteLine($"{indent}CullMode: {state.cullMode}");
            Console.WriteLine($"{indent}FrontFace: {state.frontFace}");
            Console.WriteLine($"{indent}DepthBiasEnable: {state.depthBiasEnable}");
            Console.WriteLine($"{indent}DepthBiasConstantFactor: {state.depthBiasConstantFactor}");
            Console.WriteLine($"{indent}DepthBiasClamp: {state.depthBiasClamp}");
            Console.WriteLine($"{indent}DepthBiasSlopeFactor: {state.depthBiasSlopeFactor}");
            Console.WriteLine($"{indent}LineWidth: {state.lineWidth}");
        }

        private static void DebugPipelineMultisampleState(VkPipelineMultisampleStateCreateInfo state, string indent)
        {
            Console.WriteLine($"{indent}RasterizationSamples: {state.rasterizationSamples}");
            Console.WriteLine($"{indent}SampleShadingEnable: {state.sampleShadingEnable}");
            Console.WriteLine($"{indent}MinSampleShading: {state.minSampleShading}");
            Console.WriteLine($"{indent}AlphaToCoverageEnable: {state.alphaToCoverageEnable}");
            Console.WriteLine($"{indent}AlphaToOneEnable: {state.alphaToOneEnable}");
        }

        private static void DebugPipelineDepthStencilState(VkPipelineDepthStencilStateCreateInfo state, string indent)
        {

            Console.WriteLine($"{indent}DepthTestEnable: {state.depthTestEnable}");
            Console.WriteLine($"{indent}DepthWriteEnable: {state.depthWriteEnable}");
            Console.WriteLine($"{indent}DepthCompareOp: {state.depthCompareOp}");
            Console.WriteLine($"{indent}DepthBoundsTestEnable: {state.depthBoundsTestEnable}");
            Console.WriteLine($"{indent}StencilTestEnable: {state.stencilTestEnable}");
            //if (state.front.HasValue)
            //{
            //Console.WriteLine($"{indent}Front: [FailOp: {state.front.Value.failOp}, PassOp: {state.front.Value.passOp}, DepthFailOp: {state.front.Value.depthFailOp}, CompareOp: {state.front.Value.compareOp}]");

            //}
            //else
            //{
            //    Console.WriteLine($"{indent}Front: null");
            //}
            //if (state.back.HasValue)
            //{
            //    Console.WriteLine($"{indent}Back: [FailOp: {state.back.Value.failOp}, PassOp: {state.back.Value.passOp}, DepthFailOp: {state.back.Value.depthFailOp}, CompareOp: {state.back.Value.compareOp}]");
            //}
            //else
            //{
            //    Console.WriteLine($"{indent}Back: null");
            //}
                Console.WriteLine($"{indent}MinDepthBounds: {state.minDepthBounds}");
            Console.WriteLine($"{indent}MaxDepthBounds: {state.maxDepthBounds}");
        }

        private static void DebugPipelineColorBlendState(VkPipelineColorBlendStateCreateInfo state, string indent)
        {
            Console.WriteLine($"{indent}LogicOpEnable: {state.logicOpEnable}");
            Console.WriteLine($"{indent}LogicOp: {state.logicOp}");
           // Console.WriteLine($"{indent}BlendConstants: [{state.blendConstants[0]}, {state.blendConstants[1]}, {state.blendConstants[2]}, {state.blendConstants[3]}]");
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
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VulkanPipeline VulkanPipeline_CreateRenderPipeline(VkDevice device, VulkanRenderPass vulkanRenderPass, [MarshalAs(UnmanagedType.LPStr)] string pipelineJsonFilePath, GPUIncludes gpuIncludes, ShaderPipelineData shaderPipelineData);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern VulkanPipeline VulkanPipeline_RebuildSwapChain(VkDevice device, VulkanPipeline oldPipeline, VulkanRenderPass vulkanRenderPass, [MarshalAs(UnmanagedType.LPStr)] string pipelineJsonFilePath, GPUIncludes gpuIncludes, ShaderPipelineData shaderPipelineData);
        [DllImport(GameEngineImport.DLLPath, CallingConvention = CallingConvention.StdCall)] public static extern void VulkanPipeline_Destroy(VkDevice device, VulkanPipeline vulkanPipelineDLL);
    }
}
