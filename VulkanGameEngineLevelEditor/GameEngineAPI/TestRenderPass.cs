﻿using StbImageSharp;
using System;
using System.Collections.Generic;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Net.Mail;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;

namespace VulkanGameEngineLevelEditor.GameEngineAPI
{
    public unsafe class TestRenderPass : Renderpass
    {
        public RenderedTexture texture { get; set; }
        public TestRenderPass() : base()
        {
        }

        public void BuildRenderPass(Texture renderedTexture)
        {
            texture = new RenderedTexture(new GlmSharp.ivec2((int)VulkanRenderer.SwapChainResolution.Width, (int)VulkanRenderer.SwapChainResolution.Height), VkSampleCountFlagBits.VK_SAMPLE_COUNT_1_BIT, VkFormat.VK_FORMAT_R8G8B8A8_UNORM);

            var renderPass = CreateRenderPass();
            var frameBuffer = CreateFramebuffer();
            BuildRenderPipeline(renderedTexture);
        }

        public void BuildRenderPipeline(Texture renderedTexture)
        {
            var descriptorPool = CreateDescriptorPoolBinding();
            var descriptorSetLayout = CreateDescriptorSetLayout();
            var descriptorSet = CreateDescriptorSets();
            UpdateDescriptorSet(renderedTexture);
            var descriptorPipelineLayout = CreatePipelineLayout();
            var shaderList = CreateShaders();

            var vertexInput = PipelineVertexInputStateCreate();
            var inputAssembly = PipelineInputAssemblyStateCreate();
            var viewport = PipelineViewportStateCreate();
            var blendingInfo = PipelineColorBlendAttachmentState();
            var depthInfo = PipelineDepthStencilStateCreateInfo();
            var rasterizer = PipelineRasterizationStateCreateInfo();
            var multisampling = PipelineMultisampleStateCreate();


            VkPipelineColorBlendAttachmentState blendAttachment = new VkPipelineColorBlendAttachmentState
            {
                blendEnable = VulkanConsts.VK_TRUE,
                srcColorBlendFactor = VkBlendFactor.VK_BLEND_FACTOR_SRC_ALPHA,
                dstColorBlendFactor = VkBlendFactor.VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                colorBlendOp = VkBlendOp.VK_BLEND_OP_ADD,
                srcAlphaBlendFactor = VkBlendFactor.VK_BLEND_FACTOR_ONE,
                dstAlphaBlendFactor = VkBlendFactor.VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                alphaBlendOp = VkBlendOp.VK_BLEND_OP_ADD,
                colorWriteMask = (VkColorComponentFlagBits)(
                    VkColorComponentFlagBits.VK_COLOR_COMPONENT_R_BIT |
                    VkColorComponentFlagBits.VK_COLOR_COMPONENT_G_BIT |
                    VkColorComponentFlagBits.VK_COLOR_COMPONENT_B_BIT |
                    VkColorComponentFlagBits.VK_COLOR_COMPONENT_A_BIT)
            };

            VkPipelineColorBlendStateCreateInfo blending = new VkPipelineColorBlendStateCreateInfo
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                pNext = IntPtr.Zero,
                flags = 0,
                logicOpEnable = VulkanConsts.VK_FALSE,
                logicOp = VkLogicOp.VK_LOGIC_OP_COPY,
                attachmentCount = 1,
                pAttachments = &blendAttachment,
            };
            blending.blendConstants[0] = 0.0f;
            blending.blendConstants[1] = 0.0f;
            blending.blendConstants[2] = 0.0f;
            blending.blendConstants[3] = 0.0f;


            fixed (VkPipelineShaderStageCreateInfo* shaderlist = shaderList.ToArray())
            {
                var pipelineInfo = new VkGraphicsPipelineCreateInfo
                {
                    sType = VkStructureType.VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                    pNext = IntPtr.Zero,
                    flags = 0,
                    stageCount = 1,
                    pStages = shaderlist,
                    pVertexInputState = &vertexInput,
                    pInputAssemblyState = &inputAssembly,
                    pViewportState = &viewport,
                    pRasterizationState = &rasterizer,
                    pMultisampleState = &multisampling,
                    pDepthStencilState = &depthInfo,
                    pColorBlendState = &blending,
                    layout = ShaderPipelineLayout,
                    renderPass = RenderPass,
                    subpass = 0,
                    basePipelineHandle = IntPtr.Zero,
                };

                VkPipeline pipeline;
                VulkanAPI.vkCreateGraphicsPipelines(VulkanRenderer.Device, IntPtr.Zero, 1, &pipelineInfo, null, &pipeline);
                ShaderPipeline = pipeline;

            }


        }

        public void UpdateRenderPass(Texture texture)
        {

        }

        public VkCommandBuffer Draw()
        {
            List<VkClearValue> clearValues = new List<VkClearValue>()
            {
                new VkClearValue
                {
                    color = new VkClearColorValue(new float[] { 1.0f, 0.0f, 0.0f, 1.0f }) // Using the float constructor
                }
            };

            List<VkViewport> viewportList = new List<VkViewport>()
            {
                new VkViewport
                {
                    x = 0.0f,
                    y = 0.0f,
                    width = (float)VulkanRenderer.SwapChainResolution.Width,
                    height = (float)VulkanRenderer.SwapChainResolution.Height,
                    minDepth = 0.0f,
                    maxDepth = 1.0f
                }
            };


            List<VkRect2D> rect2DList = new List<VkRect2D>()
            {
                new VkRect2D
                {
                    Offset = new VkOffset2D()
                    {
                        X = 0,
                        Y = 0
                    },
                    Extent = new VkExtent2D()
                    {
                        Width = (uint)VulkanRenderer.SwapChainResolution.Width,
                        Height = (uint)VulkanRenderer.SwapChainResolution.Height,
                    }
                }
            };

            VkRenderPassBeginInfo renderPassInfo = new VkRenderPassBeginInfo()
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                renderPass = RenderPass,
                framebuffer = FrameBufferList[(int)VulkanRenderer.ImageIndex],
                renderArea = new VkRect2D
                {
                    Offset = new VkOffset2D()
                    {
                        X = 0,
                        Y = 0
                    },
                    Extent = new VkExtent2D()
                    {
                        Width = (uint)VulkanRenderer.SwapChainResolution.Width,
                        Height = (uint)VulkanRenderer.SwapChainResolution.Height,
                    }
                },
                clearValueCount = (uint)clearValues.Count,
                pClearValues = clearValues.ToPointer()
            };

            VkCommandBufferBeginInfo CommandBufferBeginInfo = new VkCommandBufferBeginInfo()
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                flags = VkCommandBufferUsageFlagBits.VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
            };

            int imageIndex = (int)VulkanRenderer.ImageIndex;

            VkDescriptorSet[] descriptorSetList = { DescriptorSet };
            uint[] offsetList = { 0 };
            fixed (VkDescriptorSet* descriptorSet = descriptorSetList.ToArray())
            fixed (uint* offset = offsetList.ToArray())
            fixed (VkViewport* viewport = viewportList.ToArray())
            fixed (VkRect2D* rect2D = rect2DList.ToArray())
            {
                VulkanAPI.vkBeginCommandBuffer(CommandBufferList[imageIndex], &CommandBufferBeginInfo);
                VulkanAPI.vkCmdBeginRenderPass(CommandBufferList[imageIndex], &renderPassInfo, VkSubpassContents.VK_SUBPASS_CONTENTS_INLINE);
                VulkanAPI.vkCmdSetViewport(CommandBufferList[imageIndex], 0, (uint)viewportList.Count, viewport);
                VulkanAPI.vkCmdSetScissor(CommandBufferList[imageIndex], 0, (uint)rect2DList.Count, rect2D);
                VulkanAPI.vkCmdBindPipeline(CommandBufferList[imageIndex], VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, ShaderPipeline);
                VulkanAPI.vkCmdBindDescriptorSets(CommandBufferList[imageIndex], VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS, ShaderPipelineLayout, 0, 1, descriptorSet, 0, offset);
                VulkanAPI.vkCmdDraw(CommandBufferList[imageIndex], 6, 1, 0, 0);
                VulkanAPI.vkCmdEndRenderPass(CommandBufferList[imageIndex]);
                VulkanAPI.vkEndCommandBuffer(CommandBufferList[imageIndex]);
            }
            return CommandBufferList[(int)VulkanRenderer.ImageIndex];
        }

        private VkRenderPass CreateRenderPass()
        {
            VkRenderPass renderPass = new VkRenderPass();
            List<VkAttachmentDescription> attachmentDescriptionList = new List<VkAttachmentDescription>()
            {
                new VkAttachmentDescription
                {
                    format = VkFormat.VK_FORMAT_R8G8B8A8_UNORM,
                    samples = VkSampleCountFlagBits.VK_SAMPLE_COUNT_1_BIT,
                    loadOp = VkAttachmentLoadOp.VK_ATTACHMENT_LOAD_OP_CLEAR,
                    storeOp = VkAttachmentStoreOp.VK_ATTACHMENT_STORE_OP_STORE,
                    stencilLoadOp = VkAttachmentLoadOp.VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    stencilStoreOp = VkAttachmentStoreOp.VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    initialLayout = VkImageLayout.VK_IMAGE_LAYOUT_UNDEFINED,
                    finalLayout = VkImageLayout.VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                }
            };

            List<VkAttachmentReference> colorRefsList = new List<VkAttachmentReference>()
            {
                new VkAttachmentReference
                {
                    attachment = 0,
                    layout = VkImageLayout.VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                }
            };

            List<VkSubpassDependency> subpassDependencyList = new List<VkSubpassDependency>
                {
                    new VkSubpassDependency
                    {
                        srcSubpass = VulkanConsts.VK_SUBPASS_EXTERNAL,
                        dstSubpass = 0,
                        srcStageMask = VkPipelineStageFlags.VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        dstStageMask = VkPipelineStageFlags.VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        srcAccessMask = 0,
                        dstAccessMask = VkAccessFlags.VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
                    }
                };


            List<VkSubpassDescription> subpassDescriptionList = new List<VkSubpassDescription>();
            fixed (VkAttachmentReference* colorRefs = colorRefsList.ToArray())
            {
                subpassDescriptionList = new List<VkSubpassDescription>
                {
                    new VkSubpassDescription
                    {
                        pipelineBindPoint = VkPipelineBindPoint.VK_PIPELINE_BIND_POINT_GRAPHICS,
                        colorAttachmentCount = (uint)colorRefsList.Count,
                        pColorAttachments = colorRefs,
                       // pResolveAttachments = null, // Set to null unless you actually have resolve attachments
                       // pDepthStencilAttachment = null // Set to null unless using a depth/stencil attachment
                    }
                };
            }

            fixed (VkAttachmentDescription* attachments = attachmentDescriptionList.ToArray())
            fixed (VkSubpassDescription* description = subpassDescriptionList.ToArray())
            fixed (VkSubpassDependency* dependency = subpassDependencyList.ToArray())
            {
                var renderPassCreateInfo = new VkRenderPassCreateInfo()
                {
                    sType = VkStructureType.VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                    pNext = null,
                    flags = 0,
                    attachmentCount = (uint)attachmentDescriptionList.Count(),
                    pAttachments = attachments,
                    subpassCount = (uint)subpassDescriptionList.Count(),
                    pSubpasses = description,
                    dependencyCount = (uint)subpassDependencyList.Count(),
                    pDependencies = dependency,
                };

                VulkanAPI.vkCreateRenderPass(VulkanRenderer.Device, &renderPassCreateInfo, null, &renderPass);
                RenderPass = renderPass;
            }

            return renderPass;
        }

        private List<VkFramebuffer> CreateFramebuffer()
        {
            List<VkImageView> texturesViews = new List<VkImageView> { texture.View };
            List<VkFramebuffer> frameBufferList = FrameBufferList;
            for (int x = 0; x < VulkanRenderer.SwapChainImageCount; x++)
            {
                fixed (VkImageView* imageViewPtr = texturesViews.ToArray())
                {
                    VkFramebufferCreateInfo framebufferInfo = new VkFramebufferCreateInfo()
                    {
                        sType = VkStructureType.VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                        renderPass = RenderPass,
                        attachmentCount = (uint)texturesViews.Count(),
                        pAttachments = imageViewPtr,
                        width = VulkanRenderer.SwapChainResolution.Width,
                        height = VulkanRenderer.SwapChainResolution.Height,
                        layers = 1
                    };

                    VkFramebuffer frameBuffer = FrameBufferList[x];
                    VulkanAPI.vkCreateFramebuffer(VulkanRenderer.Device, &framebufferInfo, null, &frameBuffer);
                    frameBufferList[x] = frameBuffer;
                }
            }

            FrameBufferList = frameBufferList;
            return frameBufferList;
        }

        private VkDescriptorPool CreateDescriptorPoolBinding()
        {
            VkDescriptorPool descriptorPool = new VkDescriptorPool();
            List<VkDescriptorPoolSize> DescriptorPoolBinding = new List<VkDescriptorPoolSize>();
            {
                new VkDescriptorPoolSize
                {
                    type = VkDescriptorType.VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descriptorCount = 1
                };
            };

            fixed (VkDescriptorPoolSize* ptr = DescriptorPoolBinding.ToArray())
            {
                VkDescriptorPoolCreateInfo poolInfo = new VkDescriptorPoolCreateInfo()
                {
                    sType = VkStructureType.VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                    maxSets = VulkanRenderer.SwapChainImageCount,
                    poolSizeCount = (uint)DescriptorPoolBinding.Count,
                    pPoolSizes = ptr
                };
                descriptorPool = VulkanRenderer.CreateDescriptorPool(poolInfo);
            }

            DescriptorPool = descriptorPool;
            return descriptorPool;
        }

        private VkDescriptorSetLayout CreateDescriptorSetLayout()
        {
            VkDescriptorSetLayout descriptorSetLayout = new VkDescriptorSetLayout();
            List<VkDescriptorSetLayoutBinding> LayoutBindingList = new List<VkDescriptorSetLayoutBinding>()
            {
                new VkDescriptorSetLayoutBinding()
                {
                    binding = 0,
                    descriptorType = VkDescriptorType.VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descriptorCount = 1,
                    stageFlags = VkShaderStageFlags.VK_SHADER_STAGE_FRAGMENT_BIT,
                    pImmutableSamplers = IntPtr.Zero
                },

            };

            fixed (VkDescriptorSetLayoutBinding* ptr = LayoutBindingList.ToArray())
            {
                VkDescriptorSetLayoutCreateInfo layoutInfo = new VkDescriptorSetLayoutCreateInfo()
                {
                    sType = VkStructureType.VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                    bindingCount = (uint)LayoutBindingList.Count,
                    pBindings = ptr,
                };
                descriptorSetLayout = VulkanRenderer.CreateDescriptorSetLayout(layoutInfo);
            }
            DescriptorSetLayout = descriptorSetLayout;
            return descriptorSetLayout;
        }

        private VkDescriptorSet CreateDescriptorSets()
        {

            VkDescriptorSet descriptorSet = new VkDescriptorSet();
            VkDescriptorSetLayout layout = DescriptorSetLayout;
            VkDescriptorSetAllocateInfo allocInfo = new VkDescriptorSetAllocateInfo()
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                descriptorPool = DescriptorPool,
                descriptorSetCount = 1,
                pSetLayouts = &layout
            };

            VkResult result = VulkanAPI.vkAllocateDescriptorSets(VulkanRenderer.Device, &allocInfo, &descriptorSet);
            DescriptorSet = descriptorSet;
            return descriptorSet;
        }

        private void UpdateDescriptorSet(Texture texture)
        {
            List<VkDescriptorImageInfo> colorDescriptorImages = new List<VkDescriptorImageInfo>
            {
                new VkDescriptorImageInfo
                {
                    sampler = texture.Sampler,
                    imageView = texture.View,
                    imageLayout = VkImageLayout.VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                }
            };

            List<VkWriteDescriptorSet> descriptorSetList = new List<VkWriteDescriptorSet>();
            fixed (VkDescriptorImageInfo* imageInfoPtr = colorDescriptorImages.ToArray())
            {
                for (uint x = 0; x < VulkanRenderer.SwapChainImageCount; x++)
                {
                    VkWriteDescriptorSet descriptorSet = new VkWriteDescriptorSet
                    {
                        sType = VkStructureType.VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        dstSet = DescriptorSet,
                        dstBinding = 0,
                        dstArrayElement = x,
                        descriptorCount = 1,
                        descriptorType = VkDescriptorType.VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        pImageInfo = imageInfoPtr + x, // Use imageInfoPtr to access the right image info
                        pBufferInfo = null,
                        pTexelBufferView = null
                    };

                    descriptorSetList.Add(descriptorSet);
                }
            }

            fixed (VkWriteDescriptorSet* ptr = descriptorSetList.ToArray())
            {
                VulkanAPI.vkUpdateDescriptorSets(VulkanRenderer.Device, colorDescriptorImages.UCount(), ptr, 0, null);
            }
        }

        private VkPipelineLayout CreatePipelineLayout()
        {
            VkDescriptorSetLayout descriptorSetLayout = DescriptorSetLayout;

            VkPipelineLayout shaderPipelineLayout = new VkPipelineLayout();
            VkPipelineLayoutCreateInfo pipelineLayoutInfo = new VkPipelineLayoutCreateInfo
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                setLayoutCount = 1,
                pSetLayouts = &descriptorSetLayout
            };
            VulkanAPI.vkCreatePipelineLayout(VulkanRenderer.Device, &pipelineLayoutInfo, null, &shaderPipelineLayout);

            ShaderPipelineLayout = shaderPipelineLayout;
            return shaderPipelineLayout;
        }

        private List<VkPipelineShaderStageCreateInfo> CreateShaders()
        {
            return new List<VkPipelineShaderStageCreateInfo>()
            {
                VulkanRenderer.CreateShader("C:/Users/dotha/Documents/GitHub/2D-Game-Engine/Shaders/FrameBufferShaderVert.spv",  VkShaderStageFlagBits.VK_SHADER_STAGE_VERTEX_BIT),
                VulkanRenderer.CreateShader("C:/Users/dotha/Documents/GitHub/2D-Game-Engine/Shaders/FrameBufferShaderFrag.spv", VkShaderStageFlagBits.VK_SHADER_STAGE_FRAGMENT_BIT)
            };
        }

        private VkPipelineVertexInputStateCreateInfo PipelineVertexInputStateCreate()
        {
            return new VkPipelineVertexInputStateCreateInfo()
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
            };
        }

        private VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreate()
        {

            return new VkPipelineInputAssemblyStateCreateInfo()
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                topology = VkPrimitiveTopology.VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                primitiveRestartEnable = VulkanConsts.VK_FALSE
            };
        }

        private VkPipelineViewportStateCreateInfo PipelineViewportStateCreate()
        {
            List<VkViewport> viewportList = new List<VkViewport>()
            {
                new VkViewport
                {
                    x = 0.0f,
                    y = 0.0f,
                    width = (float)VulkanRenderer.SwapChainResolution.Width, // Ensure this is not zero or negative
                    height = (float)VulkanRenderer.SwapChainResolution.Height, // Ensure this is not zero or negative
                    minDepth = 0.0f,
                    maxDepth = 1.0f
                }
            };

            List<VkRect2D> rect2DList = new List<VkRect2D>()
            {
                new VkRect2D
                {
                    Offset = new VkOffset2D
                    {
                        X = 0,
                        Y = 0
                    },
                    Extent = new VkExtent2D
                    {
                        Width = VulkanRenderer.SwapChainResolution.Width, // Ensure this is correct
                        Height = VulkanRenderer.SwapChainResolution.Height // Ensure this is correct
                    }
                }
            };

            var viewportState = new VkPipelineViewportStateCreateInfo();
            fixed (VkViewport* viewport = viewportList.ToArray())
            fixed (VkRect2D* rect2D = rect2DList.ToArray())
            {
                VkPipelineViewportStateCreateInfo viewportStateInfo = new VkPipelineViewportStateCreateInfo()
                {
                    sType = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                    viewportCount = 1,
                    pViewports = viewport,
                    scissorCount = 1,
                    pScissors = rect2D,
                };
                viewportState = viewportStateInfo;
            }

            return viewportState;
        }

        private VkPipelineColorBlendStateCreateInfo PipelineColorBlendAttachmentState()
        {
            List<VkPipelineColorBlendAttachmentState> blendAttachmentList = new List<VkPipelineColorBlendAttachmentState>
            {
                new VkPipelineColorBlendAttachmentState
                {
                    blendEnable = VulkanConsts.VK_TRUE,
                    srcColorBlendFactor = VkBlendFactor.VK_BLEND_FACTOR_SRC_ALPHA,
                    dstColorBlendFactor = VkBlendFactor.VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    colorBlendOp = VkBlendOp.VK_BLEND_OP_ADD,
                    srcAlphaBlendFactor = VkBlendFactor.VK_BLEND_FACTOR_ONE,
                    dstAlphaBlendFactor = VkBlendFactor.VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    alphaBlendOp = VkBlendOp.VK_BLEND_OP_ADD,
                    colorWriteMask = (VkColorComponentFlagBits)(
                        VkColorComponentFlagBits.VK_COLOR_COMPONENT_R_BIT |
                        VkColorComponentFlagBits.VK_COLOR_COMPONENT_G_BIT |
                        VkColorComponentFlagBits.VK_COLOR_COMPONENT_B_BIT |
                        VkColorComponentFlagBits.VK_COLOR_COMPONENT_A_BIT)
                }
            };

            VkPipelineColorBlendStateCreateInfo colorBlending;
            fixed (VkPipelineColorBlendAttachmentState* blendAttachmentPtr = blendAttachmentList.ToArray())
            {
                colorBlending = new VkPipelineColorBlendStateCreateInfo()
                {
                    sType = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                    attachmentCount = (uint)blendAttachmentList.Count(),
                    pAttachments = blendAttachmentPtr
                };
            }
            return colorBlending;
        }

        private VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo()
        {

            return new VkPipelineDepthStencilStateCreateInfo()
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                depthTestEnable = VulkanConsts.VK_TRUE,
                depthWriteEnable = VulkanConsts.VK_TRUE,
                depthCompareOp = VkCompareOp.VK_COMPARE_OP_LESS,
                depthBoundsTestEnable = VulkanConsts.VK_FALSE,
                stencilTestEnable = VulkanConsts.VK_FALSE
            };
        }

        private VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo()
        {
            return new VkPipelineRasterizationStateCreateInfo()
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                depthClampEnable = VulkanConsts.VK_FALSE,
                rasterizerDiscardEnable = VulkanConsts.VK_FALSE,
                polygonMode = VkPolygonMode.VK_POLYGON_MODE_FILL,
                cullMode = VkCullModeFlags.VK_CULL_MODE_NONE,
                frontFace = VkFrontFace.VK_FRONT_FACE_COUNTER_CLOCKWISE,
                depthBiasEnable = VulkanConsts.VK_FALSE,
                lineWidth = 1.0f
            };
        }

        private VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreate()
        {
            return new VkPipelineMultisampleStateCreateInfo()
            {
                sType = VkStructureType.VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                rasterizationSamples = VkSampleCountFlagBits.VK_SAMPLE_COUNT_1_BIT
            };
        }
    }
}