#include "VulkanRenderPass.h"

 VulkanRenderPassDLL* VulkanRenderPass_ConvertToVulkanRenderPassDLL(VulkanRenderPass& renderPass)
 {
     VulkanRenderPassDLL* vulkanRenderPassDLL = new VulkanRenderPassDLL();

     vulkanRenderPassDLL->RenderPassId = renderPass.RenderPassId;
     vulkanRenderPassDLL->SampleCount = renderPass.SampleCount;
     vulkanRenderPassDLL->RenderArea = renderPass.RenderArea;
     vulkanRenderPassDLL->RenderPass = renderPass.RenderPass;
     vulkanRenderPassDLL->CommandBuffer = renderPass.CommandBuffer;
     vulkanRenderPassDLL->UseFrameBufferResolution = renderPass.UseFrameBufferResolution;

     vulkanRenderPassDLL->FrameBufferCount = renderPass.FrameBufferList.size();
     if (vulkanRenderPassDLL->FrameBufferCount > 0)
     {
         vulkanRenderPassDLL->FrameBufferList = new VkFramebuffer[vulkanRenderPassDLL->FrameBufferCount];
         std::memcpy(vulkanRenderPassDLL->FrameBufferList, renderPass.FrameBufferList.data(), vulkanRenderPassDLL->FrameBufferCount * sizeof(VkFramebuffer));
     }
     else
     {
         vulkanRenderPassDLL->FrameBufferList = nullptr;
     }

     vulkanRenderPassDLL->ClearValueCount = renderPass.ClearValueList.size();
     if (vulkanRenderPassDLL->ClearValueCount > 0)
     {
         vulkanRenderPassDLL->ClearValueList = new VkClearValue[vulkanRenderPassDLL->ClearValueCount];
         std::memcpy(vulkanRenderPassDLL->ClearValueList, renderPass.ClearValueList.data(), vulkanRenderPassDLL->ClearValueCount * sizeof(VkClearValue));
     }
     else
     {
         vulkanRenderPassDLL->ClearValueList = nullptr;
     }

     return vulkanRenderPassDLL;
 }

 void VulkanRenderPass_DeleteVulkanRenderPassDLL(VulkanRenderPassDLL* vulkanRenderPassDLL)
 {
     if (vulkanRenderPassDLL)
     {
         delete[] vulkanRenderPassDLL->FrameBufferList;
         vulkanRenderPassDLL->FrameBufferList = nullptr;
         delete[] vulkanRenderPassDLL->ClearValueList;
         vulkanRenderPassDLL->ClearValueList = nullptr;
         delete vulkanRenderPassDLL;
     }
 }

 VulkanRenderPass VulkanRenderPass_ConvertToVulkanRenderPass(VulkanRenderPassDLL* renderPassDLL)
 {
     if (!renderPassDLL)
     {
         throw std::invalid_argument("Null VulkanRenderPassDLL pointer");
     }

     VulkanRenderPass vulkanRenderPass;
     vulkanRenderPass.RenderPassId = renderPassDLL->RenderPassId;
     vulkanRenderPass.SampleCount = renderPassDLL->SampleCount;
     vulkanRenderPass.RenderArea = renderPassDLL->RenderArea;
     vulkanRenderPass.RenderPass = renderPassDLL->RenderPass;
     vulkanRenderPass.FrameBufferList = Vector<VkFramebuffer>(renderPassDLL->FrameBufferList, renderPassDLL->FrameBufferList + renderPassDLL->FrameBufferCount);
     vulkanRenderPass.ClearValueList = Vector<VkClearValue>(renderPassDLL->ClearValueList, renderPassDLL->ClearValueList + renderPassDLL->ClearValueCount);
     vulkanRenderPass.CommandBuffer = renderPassDLL->CommandBuffer;
     vulkanRenderPass.UseFrameBufferResolution = renderPassDLL->UseFrameBufferResolution;

     VulkanRenderPass_DeleteVulkanRenderPassDLL(renderPassDLL);
     return vulkanRenderPass;
 }

 VulkanRenderPassDLL* VulkanRenderPass_CreateVulkanRenderPassCS(const RendererStateCS& renderStateCS, const char* renderPassLoader, ivec2& renderPassResolution, int ConstBuffer, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture)
 {
     RendererState renderState = Renderer_RendererStateCStoCPP(renderStateCS);
     return VulkanRenderPass_CreateVulkanRenderPass(renderState, renderPassLoader, renderPassResolution,  ConstBuffer, renderedTextureListPtr, renderedTextureCount, depthTexture);
 }

 VulkanRenderPassDLL* VulkanRenderPass_CreateVulkanRenderPass(const RendererState& renderState, const char* renderPassLoaderJson, ivec2& renderPassResolution, int ConstBuffer, Texture& renderedTextureListPtr,  size_t& renderedTextureCount, Texture& depthTexture)
{
    Vector<Texture> renderedTextureList;
    RenderPassLoader renderPassLoader = JsonLoader_LoadRenderPassLoaderInfo(renderPassLoaderJson, renderPassResolution);

    VulkanRenderPass vulkanRenderPass;
    vulkanRenderPass.RenderPassId = renderPassLoader.RenderPassId;
    vulkanRenderPass.SampleCount = VK_SAMPLE_COUNT_1_BIT;
    vulkanRenderPass.RenderPass = RenderPass_BuildRenderPass(renderState, vulkanRenderPass, renderPassLoader, renderedTextureList, depthTexture);
    vulkanRenderPass.FrameBufferList = RenderPass_BuildFrameBuffer(renderState, vulkanRenderPass, renderPassLoader, renderedTextureList, depthTexture, renderPassResolution);
    vulkanRenderPass.ClearValueList = renderPassLoader.ClearValueList;
    vulkanRenderPass.RenderArea = renderPassLoader.RenderArea.RenderArea;
    RenderPass_CreateCommandBuffers(renderState, &vulkanRenderPass.CommandBuffer, 1);

    renderedTextureCount = renderedTextureList.size();
    renderedTextureListPtr = *renderedTextureList.data();
    return VulkanRenderPass_ConvertToVulkanRenderPassDLL(vulkanRenderPass);
}

void VulkanRenderPass_DestroyRenderPass(RendererState& renderState, VulkanRenderPassDLL& renderPass)
{
    Renderer_DestroyRenderPass(renderState.Device, &renderPass.RenderPass);
    Renderer_DestroyCommandBuffers(renderState.Device, &renderState.CommandPool, &renderPass.CommandBuffer, 1);
    Renderer_DestroyFrameBuffers(renderState.Device, &renderPass.FrameBufferList[0], renderState.SwapChainImageCount);

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

VkResult RenderPass_CreateCommandBuffers(const RendererState& renderState, VkCommandBuffer* commandBufferList, uint32 commandBufferCount)
{
    for (size_t x = 0; x < commandBufferCount; x++)
    {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = renderState.CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VULKAN_RESULT(vkAllocateCommandBuffers(renderState.Device, &commandBufferAllocateInfo, &commandBufferList[x]));
    }
    return VK_SUCCESS;
}

VkRenderPass RenderPass_BuildRenderPass(const RendererState& renderState, VulkanRenderPass& vulkanRenderPass, const RenderPassLoader& renderPassJsonLoader, Vector<Texture>& renderedTextureList, Texture& depthTexture)
{
    for (auto& texture : renderPassJsonLoader.RenderedTextureInfoModelList)
    {
        VkImageCreateInfo imageCreateInfo = texture.ImageCreateInfo;
        VkSamplerCreateInfo samplerCreateInfo = texture.SamplerCreateInfo;
        switch (texture.TextureType)
        {
            case ColorRenderedTexture: renderedTextureList.emplace_back(Texture_CreateTexture(renderState, VK_IMAGE_ASPECT_COLOR_BIT, imageCreateInfo, samplerCreateInfo)); break;
            case InputAttachmentTexture: renderedTextureList.emplace_back(Texture_CreateTexture(renderState, VK_IMAGE_ASPECT_COLOR_BIT, imageCreateInfo, samplerCreateInfo)); break;
            case ResolveAttachmentTexture: renderedTextureList.emplace_back(Texture_CreateTexture(renderState, VK_IMAGE_ASPECT_COLOR_BIT, imageCreateInfo, samplerCreateInfo)); break;
            case DepthRenderedTexture: depthTexture = Texture_CreateTexture(renderState, VK_IMAGE_ASPECT_DEPTH_BIT, imageCreateInfo, samplerCreateInfo); break;
        };
    }

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
    VULKAN_RESULT(vkCreateRenderPass(renderState.Device, &renderPassInfo, nullptr, &renderPass));
    return renderPass;
}

Vector<VkFramebuffer> RenderPass_BuildFrameBuffer(const RendererState& renderState, const VulkanRenderPass& vulkanRenderPass, const RenderPassLoader& renderPassJsonLoader, Vector<Texture>& renderedTextureList, Texture& depthTexture, ivec2& renderPassResolution)
{
    Vector<VkFramebuffer> frameBufferList = Vector<VkFramebuffer>(renderState.SwapChainImageViews.size());
    for (size_t x = 0; x < renderState.SwapChainImageViews.size(); x++)
    {
        std::vector<VkImageView> TextureAttachmentList;
        for (int y = 0; y < renderedTextureList.size(); y++)
        {
            if (renderPassJsonLoader.IsRenderedToSwapchain)
            {
                TextureAttachmentList.emplace_back(renderState.SwapChainImageViews[x]);
            }
            else
            {
                TextureAttachmentList.emplace_back(renderedTextureList[y].textureView);
            }
        }
        if (depthTexture.textureView != VK_NULL_HANDLE)
        {
            TextureAttachmentList.emplace_back(depthTexture.textureView);
        }

        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = vulkanRenderPass.RenderPass,
            .attachmentCount = static_cast<uint32_t>(TextureAttachmentList.size()),
            .pAttachments = TextureAttachmentList.data(),
            .width = static_cast<uint32_t>(renderPassResolution.x),
            .height = static_cast<uint32_t>(renderPassResolution.y),
            .layers = 1,
        };
        VULKAN_RESULT(vkCreateFramebuffer(renderState.Device, &framebufferInfo, nullptr, &frameBufferList[x]));
    }
    return frameBufferList;
}