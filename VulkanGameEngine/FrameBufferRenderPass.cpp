#include "FrameBufferRenderPass.h"
#include <CVulkanRenderer.h>
#include "ShaderCompiler.h"
#include <stdexcept>

FrameBufferRenderPass::FrameBufferRenderPass()
{
}

FrameBufferRenderPass::~FrameBufferRenderPass()
{
}

void FrameBufferRenderPass::BuildRenderPass(SharedPtr<Texture> texture)
{
    RenderPassResolution = glm::ivec2((int)cRenderer.SwapChain.SwapChainResolution.width, (int)cRenderer.SwapChain.SwapChainResolution.height);
    SampleCount = VK_SAMPLE_COUNT_1_BIT;
    FrameBufferList.resize(cRenderer.SwapChain.SwapChainImageCount);

    VULKAN_RESULT(renderer.CreateCommandBuffer(CommandBuffer));
    RenderPass = CreateRenderPass();
    FrameBufferList = CreateFramebuffer();
    BuildRenderPipeline(texture);
}

VkRenderPass FrameBufferRenderPass::CreateRenderPass()
{
    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkAttachmentDescription> attachmentDescriptionList
    {
        VkAttachmentDescription
        {
            .format = VK_FORMAT_B8G8R8A8_UNORM,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        }
    };

    std::vector<VkAttachmentReference> colorRefsList
    {
        VkAttachmentReference
        {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }
    };

    std::vector<VkAttachmentReference> multiSampleReferenceList{};
    std::vector<VkAttachmentReference> depthReference{};

    std::vector<VkSubpassDescription> subpassDescriptionList
    {
        VkSubpassDescription
        {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = static_cast<uint32>(colorRefsList.size()),
            .pColorAttachments = colorRefsList.data(),
            .pResolveAttachments = multiSampleReferenceList.data(),
            .pDepthStencilAttachment = depthReference.data()
        }
    };

    std::vector<VkSubpassDependency> subpassDependencyList
    {
        VkSubpassDependency
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        },
    };

    VkRenderPassCreateInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32>(attachmentDescriptionList.size()),
        .pAttachments = attachmentDescriptionList.data(),
        .subpassCount = static_cast<uint32>(subpassDescriptionList.size()),
        .pSubpasses = subpassDescriptionList.data(),
        .dependencyCount = static_cast<uint32>(subpassDependencyList.size()),
        .pDependencies = subpassDependencyList.data()
    };

    VULKAN_RESULT(vkCreateRenderPass(cRenderer.Device, &renderPassInfo, nullptr, &renderPass));

    return renderPass;
}

Vector<VkFramebuffer> FrameBufferRenderPass::CreateFramebuffer()
{
    Vector<VkFramebuffer> frameBuffer = Vector<VkFramebuffer>(cRenderer.SwapChain.SwapChainImageCount);
    for (size_t x = 0; x < cRenderer.SwapChain.SwapChainImageCount; x++)
    {
        std::vector<VkImageView> TextureAttachmentList;
        TextureAttachmentList.emplace_back(cRenderer.SwapChain.SwapChainImageViews[x]);

        VkFramebufferCreateInfo framebufferInfo
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = RenderPass,
            .attachmentCount = static_cast<uint32>(TextureAttachmentList.size()),
            .pAttachments = TextureAttachmentList.data(),
            .width = static_cast<uint32>(RenderPassResolution.x),
            .height = static_cast<uint32>(RenderPassResolution.y),
            .layers = 1
        };
        VULKAN_RESULT(vkCreateFramebuffer(cRenderer.Device, &framebufferInfo, nullptr, &frameBuffer[x]));
    }
    return frameBuffer;
}

VkDescriptorPool FrameBufferRenderPass::CreateDescriptorPoolBinding()
{
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorPoolSize> DescriptorPoolBinding =
    {
        VkDescriptorPoolSize
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1
        }
    };
    VkDescriptorPoolCreateInfo poolInfo =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = cRenderer.SwapChain.SwapChainImageCount,
        .poolSizeCount = static_cast<uint32>(DescriptorPoolBinding.size()),
        .pPoolSizes = DescriptorPoolBinding.data(),
    };
    VULKAN_RESULT(Renderer_CreateDescriptorPool(cRenderer.Device, &descriptorPool, &poolInfo));
    return descriptorPool;
}

VkDescriptorSetLayout FrameBufferRenderPass::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayoutBinding> LayoutBindingList =
    {
        { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
        { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
    };
    VkDescriptorSetLayoutCreateInfo layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32>(LayoutBindingList.size()),
        .pBindings = LayoutBindingList.data(),
    };
    VULKAN_RESULT(Renderer_CreateDescriptorSetLayout(cRenderer.Device, &descriptorSetLayout, &layoutInfo));
    return descriptorSetLayout;
}

VkDescriptorSet FrameBufferRenderPass::CreateDescriptorSets()
{
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetAllocateInfo allocInfo =
    {
         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = DescriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &DescriptorSetLayout
    };
    VULKAN_RESULT(Renderer_AllocateDescriptorSets(cRenderer.Device, &descriptorSet, &allocInfo));
    return descriptorSet;
}

void FrameBufferRenderPass::UpdateDescriptorSet(SharedPtr<Texture> texture)
{
    std::vector<VkDescriptorImageInfo> ColorDescriptorImage
    {
        VkDescriptorImageInfo
        {
            .sampler = texture->Sampler,
            .imageView = texture->View,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        }
    };
    for (size_t x = 0; x < cRenderer.SwapChain.SwapChainImageCount; x++)
    {
        std::vector<VkWriteDescriptorSet> descriptorSets
        {
            VkWriteDescriptorSet
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = DescriptorSet,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = ColorDescriptorImage.data(),
            }
        };
        Renderer_UpdateDescriptorSet(cRenderer.Device, descriptorSets.data(), static_cast<uint32_t>(descriptorSets.size()));
    }
}

VkPipelineLayout FrameBufferRenderPass::CreatePipelineLayout()
{
    VkPipelineLayout shaderPipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &DescriptorSetLayout
    };
    VULKAN_RESULT(Renderer_CreatePipelineLayout(cRenderer.Device, &shaderPipelineLayout, &pipelineLayoutInfo));
    return shaderPipelineLayout;
}

Vector<VkPipelineShaderStageCreateInfo> FrameBufferRenderPass::CreateShaders()
{
    return Vector<VkPipelineShaderStageCreateInfo>
    {
        ShaderCompiler::CreateShader("C:/Users/dotha/Documents/GitHub/VulkanGameEngine/Shaders/FrameBufferShaderVert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        ShaderCompiler::CreateShader("C:/Users/dotha/Documents/GitHub/VulkanGameEngine/Shaders/FrameBufferShaderFrag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    };
}

void FrameBufferRenderPass::BuildRenderPipeline(SharedPtr<Texture> texture)
{
    DescriptorPool = CreateDescriptorPoolBinding();
    DescriptorSetLayout = CreateDescriptorSetLayout();
    DescriptorSet = CreateDescriptorSets();
    UpdateDescriptorSet(texture);
    ShaderPipelineLayout = CreatePipelineLayout();
    Vector<VkPipelineShaderStageCreateInfo> PipelineShaderStageList = CreateShaders();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    std::vector<VkViewport> viewport
    {
        VkViewport
        {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float)RenderPassResolution.x,
            .height = (float)RenderPassResolution.y,
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        }
    };

    std::vector<VkRect2D> rect2D
    {
        VkRect2D
        {
            .offset = { 0, 0 },
            .extent = 
            { 
                static_cast<uint32>(RenderPassResolution.x), 
                static_cast<uint32>(RenderPassResolution.y) 
            }
        }
    };

    VkPipelineViewportStateCreateInfo viewportState
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = viewport.data(),
        .scissorCount = 1,
        .pScissors = rect2D.data(),
    };

    std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentList
    {
        VkPipelineColorBlendAttachmentState
        {
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                              VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT |
                              VK_COLOR_COMPONENT_A_BIT
        }
    };

    VkPipelineDepthStencilStateCreateInfo blendDepthAttachment
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE
    };

    VkPipelineRasterizationStateCreateInfo rasterizer
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampling
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlending
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = static_cast<uint32>(blendAttachmentList.size()),
        .pAttachments = blendAttachmentList.data()
    };

    std::vector<VkGraphicsPipelineCreateInfo> pipelineInfo
    {
        VkGraphicsPipelineCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = static_cast<uint32>(PipelineShaderStageList.size()),
            .pStages = PipelineShaderStageList.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = &blendDepthAttachment,
            .pColorBlendState = &colorBlending,
            .layout = ShaderPipelineLayout,
            .renderPass = RenderPass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE
        }
    };
   VULKAN_RESULT(Renderer_CreateGraphicsPipelines(cRenderer.Device, &ShaderPipeline, pipelineInfo.data(), static_cast<uint32>(pipelineInfo.size())));

   for (auto& shader : PipelineShaderStageList)
   {
       vkDestroyShaderModule(cRenderer.Device, shader.module, nullptr);
   }
}

void FrameBufferRenderPass::UpdateRenderPass(SharedPtr<Texture> texture)
{
    RenderPassResolution = glm::ivec2((int)cRenderer.SwapChain.SwapChainResolution.width, (int)cRenderer.SwapChain.SwapChainResolution.height);
    SampleCount = VK_SAMPLE_COUNT_1_BIT;

    renderer.DestroyFrameBuffers(FrameBufferList);
    renderer.DestroyRenderPass(RenderPass);
    renderer.DestroyPipeline(ShaderPipeline);
    renderer.DestroyPipelineLayout(ShaderPipelineLayout);
    renderer.DestroyPipelineCache(PipelineCache);
    renderer.DestroyDescriptorSetLayout(DescriptorSetLayout);
    renderer.DestroyDescriptorPool(DescriptorPool);
    BuildRenderPass(texture);
}

VkCommandBuffer FrameBufferRenderPass::Draw()
{
    std::vector<VkClearValue> clearValues
    {
        VkClearValue{.color = { {1.0f, 1.0f, 1.0f, 1.0f} } }
    };

    VkRenderPassBeginInfo renderPassInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = RenderPass,
        .framebuffer = FrameBufferList[cRenderer.ImageIndex],
        .renderArea
        {
            .offset = {0, 0},
            .extent =
            {
                .width = static_cast<uint32>(RenderPassResolution.x),
                .height = static_cast<uint32>(RenderPassResolution.y)
            }
        },
        .clearValueCount = static_cast<uint32>(clearValues.size()),
        .pClearValues = clearValues.data()
    };

    VkViewport viewport = VkViewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(RenderPassResolution.x),
        .height = static_cast<float>(RenderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = VkRect2D
    {
        .offset = VkOffset2D(0, 0),
        .extent = VkExtent2D
        {
          .width = static_cast<uint32>(RenderPassResolution.x),
          .height = static_cast<uint32>(RenderPassResolution.y)
        }
    };

    VkCommandBufferBeginInfo CommandBufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
    };

    VULKAN_RESULT(vkResetCommandBuffer(CommandBuffer, 0));
    VULKAN_RESULT(vkBeginCommandBuffer(CommandBuffer, &CommandBufferBeginInfo));
    vkCmdBeginRenderPass(CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(CommandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(CommandBuffer, 0, 1, &scissor);
    vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ShaderPipeline);
    vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ShaderPipelineLayout, 0, 1, &DescriptorSet, 0, nullptr);
    vkCmdDraw(CommandBuffer, 6, 1, 0, 0);
    vkCmdEndRenderPass(CommandBuffer);
    vkEndCommandBuffer(CommandBuffer);
    return CommandBuffer;
}

void FrameBufferRenderPass::Destroy()
{
    renderer.DestroyRenderPass(RenderPass);
    renderer.DestroyCommandBuffers(CommandBuffer);
    renderer.DestroyFrameBuffers(FrameBufferList);
    renderer.DestroyPipeline(ShaderPipeline);
    renderer.DestroyPipelineLayout(ShaderPipelineLayout);
    renderer.DestroyPipelineCache(PipelineCache);
    renderer.DestroyDescriptorSetLayout(DescriptorSetLayout);
    renderer.DestroyDescriptorPool(DescriptorPool);
}
