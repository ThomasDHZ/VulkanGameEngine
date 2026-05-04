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
#include <algorithm>

RenderSystem& renderSystem = RenderSystem::Get();

void RenderSystem::StartUp(void* windowHandle, VkInstance& instance, VkSurfaceKHR& surface)
{
    vulkanSystem.RendererSetUp(windowHandle, instance, surface);
}

void RenderSystem::Update(void* windowHandle, const float& deltaTime)
{
    if (vulkanSystem.RebuildRendererFlag)
    {
        RecreateSwapchain(windowHandle, deltaTime);
        vulkanSystem.RebuildRendererFlag = false;
    }
}

RenderPassGuid RenderSystem::LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath)
{
    RenderPassLoader renderPassLoader = fileSystem.LoadJsonFile(jsonPath).get<RenderPassLoader>();
    return LoadRenderPass(levelGuid, renderPassLoader);
}

RenderPassGuid RenderSystem::LoadRenderPass(LevelGuid& levelGuid, RenderPassLoader& renderPassLoader)
{
    VulkanRenderPass vulkanRenderPass = VulkanRenderPass
    {
        .RenderPassId = renderPassLoader.RenderPassId,
        .SubPassCount = renderPassLoader.SubPassCount,
        .SampleCount = renderPassLoader.SampleCount >= vulkanSystem.MaxSampleCount ? vulkanSystem.MaxSampleCount : renderPassLoader.SampleCount,
        .RenderPass = VK_NULL_HANDLE,
        .InputTextureIdList = renderPassLoader.InputTextureList,
        .FrameBufferList = Vector<VkFramebuffer>(),
        .ClearValueList = renderPassLoader.ClearValueList,
        .RenderPassResolution = renderPassLoader.UseDefaultRenderResolution ? vulkanSystem.DefaultRenderPassResolution : renderPassLoader.RenderPassResolution,
        .MaxPushConstantSize = 0,
        .UseDefaultRenderResolution = renderPassLoader.UseDefaultRenderResolution,
        .UseCubeMapMultiView = renderPassLoader.UseCubeMapMultiView,
        .IsCubeMapRenderPass = renderPassLoader.IsCubeMapRenderPass
    };

    RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId] = renderPassLoader.RenderAttachmentList;
    BuildRenderPass(vulkanRenderPass, renderPassLoader);
    BuildFrameBuffer(vulkanRenderPass);
    AddRenderPass(vulkanRenderPass);

    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    for (const auto& pipelinePath : renderPassLoader.RenderPipelineList)
    {
        VkPipeline                    pipeline = VK_NULL_HANDLE;
        VkPipelineLayout              pipelineLayout = VK_NULL_HANDLE;
        Vector<VkDescriptorSet>       descriptorSetList = { memoryPoolSystem.GlobalBindlessDescriptorSet };
        Vector<VkDescriptorSetLayout> descriptorSetLayoutList = { memoryPoolSystem.GlobalBindlessDescriptorSetLayout };

        nlohmann::json pipelineJson = fileSystem.LoadJsonFile(pipelinePath.c_str());
        RenderPipelineLoader renderPipelineLoader = pipelineJson.get<RenderPipelineLoader>();
        renderPipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = vulkanRenderPass.SampleCount;
        renderPipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = (vulkanRenderPass.SampleCount > VK_SAMPLE_COUNT_1_BIT);
        renderPipelineLoader.RenderPassId = vulkanRenderPass.RenderPassId;
        renderPipelineLoader.RenderPass = vulkanRenderPass.RenderPass;
        renderPipelineLoader.RenderPassResolution = vulkanRenderPass.RenderPassResolution;
        renderPipelineLoader.ShaderPiplineInfo = shaderSystem.LoadPipelineShaderData(Vector<String>{ pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] });

        std::unordered_set<uint32> uniqueSets;
        for (const auto& descriptorSet : renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList)
        {
            if (descriptorSet.DescriptorSet != UINT32_MAX) uniqueSets.insert(descriptorSet.DescriptorSet);
        }
        size_t uniqueDescriptorSetCount = uniqueSets.size();

        Vector<Vector<ShaderDescriptorBinding>> descriptorSetLists;
        descriptorSetLists.resize(uniqueDescriptorSetCount);
        if (uniqueDescriptorSetCount > 1)
        {
            for (auto& descriptorSet : renderPipelineLoader.ShaderPiplineInfo.DescriptorBindingsList)
            {
                descriptorSetLists[descriptorSet.DescriptorSet].emplace_back(descriptorSet);
            }

            for (int x = 1; x < descriptorSetLists.size(); x++)
            {   //set 0 = global descriptor set

                VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
                VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
                Vector<VkDescriptorSetLayoutBinding> descriptorSetBindingList;
                for (int y = 0; y < descriptorSetLists[x].size(); y++)
                {
                    descriptorSetBindingList.emplace_back(VkDescriptorSetLayoutBinding
                        { 
                            .binding = descriptorSetLists[x][y].Binding,
                            .descriptorType = descriptorSetLists[x][y].DescripterType,
                            .descriptorCount = 1,
                            .stageFlags = descriptorSetLists[x][y].ShaderStageFlags,
                            .pImmutableSamplers = nullptr
                        });
                }

                VkDescriptorSetLayoutCreateInfo layoutInfo =
                {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
                    .bindingCount = static_cast<uint32>(descriptorSetBindingList.size()),
                    .pBindings = descriptorSetBindingList.data()
                };
                VULKAN_THROW_IF_FAIL(vkCreateDescriptorSetLayout(vulkanSystem.Device, &layoutInfo, nullptr, &descriptorSetLayout));

                VkDescriptorSetAllocateInfo allocInfo =
                {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                    .pNext = nullptr,
                    .descriptorPool = memoryPoolSystem.GlobalBindlessPool,
                    .descriptorSetCount = 1,
                    .pSetLayouts = &descriptorSetLayout
                };
                VULKAN_THROW_IF_FAIL(vkAllocateDescriptorSets(vulkanSystem.Device, &allocInfo, &descriptorSet));

                Vector<VkDescriptorImageInfo> subpassInputInfo = memoryPoolSystem.GetSubPassInputTextureDescriptor(vulkanRenderPass.RenderPassId);
                if (subpassInputInfo.size() > descriptorSetBindingList.size()) subpassInputInfo.resize(descriptorSetBindingList.size());

                Vector<VkWriteDescriptorSet> writeDescriptorSetList;
                for (uint32 binding = 0; binding < descriptorSetBindingList.size(); ++binding)
                {
                    VkDescriptorImageInfo* pInfo = binding < subpassInputInfo.size() ? &subpassInputInfo[binding] : nullptr;
                    writeDescriptorSetList.push_back(VkWriteDescriptorSet
                        {
                            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                            .pNext = nullptr,
                            .dstSet = descriptorSet,
                            .dstBinding = binding,
                            .dstArrayElement = 0,
                            .descriptorCount = 1,
                            .descriptorType = descriptorSetLists[x][binding].DescripterType,
                            .pImageInfo = &subpassInputInfo[binding],
                            .pTexelBufferView = nullptr
                        });
                }
                vkUpdateDescriptorSets(vulkanSystem.Device, static_cast<uint32>(writeDescriptorSetList.size()), writeDescriptorSetList.data(), 0, nullptr);
                descriptorSetList.emplace_back(descriptorSet);
                descriptorSetLayoutList.emplace_back(descriptorSetLayout);
            }
        }
        pipelineLayout = CreatePipelineLayout(renderPipelineLoader, descriptorSetLayoutList.data(), descriptorSetLayoutList.size());
        pipeline = CreatePipeline(renderPipelineLoader, pipelineCache, pipelineLayout, descriptorSetList.data(), descriptorSetList.size());

        renderSystem.RenderPipelineMap[renderPassLoader.RenderPassId].emplace_back
        (
            VulkanPipeline
            {
                .RenderPipelineId = renderPipelineLoader.PipelineId,
                .DescriptorSetLayoutList = descriptorSetLayoutList,
                .DescriptorSetList = descriptorSetList,
                .Pipeline = pipeline,
                .PipelineLayout = pipelineLayout,
                .PipelineCache = pipelineCache
            }
        );
    }

    return renderPassLoader.RenderPassId;
}

void RenderSystem::RecreateSwapchain(void* windowHandle, const float& deltaTime)
{
    vkDeviceWaitIdle(vulkanSystem.Device);
    for (auto& renderPass : renderSystem.RenderPassList()) vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, renderPass.FrameBufferList);
    vulkanSystem.DestroySwapChainImageView(vulkanSystem.Device, vulkanSystem.SwapChainImageViews);
    vulkanSystem.DestroySwapChain(vulkanSystem.Device, &vulkanSystem.Swapchain);

    vulkanSystem.SetUpSwapChain(windowHandle);
    for (auto& renderPass : renderSystem.RenderPassList())
    {
        BuildFrameBuffer(renderPass);
    }
    // ImGui_RebuildSwapChain(renderer, imGuiRenderer);
}

void RenderSystem::BuildRenderPass(VulkanRenderPass& vulkanRenderPass, const RenderPassLoader& renderPassJsonLoader)
{
    VkAttachmentReference unusedRef = {};
    VkAttachmentReference depthReference = VkAttachmentReference();
    Vector<bool> useDepthReferences(vulkanRenderPass.SubPassCount, false);
    Vector<VkAttachmentReference> depthReferences(vulkanRenderPass.SubPassCount);
    Vector<VkSubpassDescription> subPassDescriptionList;
    Vector<Vector<VkAttachmentReference>> inputAttachmentReferenceList(vulkanRenderPass.SubPassCount);
    Vector<Vector<VkAttachmentReference>> colorAttachmentReferenceList(vulkanRenderPass.SubPassCount);
    Vector<Vector<VkAttachmentReference>> resolveAttachmentReferenceList(vulkanRenderPass.SubPassCount);
    Vector<Vector<VkSubpassDescription>> preserveAttachmentReferenceList(vulkanRenderPass.SubPassCount);
    Vector<RenderPassAttachmentTexture> renderPassAttachmentTextureInfoMap = RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId];
    for (int x = 0; x < vulkanRenderPass.SubPassCount; x++)
    {
        bool useDepthForThisSubpass = false;
        VkAttachmentReference depthRefForThisSubpass = {};
        for (int y = 0; y < renderPassAttachmentTextureInfoMap.size(); y++)
        {
            RenderPassAttachmentTexture renderAttachment = renderPassAttachmentTextureInfoMap[y];
            switch (renderAttachment.RenderAttachmentTypes[x])
            {
            case RenderAttachmentTypeEnum::ColorRenderedTexture: colorAttachmentReferenceList[x].emplace_back(VkAttachmentReference{ .attachment = static_cast<uint32>(y), .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }); break;
            case RenderAttachmentTypeEnum::InputAttachmentTexture: {
                bool is_depth = (renderAttachment.Format >= VK_FORMAT_D16_UNORM && renderAttachment.Format <= VK_FORMAT_D32_SFLOAT_S8_UINT); // Adjust for your formats
                VkImageLayout input_layout = is_depth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                inputAttachmentReferenceList[x].emplace_back(VkAttachmentReference{ .attachment = static_cast<uint32>(y), .layout = input_layout });
                break;
            }
            case RenderAttachmentTypeEnum::ResolveAttachmentTexture: resolveAttachmentReferenceList[x].emplace_back(VkAttachmentReference{ .attachment = static_cast<uint32>(y), .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }); break;
            case RenderAttachmentTypeEnum::DepthRenderedTexture:  depthRefForThisSubpass = VkAttachmentReference{ .attachment = (uint)(y), .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }; useDepthForThisSubpass = true; break;
            case RenderAttachmentTypeEnum::SkipSubPass: break;
            default: throw std::runtime_error("Case doesn't exist: RenderedTextureType");
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

    Vector<VkAttachmentDescription> attachmentDescriptionList = BuildRenderPassAttachments(vulkanRenderPass);
    Vector<Texture> frameBufferTextureList = BuildRenderPassAttachmentTextures(vulkanRenderPass);

    VkRenderPassMultiviewCreateInfo multiviewCreateInfo{};
    if (renderPassJsonLoader.UseCubeMapMultiView)
    {
        const uint32 viewMask =     0b0000111111;  // bits 0-5 for 6 faces
        multiviewCreateInfo = VkRenderPassMultiviewCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
            .subpassCount = 1,
            .pViewMasks = &viewMask,
            .correlationMaskCount = 1,
            .pCorrelationMasks = &viewMask
        };
    }

    Vector<VkSubpassDependency> subpassDependencies = renderPassJsonLoader.SubpassDependencyModelList;
    //if (renderPassJsonLoader.RenderPassId == VkGuid("d5b5ad49-d004-4d5e-8260-4ba9e248f863"))
    //{
    //    subpassDependencies =
    //    {
    //        // EXTERNAL → Subpass 0
    //        {
    //            .srcSubpass = VK_SUBPASS_EXTERNAL,
    //            .dstSubpass = 0,
    //            .srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
    //            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
    //            .srcAccessMask = 0,
    //            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    //            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
    //        },

    //        // Subpass 0 → Subpass 1
    //        {
    //            .srcSubpass = 0,
    //            .dstSubpass = 1,
    //            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    //            .dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    //            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    //            .dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
    //            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
    //        },

    //        // Subpass 1 → EXTERNAL (final)
    //        {
    //            .srcSubpass = 1,
    //            .dstSubpass = VK_SUBPASS_EXTERNAL,
    //            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    //            .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    //            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    //            .dstAccessMask = 0,
    //            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
    //        }
    //    };
    //}

    VkRenderPassCreateInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = renderPassJsonLoader.UseCubeMapMultiView ? &multiviewCreateInfo : nullptr,
        .attachmentCount = static_cast<uint32>(attachmentDescriptionList.size()),
        .pAttachments = attachmentDescriptionList.data(),
        .subpassCount = static_cast<uint32>(subPassDescriptionList.size()),
        .pSubpasses = subPassDescriptionList.data(),
        .dependencyCount = static_cast<uint32>(subpassDependencies.size()),
        .pDependencies = subpassDependencies.data(),
    };
    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkanSystem.Device, &renderPassInfo, nullptr, &vulkanRenderPass.RenderPass));
}

Vector<VkAttachmentDescription> RenderSystem::BuildRenderPassAttachments(VulkanRenderPass& vulkanRenderPass)
{
    Vector<VkAttachmentDescription> attachmentDescriptionList;
    Vector<RenderPassAttachmentTexture> renderPassAttachmentTextureInfoList = RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId];
    for (int x = 0; x < renderPassAttachmentTextureInfoList.size(); x++)
    {
        VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        const RenderPassAttachmentTexture& renderAttachment = renderPassAttachmentTextureInfoList[x];
        switch (renderAttachment.RenderTextureType)
        {
            case RenderType_SwapChainTexture:      initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;         finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                  break;
            case RenderType_OffscreenColorTexture: initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case RenderType_DepthBufferTexture:    initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;  break;
            case RenderType_GBufferTexture:        initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case RenderType_IrradianceTexture:     initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case RenderType_PrefilterTexture:      initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;         finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case RenderType_CubeMapTexture:        initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case RenderType_BRDFTexture:           initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            default: throw std::runtime_error("Unknown RenderTextureType");
        }

        attachmentDescriptionList.emplace_back(VkAttachmentDescription
            {
            .format = renderAttachment.Format,
            .samples = vulkanRenderPass.SampleCount >= vulkanSystem.MaxSampleCount ? vulkanSystem.MaxSampleCount : vulkanRenderPass.SampleCount,
            .loadOp = renderAttachment.LoadOp,
            .storeOp = renderAttachment.StoreOp,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = initialLayout,
            .finalLayout = finalLayout
            });
    }
    return attachmentDescriptionList;
}

Vector<Texture> RenderSystem::BuildRenderPassAttachmentTextures(VulkanRenderPass& vulkanRenderPass)
{
    SceneDataBuffer& sceneDataBuffer = memoryPoolSystem.UpdateSceneDataBuffer();

    Texture depthTexture;
    Vector<Texture> renderedTextureList;
    Vector<Texture> frameBufferTextureList;
    Vector<RenderPassAttachmentTexture> renderPassAttachmentTextureInfoList = RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId];
    for (int x = 0; x < renderPassAttachmentTextureInfoList.size(); x++)
    {
        Texture texture = textureSystem.CreateRenderPassTexture(vulkanRenderPass, x);
        renderedTextureList.emplace_back(texture);
        frameBufferTextureList.emplace_back(texture);

        if (texture.textureType == TextureType_DepthTexture)
        {
            depthTexture = texture;
        }

        if (!renderSystem.UsingMaterialBaker)
        {
            if (texture.textureType == TextureType_IrradianceMapTexture ||
                texture.textureType == TextureType_PrefilterMapTexture ||
                texture.textureType == TextureType_SkyboxTexture)
            {
                memoryPoolSystem.UpdateTextureDescriptorSet(texture, memoryPoolSystem.CubeMapDescriptorBinding);
            }
            else
            {
                memoryPoolSystem.UpdateTextureDescriptorSet(texture, memoryPoolSystem.Texture2DBinding);
            }
        }
    }
    if (!renderedTextureList.empty()) textureSystem.AddRenderedTexture(vulkanRenderPass.RenderPassId, renderedTextureList);
    if (depthTexture.textureImage != VK_NULL_HANDLE) textureSystem.AddDepthTexture(vulkanRenderPass.RenderPassId, depthTexture);
    return frameBufferTextureList;
}

void RenderSystem::BuildFrameBuffer(VulkanRenderPass& vulkanRenderPass)
{
    Vector<Texture>  frameBufferAttachment = textureSystem.FindRenderedTextureList(vulkanRenderPass.RenderPassId);
    if (!frameBufferAttachment.empty() &&
            (frameBufferAttachment[0].textureType == TextureType_SkyboxTexture ||
             frameBufferAttachment[0].textureType == TextureType_IrradianceMapTexture ||
             frameBufferAttachment[0].textureType == TextureType_PrefilterMapTexture))
    {
        uint32 mipLevels = frameBufferAttachment[0].mipMapLevels;
        uint32 baseSize = frameBufferAttachment[0].width;
        vulkanRenderPass.FrameBufferList.resize(mipLevels);
        for (uint32_t mip = 0; mip < mipLevels; ++mip)
        {
            uint32 mipWidth = std::max(1u, baseSize >> mip);
            uint32 mipHeight = std::max(1u, baseSize >> mip);
            Vector<VkImageView> attachments{ frameBufferAttachment[0].textureViewList[mip] };
            VkFramebufferCreateInfo info = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = vulkanRenderPass.RenderPass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = mipWidth,
                .height = mipHeight,
                .layers = vulkanRenderPass.UseCubeMapMultiView ? 1u : 6u
            };
            VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkanSystem.Device, &info, nullptr, &vulkanRenderPass.FrameBufferList[mip]));
        }
    }
    else
    {
        vulkanRenderPass.FrameBufferList.resize(1);
        std::vector<VkImageView> attachments;
        for (const auto& texture : frameBufferAttachment)
        {
            attachments.emplace_back(texture.textureViewList.front());
        }
        VkFramebufferCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = vulkanRenderPass.RenderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = static_cast<uint32_t>(vulkanRenderPass.RenderPassResolution.x),
            .height = static_cast<uint32_t>(vulkanRenderPass.RenderPassResolution.y),
            .layers = 1
        };
        VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkanSystem.Device, &info, nullptr, &vulkanRenderPass.FrameBufferList[0]));
    }
}

void RenderSystem::AddRenderPass(const VulkanRenderPass& vulkanRenderPass)
{
    GuidToRenderPassNodeIndex[vulkanRenderPass.RenderPassId] = RenderPassNodes.size();
    RenderPassNodes.emplace_back(vulkanRenderPass);
}


VkPipelineLayout RenderSystem::CreatePipelineLayout(RenderPipelineLoader& renderPipelineLoader, VkDescriptorSetLayout* descriptorSetLayoutList, size_t descriptorSetLayoutCount)
{
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    Vector<VkPushConstantRange> pushConstantRangeList = Vector<VkPushConstantRange>();
    if (!renderPipelineLoader.ShaderPiplineInfo.PushConstantList.empty())
    {
        pushConstantRangeList.emplace_back(VkPushConstantRange
            {
                .stageFlags = renderPipelineLoader.ShaderPiplineInfo.PushConstantList[0].ShaderStageFlags,
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
//    vulkanSystem.DestroyCommandBuffers(vulkanSystem.Device, &vulkanSystem.CommandPool, &renderPass.com, 1);
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, renderPass.FrameBufferList);

    renderPass.RenderPassId = VkGuid();
    renderPass.SubPassCount = UINT32_MAX;
    renderPass.SampleCount = VK_SAMPLE_COUNT_1_BIT;
    renderPass.RenderPass = VK_NULL_HANDLE;
    renderPass.InputTextureIdList.clear();
    renderPass.FrameBufferList.clear();
    renderPass.ClearValueList.clear();
    renderPass.RenderPassResolution = ivec2();
    renderPass.MaxPushConstantSize = 0;
    renderPass.UseDefaultRenderResolution = false;
    renderPass.UseCubeMapMultiView = false;
    renderPass.IsCubeMapRenderPass = false;
}

void RenderSystem::Destroy()
{
    DestroyRenderPipelines();
    DestroyRenderPasses();
}

void RenderSystem::DestroyRenderPasses()
{
    for (auto& renderPass : renderSystem.RenderPassList())
    {
        DestroyRenderPass(renderPass);
    }
    //renderSystem.RenderPassMap.clear();
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
}

void RenderSystem::DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList)
{
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, frameBufferList);
}

void RenderSystem::DestroyCommandBuffers(Vector<VkCommandBuffer>& commandBuffer)
{
    vulkanSystem.DestroyCommandBuffers(vulkanSystem.Device, &vulkanSystem.CommandPool, commandBuffer);
}

void RenderSystem::DestroyBuffer(VkBuffer& buffer)
{
    vulkanSystem.DestroyBuffer(vulkanSystem.Device, &buffer);
}

Vector<VkDescriptorImageInfo> RenderSystem::GetTexturePropertiesBuffer(const RenderPassGuid& renderPassGuid)
{
    Vector<Texture> textureList;
    const VulkanRenderPass& renderPass = FindRenderPass(renderPassGuid);
    if (renderPass.InputTextureIdList.size() > 0)
    {
        for (auto& inputTexture : renderPass.InputTextureIdList)
        {
            Texture texture = textureSystem.FindTexture(inputTexture);
            if (!texture.RenderedCubeMapView)
            {
                if (texture.textureImageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                {
                    continue;
                }
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
            if (texture.textureImageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ||
                texture.textureImageLayout == VK_IMAGE_LAYOUT_UNDEFINED)
            {
                continue;
            }

            textureSystem.GetTexturePropertiesBuffer(texture, texturePropertiesBuffer);
        }
    }
    
    return texturePropertiesBuffer;
}

Vector<VkDescriptorImageInfo> RenderSystem::GetTexture3DPropertiesBuffer(const RenderPassGuid& renderPassGuid)
{
    Vector<VkDescriptorImageInfo>	texturePropertiesBuffer;
    if (textureSystem.Texture3DList.empty())
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
        for (auto& texture : textureSystem.Texture3DList)
        {
            textureSystem.GetTexture3DPropertiesBuffer(texture, texturePropertiesBuffer);
        }
    }

    return texturePropertiesBuffer;
}

Vector<VkDescriptorImageInfo> RenderSystem::GetCubeMapTextureBuffer()
{
    Vector<VkDescriptorImageInfo>	texturePropertiesBuffer;
    for (auto& cubeMap : textureSystem.CubeMapTextureList)
    {

        texturePropertiesBuffer.emplace_back(VkDescriptorImageInfo
        {
            .sampler = cubeMap.textureSampler,
            .imageView = cubeMap.textureViewList.front(),
            .imageLayout = cubeMap.textureImageLayout
        });
    }
    return texturePropertiesBuffer;
}

Vector<VulkanRenderPass>& RenderSystem::RenderPassList()
{
    return RenderPassNodes;
}

VulkanRenderPass RenderSystem::FindRenderPass(const RenderPassGuid& renderPassGuid)
{
    auto it = GuidToRenderPassNodeIndex.find(renderPassGuid);
    uint32 index = it != GuidToRenderPassNodeIndex.end() ? it->second : UINT32_MAX;
    return RenderPassNodes[index];
}

const Vector<VulkanPipeline> RenderSystem::FindRenderPipelineList(const RenderPassGuid& renderPassGuid)
{
    return RenderPipelineMap.at(renderPassGuid);
}

VulkanPipeline RenderSystem::FindRenderPipeline(const RenderPassGuid& renderPassGuid, const VkGuid& pipelineGuid)
{
    Vector<VulkanPipeline> pipelineList = FindRenderPipelineList(renderPassGuid);
    for (const auto& pipeline : pipelineList)
    {
        if (pipeline.RenderPipelineId == pipelineGuid) return pipeline;
    }
    return VulkanPipeline();
}

uint32 RenderSystem::SampleRenderPassPixel(const TextureGuid& textureGuid, ivec2 mousePosition)
{
    Texture* texture = &textureSystem.FindRenderedTexture(textureGuid);
    if (!texture || texture->textureImage == VK_NULL_HANDLE)
    {
        std::cout << "[SamplePixel] Texture not found" << std::endl;
        return UINT32_MAX;
    }

    int x = std::clamp(mousePosition.x, 0, texture->width - 1);
    int y = std::clamp(mousePosition.y, 0, texture->height - 1);

    VkCommandBuffer cmd = vulkanSystem.BeginSingleUseCommand();

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
        .oldLayout = texture->textureImageLayout,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = texture->textureImage,
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
    };

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Create staging buffer (R32_UINT = 4 bytes per pixel)
    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(texture->width) * texture->height * 4;

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT
    };

    VmaAllocationCreateInfo allocInfo = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo allocOut = {};

    if (vmaCreateBuffer(bufferSystem.vmaAllocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAlloc, &allocOut) != VK_SUCCESS)
    {
        std::cout << "[SamplePixel] Failed to create staging buffer" << std::endl;
        vulkanSystem.EndSingleUseCommand(cmd);
        return UINT32_MAX;
    }

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
        .imageOffset = { 0, 0, 0 },
        .imageExtent = { static_cast<uint32>(texture->width), static_cast<uint32>(texture->height), 1 }
    };

    vkCmdCopyImageToBuffer(cmd, texture->textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);

    vulkanSystem.EndSingleUseCommand(cmd);
    vkDeviceWaitIdle(vulkanSystem.Device);

    const uint32* pData = static_cast<const uint32*>(allocOut.pMappedData);
    uint32 pickedId = pData[y * texture->width + x];

    vmaDestroyBuffer(bufferSystem.vmaAllocator, stagingBuffer, stagingAlloc);

    return pickedId;
}

void RenderSystem::AddRenderNode(RenderPassNode renderPassNode)
{
    RenderPassNodess.emplace_back(renderPassNode);
}

void RenderSystem::BeginRenderPass(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, uint mipLevel)
{
    const uint32 renderPassWidth = std::max(1, renderPass.RenderPassResolution.x >> mipLevel);
    const uint32 renderPassHeight = std::max(1, renderPass.RenderPassResolution.y >> mipLevel);
    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList[mipLevel],
        .renderArea = VkRect2D
        {
           .offset = VkOffset2D
            {
                .x = 0,
                .y = 0
            },
           .extent = VkExtent2D
            {
                .width = renderPassWidth,
                .height = renderPassHeight
            }
        },
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderSystem::BindViewPort(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, uint mipLevel)
{
    if (renderPass.UseDefaultRenderResolution)
    {
        return;
    }

    const uint32 renderPassWidth = std::max(1, renderPass.RenderPassResolution.x >> mipLevel);
    const uint32 renderPassHeight = std::max(1, renderPass.RenderPassResolution.y >> mipLevel);

    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderPassWidth),
        .height = static_cast<float>(renderPassHeight),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D rect2D = VkRect2D
    {
       .offset = VkOffset2D {.x = 0, .y = 0 },
       .extent = VkExtent2D {.width = renderPassWidth, .height = renderPassHeight }
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &rect2D);
}

void RenderSystem::BindPushConstants(VkCommandBuffer& commandBuffer, const VulkanPipeline& pipeline, const ShaderPushConstant& pushConstant, VkShaderStageFlags stages)
{
    vkCmdPushConstants(commandBuffer, pipeline.PipelineLayout, stages, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer.data());
}

void RenderSystem::BindRenderPassPipeline(VkCommandBuffer& commandBuffer, const VulkanPipeline& pipeline, uint32 firstSet)
{
    if (pipeline.Pipeline == nullptr)
    {
        std::cout << "Pipeline not set" << std::endl;
        return;
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, firstSet, pipeline.DescriptorSetList.size(), pipeline.DescriptorSetList.data(), 0, nullptr);
}

void RenderSystem::DrawVertexMesh(VkCommandBuffer& commandBuffer, uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance)
{
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void RenderSystem::DrawIndexedMesh(VkCommandBuffer& commandBuffer, VulkanDrawMessage& drawMessage)
{
    for (auto vertexDraw : drawMessage.VertexBufferList)
    {
        vkCmdBindVertexBuffers(commandBuffer, drawMessage.FirstVertexBinding, drawMessage.VertexBufferList.size() - drawMessage.FirstVertexBinding, &vertexDraw.vertexBuffer, &vertexDraw.offsets);
    }
    vkCmdBindIndexBuffer(commandBuffer, drawMessage.IndexBuffer, drawMessage.FirstIndex * sizeof(uint32_t), VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, drawMessage.IndexCount, drawMessage.InstanceCount, 0, drawMessage.VertexOffset, drawMessage.FirstInstance);
}

void RenderSystem::DrawIndexedMesh(VkCommandBuffer& commandBuffer, Vector<VulkanDrawMessage>& vulkanDrawMessageList)
{
    for (auto drawMessage : vulkanDrawMessageList)
    {
        for (auto vertexDraw : drawMessage.VertexBufferList)
        {
            vkCmdBindVertexBuffers(commandBuffer, drawMessage.FirstVertexBinding, drawMessage.VertexBufferList.size() - drawMessage.FirstVertexBinding, &vertexDraw.vertexBuffer, &vertexDraw.offsets);
        }
        vkCmdBindIndexBuffer(commandBuffer, drawMessage.IndexBuffer, drawMessage.FirstIndex * sizeof(uint32_t), VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, drawMessage.IndexCount, drawMessage.InstanceCount, 0, drawMessage.VertexOffset, drawMessage.FirstInstance);
    }
}

void RenderSystem::NextSubpass(VkCommandBuffer& commandBuffer)
{
    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderSystem::EndRenderPass(VkCommandBuffer& commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

void RenderSystem::Draw(VkCommandBuffer& commandBuffer)
{
    for (auto& renderPassNode : RenderPassNodess)
    {
        const VulkanRenderPass& renderPass = FindRenderPass(renderPassNode.RenderPassGuid);

        if (renderPassNode.PreRenderPassCmd) renderPassNode.PreRenderPassCmd(commandBuffer, renderPassNode);
        for (int x = 0; x <= renderPassNode.MipCount; x++)
        {
            bool firstSubPass = true;
            BeginRenderPass(commandBuffer, renderPass);
            BindViewPort(commandBuffer, renderPass, x);
            for (auto& subPass : renderPassNode.RenderPassDrawMessage)
            {
                if (!firstSubPass)
                {
                    NextSubpass(commandBuffer);
                    if (renderPassNode.PrepairSubpassCmd != nullptr) renderPassNode.PrepairSubpassCmd(commandBuffer, renderPassNode);
                }

                for (auto& renderPassLayer : subPass)
                {
                    Texture inputTexture;
                    if (!renderPassLayer.RenderPassInputs.empty()) inputTexture = textureSystem.FindRenderedTexture(renderPassLayer.RenderPassInputs[0].TextureGuid);

                    const VulkanPipeline& pipeline = FindRenderPipeline(renderPass.RenderPassId, renderPassLayer.PipelineGuid);
                    if (renderPassLayer.PreDrawLayerCmd) renderPassLayer.PreDrawLayerCmd(commandBuffer, renderPassLayer);
                    BindRenderPassPipeline(commandBuffer, pipeline);

                    if (renderPassLayer.UpdatePushConstantsCmd)
                    {
                        renderPassLayer.UpdatePushConstantsCmd(commandBuffer, renderPassLayer, ivec2(inputTexture.width), x);
                    }

                    if (renderPassLayer.PushConstant)
                    {
                        BindPushConstants(commandBuffer, pipeline, renderPassLayer.PushConstant.value());
                    }

                    if (renderPassLayer.IndexBuffer)
                    {
                        renderSystem.DrawIndexedMesh(commandBuffer, renderPassLayer);
                    }
                    else
                    {
                        renderSystem.DrawVertexMesh(commandBuffer, renderPassLayer.VertexCount, renderPassLayer.InstanceCount, renderPassLayer.FirstIndex, renderPassLayer.FirstInstance);
                    }
                    if (renderPassLayer.PostDrawLayerCmd) renderPassLayer.PostDrawLayerCmd(commandBuffer, renderPassLayer);
                }

                firstSubPass = false;
            }
            if (renderPassNode.PostRenderPassCmd) renderPassNode.PostRenderPassCmd(commandBuffer, renderPassNode);
            EndRenderPass(commandBuffer);
        }
    }
}
