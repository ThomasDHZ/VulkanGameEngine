﻿using GlmSharp;
using Newtonsoft.Json;
using Silk.NET.Vulkan;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using VulkanGameEngineLevelEditor.Models;
using VulkanGameEngineLevelEditor.Vulkan;

namespace VulkanGameEngineLevelEditor.GameEngineAPI
{
    public enum DescriptorBindingPropertiesEnum
    {
        kMeshPropertiesDescriptor,
        kTextureDescriptor,
        kMaterialDescriptor,
        kBRDFMapDescriptor,
        kIrradianceMapDescriptor,
        kPrefilterMapDescriptor,
        kCubeMapDescriptor,
        kEnvironmentDescriptor,
        kSunLightDescriptor,
        kDirectionalLightDescriptor,
        kPointLightDescriptor,
        kSpotLightDescriptor,
        kReflectionViewDescriptor,
        kDirectionalShadowDescriptor,
        kPointShadowDescriptor,
        kSpotShadowDescriptor,
        kViewTextureDescriptor,
        kViewDepthTextureDescriptor,
        kCubeMapSamplerDescriptor,
        kRotatingPaletteTextureDescriptor,
        kMathOpperation1Descriptor,
        kMathOpperation2Descriptor,
    };

    public unsafe class JsonPipeline
    {
        Vk vk = Vk.GetApi();
        Device _device { get; set; }
        public DescriptorPool descriptorPool { get; protected set; }
        public List<DescriptorSetLayout> descriptorSetLayoutList { get; protected set; } = new List<DescriptorSetLayout>();
        public DescriptorSet descriptorSet { get; protected set; }
        public Pipeline pipeline { get; protected set; }
        public PipelineLayout pipelineLayout { get; protected set; }
        public PipelineCache pipelineCache { get; protected set; }
        
        public JsonPipeline()
        {
          
        }

        public JsonPipeline(String jsonPipelineFilePath, RenderPass renderPass, uint ConstBufferSize)
        {
            _device = VulkanRenderer.device;

            //SavePipeline();

            string jsonContent = File.ReadAllText(jsonPipelineFilePath);
            RenderPipelineModel model = JsonConvert.DeserializeObject<RenderPipelineModel>(jsonContent);

            LoadDescriptorSets(model);
            LoadPipeline(model, renderPass, ConstBufferSize);
        }

        private void LoadDescriptorSets(RenderPipelineModel model)
        {
            var meshProperties = MemoryManager.GetGameObjectPropertiesBuffer();
            var textures = MemoryManager.GetTexturePropertiesBuffer();

            //CreateDescriptorPool
            {
                List<DescriptorPoolSize> descriptorPoolSizeList = new List<DescriptorPoolSize>();
                foreach (var binding in model.PipelineDescriptorModelsList)
                {
                    switch (binding.BindingPropertiesList)
                    {
                        case DescriptorBindingPropertiesEnum.kMeshPropertiesDescriptor: descriptorPoolSizeList.Add(new DescriptorPoolSize() { Type = DescriptorType.StorageBuffer, DescriptorCount = meshProperties.UCount() }); break;
                        case DescriptorBindingPropertiesEnum.kTextureDescriptor: descriptorPoolSizeList.Add(new DescriptorPoolSize() { Type = DescriptorType.CombinedImageSampler, DescriptorCount = textures.UCount() }); break;
                        default:
                            {
                                throw new Exception($"{binding} case hasn't been handled yet");
                            }
                    }
                }

                fixed (DescriptorPoolSize* descriptorPoolSize = descriptorPoolSizeList.ToArray())
                {
                    DescriptorPoolCreateInfo poolCreateInfo = new DescriptorPoolCreateInfo()
                    {
                        SType = StructureType.DescriptorPoolCreateInfo,
                        MaxSets = 500,
                        PPoolSizes = descriptorPoolSize,
                        PoolSizeCount = descriptorPoolSizeList.UCount(),
                        Flags = 0,
                        PNext = null
                    };
                    vk.CreateDescriptorPool(_device, in poolCreateInfo, null, out DescriptorPool descriptorPoolPtr);
                    descriptorPool = descriptorPoolPtr;
                }
            }

            //CreateDescriptorSetLayout
            {
                List<DescriptorSetLayoutBinding> descriptorSetLayoutBindingList = new List<DescriptorSetLayoutBinding>();
                foreach (var binding in model.PipelineDescriptorModelsList)
                {
                    switch (binding.BindingPropertiesList)
                    {
                        case DescriptorBindingPropertiesEnum.kMeshPropertiesDescriptor:
                            {
                                descriptorSetLayoutBindingList.Add(new DescriptorSetLayoutBinding()
                                {
                                    Binding = binding.BindingNumber,
                                    DescriptorCount = meshProperties.UCount(),
                                    DescriptorType = DescriptorType.StorageBuffer,
                                    PImmutableSamplers = null,
                                    StageFlags = ShaderStageFlags.All
                                });
                                break;
                            }
                        case DescriptorBindingPropertiesEnum.kTextureDescriptor:
                            {
                                descriptorSetLayoutBindingList.Add(new DescriptorSetLayoutBinding()
                                {
                                    Binding = binding.BindingNumber,
                                    DescriptorCount = textures.UCount(),
                                    DescriptorType = DescriptorType.CombinedImageSampler,
                                    PImmutableSamplers = null,
                                    StageFlags = ShaderStageFlags.All
                                });
                                break;
                            }
                        default:
                            {
                                throw new Exception($"{binding} case hasn't been handled yet");
                            }
                    }
                }

                fixed (DescriptorSetLayoutBinding* descriptorSetLayouts = descriptorSetLayoutBindingList.ToArray())
                {
                    DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = new DescriptorSetLayoutCreateInfo()
                    {
                        SType = StructureType.DescriptorSetLayoutCreateInfo,
                        BindingCount = descriptorSetLayoutBindingList.UCount(),
                        PBindings = descriptorSetLayouts,
                        Flags = 0,
                        PNext = null
                    };
                    vk.CreateDescriptorSetLayout(_device, &descriptorSetLayoutCreateInfo, null, out DescriptorSetLayout descriptorsetLayoutPtr);
                    descriptorSetLayoutList.Add(descriptorsetLayoutPtr);
                }
            }

            //AllocateDescriptorSets
            {
                DescriptorSetLayout* layouts = stackalloc DescriptorSetLayout[VulkanRenderer.MAX_FRAMES_IN_FLIGHT];

                for (int x = 0; x < VulkanRenderer.MAX_FRAMES_IN_FLIGHT; x++)
                {
                    layouts[x] = descriptorSetLayoutList[0];
                }

                DescriptorSetAllocateInfo allocInfo = new
                (
                    descriptorPool: descriptorPool,
                    descriptorSetCount: VulkanRenderer.MAX_FRAMES_IN_FLIGHT,
                    pSetLayouts: layouts
                );
                vk.AllocateDescriptorSets(VulkanRenderer.device, &allocInfo, out DescriptorSet descriptorSetPtr);
                descriptorSet = descriptorSetPtr;
            }

            //UpdateDescriptorSets
            {
                List<WriteDescriptorSet> descriptorSetList = new List<WriteDescriptorSet>();
                for (uint x = 0; x < VulkanRenderer.swapChain.ImageCount; x++)
                {
                    foreach (var binding in model.PipelineDescriptorModelsList)
                    {
                        switch (binding.BindingPropertiesList)
                        {
                            case DescriptorBindingPropertiesEnum.kMeshPropertiesDescriptor:
                                {
                                    fixed (DescriptorBufferInfo* meshInfo = meshProperties.ToArray())
                                    {
                                        descriptorSetList.Add(new WriteDescriptorSet()
                                        {
                                            SType = StructureType.WriteDescriptorSet,
                                            DescriptorCount = meshProperties.UCount(),
                                            DescriptorType = DescriptorType.UniformBuffer,
                                            DstBinding = binding.BindingNumber,
                                            DstArrayElement = 0,
                                            DstSet = descriptorSet,
                                            PBufferInfo = meshInfo,
                                            PImageInfo = null,
                                            PTexelBufferView = null,
                                            PNext = null
                                        });
                                    }
                                    break;
                                }
                            case DescriptorBindingPropertiesEnum.kTextureDescriptor:
                                {
                                    fixed (DescriptorImageInfo* texturesPtr = textures.ToArray())
                                    {
                                        descriptorSetList.Add(new WriteDescriptorSet()
                                        {
                                            SType = StructureType.WriteDescriptorSet,
                                            DescriptorCount = meshProperties.UCount(),
                                            DescriptorType = DescriptorType.CombinedImageSampler,
                                            DstBinding = binding.BindingNumber,
                                            DstArrayElement = 0,
                                            DstSet = descriptorSet,
                                            PBufferInfo = null,
                                            PImageInfo = texturesPtr,
                                            PTexelBufferView = null,
                                            PNext = null
                                        });
                                    }
                                    break;
                                }
                            default:
                                {
                                    throw new Exception($"{binding} case hasn't been handled yet");
                                }
                        }
                    }
                    fixed (WriteDescriptorSet* writeDescriptorSet = descriptorSetList.ToArray())
                    {
                        vk.UpdateDescriptorSets(_device, descriptorSetList.UCount(), writeDescriptorSet, 0, null);
                    }
                }
            }
        }

        private void LoadPipeline(RenderPipelineModel model, RenderPass renderPass, uint ConstBufferSize)
        {
            List<PushConstantRange> pushConstantRangeList = new List<PushConstantRange>();
            fixed (DescriptorSetLayout* descriptorSet = descriptorSetLayoutList.ToArray())
            {
                pushConstantRangeList = new List<PushConstantRange>()
                {
                    new PushConstantRange()
                    {
                        StageFlags = ShaderStageFlags.VertexBit | ShaderStageFlags.FragmentBit,
                        Offset = 0,
                        Size = ConstBufferSize
                    }
                };

                fixed (PushConstantRange* pushConstantRange = pushConstantRangeList.ToArray())
                {
                    PipelineLayoutCreateInfo pipelineLayoutInfo = new PipelineLayoutCreateInfo
                    {
                        SType = StructureType.PipelineLayoutCreateInfo,
                        SetLayoutCount = descriptorSetLayoutList.UCount(),
                        PSetLayouts = descriptorSet,
                        PushConstantRangeCount = pushConstantRangeList.UCount(),
                        PPushConstantRanges = pushConstantRange,
                        Flags = PipelineLayoutCreateFlags.None,
                        PNext = null,
                    };
                    vk.CreatePipelineLayout(VulkanRenderer.device, &pipelineLayoutInfo, null, out PipelineLayout pipelinelayoutPtr);
                    pipelineLayout = pipelinelayoutPtr;
                }
            }

            PipelineVertexInputStateCreateInfo vertexInputInfo = new PipelineVertexInputStateCreateInfo();
            fixed (VertexInputBindingDescription* vertexBindings = Vertex3D.GetBindingDescriptions().ToArray())
            fixed (VertexInputAttributeDescription* attributeBindings = Vertex3D.GetAttributeDescriptions().ToArray())
            {
                vertexInputInfo = new PipelineVertexInputStateCreateInfo()
                {
                    SType = StructureType.PipelineVertexInputStateCreateInfo,
                    PVertexAttributeDescriptions = attributeBindings,
                    PVertexBindingDescriptions = vertexBindings,
                    VertexAttributeDescriptionCount = Vertex3D.GetAttributeDescriptions().UCount(),
                    VertexBindingDescriptionCount = Vertex3D.GetBindingDescriptions().UCount(),
                    Flags = 0,
                    PNext = null
                };
            }

            PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = new PipelineInputAssemblyStateCreateInfo()
            {
                SType = StructureType.PipelineInputAssemblyStateCreateInfo,
                Topology = model.PipelineInputAssemblyStateCreateInfo.Topology,
                PrimitiveRestartEnable = model.PipelineInputAssemblyStateCreateInfo.PrimitiveRestartEnable,
                Flags = 0,
                PNext = null,
            };

            PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = new PipelineViewportStateCreateInfo();
            fixed (VkViewport* viewportPtr = model.ViewportList.ToArray())
            fixed (VkRect2D* scissorPtr = model.ScissorList.ToArray())
            {
                pipelineViewportStateCreateInfo = new PipelineViewportStateCreateInfo
                {
                    SType = StructureType.PipelineViewportStateCreateInfo,
                    ViewportCount = model.ViewportList.UCount() + 1,
                    PViewports = (Viewport*)viewportPtr,
                    ScissorCount = model.ScissorList.UCount() + 1,
                    PScissors = (Rect2D*)scissorPtr,
                    Flags = 0,
                    PNext = null
                };
            }

            PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = new PipelineColorBlendStateCreateInfo();
            fixed (PipelineColorBlendAttachmentState* attachments = VkPipelineColorBlendAttachmentState.ConvertPtrArray(model.PipelineColorBlendAttachmentStateList))
            {
                pipelineColorBlendStateCreateInfo = new PipelineColorBlendStateCreateInfo()
                {
                    SType = StructureType.PipelineColorBlendStateCreateInfo,
                    LogicOpEnable = model.PipelineColorBlendStateCreateInfoModel.LogicOpEnable,
                    LogicOp = model.PipelineColorBlendStateCreateInfoModel.LogicOp,
                    AttachmentCount = model.PipelineColorBlendAttachmentStateList.UCount(),
                    PAttachments = attachments,
                    Flags = 0,
                    PNext = null
                };
                pipelineColorBlendStateCreateInfo.BlendConstants[0] = 0.0f;
                pipelineColorBlendStateCreateInfo.BlendConstants[1] = 0.0f;
                pipelineColorBlendStateCreateInfo.BlendConstants[2] = 0.0f;
                pipelineColorBlendStateCreateInfo.BlendConstants[3] = 0.0f;
            }

            List<DynamicState> dynamicStateList = new List<DynamicState>()
            {
                DynamicState.Viewport,
                DynamicState.Scissor
            };

            PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = new PipelineDynamicStateCreateInfo();
            fixed (DynamicState* dynamicState = dynamicStateList.ToArray())
                pipelineDynamicStateCreateInfo = new PipelineDynamicStateCreateInfo()
                {
                    SType = StructureType.PipelineDynamicStateCreateInfo,
                    DynamicStateCount = dynamicStateList.UCount(),
                    PDynamicStates = dynamicState,
                    Flags = 0,
                    PNext = null
                };

            List<PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfoList = new List<PipelineShaderStageCreateInfo>()
            {
                VulkanRenderer.CreateShader(model.VertexShader,  ShaderStageFlags.VertexBit),
                VulkanRenderer.CreateShader(model.FragmentShader, ShaderStageFlags.FragmentBit)
            };

            PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfoRef = new PipelineRasterizationStateCreateInfo()
            {
                SType = StructureType.PipelineRasterizationStateCreateInfo,
                CullMode = model.PipelineRasterizationStateCreateInfo.CullMode,
                DepthBiasClamp = model.PipelineRasterizationStateCreateInfo.DepthBiasClamp,
                DepthBiasConstantFactor = model.PipelineRasterizationStateCreateInfo.DepthBiasConstantFactor,
                DepthBiasEnable = model.PipelineRasterizationStateCreateInfo.DepthBiasEnable,
                DepthBiasSlopeFactor = model.PipelineRasterizationStateCreateInfo.DepthBiasSlopeFactor,
                DepthClampEnable = model.PipelineRasterizationStateCreateInfo.DepthBiasEnable,
                RasterizerDiscardEnable = model.PipelineRasterizationStateCreateInfo.RasterizerDiscardEnable,
                FrontFace = model.PipelineRasterizationStateCreateInfo.FrontFace,
                LineWidth = model.PipelineRasterizationStateCreateInfo.LineWidth,
                PolygonMode = model.PipelineRasterizationStateCreateInfo.PolygonMode,
                Flags = 0,
                PNext = null
            };

            PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = new PipelineMultisampleStateCreateInfo()
            {
                SType = StructureType.PipelineMultisampleStateCreateInfo,
                AlphaToCoverageEnable = model.PipelineMultisampleStateCreateInfo.AlphaToCoverageEnable,
                AlphaToOneEnable = model.PipelineMultisampleStateCreateInfo.AlphaToOneEnable,
                MinSampleShading = model.PipelineMultisampleStateCreateInfo.MinSampleShading,
                PSampleMask = model.PipelineMultisampleStateCreateInfo.PSampleMask,
                RasterizationSamples = model.PipelineMultisampleStateCreateInfo.RasterizationSamples,
                SampleShadingEnable = model.PipelineMultisampleStateCreateInfo.SampleShadingEnable,
                Flags = 0,
                PNext = null
            };

            PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = new PipelineDepthStencilStateCreateInfo()
            {
                SType = StructureType.PipelineDepthStencilStateCreateInfo,
                Back = model.PipelineDepthStencilStateCreateInfo.Back,
                DepthBoundsTestEnable = model.PipelineDepthStencilStateCreateInfo.DepthBoundsTestEnable,
                DepthCompareOp = model.PipelineDepthStencilStateCreateInfo.DepthCompareOp,
                DepthTestEnable = model.PipelineDepthStencilStateCreateInfo.DepthTestEnable,
                DepthWriteEnable = model.PipelineDepthStencilStateCreateInfo.DepthWriteEnable,
                MaxDepthBounds = model.PipelineDepthStencilStateCreateInfo.MaxDepthBounds,
                Front = model.PipelineDepthStencilStateCreateInfo.Front,
                MinDepthBounds = model.PipelineDepthStencilStateCreateInfo.MinDepthBounds,
                StencilTestEnable = model.PipelineDepthStencilStateCreateInfo.DepthTestEnable,
                Flags = 0,
                PNext = null,
            };

            GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = new GraphicsPipelineCreateInfo();
            fixed (PipelineShaderStageCreateInfo* pipelineShaderStageCreateInfo = pipelineShaderStageCreateInfoList.ToArray())
            {
                graphicsPipelineCreateInfo = new GraphicsPipelineCreateInfo()
                {
                    SType = StructureType.GraphicsPipelineCreateInfo,
                    PStages = pipelineShaderStageCreateInfo,
                    PVertexInputState = &vertexInputInfo,
                    PInputAssemblyState = &pipelineInputAssemblyStateCreateInfo,
                    PViewportState = &pipelineViewportStateCreateInfo,
                    PRasterizationState = &pipelineRasterizationStateCreateInfoRef,
                    PMultisampleState = &pipelineMultisampleStateCreateInfo,
                    PDepthStencilState = &pipelineDepthStencilStateCreateInfo,
                    PColorBlendState = &pipelineColorBlendStateCreateInfo,
                    PDynamicState = &pipelineDynamicStateCreateInfo,
                    PTessellationState = null,
                    Layout = pipelineLayout,
                    RenderPass = renderPass,
                    StageCount = pipelineShaderStageCreateInfoList.UCount(),
                    Subpass = 0,
                    // BasePipelineHandle = ,
                    BasePipelineIndex = 0,
                    Flags = 0,
                    PNext = null
                };
            }

            vk.CreateGraphicsPipelines(VulkanRenderer.device, pipelineCache, 1, &graphicsPipelineCreateInfo, null, out Pipeline tempPipelinePtr);
            pipeline = tempPipelinePtr;
        }

        public void SavePipeline()
        {
            List<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentState = new List<VkPipelineColorBlendAttachmentState>()
            {
                new VkPipelineColorBlendAttachmentState()
                {
                    blendEnable = true,
                    srcColorBlendFactor = Silk.NET.Vulkan.BlendFactor.SrcAlpha,
                    dstColorBlendFactor = Silk.NET.Vulkan.BlendFactor.OneMinusSrcAlpha,
                    colorBlendOp = BlendOp.Add,
                    srcAlphaBlendFactor = Silk.NET.Vulkan.BlendFactor.One,
                    dstAlphaBlendFactor = Silk.NET.Vulkan.BlendFactor.Zero,
                    alphaBlendOp = BlendOp.Add,
                    colorWriteMask = ColorComponentFlags.RBit | ColorComponentFlags.GBit | ColorComponentFlags.BBit | ColorComponentFlags.ABit
                }
            };


            var jsonObj = new RenderPipelineModel
            {
                _name = "DefaultPipeline",
                VertexShader = "C:\\Users\\dotha\\Documents\\GitHub\\VulkanGameEngine\\Shaders\\vertshader.spv",
                FragmentShader = "C:\\Users\\dotha\\Documents\\GitHub\\VulkanGameEngine\\Shaders\\fragshader.spv",
                PipelineColorBlendAttachmentStateList = pipelineColorBlendAttachmentState,
                PipelineDepthStencilStateCreateInfo = new PipelineDepthStencilStateCreateInfoModel()
                {
                    DepthTestEnable = true,
                    DepthWriteEnable = true,
                    DepthCompareOp = CompareOp.LessOrEqual,
                    DepthBoundsTestEnable = false,
                    StencilTestEnable = false,
                    Front = new(StencilOp.Keep, StencilOp.Keep, StencilOp.Keep, CompareOp.Always),
                    Back = new(StencilOp.Keep, StencilOp.Keep, StencilOp.Keep, CompareOp.Always)
                },
                PipelineMultisampleStateCreateInfo = new PipelineMultisampleStateCreateInfoModel()
                {
                    RasterizationSamples = SampleCountFlags.SampleCount1Bit
                },
                PipelineRasterizationStateCreateInfo = new PipelineRasterizationStateCreateInfoModel()
                {
                    DepthClampEnable = false,
                    RasterizerDiscardEnable = false,
                    PolygonMode = PolygonMode.Fill,
                    CullMode = CullModeFlags.CullModeBackBit,
                    FrontFace = FrontFace.CounterClockwise,
                    DepthBiasEnable = false,
                    DepthBiasConstantFactor = 0.0f,
                    DepthBiasClamp = 0.0f,
                    DepthBiasSlopeFactor = 0.0f,
                    LineWidth = 1.0f
                },
                ScissorList = new List<VkRect2D>(),
                ViewportList = new List<VkViewport>(),
                PipelineColorBlendStateCreateInfoModel = new PipelineColorBlendStateCreateInfoModel()
                {
                    LogicOpEnable = false,
                    LogicOp = LogicOp.NoOp,
                    BlendConstants = new float[4] { 0.0f, 0.0f, 0.0f, 0.0f }
                },
                PipelineInputAssemblyStateCreateInfo = new PipelineInputAssemblyStateCreateInfoModel()
                {
                    PrimitiveRestartEnable = false,
                    Topology = PrimitiveTopology.TriangleList
                },
                PipelineDescriptorModelsList = new List<PipelineDescriptorModel>()
                    {
                        new PipelineDescriptorModel
                        {
                            BindingNumber = 0,
                            BindingPropertiesList = DescriptorBindingPropertiesEnum.kMeshPropertiesDescriptor,
                            descriptorType = DescriptorType.StorageBuffer
                        },
                        new PipelineDescriptorModel
                        {
                            BindingNumber = 1,
                            BindingPropertiesList = DescriptorBindingPropertiesEnum.kTextureDescriptor,
                            descriptorType = DescriptorType.CombinedImageSampler
                        }
                    },
                LayoutBindingList = new List<VkDescriptorSetLayoutBinding>()
                    {
                        new VkDescriptorSetLayoutBinding()
                        {
                            binding = 0,
                            descriptorType = DescriptorType.StorageBuffer,
                            descriptorCount = 1,
                            stageFlags = ShaderStageFlags.VertexBit | ShaderStageFlags.FragmentBit,
                            pImmutableSamplers = null
                        },
                        new VkDescriptorSetLayoutBinding()
                        {
                            binding = 1,
                            descriptorType = DescriptorType.CombinedImageSampler,
                            descriptorCount = 1,
                            stageFlags = ShaderStageFlags.FragmentBit,
                            pImmutableSamplers = null
                        }
                    }
            };

            string finalfilePath = @"C:\Users\dotha\Documents\GitHub\VulkanGameEngine\Pipelines\DefaultPipeline.json";
            string jsonString = JsonConvert.SerializeObject(jsonObj, Formatting.Indented);
            File.WriteAllText(finalfilePath, jsonString);
        }

        public CommandBuffer Draw(CommandBuffer[] commandBufferList, RenderPass renderPass, Framebuffer[] frameBufferList, ivec2 renderPassResolution, List<GameObject> gameObjectList, SceneDataBuffer sceneDataBuffer)
        {
            var commandIndex = (int)VulkanRenderer.CommandIndex;
            var imageIndex = (int)VulkanRenderer.ImageIndex;
            var commandBuffer = commandBufferList[commandIndex];

            List<ClearValue> clearValueList = new List<ClearValue>()
            {
                new ClearValue(new ClearColorValue(0, 0, 0, 1)),
                new ClearValue(null, new ClearDepthStencilValue(1.0f, 0))
            };

            RenderPassBeginInfo renderPassInfo = new RenderPassBeginInfo();
            fixed (ClearValue* clearValuePtr = clearValueList.ToArray())
            {
                renderPassInfo = new RenderPassBeginInfo()
                {
                    RenderPass = renderPass,
                    Framebuffer = frameBufferList[imageIndex],
                    ClearValueCount = clearValueList.UCount(),
                    PClearValues = clearValuePtr,
                    RenderArea = new Rect2D
                    {
                        Offset = new Offset2D(0, 0),
                        Extent = new Extent2D()
                        {
                            Width = (uint)renderPassResolution.x,
                            Height = (uint)renderPassResolution.y
                        }
                    }
                };
            }

            var viewport = new Viewport
            {
                X = 0.0f,
                Y = 0.0f,
                Width = (uint)renderPassResolution.x,
                Height = (uint)renderPassResolution.y,
                MinDepth = 0.0f,
                MaxDepth = 1.0f
            };

            var scissor = new Rect2D
            {
                Offset = new Offset2D(0, 0),
                Extent = new Extent2D()
                {
                    Width = (uint)renderPassResolution.x,
                    Height = (uint)renderPassResolution.y
                }
            };

            var descSet = descriptorSet;
            var commandInfo = new CommandBufferBeginInfo(flags: 0);

            vk.BeginCommandBuffer(commandBuffer, &commandInfo);
            vk.CmdBeginRenderPass(commandBuffer, &renderPassInfo, SubpassContents.Inline);
            vk.CmdSetViewport(commandBuffer, 0, 1, &viewport);
            vk.CmdSetScissor(commandBuffer, 0, 1, &scissor);
            vk.CmdBindPipeline(commandBuffer, PipelineBindPoint.Graphics, pipeline);
            foreach (var obj in gameObjectList)
            {
                obj.Draw(commandBuffer, pipeline, pipelineLayout, descSet, sceneDataBuffer);
            }
            vk.CmdEndRenderPass(commandBuffer);
            vk.EndCommandBuffer(commandBuffer);

            return commandBuffer;
        }
    }
}