#include "VulkanRenderPass.h"

void RenderPass_BuildRenderPass(VkDevice device, VkRenderPass& renderPass, const RenderPassBuildInfoModel& renderPassBuildInfo, Vector<SharedPtr<RenderedTexture>>& renderedColorTextureList, SharedPtr<DepthTexture>& depthTexture)
{
    Vector<VkAttachmentDescription> attachmentDescriptionList = Vector<VkAttachmentDescription>();
    Vector<VkAttachmentReference> inputAttachmentReferenceList = Vector<VkAttachmentReference>();
    Vector<VkAttachmentReference> colorAttachmentReferenceList = Vector<VkAttachmentReference>();
    Vector<VkAttachmentReference> resolveAttachmentReferenceList = Vector<VkAttachmentReference>();
    Vector<VkSubpassDescription> preserveAttachmentReferenceList = Vector<VkSubpassDescription>();
    Vector<VkAttachmentReference> depthReference = Vector<VkAttachmentReference>();

    for (RenderedTextureInfoModel renderedTextureInfoModel : renderPassBuildInfo.RenderedTextureInfoModelList)
    {
        attachmentDescriptionList.emplace_back(renderedTextureInfoModel.AttachmentDescription);
        switch (renderedTextureInfoModel.TextureType)
        {
        case RenderedTextureType::ColorRenderedTexture:
        {
            renderedColorTextureList.emplace_back(std::make_shared<RenderedTexture>(RenderedTexture(VK_IMAGE_ASPECT_COLOR_BIT, renderedTextureInfoModel.ImageCreateInfo, renderedTextureInfoModel.SamplerCreateInfo)));
            colorAttachmentReferenceList.emplace_back(VkAttachmentReference
                {
                    .attachment = static_cast<uint32>(colorAttachmentReferenceList.size()),
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                });
            break;
        }
        case RenderedTextureType::DepthRenderedTexture:
        {
            depthTexture = std::make_shared<DepthTexture>(DepthTexture(renderedTextureInfoModel.ImageCreateInfo, renderedTextureInfoModel.SamplerCreateInfo));
            depthReference.emplace_back(VkAttachmentReference
                {
                    .attachment = (uint)(colorAttachmentReferenceList.size() + resolveAttachmentReferenceList.size()),
                    .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
                });
            break;
        }
        case RenderedTextureType::InputAttachmentTexture:
        {
            renderedColorTextureList.emplace_back(std::make_shared<RenderedTexture>(RenderedTexture(VK_IMAGE_ASPECT_COLOR_BIT, renderedTextureInfoModel.ImageCreateInfo, renderedTextureInfoModel.SamplerCreateInfo)));
            inputAttachmentReferenceList.emplace_back(VkAttachmentReference
                {
                    .attachment = static_cast<uint32>(inputAttachmentReferenceList.size()),
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                });
            break;
        }
        case RenderedTextureType::ResolveAttachmentTexture:
        {
            renderedColorTextureList.emplace_back(std::make_shared<RenderedTexture>(RenderedTexture(VK_IMAGE_ASPECT_COLOR_BIT, renderedTextureInfoModel.ImageCreateInfo, renderedTextureInfoModel.SamplerCreateInfo)));
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
    for (VkSubpassDependency subpass : renderPassBuildInfo.SubpassDependencyModelList)
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
    VULKAN_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}

void RenderPass_BuildFrameBuffer(VkDevice device, VkRenderPass renderPass, const RenderPassBuildInfoModel& renderPassBuildInfo, Vector<VkFramebuffer>& frameBufferList, Vector<VkImageView>& imageViewList, SharedPtr<VkImageView> depthImageView, Vector<VkImageView>& swapChainImageViews, ivec2 renderPassResolution)
{
    for (size_t x = 0; x < swapChainImageViews.size(); x++)
    {
        std::vector<VkImageView> TextureAttachmentList;
        for (int y = 0; y < imageViewList.size(); y++)
        {
            if (renderPassBuildInfo.IsRenderedToSwapchain)
            {
                TextureAttachmentList.emplace_back(swapChainImageViews[x]);
            }
            else
            {
                TextureAttachmentList.emplace_back(imageViewList[y]);
            }
        }
        if (depthImageView)
        {
            TextureAttachmentList.emplace_back(*depthImageView.get());
        }

        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = static_cast<uint32_t>(TextureAttachmentList.size()),
            .pAttachments = TextureAttachmentList.data(),
            .width = static_cast<uint32_t>(renderPassResolution.x),
            .height = static_cast<uint32_t>(renderPassResolution.y),
            .layers = 1,
        };
        VULKAN_RESULT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &frameBufferList[x]));
    }
}