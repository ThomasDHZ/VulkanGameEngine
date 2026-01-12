#include "RenderSystem.h"
#include <vulkan/vulkan.h>
#include <iostream>
#include "MaterialSystem.h"
#include "MeshSystem.h"
#include "BufferSystem.h"
#include "LightSystem.h"
#include "RenderSystem.h"
#include "from_json.h"
#include <unordered_set>

RenderSystem& renderSystem = RenderSystem::Get();

void RenderSystem::StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface)
{
    vulkanSystem.RendererSetUp(windowHandle, instance, surface);
}

void RenderSystem::Update(void* windowHandle, LevelGuid& levelGuid, const float& deltaTime)
{
    if (vulkanSystem.RebuildRendererFlag)
    {
        //  RecreateSwapchain(windowHandle, spriteRenderPass2DGuid, levelGuid, deltaTime);
        vulkanSystem.RebuildRendererFlag = false;
    }
}

void RenderSystem::GenerateTexture(VkGuid& renderPassId)
{
    const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
    VulkanPipeline pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
    Vector<Texture> renderPassTexture = textureSystem.FindRenderedTextureList(renderPassId);

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo allocInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vulkanSystem.CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };
    VULKAN_THROW_IF_FAIL(vkAllocateCommandBuffers(vulkanSystem.Device, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo =
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderPass.RenderPassResolution.x),
        .height = static_cast<float>(renderPass.RenderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList[vulkanSystem.ImageIndex],
        .renderArea = VkRect2D
        {
           .offset = VkOffset2D {.x = 0, .y = 0 },
           .extent = VkExtent2D {.width = static_cast<uint>(renderPass.RenderPassResolution.x), .height = static_cast<uint>(renderPass.RenderPassResolution.y) }
        },
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };

    VkSubmitInfo submitInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    VkFenceCreateInfo fenceCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = 0
    };

    VkFence fence = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &renderPassBeginInfo.renderArea);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetList.size(), pipeline.DescriptorSetList.data(), 0, nullptr);
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    VULKAN_THROW_IF_FAIL(vkEndCommandBuffer(commandBuffer));
    VULKAN_THROW_IF_FAIL(vkCreateFence(vulkanSystem.Device, &fenceCreateInfo, nullptr, &fence));
    VULKAN_THROW_IF_FAIL(vkQueueSubmit(vulkanSystem.GraphicsQueue, 1, &submitInfo, fence));
    VULKAN_THROW_IF_FAIL(vkWaitForFences(vulkanSystem.Device, 1, &fence, VK_TRUE, UINT64_MAX));
    vkDestroyFence(vulkanSystem.Device, fence, nullptr);
}

RenderPassGuid RenderSystem::LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath, ivec2 renderPassResolution)
{
    RenderPassLoader renderPassLoader = fileSystem.LoadJsonFile(jsonPath).get<RenderPassLoader>();
    renderSystem.RenderPassLoaderJsonMap[renderPassLoader.RenderPassId] = jsonPath;

    VulkanRenderPass vulkanRenderPass = VulkanRenderPass
    {
        .RenderPassId = renderPassLoader.RenderPassId,
        .SampleCount = renderPassLoader.RenderAttachmentList[0].SampleCount >= vulkanSystem.MaxSampleCount ? vulkanSystem.MaxSampleCount : renderPassLoader.RenderAttachmentList[0].SampleCount,
        .InputTextureIdList = renderPassLoader.InputTextureList,
        .ClearValueList = renderPassLoader.ClearValueList,
        .RenderPassResolution = renderPassResolution,
        .IsRenderedToSwapchain = renderPassLoader.IsRenderedToSwapchain
    };
    BuildRenderPass(vulkanRenderPass, renderPassLoader);
    RenderPassMap[vulkanRenderPass.RenderPassId] = vulkanRenderPass;

    for (auto& renderPipeline : renderPassLoader.RenderPipelineList)
    {
        nlohmann::json pipelineJson = fileSystem.LoadJsonFile(renderPipeline.c_str());
        RenderPipelineLoader renderPipelineLoader = pipelineJson.get<RenderPipelineLoader>();
        renderPipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = vulkanRenderPass.SampleCount;
        renderPipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = vulkanRenderPass.SampleCount;
        renderPipelineLoader.RenderPassId = vulkanRenderPass.RenderPassId;
        renderPipelineLoader.RenderPass = vulkanRenderPass.RenderPass;
        renderPipelineLoader.RenderPassResolution = vulkanRenderPass.RenderPassResolution;
        renderPipelineLoader.ShaderPiplineInfo = shaderSystem.LoadPipelineShaderData(Vector<String> { pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] });

        VkPipelineCache pipelineCache = VK_NULL_HANDLE;
        PipelineBindingData(renderPipelineLoader);
        VkDescriptorPool descriptorPool = CreatePipelineDescriptorPool(renderPipelineLoader);
        Vector<VkDescriptorSetLayout> descriptorSetLayoutList = CreatePipelineDescriptorSetLayout(renderPipelineLoader);
        Vector<VkDescriptorSet> descriptorSetList = AllocatePipelineDescriptorSets(renderPipelineLoader, descriptorPool, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
        UpdatePipelineDescriptorSets(renderPipelineLoader, descriptorSetList.data(), descriptorSetList.size());
        VkPipelineLayout pipelineLayout = CreatePipelineLayout(renderPipelineLoader, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
        VkPipeline pipeline = CreatePipeline(renderPipelineLoader, pipelineCache, pipelineLayout, descriptorSetList.data(), descriptorSetList.size());

        renderSystem.RenderPipelineMap[renderPassLoader.RenderPassId].emplace_back(VulkanPipeline
            {
                .RenderPipelineId = renderPipelineLoader.PipelineId,
                .DescriptorPool = descriptorPool,
                .DescriptorSetLayoutList = descriptorSetLayoutList,
                .DescriptorSetList = descriptorSetList,
                .Pipeline = pipeline,
                .PipelineLayout = pipelineLayout,
                .PipelineCache = pipelineCache
            });
    }
    return renderPassLoader.RenderPassId;
}

void RenderSystem::RecreateSwapchain(void* windowHandle, RenderPassGuid& spriteRenderPass2DGuid, LevelGuid& levelGuid, const float& deltaTime)
{
    vkDeviceWaitIdle(vulkanSystem.Device);
    vulkanSystem.RebuildSwapChain(windowHandle);
    for (auto& renderPassPair : renderSystem.RenderPassMap)
    {
        VulkanRenderPass& renderPass = renderPassPair.second;
        ivec2 swapChainResolution = ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height);
        String renderPassJsonLoader = renderSystem.RenderPassLoaderJsonMap[renderPass.RenderPassId];
        Vector<Texture>& renderedTextureList = textureSystem.FindRenderedTextureList(renderPass.RenderPassId);

        Texture depthTexture = Texture();
        if (textureSystem.DepthTextureExists(renderPass.RenderPassId))
        {
            depthTexture = textureSystem.FindDepthTexture(renderPass.RenderPassId);
        }

        size_t size = renderedTextureList.size();
        renderPass = RebuildSwapChain(renderPass, renderSystem.RenderPassLoaderJsonMap[renderPass.RenderPassId].c_str(), swapChainResolution, *renderedTextureList.data(), size, depthTexture);
    }
    // ImGui_RebuildSwapChain(renderer, imGuiRenderer);
}

VulkanRenderPass RenderSystem::RebuildSwapChain(VulkanRenderPass& vulkanRenderPass, const char* renderPassJsonFilePath, ivec2& renderPassResolution, Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture)
{
    RenderPassLoader renderPassLoader = fileSystem.LoadJsonFile(renderPassJsonFilePath).get<RenderPassLoader>();

    Vector<Texture> renderedTextureList;
    Vector<VkFramebuffer> frameBufferList;
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, vulkanRenderPass.FrameBufferList.data(), vulkanSystem.SwapChainImageCount);
    if (vulkanRenderPass.IsRenderedToSwapchain)
    {
        vulkanRenderPass.RenderPassResolution = ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height);
        renderedTextureList = Vector<Texture>(&renderedTextureListPtr, &renderedTextureListPtr + renderedTextureCount);

        DestoryRenderPassSwapChainTextures(renderedTextureListPtr, renderedTextureCount, depthTexture);
        vulkanSystem.DestroyRenderPass(vulkanSystem.Device, &vulkanRenderPass.RenderPass);
        renderedTextureList.clear();

        //  vulkanRenderPass.RenderPass = BuildRenderPass(renderPassLoader);
       //   frameBufferList = BuildFrameBuffer(vulkanRenderPass);
    }
    else
    {
        //   renderedTextureList = Vector<Texture>(&renderedTextureListPtr, &renderedTextureListPtr + renderedTextureCount);
        //   frameBufferList = BuildFrameBuffer(vulkanRenderPass);
    }

    vulkanRenderPass.InputTextureIdList = renderPassLoader.InputTextureList;
    vulkanRenderPass.ClearValueList = renderPassLoader.ClearValueList;
    vulkanRenderPass.FrameBufferList = frameBufferList;

    renderedTextureCount = renderedTextureList.size();
    renderedTextureListPtr = *renderedTextureList.data();
    return vulkanRenderPass;
}

VulkanRenderPass RenderSystem::FindRenderPass(const RenderPassGuid& renderPassGuid)
{
    return RenderPassMap.at(renderPassGuid);
}

const Vector<VulkanPipeline> RenderSystem::FindRenderPipelineList(const RenderPassGuid& renderPassGuid)
{
    return RenderPipelineMap.at(renderPassGuid);
}

void RenderSystem::BuildRenderPass(VulkanRenderPass& renderPass, const RenderPassLoader& renderPassJsonLoader)
{
    VkAttachmentReference depthReference = VkAttachmentReference();
    Vector<Vector<VkAttachmentReference>>   inputAttachmentReferenceList = Vector<Vector<VkAttachmentReference>>(renderPassJsonLoader.RenderPassCount);
    Vector<Vector<VkAttachmentReference>>   colorAttachmentReferenceList = Vector<Vector<VkAttachmentReference>>(renderPassJsonLoader.RenderPassCount);
    Vector<Vector<VkAttachmentReference>>   resolveAttachmentReferenceList = Vector<Vector<VkAttachmentReference>>(renderPassJsonLoader.RenderPassCount);
    Vector<Vector<VkSubpassDescription>>   preserveAttachmentReferenceList = Vector<Vector<VkSubpassDescription>>(renderPassJsonLoader.RenderPassCount);

    Vector<VkSubpassDescription> subPassDescriptionList = Vector<VkSubpassDescription>();
    Vector<VkAttachmentReference> depthReferences(renderPassJsonLoader.RenderPassCount);
    Vector<bool> useDepthReferences(renderPassJsonLoader.RenderPassCount, false);
    VkAttachmentReference unusedRef = {};

    for (int x = 0; x < renderPassJsonLoader.RenderPassCount; x++)
    {
        bool useDepthForThisSubpass = false;
        VkAttachmentReference depthRefForThisSubpass = {};

        for (int y = 0; y < renderPassJsonLoader.RenderAttachmentList.size(); y++)
        {
            RenderAttachmentLoader renderAttachment = renderPassJsonLoader.RenderAttachmentList[y];
            switch (renderAttachment.RenderAttachmentType[x])
            {
            case RenderAttachmentTypeEnum::ColorRenderedTexture: colorAttachmentReferenceList[x].emplace_back(VkAttachmentReference{ .attachment = static_cast<uint32>(y), .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }); break;
            case RenderAttachmentTypeEnum::InputAttachmentTexture: inputAttachmentReferenceList[x].emplace_back(VkAttachmentReference{ .attachment = static_cast<uint32>(y), .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }); break;
            case RenderAttachmentTypeEnum::ResolveAttachmentTexture: resolveAttachmentReferenceList[x].emplace_back(VkAttachmentReference{ .attachment = static_cast<uint32>(y), .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }); break;
            case RenderAttachmentTypeEnum::DepthRenderedTexture:
                depthRefForThisSubpass = VkAttachmentReference{ .attachment = (uint)(y), .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
                useDepthForThisSubpass = true;
                break;
            case RenderAttachmentTypeEnum::SkipSubPass: break;
            default:
                throw std::runtime_error("Case doesn't exist: RenderedTextureType");
            }
        }

        depthReferences[x] = depthRefForThisSubpass;
        useDepthReferences[x] = useDepthForThisSubpass;

        subPassDescriptionList.emplace_back(VkSubpassDescription{
                    .flags = 0,
                    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                    .inputAttachmentCount = static_cast<uint32>(inputAttachmentReferenceList[x].size()),
                    .pInputAttachments = inputAttachmentReferenceList[x].empty() ? nullptr : inputAttachmentReferenceList[x].data(),
                    .colorAttachmentCount = static_cast<uint32>(colorAttachmentReferenceList[x].size()),
                    .pColorAttachments = colorAttachmentReferenceList[x].empty() ? nullptr : colorAttachmentReferenceList[x].data(),
                    .pResolveAttachments = resolveAttachmentReferenceList[x].empty() ? nullptr : resolveAttachmentReferenceList[x].data(),
                    .pDepthStencilAttachment = useDepthReferences[x] ? &depthReferences[x] : nullptr,
                    .preserveAttachmentCount = 0,
                    .pPreserveAttachments = nullptr
            });
    }

    Texture                         depthTexture;
    Vector<Texture>                 renderedTextureList = Vector<Texture>();
    Vector<VkImageView>             imageAttachmentViewList = Vector<VkImageView>();
    Vector<VkAttachmentDescription> attachmentDescriptionList = Vector<VkAttachmentDescription>();
    for (int x = 0; x < renderPassJsonLoader.RenderAttachmentList.size(); x++)
    {
        VkImageLayout initialLayout;
        VkImageLayout finalLayout;
        RenderAttachmentLoader renderAttachment = renderPassJsonLoader.RenderAttachmentList[x];
        switch (renderAttachment.RenderTextureType)
        {
        case RenderType_SwapChainTexture:      initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;         finalLayout = VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR; break;
        case RenderType_OffscreenColorTexture: initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;         finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; break;
        case RenderType_DepthBufferTexture:    initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;  break;
        case RenderType_GBufferTexture:        initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;         finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;  break;
        case RenderType_IrradianceTexture:     initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; break;
        case RenderType_PrefilterTexture:      initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; break;
        default: throw std::runtime_error("Unknown RenderTextureType");
        }

        attachmentDescriptionList.emplace_back(VkAttachmentDescription
            {
                .format = renderAttachment.Format,
                .samples = renderAttachment.SampleCount >= vulkanSystem.MaxSampleCount ? vulkanSystem.MaxSampleCount : renderAttachment.SampleCount,
                .loadOp = renderAttachment.LoadOp,
                .storeOp = renderAttachment.StoreOp,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = finalLayout
            });

        Texture texture = textureSystem.CreateRenderPassTexture(renderAttachment, ivec2(renderPassJsonLoader.RenderPassWidth, renderPassJsonLoader.RenderPassHeight));
        if (texture.textureType == TextureType_IrradianceMapTexture)
        {
            textureSystem.IrradianceCubeMap = texture;
            renderedTextureList.emplace_back(texture);
            imageAttachmentViewList.emplace_back(texture.AttachmentArrayView);
        }
        else if (texture.textureType == TextureType_PrefilterMapTexture)
        {
            textureSystem.PrefilterCubeMap.PrefilterCubeMap = texture;
            renderedTextureList.emplace_back(texture);
            imageAttachmentViewList.emplace_back(texture.AttachmentArrayView);
        }
        else if (texture.textureType == TextureType_DepthTexture)
        {
            depthTexture = texture;
            imageAttachmentViewList.emplace_back(texture.textureView);
        }
        else
        {
            renderedTextureList.emplace_back(texture);
            if (renderPass.IsRenderedToSwapchain)
            {
                imageAttachmentViewList.emplace_back(vulkanSystem.SwapChainImageViews[x]);
            }
            else
            {
                imageAttachmentViewList.emplace_back(texture.textureView);
            }
        }
    }

    VkRenderPassMultiviewCreateInfo multiviewCreateInfo;
    if (renderPassJsonLoader.UseCubeMapMultiView)
    {
        const uint32_t viewMask = 0b00111111;
        const uint32_t correlationMask = 0b00111111;
        multiviewCreateInfo = VkRenderPassMultiviewCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
            .subpassCount = 1,
            .pViewMasks = &viewMask,
            .correlationMaskCount = 1,
            .pCorrelationMasks = &correlationMask,
        };
    }

    Vector<VkSubpassDependency> subPassDependencyList = Vector<VkSubpassDependency>();
    for (VkSubpassDependency subpass : renderPassJsonLoader.SubpassDependencyModelList)
    {
        subPassDependencyList.emplace_back(subpass);
    }

    VkRenderPassCreateInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = renderPassJsonLoader.UseCubeMapMultiView ? &multiviewCreateInfo : nullptr,
            .attachmentCount = static_cast<uint32>(attachmentDescriptionList.size()),
            .pAttachments = attachmentDescriptionList.data(),
            .subpassCount = static_cast<uint32>(subPassDescriptionList.size()),
            .pSubpasses = subPassDescriptionList.data(),
            .dependencyCount = static_cast<uint32>(subPassDependencyList.size()),
            .pDependencies = subPassDependencyList.data(),
    };

    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkanSystem.Device, &renderPassInfo, nullptr, &renderPass.RenderPass));
    if (textureSystem.PrefilterCubeMap.PrefilterCubeMap.textureImage != VK_NULL_HANDLE)
    {
        textureSystem.CreatePrefilterSkyBoxTexture(renderPass.RenderPass, textureSystem.PrefilterCubeMap.PrefilterCubeMap);
    }

    RenderPassGuid renderPassId = renderPassJsonLoader.RenderPassId;
    if (!renderedTextureList.empty()) textureSystem.AddRenderedTexture(renderPassId, renderedTextureList);
    if (depthTexture.textureImage != VK_NULL_HANDLE)textureSystem.AddDepthTexture(renderPassId, depthTexture);

    renderPass.FrameBufferList.resize(vulkanSystem.SwapChainImageCount);
    for (size_t x = 0; x < vulkanSystem.SwapChainImageCount; x++)
    {
        VkFramebufferCreateInfo framebufferInfo =
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass.RenderPass,
            .attachmentCount = static_cast<uint32_t>(imageAttachmentViewList.size()),
            .pAttachments = imageAttachmentViewList.data(),
            .width = static_cast<uint32_t>(renderPass.RenderPassResolution.x),
            .height = static_cast<uint32_t>(renderPass.RenderPassResolution.y),
            .layers = 1,
        };
        VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkanSystem.Device, &framebufferInfo, nullptr, &renderPass.FrameBufferList[x]));
    }
}

VkDescriptorPool RenderSystem::CreatePipelineDescriptorPool(RenderPipelineLoader& renderPipelineLoader)
{
    Vector<VkDescriptorPoolSize> descriptorPoolSizeList = Vector<VkDescriptorPoolSize>();
    for (const auto& binding : renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList)
    {
        descriptorPoolSizeList.emplace_back(VkDescriptorPoolSize{
            .type = binding.DescripterType,
            .descriptorCount = static_cast<uint32>(binding.DescriptorCount)
            });
    }

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorPoolCreateInfo poolCreateInfo = VkDescriptorPoolCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.empty() ? 1000 : static_cast<uint32>(descriptorPoolSizeList.size()) * 1000,
        .poolSizeCount = static_cast<uint32>(descriptorPoolSizeList.size()),
        .pPoolSizes = descriptorPoolSizeList.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreateDescriptorPool(vulkanSystem.Device, &poolCreateInfo, nullptr, &descriptorPool));
    return descriptorPool;
}

Vector<VkDescriptorSetLayout> RenderSystem::CreatePipelineDescriptorSetLayout(RenderPipelineLoader& renderPipelineLoader)
{
    std::unordered_set<int> uniqueValues;
    std::for_each(renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.begin(), renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.end(), [&](const ShaderDescriptorBindingDLL& binding) {
        uniqueValues.insert(binding.DescriptorSet);
        });
    size_t countDistinct = uniqueValues.size();

    Vector<Vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutBindingList = Vector<Vector<VkDescriptorSetLayoutBinding>>(countDistinct);
    for (auto& descriptorBinding : renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList)
    {
        descriptorSetLayoutBindingList[descriptorBinding.DescriptorSet].emplace_back(VkDescriptorSetLayoutBinding
            {
                .binding = descriptorBinding.Binding,
                .descriptorType = descriptorBinding.DescripterType,
                .descriptorCount = static_cast<uint32>(descriptorBinding.DescriptorCount),
                .stageFlags = descriptorBinding.ShaderStageFlags,
                .pImmutableSamplers = nullptr
            });
    }

    Vector<VkDescriptorSetLayoutCreateInfo> descriptorSetLayoutCreateInfoList = Vector<VkDescriptorSetLayoutCreateInfo>(countDistinct);
    for (int x = 0; x < descriptorSetLayoutCreateInfoList.size(); x++)
    {
        descriptorSetLayoutCreateInfoList[x] = VkDescriptorSetLayoutCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV,
            .bindingCount = static_cast<uint32>(descriptorSetLayoutBindingList[x].size()),
            .pBindings = descriptorSetLayoutBindingList[x].data()
        };
    }

    Vector<VkDescriptorSetLayout> descriptorSetLayoutList = Vector<VkDescriptorSetLayout>(descriptorSetLayoutCreateInfoList.size());
    for (int x = 0; x < descriptorSetLayoutList.size(); x++)
    {
        vkCreateDescriptorSetLayout(vulkanSystem.Device, &descriptorSetLayoutCreateInfoList[x], nullptr, &descriptorSetLayoutList[x]);
    }

    return descriptorSetLayoutList;
}

Vector<VkDescriptorSet> RenderSystem::AllocatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader, const VkDescriptorPool& descriptorPool, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount)
{
    Vector<VkDescriptorSet> descriptorSetList = Vector<VkDescriptorSet>(descriptorSetLayoutCount, VK_NULL_HANDLE);
    for (int x = 0; x < descriptorSetLayoutCount; x++)
    {
        VkDescriptorSetAllocateInfo allocInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &descriptorSetLayoutList[x]
        };
        vkAllocateDescriptorSets(vulkanSystem.Device, &allocInfo, &descriptorSetList[x]);
    }
    return descriptorSetList;
}

void RenderSystem::UpdatePipelineDescriptorSets(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount)
{

    Span<VkDescriptorSet> descriptorSetLayouts(descriptorSetList, descriptorSetCount);
    for (int x = 0; x < descriptorSetLayouts.size(); x++)
    {
        Vector<VkWriteDescriptorSet> writeDescriptorSet = Vector<VkWriteDescriptorSet>();
        for (auto& descriptorSetBinding : renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList)
        {
            if (descriptorSetBinding.DescriptorSet != x) continue;
            writeDescriptorSet.emplace_back(VkWriteDescriptorSet
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = descriptorSetLayouts[x],
                    .dstBinding = descriptorSetBinding.Binding,
                    .dstArrayElement = 0,
                    .descriptorCount = static_cast<uint32>(descriptorSetBinding.DescriptorCount),
                    .descriptorType = descriptorSetBinding.DescripterType,
                    .pImageInfo = descriptorSetBinding.DescriptorImageInfo.data(),
                    .pBufferInfo = descriptorSetBinding.DescriptorBufferInfo.data(),
                    .pTexelBufferView = nullptr
                });
        }
        vkUpdateDescriptorSets(vulkanSystem.Device, static_cast<uint32>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
    }
}

VkPipelineLayout RenderSystem::CreatePipelineLayout(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount)
{
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    Vector<VkPushConstantRange> pushConstantRangeList = Vector<VkPushConstantRange>();
    if (!renderPipelineLoader.ShaderPiplineInfo.PushConstantList.empty())
    {
        pushConstantRangeList.emplace_back(VkPushConstantRange
            {
                .stageFlags = VK_SHADER_STAGE_ALL,
                .offset = 0,
                .size = static_cast<uint>(renderPipelineLoader.ShaderPiplineInfo.PushConstantList[0].PushConstantSize)
            });
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = VkPipelineLayoutCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = static_cast<uint32>(descriptorSetLayoutCount),
        .pSetLayouts = descriptorSetLayoutList,
        .pushConstantRangeCount = static_cast<uint32>(pushConstantRangeList.size()),
        .pPushConstantRanges = pushConstantRangeList.data()
    };
    VULKAN_THROW_IF_FAIL(vkCreatePipelineLayout(vulkanSystem.Device, &pipelineLayoutInfo, nullptr, &pipelineLayout));
    return pipelineLayout;
}

VkPipeline RenderSystem::CreatePipeline(RenderPipelineLoader& renderPipelineLoader, VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, VkDescriptorSet* descriptorSetList, size_t descriptorSetCount)
{
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = VkPipelineVertexInputStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = static_cast<uint>(renderPipelineLoader.ShaderPiplineInfo.VertexInputBindingList.size()),
        .pVertexBindingDescriptions = renderPipelineLoader.ShaderPiplineInfo.VertexInputBindingList.data(),
        .vertexAttributeDescriptionCount = static_cast<uint>(renderPipelineLoader.ShaderPiplineInfo.VertexInputAttributeList.size()),
        .pVertexAttributeDescriptions = renderPipelineLoader.ShaderPiplineInfo.VertexInputAttributeList.data()
    };

    for (auto& viewPort : renderPipelineLoader.ViewportList)
    {
        viewPort.width = static_cast<float>(renderPipelineLoader.RenderPassResolution.x);
        viewPort.height = static_cast<float>(renderPipelineLoader.RenderPassResolution.y);
    }

    for (auto& scissor : renderPipelineLoader.ScissorList)
    {
        scissor.extent.width = static_cast<float>(renderPipelineLoader.RenderPassResolution.x);
        scissor.extent.height = static_cast<float>(renderPipelineLoader.RenderPassResolution.y);
    }

    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = static_cast<uint32>(renderPipelineLoader.ViewportList.size() ? renderPipelineLoader.ViewportList.size() : 1),
        .pViewports = renderPipelineLoader.ViewportList.data(),
        .scissorCount = static_cast<uint32>(renderPipelineLoader.ScissorList.size() ? renderPipelineLoader.ScissorList.size() : 1),
        .pScissors = renderPipelineLoader.ScissorList.data()
    };

    Vector<VkDynamicState> dynamicStateList;
    if (renderPipelineLoader.ViewportList.empty() || renderPipelineLoader.ScissorList.empty())
    {
        dynamicStateList.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        dynamicStateList.push_back(VK_DYNAMIC_STATE_SCISSOR);
    }

    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = static_cast<uint32>(dynamicStateList.size()),
        .pDynamicStates = dynamicStateList.data()
    };

    Vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfoList = Vector<VkPipelineShaderStageCreateInfo>
    {
        shaderSystem.LoadShader(renderPipelineLoader.ShaderPiplineInfo.ShaderList[0].c_str(), VK_SHADER_STAGE_VERTEX_BIT),
        shaderSystem.LoadShader(renderPipelineLoader.ShaderPiplineInfo.ShaderList[1].c_str(), VK_SHADER_STAGE_FRAGMENT_BIT)
    };

    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfoModel = renderPipelineLoader.PipelineColorBlendStateCreateInfoModel;
    pipelineColorBlendStateCreateInfoModel.attachmentCount = renderPipelineLoader.PipelineColorBlendAttachmentStateList.size();
    pipelineColorBlendStateCreateInfoModel.pAttachments = renderPipelineLoader.PipelineColorBlendAttachmentStateList.data();

    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = renderPipelineLoader.PipelineMultisampleStateCreateInfo;
    pipelineMultisampleStateCreateInfo.rasterizationSamples = pipelineMultisampleStateCreateInfo.rasterizationSamples >= vulkanSystem.MaxSampleCount ? vulkanSystem.MaxSampleCount : pipelineMultisampleStateCreateInfo.rasterizationSamples;
    pipelineMultisampleStateCreateInfo.sampleShadingEnable = pipelineMultisampleStateCreateInfo.rasterizationSamples > VK_SAMPLE_COUNT_1_BIT ? VK_TRUE : VK_FALSE;
    pipelineMultisampleStateCreateInfo.pSampleMask = nullptr;

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = VkGraphicsPipelineCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = static_cast<uint32>(pipelineShaderStageCreateInfoList.size()),
        .pStages = pipelineShaderStageCreateInfoList.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &renderPipelineLoader.PipelineInputAssemblyStateCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &pipelineViewportStateCreateInfo,
        .pRasterizationState = &renderPipelineLoader.PipelineRasterizationStateCreateInfo,
        .pMultisampleState = &pipelineMultisampleStateCreateInfo,
        .pDepthStencilState = &renderPipelineLoader.PipelineDepthStencilStateCreateInfo,
        .pColorBlendState = &pipelineColorBlendStateCreateInfoModel,
        .pDynamicState = &pipelineDynamicStateCreateInfo,
        .layout = pipelineLayout,
        .renderPass = renderPipelineLoader.RenderPass,
        .subpass = renderPipelineLoader.SubPassId,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0
    };

    VULKAN_THROW_IF_FAIL(vkCreateGraphicsPipelines(vulkanSystem.Device, pipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline));
    for (auto& shader : pipelineShaderStageCreateInfoList)
    {
        vkDestroyShaderModule(vulkanSystem.Device, shader.module, nullptr);
    }

    return pipeline;
}

void RenderSystem::PipelineBindingData(RenderPipelineLoader& renderPipelineLoader)
{
    Vector<ShaderDescriptorBinding> bindingList;
    for (int x = 0; x < renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList.size(); x++)
    {
        switch (renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBindingType)
        {
        case kMeshPropertiesDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetMeshPropertiesBuffer(renderPipelineLoader.LevelId).size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = renderSystem.GetMeshPropertiesBuffer(renderPipelineLoader.LevelId);
            break;
        }
        case kTextureDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetTexturePropertiesBuffer(renderPipelineLoader.RenderPassId).size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = renderSystem.GetTexturePropertiesBuffer(renderPipelineLoader.RenderPassId);
            break;
        }
        case kMaterialDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = materialSystem.GetMaterialPropertiesBuffer().size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = materialSystem.GetMaterialPropertiesBuffer();
            break;
        }
        case kDirectionalLightDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = lightSystem.GetDirectionalLightPropertiesBuffer().size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = lightSystem.GetDirectionalLightPropertiesBuffer();
            break;
        }
        case kPointLightDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = lightSystem.GetPointLightPropertiesBuffer().size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = lightSystem.GetPointLightPropertiesBuffer();
            break;
        }
        case kVertexDescsriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetVertexPropertiesBuffer().size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = renderSystem.GetVertexPropertiesBuffer();
            break;
        }
        case kIndexDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetIndexPropertiesBuffer().size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = renderSystem.GetIndexPropertiesBuffer();
            break;
        }
        case kTransformDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetGameObjectTransformBuffer().size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorBufferInfo = renderSystem.GetGameObjectTransformBuffer();
            break;
        }
        case kSkyBoxDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetSkyBoxTextureBuffer().size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = renderSystem.GetSkyBoxTextureBuffer();
            break;
        }
        case kIrradianceMapDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetIrradianceMapTextureBuffer(renderPipelineLoader.RenderPassId).size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = renderSystem.GetIrradianceMapTextureBuffer(renderPipelineLoader.RenderPassId);
            break;
        }
        case kPrefilterMapDescriptor:
        {
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = renderSystem.GetPrefilterMapTextureBuffer(renderPipelineLoader.RenderPassId).size();
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = renderSystem.GetPrefilterMapTextureBuffer(renderPipelineLoader.RenderPassId);
            break;
        }
        case kSubpassInputDescriptor:
        {
            Texture inputTexture = textureSystem.FindRenderedTextureList(renderPipelineLoader.RenderPassId)[x];
            VkDescriptorImageInfo descriptorImage = VkDescriptorImageInfo
                {
                    .sampler = inputTexture.textureSampler,
                    .imageView = inputTexture.textureView,
                    .imageLayout = inputTexture.textureImageLayout,
                };
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorCount = 1;
            renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList[x].DescriptorImageInfo = Vector<VkDescriptorImageInfo>{ descriptorImage };
            break;
        }
        default:
        {
            throw std::runtime_error("Binding case hasn't been handled yet");
        }
        }
    }
}

void RenderSystem::DestoryRenderPassSwapChainTextures(Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture)
{
    Vector<Texture> renderedTextureList = Vector<Texture>(&renderedTextureListPtr, &renderedTextureListPtr + renderedTextureCount);
    for (auto& renderedTexture : renderedTextureList)
    {
        textureSystem.DestroyTexture(renderedTexture);
    }
    std::memset(static_cast<void*>(&renderedTextureListPtr), 0x00, sizeof(Texture) * renderedTextureCount);
    renderedTextureCount = 0;
    renderedTextureList.clear();
}

void RenderSystem::DestroyRenderPass(VulkanRenderPass& renderPass)
{
    vulkanSystem.DestroyRenderPass(vulkanSystem.Device, &renderPass.RenderPass);
    //vulkanSystem.DestroyCommandBuffers(vulkanSystem.Device, &vulkanSystem.CommandPool, &renderPass.CommandBuffer, 1);
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, &renderPass.FrameBufferList[0], vulkanSystem.SwapChainImageCount);

    renderPass.RenderPassId = VkGuid();
    renderPass.SampleCount = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
    renderPass.RenderPass = VK_NULL_HANDLE;
    renderPass.IsRenderedToSwapchain = false;
}

void RenderSystem::Destroy()
{
    DestroyRenderPasses();
    DestroyRenderPipelines();
    vulkanSystem.DestroyRenderer();
}

void RenderSystem::DestroyRenderPasses()
{
    for (auto& renderPass : renderSystem.RenderPassMap)
    {
        DestroyRenderPass(renderPass.second);
    }
    renderSystem.RenderPassMap.clear();
}

void RenderSystem::DestroyRenderPipelines()
{
    for (auto& renderPipelineList : renderSystem.RenderPipelineMap)
    {
        for (auto& renderPipeline : renderPipelineList.second)
        {
            DestroyPipeline(renderPipeline);
        }
    }
    renderSystem.RenderPipelineMap.clear();
}

void RenderSystem::DestroyPipeline(VulkanPipeline& vulkanPipeline)
{
    vulkanPipeline.RenderPipelineId = VkGuid();
    vulkanSystem.DestroyPipeline(vulkanSystem.Device, &vulkanPipeline.Pipeline);
    vulkanSystem.DestroyPipelineLayout(vulkanSystem.Device, &vulkanPipeline.PipelineLayout);
    vulkanSystem.DestroyPipelineCache(vulkanSystem.Device, &vulkanPipeline.PipelineCache);
    vulkanSystem.DestroyDescriptorPool(vulkanSystem.Device, &vulkanPipeline.DescriptorPool);
}

void RenderSystem::DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList)
{
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, frameBufferList.data(), frameBufferList.size());
}

void RenderSystem::DestroyCommandBuffers(VkCommandBuffer& commandBuffer)
{
    vulkanSystem.DestroyCommandBuffers(vulkanSystem.Device, &vulkanSystem.CommandPool, &commandBuffer, 1);
}

void RenderSystem::DestroyBuffer(VkBuffer& buffer)
{
    vulkanSystem.DestroyBuffer(vulkanSystem.Device, &buffer);
}

Vector<VkDescriptorBufferInfo> RenderSystem::GetVertexPropertiesBuffer()
{
    //Vector<MeshStruct> meshList;
        //meshList.reserve(meshSystem.SpriteMeshList.size());
        //std::transform(meshSystem.SpriteMeshList.begin(), meshSystem.SpriteMeshList.end(),
        //    std::back_inserter(meshList),
        //    [](const auto& pair) { return pair.second; });


    Vector<VkDescriptorBufferInfo> vertexPropertiesBuffer;
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
};

Vector<VkDescriptorBufferInfo> RenderSystem::GetIndexPropertiesBuffer()
{
    //Vector<MeshStruct> meshList;
    //meshList.reserve(meshSystem.SpriteMeshList.size());
    //std::transform(meshSystem.SpriteMeshList.begin(), meshSystem.SpriteMeshList.end(),
    //    std::back_inserter(meshList),
    //    [](const auto& pair) { return pair.second; });

    std::vector<VkDescriptorBufferInfo>	indexPropertiesBuffer;
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
};

Vector<VkDescriptorBufferInfo> RenderSystem::GetGameObjectTransformBuffer()
{
    //Vector<MeshStruct> meshList;
    //meshList.reserve(meshSystem.SpriteMeshList.size());
    //std::transform(meshSystem.SpriteMeshList.begin(), meshSystem.SpriteMeshList.end(),
    //    std::back_inserter(meshList),
    //    [](const auto& pair) { return pair.second; });

    std::vector<VkDescriptorBufferInfo>	transformPropertiesBuffer;
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
};

Vector<VkDescriptorBufferInfo> RenderSystem::GetMeshPropertiesBuffer(const LevelGuid& levelLayerId)
{
    Vector<VkDescriptorBufferInfo> meshPropertiesBuffer;
    if (meshSystem.MeshList.empty())
    {
        meshPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
            {
                .buffer = VK_NULL_HANDLE,
                .offset = 0,
                .range = VK_WHOLE_SIZE
            });
    }
    else
    {
        for (auto& mesh : meshSystem.MeshList)
        {
            const VulkanBuffer& meshProperties = bufferSystem.FindVulkanBuffer(mesh.PropertiesBufferId);
            meshPropertiesBuffer.emplace_back(VkDescriptorBufferInfo
                {
                    .buffer = meshProperties.Buffer,
                    .offset = 0,
                    .range = VK_WHOLE_SIZE
                });
        }
    }
    return meshPropertiesBuffer;
};

Vector<VkDescriptorImageInfo> RenderSystem::GetTexturePropertiesBuffer(const RenderPassGuid& renderPassGuid)
{
    Vector<Texture> textureList;
    const VulkanRenderPass& renderPass = FindRenderPass(renderPassGuid);
    if (renderPass.InputTextureIdList.size() > 0)
    {
        for (auto& inputTexture : renderPass.InputTextureIdList)
        {
            Texture texture = textureSystem.FindTexture(inputTexture, 0);
            if (!texture.RenderedCubeMapView)
            {
                textureList.emplace_back(texture);
            }
        }
    }
    else
    {
        textureList = textureSystem.TextureList;
    }

    Vector<VkDescriptorImageInfo>	texturePropertiesBuffer;
    if (textureList.empty())
    {
        VkSamplerCreateInfo NullSamplerInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = 16.0f,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0,
            .maxLod = 0,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };

        VkSampler nullSampler = VK_NULL_HANDLE;
        if (vkCreateSampler(vulkanSystem.Device, &NullSamplerInfo, nullptr, &nullSampler))
        {
            throw std::runtime_error("Failed to create Sampler.");
        }

        VkDescriptorImageInfo nullBuffer =
        {
            .sampler = nullSampler,
            .imageView = VK_NULL_HANDLE,
            .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        texturePropertiesBuffer.emplace_back(nullBuffer);
    }
    else
    {
        for (auto& texture : textureList)
        {
            textureSystem.GetTexturePropertiesBuffer(texture, texturePropertiesBuffer);
        }
    }

    return texturePropertiesBuffer;
}

Vector<VkDescriptorImageInfo> RenderSystem::GetSkyBoxTextureBuffer()
{
    Vector<VkDescriptorImageInfo>	texturePropertiesBuffer;
    textureSystem.GetTexturePropertiesBuffer(textureSystem.CubeMap, texturePropertiesBuffer);
    return texturePropertiesBuffer;
}

Vector<VkDescriptorImageInfo> RenderSystem::GetIrradianceMapTextureBuffer(const RenderPassGuid& renderPassGuid)
{
    Vector<VkDescriptorImageInfo>	texturePropertiesBuffer;
    texturePropertiesBuffer.emplace_back(VkDescriptorImageInfo
        {
            .sampler = textureSystem.IrradianceCubeMap.textureSampler,
            .imageView = textureSystem.IrradianceCubeMap.RenderedCubeMapView,
            .imageLayout = textureSystem.IrradianceCubeMap.textureImageLayout
        });
    return texturePropertiesBuffer;
}

Vector<VkDescriptorImageInfo> RenderSystem::GetPrefilterMapTextureBuffer(const RenderPassGuid& renderPassGuid)
{
    Vector<VkDescriptorImageInfo>	texturePropertiesBuffer;
    texturePropertiesBuffer.emplace_back(VkDescriptorImageInfo
        {
            .sampler = textureSystem.PrefilterCubeMap.PrefilterCubeMap.textureSampler,
            .imageView = textureSystem.PrefilterCubeMap.PrefilterCubeMap.RenderedCubeMapView,
            .imageLayout = textureSystem.PrefilterCubeMap.PrefilterCubeMap.textureImageLayout,
        });
    return texturePropertiesBuffer;
}