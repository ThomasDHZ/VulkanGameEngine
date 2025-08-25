#include "VulkanRenderPass.h"
#include "../VulkanGameEngine/SceneDataBuffer.h"

VulkanRenderPass VulkanRenderPass_CreateVulkanRenderPass(GraphicsRenderer& renderer, const char* renderPassJsonFilePath, ivec2& renderPassResolution, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture)
{
    const char* jsonDataString = File_Read(renderPassJsonFilePath).Data;
    RenderPassLoader renderPassLoader = nlohmann::json::parse(jsonDataString).get<RenderPassLoader>();
    if (renderPassLoader.RenderArea.UseDefaultRenderArea)
    {
        renderPassLoader.RenderArea.RenderArea.extent.width = renderPassResolution.x;
        renderPassLoader.RenderArea.RenderArea.extent.height = renderPassResolution.y;
        for (auto& renderTexture : renderPassLoader.RenderedTextureInfoModelList)
        {
            renderTexture.ImageCreateInfo.extent.width = renderPassResolution.x;
            renderTexture.ImageCreateInfo.extent.height = renderPassResolution.y;
            renderTexture.ImageCreateInfo.extent.depth = 1;
        }
    }

    VulkanRenderPass* vulkanRenderPassPtr = new VulkanRenderPass
    {
        .RenderPassId = renderPassLoader.RenderPassId,
        .SampleCount = VK_SAMPLE_COUNT_1_BIT,
        .RenderArea = renderPassLoader.RenderArea.RenderArea,
        .InputTextureIdListCount = renderPassLoader.InputTextureList.size(),
        .ClearValueCount = renderPassLoader.ClearValueList.size(),
        .CommandBuffer = VK_NULL_HANDLE,
        .RenderPassResolution = renderPassResolution,
        .IsRenderedToSwapchain = renderPassLoader.IsRenderedToSwapchain
    };

    Vector<Texture> renderedTextureList;
    vulkanRenderPassPtr->RenderPass = RenderPass_BuildRenderPass(renderer, renderPassLoader, renderedTextureList, depthTexture);
    RenderPass_BuildRenderPassAttachments(renderer, renderPassLoader, renderedTextureList, depthTexture);
    Vector<VkFramebuffer> frameBufferList = RenderPass_BuildFrameBuffer(renderer, *vulkanRenderPassPtr, renderedTextureList, depthTexture);
    RenderPass_CreateCommandBuffers(renderer, &vulkanRenderPassPtr->CommandBuffer, 1);

    vulkanRenderPassPtr->InputTextureIdList = nullptr;
    if (renderPassLoader.InputTextureList.size() > 0)
    {
        vulkanRenderPassPtr->InputTextureIdList = memorySystem.AddPtrBuffer<VkGuid>(renderPassLoader.InputTextureList.size(), __FILE__, __LINE__, __func__);
        std::memcpy(vulkanRenderPassPtr->InputTextureIdList, renderPassLoader.InputTextureList.data(), renderPassLoader.InputTextureList.size() * sizeof(VkGuid));
    }

    vulkanRenderPassPtr->ClearValueList = nullptr;
    if (renderPassLoader.ClearValueList.size() > 0)
    {
        vulkanRenderPassPtr->ClearValueList = memorySystem.AddPtrBuffer<VkClearValue>(renderPassLoader.ClearValueList.size(), __FILE__, __LINE__, __func__);
        std::memcpy(vulkanRenderPassPtr->ClearValueList, renderPassLoader.ClearValueList.data(), renderPassLoader.ClearValueList.size() * sizeof(VkClearValue));
    }

    vulkanRenderPassPtr->FrameBufferList = nullptr;
    if (frameBufferList.size() > 0)
    {
        vulkanRenderPassPtr->FrameBufferCount = frameBufferList.size();
        vulkanRenderPassPtr->FrameBufferList = memorySystem.AddPtrBuffer<VkFramebuffer>(frameBufferList.size(), __FILE__, __LINE__, __func__);
        std::memcpy(vulkanRenderPassPtr->FrameBufferList, frameBufferList.data(), frameBufferList.size() * sizeof(VkFramebuffer));
    }

    renderedTextureCount = renderedTextureList.size();
    renderedTextureListPtr = *renderedTextureList.data();

    VulkanRenderPass vulkanRenderPass = *vulkanRenderPassPtr;
    delete vulkanRenderPassPtr;
    return vulkanRenderPass;
}

VulkanRenderPass VulkanRenderPass_RebuildSwapChain(GraphicsRenderer& renderer, VulkanRenderPass& vulkanRenderPass, const char* renderPassJsonFilePath, ivec2& renderPassResolution, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture)
{
    const char* jsonDataString = File_Read(renderPassJsonFilePath).Data;
    RenderPassLoader renderPassLoader = nlohmann::json::parse(jsonDataString).get<RenderPassLoader>();

    renderPassLoader.RenderArea.RenderArea.extent.width = renderPassResolution.x;
    renderPassLoader.RenderArea.RenderArea.extent.height = renderPassResolution.y;
    for (auto& renderTexture : renderPassLoader.RenderedTextureInfoModelList)
    {
        renderTexture.ImageCreateInfo.extent.width = renderPassResolution.x;
        renderTexture.ImageCreateInfo.extent.height = renderPassResolution.y;
        renderTexture.ImageCreateInfo.extent.depth = 1;
    }

    Vector<Texture> renderedTextureList;
    Vector<VkFramebuffer> frameBufferList;
    Renderer_DestroyFrameBuffers(renderer.Device, vulkanRenderPass.FrameBufferList, renderer.SwapChainImageCount);    
    if(vulkanRenderPass.IsRenderedToSwapchain)
    { 
        vulkanRenderPass.RenderPassResolution = ivec2(renderer.SwapChainResolution.width, renderer.SwapChainResolution.height);
        renderedTextureList = Vector<Texture>(&renderedTextureListPtr, &renderedTextureListPtr + renderedTextureCount);

        VulkanRenderPass_DestoryRenderPassSwapChainTextures(renderer, renderedTextureListPtr, renderedTextureCount, depthTexture);
        Renderer_DestroyRenderPass(renderer.Device, &vulkanRenderPass.RenderPass);
        renderedTextureList.clear();

        vulkanRenderPass.RenderPass = RenderPass_BuildRenderPass(renderer, renderPassLoader, renderedTextureList, depthTexture);
        RenderPass_BuildRenderPassAttachments(renderer, renderPassLoader, renderedTextureList, depthTexture);
        frameBufferList = RenderPass_BuildFrameBuffer(renderer, vulkanRenderPass, renderedTextureList, depthTexture);
    }
    else
    {
        renderedTextureList = Vector<Texture>(&renderedTextureListPtr, &renderedTextureListPtr + renderedTextureCount);
        frameBufferList = RenderPass_BuildFrameBuffer(renderer, vulkanRenderPass, renderedTextureList, depthTexture);
    }

    vulkanRenderPass.InputTextureIdList = nullptr;
    if (renderPassLoader.InputTextureList.size() > 0)
    {
        vulkanRenderPass.InputTextureIdList = memorySystem.AddPtrBuffer<VkGuid>(renderPassLoader.InputTextureList.size(), __FILE__, __LINE__, __func__);
        std::memcpy(vulkanRenderPass.InputTextureIdList, renderPassLoader.InputTextureList.data(), renderPassLoader.InputTextureList.size() * sizeof(VkGuid));
    }

    vulkanRenderPass.ClearValueList = nullptr;
    if (renderPassLoader.ClearValueList.size() > 0)
    {
        vulkanRenderPass.ClearValueList = memorySystem.AddPtrBuffer<VkClearValue>(renderPassLoader.ClearValueList.size(), __FILE__, __LINE__, __func__);
        std::memcpy(vulkanRenderPass.ClearValueList, renderPassLoader.ClearValueList.data(), renderPassLoader.ClearValueList.size() * sizeof(VkClearValue));
    }

    vulkanRenderPass.FrameBufferList = nullptr;
    if (frameBufferList.size() > 0)
    {
        vulkanRenderPass.FrameBufferList = memorySystem.AddPtrBuffer<VkFramebuffer>(frameBufferList.size(), __FILE__, __LINE__, __func__);
        std::memcpy(vulkanRenderPass.FrameBufferList, frameBufferList.data(), frameBufferList.size() * sizeof(VkFramebuffer));
    }

    renderedTextureCount = renderedTextureList.size();
    renderedTextureListPtr = *renderedTextureList.data();
    return vulkanRenderPass;
}

void VulkanRenderPass_DestoryRenderPassSwapChainTextures(GraphicsRenderer& renderer, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture)
{
    Vector<Texture> renderedTextureList = Vector<Texture>(&renderedTextureListPtr, &renderedTextureListPtr + renderedTextureCount);
    for (auto& renderedTexture : renderedTextureList)
    {
        Texture_DestroyTexture(renderer, renderedTexture);
    }
    std::memset(static_cast<void*>(&renderedTextureListPtr), 0x00, sizeof(Texture) * renderedTextureCount);
    renderedTextureCount = 0;
    renderedTextureList.clear();
}

void VulkanRenderPass_DestroyRenderPass(GraphicsRenderer& renderer, VulkanRenderPass& renderPass)
{
    Renderer_DestroyRenderPass(renderer.Device, &renderPass.RenderPass);
    Renderer_DestroyCommandBuffers(renderer.Device, &renderer.CommandPool, &renderPass.CommandBuffer, 1);
    Renderer_DestroyFrameBuffers(renderer.Device, &renderPass.FrameBufferList[0], renderer.SwapChainImageCount);

    memorySystem.RemovePtrBuffer<VkGuid>(renderPass.InputTextureIdList);
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
    renderPass.IsRenderedToSwapchain = false;
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
        .pDependencies = subPassList.data(),
    };

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VULKAN_RESULT(vkCreateRenderPass(renderer.Device, &renderPassInfo, nullptr, &renderPass));
    return renderPass;
}

void RenderPass_BuildRenderPassAttachments(const GraphicsRenderer& renderer, const RenderPassLoader& renderPassJsonLoader, Vector<Texture>& renderedTextureList, Texture& depthTexture)
{
    for (auto& texture : renderPassJsonLoader.RenderedTextureInfoModelList)
    {
        VkGuid renderedTextureId = texture.RenderedTextureId;
        VkImageCreateInfo imageCreateInfo = texture.ImageCreateInfo;
        VkSamplerCreateInfo samplerCreateInfo = texture.SamplerCreateInfo;
        switch (texture.TextureType)
        {
            case ColorRenderedTexture: renderedTextureList.emplace_back(Texture_CreateTexture(renderer, renderedTextureId, VK_IMAGE_ASPECT_COLOR_BIT, imageCreateInfo, samplerCreateInfo)); break;
            case InputAttachmentTexture: renderedTextureList.emplace_back(Texture_CreateTexture(renderer, renderedTextureId, VK_IMAGE_ASPECT_COLOR_BIT, imageCreateInfo, samplerCreateInfo)); break;
            case ResolveAttachmentTexture: renderedTextureList.emplace_back(Texture_CreateTexture(renderer, renderedTextureId, VK_IMAGE_ASPECT_COLOR_BIT, imageCreateInfo, samplerCreateInfo)); break;
            case DepthRenderedTexture: depthTexture = Texture_CreateTexture(renderer, renderedTextureId, VK_IMAGE_ASPECT_DEPTH_BIT, imageCreateInfo, samplerCreateInfo); break;
        };
    }
}

Vector<VkFramebuffer> RenderPass_BuildFrameBuffer(const GraphicsRenderer& renderer, VulkanRenderPass& renderPass, Vector<Texture>& renderedTextureList, Texture& depthTexture)
{
    Vector<VkFramebuffer> frameBufferList = Vector<VkFramebuffer>(renderer.SwapChainImageCount);
    for (size_t x = 0; x < renderer.SwapChainImageCount; x++)
    {
        std::vector<VkImageView> TextureAttachmentList;
        for (int y = 0; y < renderedTextureList.size(); y++)
        {
            if (renderPass.IsRenderedToSwapchain)
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
            .renderPass = renderPass.RenderPass,
            .attachmentCount = static_cast<uint32_t>(TextureAttachmentList.size()),
            .pAttachments = TextureAttachmentList.data(),
            .width = static_cast<uint32_t>(renderPass.RenderPassResolution.x),
            .height = static_cast<uint32_t>(renderPass.RenderPassResolution.y),
            .layers = 1,
        };
        VULKAN_RESULT(vkCreateFramebuffer(renderer.Device, &framebufferInfo, nullptr, &frameBufferList[x]));
    }

    return frameBufferList;
}