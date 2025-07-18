#include "VulkanRenderPass.h"

VulkanRenderPass VulkanRenderPass_CreateVulkanRenderPass(GraphicsRenderer& renderer, const char* renderPassLoaderJson, ivec2& renderPassResolution, int ConstBuffer, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture)
{
    const RenderPassLoader renderPassLoader = JsonLoader_LoadRenderPassLoaderInfo(renderPassLoaderJson, renderPassResolution);

    Vector<Texture> renderedTextureList;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkRenderPass renderPass = RenderPass_BuildRenderPass(renderer, renderPassLoader, renderedTextureList, depthTexture);
    RenderPass_BuildRenderPassAttachments(renderer, renderPassLoader, renderedTextureList, depthTexture);
    Vector<VkFramebuffer> frameBufferList = RenderPass_BuildFrameBuffer(renderer, renderPass, renderPassLoader, renderedTextureList, depthTexture, renderPassResolution);
    Vector<VkClearValue> clearValueList = renderPassLoader.ClearValueList;
    RenderPass_CreateCommandBuffers(renderer, &commandBuffer, 1);

    VulkanRenderPass* vulkanRenderPassPtr = new VulkanRenderPass
    {
        .RenderPassId = renderPassLoader.RenderPassId,
        .SampleCount = VK_SAMPLE_COUNT_1_BIT,
        .RenderArea = renderPassLoader.RenderArea.RenderArea,
        .RenderPass = renderPass,
        .FrameBufferCount = frameBufferList.size(),
        .ClearValueCount = clearValueList.size(),
        .CommandBuffer = commandBuffer,
        .UseFrameBufferResolution = renderPassLoader.IsRenderedToSwapchain
    };

    vulkanRenderPassPtr->FrameBufferList = nullptr;
    if (vulkanRenderPassPtr->FrameBufferCount > 0)
    {
        vulkanRenderPassPtr->FrameBufferList = memorySystem.AddPtrBuffer<VkFramebuffer>(frameBufferList.size(), __FILE__, __LINE__, __func__);
        std::memcpy(vulkanRenderPassPtr->FrameBufferList, frameBufferList.data(), vulkanRenderPassPtr->FrameBufferCount * sizeof(VkFramebuffer));
    }

    vulkanRenderPassPtr->ClearValueList = nullptr;
    if (vulkanRenderPassPtr->ClearValueCount > 0)
    {
        vulkanRenderPassPtr->ClearValueList = memorySystem.AddPtrBuffer<VkClearValue>(clearValueList.size(), __FILE__, __LINE__, __func__);
        std::memcpy(vulkanRenderPassPtr->ClearValueList, clearValueList.data(), vulkanRenderPassPtr->ClearValueCount * sizeof(VkClearValue));
    }

    renderedTextureCount = renderedTextureList.size();
    renderedTextureListPtr = *renderedTextureList.data();

    VulkanRenderPass vulkanRenderPass = *vulkanRenderPassPtr;
    delete vulkanRenderPassPtr;
    return vulkanRenderPass;
}

VulkanRenderPass VulkanRenderPass_RebuildSwapChain(GraphicsRenderer& renderer, VulkanRenderPass& oldVulkanRenderPass, RenderPassLoader renderPassLoader, ivec2& newSwapChainSize, Texture& renderedTextureListPtr, size_t renderedTextureCount, Texture& depthTexture)
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    Vector<Texture> renderedTextureList = Vector<Texture>(&renderedTextureListPtr, &renderedTextureListPtr + renderedTextureCount);

    Renderer_DestroyRenderPass(renderer.Device, &oldVulkanRenderPass.RenderPass);
    Renderer_DestroyCommandBuffers(renderer.Device, &renderer.CommandPool, &oldVulkanRenderPass.CommandBuffer, 1);
    Renderer_DestroyFrameBuffers(renderer.Device, &oldVulkanRenderPass.FrameBufferList[0], renderer.SwapChainImageCount);

    memorySystem.RemovePtrBuffer<VkFramebuffer>(oldVulkanRenderPass.FrameBufferList);
    memorySystem.RemovePtrBuffer<VkClearValue>(oldVulkanRenderPass.ClearValueList);

    VkRenderPass renderPass = RenderPass_BuildRenderPass(renderer, renderPassLoader, renderedTextureList, depthTexture);
    Vector<VkFramebuffer> frameBufferList = RenderPass_BuildFrameBuffer(renderer, renderPass, renderPassLoader, renderedTextureList, depthTexture, newSwapChainSize);
    Vector<VkClearValue> clearValueList = renderPassLoader.ClearValueList;
    RenderPass_CreateCommandBuffers(renderer, &commandBuffer, 1);

    VulkanRenderPass* vulkanRenderPassPtr = new VulkanRenderPass
    {
        .RenderPassId = renderPassLoader.RenderPassId,
        .SampleCount = VK_SAMPLE_COUNT_1_BIT,
        .RenderArea = renderPassLoader.RenderArea.RenderArea,
        .RenderPass = renderPass,
        .FrameBufferCount = frameBufferList.size(),
        .ClearValueCount = clearValueList.size(),
        .CommandBuffer = commandBuffer,
        .UseFrameBufferResolution = renderPassLoader.IsRenderedToSwapchain
    };

    vulkanRenderPassPtr->FrameBufferList = nullptr;
    if (vulkanRenderPassPtr->FrameBufferCount > 0)
    {
        vulkanRenderPassPtr->FrameBufferList = memorySystem.AddPtrBuffer<VkFramebuffer>(frameBufferList.size(), __FILE__, __LINE__, __func__);
        std::memcpy(vulkanRenderPassPtr->FrameBufferList, frameBufferList.data(), vulkanRenderPassPtr->FrameBufferCount * sizeof(VkFramebuffer));
    }

    vulkanRenderPassPtr->ClearValueList = nullptr;
    if (vulkanRenderPassPtr->ClearValueCount > 0)
    {
        vulkanRenderPassPtr->ClearValueList = memorySystem.AddPtrBuffer<VkClearValue>(clearValueList.size(), __FILE__, __LINE__, __func__);
        std::memcpy(vulkanRenderPassPtr->ClearValueList, clearValueList.data(), vulkanRenderPassPtr->ClearValueCount * sizeof(VkClearValue));
    }

    renderedTextureCount = renderedTextureList.size();
    renderedTextureListPtr = *renderedTextureList.data();

    VulkanRenderPass vulkanRenderPass = *vulkanRenderPassPtr;
    delete vulkanRenderPassPtr;
    return vulkanRenderPass;
}

void VulkanRenderPass_DestroyRenderPass(GraphicsRenderer& renderer, VulkanRenderPass& renderPass)
{
    Renderer_DestroyRenderPass(renderer.Device, &renderPass.RenderPass);
    Renderer_DestroyCommandBuffers(renderer.Device, &renderer.CommandPool, &renderPass.CommandBuffer, 1);
    Renderer_DestroyFrameBuffers(renderer.Device, &renderPass.FrameBufferList[0], renderer.SwapChainImageCount);

    memorySystem.RemovePtrBuffer<VkFramebuffer>(renderPass.FrameBufferList);
    memorySystem.RemovePtrBuffer<VkClearValue>(renderPass.ClearValueList);

    renderPass.RenderPassId = VkGuid();
    renderPass.SampleCount = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
    renderPass.RenderArea = VkRect2D();
    renderPass.RenderPass = VK_NULL_HANDLE;
    renderPass.FrameBufferList = nullptr;
    renderPass.ClearValueList = nullptr;
    renderPass.FrameBufferCount = 0;
    renderPass.ClearValueCount = 0;
    renderPass.CommandBuffer = VK_NULL_HANDLE;
    renderPass.UseFrameBufferResolution = false;
}

VkResult RenderPass_CreateCommandBuffers(const GraphicsRenderer& renderer, VkCommandBuffer* commandBufferList, size_t commandBufferCount)
{
    for (size_t x = 0; x < commandBufferCount; x++)
    {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = renderer.CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32>(commandBufferCount)
        };

        VULKAN_RESULT(vkAllocateCommandBuffers(renderer.Device, &commandBufferAllocateInfo, &commandBufferList[x]));
    }
    return VK_SUCCESS;
}

VkRenderPass RenderPass_BuildRenderPass(const GraphicsRenderer& renderer, const RenderPassLoader& renderPassJsonLoader, Vector<Texture>& renderedTextureList, Texture& depthTexture)
{
    Vector<VkAttachmentDescription> attachmentDescriptionList = Vector<VkAttachmentDescription>();
    Vector<VkAttachmentReference> inputAttachmentReferenceList = Vector<VkAttachmentReference>();
    Vector<VkAttachmentReference> colorAttachmentReferenceList = Vector<VkAttachmentReference>();
    Vector<VkAttachmentReference> resolveAttachmentReferenceList = Vector<VkAttachmentReference>();
    Vector<VkSubpassDescription> preserveAttachmentReferenceList = Vector<VkSubpassDescription>();
    Vector<VkAttachmentReference> depthReference = Vector<VkAttachmentReference>();
    for (RenderedTextureLoader renderedTextureInfoModel : renderPassJsonLoader.RenderedTextureInfoModelList)
    {
        attachmentDescriptionList.emplace_back(renderedTextureInfoModel.AttachmentDescription);
        switch (renderedTextureInfoModel.TextureType)
        {
        case RenderedTextureType::ColorRenderedTexture:
        {
            colorAttachmentReferenceList.emplace_back(VkAttachmentReference
                {
                    .attachment = static_cast<uint32>(colorAttachmentReferenceList.size()),
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                });
            break;
        }
        case RenderedTextureType::InputAttachmentTexture:
        {
            inputAttachmentReferenceList.emplace_back(VkAttachmentReference
                {
                    .attachment = static_cast<uint32>(inputAttachmentReferenceList.size()),
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                });
            break;
        }
        case RenderedTextureType::ResolveAttachmentTexture:
        {
            resolveAttachmentReferenceList.emplace_back(VkAttachmentReference
                {
                    .attachment = static_cast<uint32>(colorAttachmentReferenceList.size() + 1),
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                });
            break;
        }
        default:
        {
            throw std::runtime_error("Case doesn't exist: RenderedTextureType");
        }
        case RenderedTextureType::DepthRenderedTexture:
        {
            depthReference.emplace_back(VkAttachmentReference
                {
                    .attachment = (uint)(colorAttachmentReferenceList.size() + resolveAttachmentReferenceList.size()),
                    .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
                });
            break;
        }
        }
    }

    Vector<VkSubpassDescription> subpassDescriptionList =
    {
        VkSubpassDescription
        {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = static_cast<uint32>(inputAttachmentReferenceList.size()),
            .pInputAttachments = inputAttachmentReferenceList.data(),
            .colorAttachmentCount = static_cast<uint32>(colorAttachmentReferenceList.size()),
            .pColorAttachments = colorAttachmentReferenceList.data(),
            .pResolveAttachments = resolveAttachmentReferenceList.data(),
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = static_cast<uint32>(inputAttachmentReferenceList.size()),
            .pPreserveAttachments = nullptr,
        }
    };
    if (depthReference.size() > 0)
    {
        subpassDescriptionList[0].pDepthStencilAttachment = &depthReference[0];
    }

    Vector<VkSubpassDependency> subPassList = Vector<VkSubpassDependency>();
    for (VkSubpassDependency subpass : renderPassJsonLoader.SubpassDependencyModelList)
    {
        subPassList.emplace_back(subpass);
    }

    VkRenderPassCreateInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32>(attachmentDescriptionList.size()),
        .pAttachments = attachmentDescriptionList.data(),
        .subpassCount = static_cast<uint32>(subpassDescriptionList.size()),
        .pSubpasses = subpassDescriptionList.data(),
        .dependencyCount = static_cast<uint32>(subPassList.size()),
        .pDependencies = subPassList.data()
    };

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VULKAN_RESULT(vkCreateRenderPass(renderer.Device, &renderPassInfo, nullptr, &renderPass));
    return renderPass;
}

void RenderPass_BuildRenderPassAttachments(const GraphicsRenderer& renderer, const RenderPassLoader& renderPassJsonLoader, Vector<Texture>& renderedTextureList, Texture& depthTexture)
{
    for (auto& texture : renderPassJsonLoader.RenderedTextureInfoModelList)
    {
        VkImageCreateInfo imageCreateInfo = texture.ImageCreateInfo;
        VkSamplerCreateInfo samplerCreateInfo = texture.SamplerCreateInfo;
        switch (texture.TextureType)
        {
            case ColorRenderedTexture: renderedTextureList.emplace_back(Texture_CreateTexture(renderer, VK_IMAGE_ASPECT_COLOR_BIT, imageCreateInfo, samplerCreateInfo)); break;
            case InputAttachmentTexture: renderedTextureList.emplace_back(Texture_CreateTexture(renderer, VK_IMAGE_ASPECT_COLOR_BIT, imageCreateInfo, samplerCreateInfo)); break;
            case ResolveAttachmentTexture: renderedTextureList.emplace_back(Texture_CreateTexture(renderer, VK_IMAGE_ASPECT_COLOR_BIT, imageCreateInfo, samplerCreateInfo)); break;
            case DepthRenderedTexture: depthTexture = Texture_CreateTexture(renderer, VK_IMAGE_ASPECT_DEPTH_BIT, imageCreateInfo, samplerCreateInfo); break;
        };
    }
}

Vector<VkFramebuffer> RenderPass_BuildFrameBuffer(const GraphicsRenderer& renderer, const VkRenderPass& renderPass, const RenderPassLoader& renderPassBuildInfo, Vector<Texture>& renderedTextureList, Texture& depthTexture, ivec2& renderPassResolution)
{
    Vector<VkFramebuffer> frameBufferList = Vector<VkFramebuffer>(renderer.SwapChainImageCount);
    for (size_t x = 0; x < renderer.SwapChainImageCount; x++)
    {
        std::vector<VkImageView> TextureAttachmentList;
        for (int y = 0; y < renderedTextureList.size(); y++)
        {
            if (renderPassBuildInfo.IsRenderedToSwapchain)
            {
                TextureAttachmentList.emplace_back(renderer.SwapChainImageViews[x]);
            }
            else
            {
                TextureAttachmentList.emplace_back(renderedTextureList[y].textureView);
            }
        }
        if (depthTexture.textureMemory != VK_NULL_HANDLE)
        {
            TextureAttachmentList.emplace_back(depthTexture.textureView);
        }

        VkFramebufferCreateInfo framebufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = static_cast<uint32_t>(TextureAttachmentList.size()),
            .pAttachments = TextureAttachmentList.data(),
            .width = static_cast<uint32_t>(renderPassResolution.x),
            .height = static_cast<uint32_t>(renderPassResolution.y),
            .layers = 1,
        };
        VULKAN_RESULT(vkCreateFramebuffer(renderer.Device, &framebufferInfo, nullptr, &frameBufferList[x]));
    }
    return frameBufferList;
}