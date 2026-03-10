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
        RecreateSwapchain(windowHandle, deltaTime);
        vulkanSystem.RebuildRendererFlag = false;
    }
}

RenderPassGuid RenderSystem::LoadRenderPass(LevelGuid& levelGuid, const String& jsonPath, bool useGlobalDescriptorSet)
{
    RenderPassLoader renderPassLoader = fileSystem.LoadJsonFile(jsonPath).get<RenderPassLoader>();
    renderSystem.RenderPassLoaderJsonMap[renderPassLoader.RenderPassId] = jsonPath;
    return LoadRenderPass(levelGuid, renderPassLoader, useGlobalDescriptorSet);
}

RenderPassGuid RenderSystem::LoadRenderPass(LevelGuid& levelGuid, RenderPassLoader& renderPassLoader, bool useGlobalDescriptorSet)
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
        .RenderPassResolution = renderPassLoader.UseDefaultSwapChainResolution ? ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height) : renderPassLoader.RenderPassResolution,
        .MaxPushConstantSize = 0,
        .UseDefaultSwapChainResolution = renderPassLoader.UseDefaultSwapChainResolution,
        .IsRenderedToSwapchain = renderPassLoader.IsRenderedToSwapchain,
        .UseCubeMapMultiView = renderPassLoader.UseCubeMapMultiView,
        .IsCubeMapRenderPass = renderPassLoader.IsCubeMapRenderPass
    };

    static bool s_alreadyCreated = false;
    RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId] = renderPassLoader.RenderAttachmentList;
    BuildRenderPass(vulkanRenderPass, renderPassLoader, useGlobalDescriptorSet);
    BuildFrameBuffer(vulkanRenderPass);
    RenderPassMap[vulkanRenderPass.RenderPassId] = vulkanRenderPass;

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

        Vector<Vector<ShaderDescriptorBindingDLL>> descriptorSetLists;
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
    vulkanSystem.RebuildSwapChain(windowHandle);
    for (auto& renderPassPair : renderSystem.RenderPassMap)
    {
        VulkanRenderPass& renderPass = renderPassPair.second;
        RebuildSwapChain(renderPass);
    }
    // ImGui_RebuildSwapChain(renderer, imGuiRenderer);
}

void RenderSystem::RebuildSwapChain(VulkanRenderPass& vulkanRenderPass)
{
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, vulkanRenderPass.FrameBufferList.data(), vulkanSystem.SwapChainImageCount);
    if (vulkanRenderPass.IsRenderedToSwapchain)
    {
//vulkanRenderPass.RenderPassResolution = ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height);
        //DestoryRenderPassSwapChainTextures(renderedTextureList.data(), renderedTextureList.size(), depthTexture);
       // BuildRenderPassAttachmentTextures(vulkanRenderPass);
      //  BuildFrameBuffer(vulkanRenderPass);
    }
    else
    {
       // vulkanRenderPass.RenderPassResolution = vulkanRenderPass.UseDefaultSwapChainResolution ? ivec2(vulkanSystem.SwapChainResolution.width, vulkanSystem.SwapChainResolution.height) : vulkanRenderPass.RenderPassResolution;
      ///  BuildRenderPassAttachmentTextures(vulkanRenderPass);
       // BuildFrameBuffer(vulkanRenderPass);
    }
}

void RenderSystem::BuildRenderPass(VulkanRenderPass& vulkanRenderPass, const RenderPassLoader& renderPassJsonLoader, bool globalDescriptorSet)
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
            .inputAttachmentCount = static_cast<uint32_t>(inputAttachmentReferenceList[x].size()),
            .pInputAttachments = inputAttachmentReferenceList[x].empty() ? nullptr : inputAttachmentReferenceList[x].data(),
            .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferenceList[x].size()),
            .pColorAttachments = colorAttachmentReferenceList[x].empty() ? nullptr : colorAttachmentReferenceList[x].data(),
            .pResolveAttachments = resolveAttachmentReferenceList[x].empty() ? nullptr : resolveAttachmentReferenceList[x].data(),
            .pDepthStencilAttachment = useDepthReferences[x] ? &depthReferences[x] : nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
            });
    }

    Vector<VkAttachmentDescription> attachmentDescriptionList = BuildRenderPassAttachments(vulkanRenderPass, globalDescriptorSet);
    Vector<Texture> frameBufferTextureList = BuildRenderPassAttachmentTextures(vulkanRenderPass, globalDescriptorSet);

    VkRenderPassMultiviewCreateInfo multiviewCreateInfo{};
    if (renderPassJsonLoader.UseCubeMapMultiView)
    {
        const uint32_t viewMask = 0b00111111;
        const uint32_t correlationMask = 0b00111111;
        multiviewCreateInfo = VkRenderPassMultiviewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO,
            .subpassCount = 1,
            .pViewMasks = &viewMask,
            .correlationMaskCount = 1,
            .pCorrelationMasks = &correlationMask,
        };
    }

    Vector<VkSubpassDependency> subpassDependencies = renderPassJsonLoader.SubpassDependencyModelList;
    VkRenderPassCreateInfo renderPassInfo =
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = renderPassJsonLoader.UseCubeMapMultiView ? &multiviewCreateInfo : nullptr,
        .attachmentCount = static_cast<uint32_t>(attachmentDescriptionList.size()),
        .pAttachments = attachmentDescriptionList.data(),
        .subpassCount = static_cast<uint32_t>(subPassDescriptionList.size()),
        .pSubpasses = subPassDescriptionList.data(),
        .dependencyCount = static_cast<uint32_t>(subpassDependencies.size()),
        .pDependencies = subpassDependencies.data(),
    };
    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkanSystem.Device, &renderPassInfo, nullptr, &vulkanRenderPass.RenderPass));
}

Vector<VkAttachmentDescription> RenderSystem::BuildRenderPassAttachments(VulkanRenderPass& vulkanRenderPass, bool globalDescriptorSet)
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
            case RenderType_DepthBufferTexture:    initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;  break;
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

Vector<Texture> RenderSystem::BuildRenderPassAttachmentTextures(VulkanRenderPass& vulkanRenderPass, bool globalDescriptorSet)
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
    if (vulkanRenderPass.IsRenderedToSwapchain)
    {
        vulkanRenderPass.FrameBufferList.resize(vulkanSystem.SwapChainImageCount);
        for (size_t i = 0; i < vulkanSystem.SwapChainImageCount; ++i)
        {
            std::vector<VkImageView> attachments{ vulkanSystem.SwapChainImageViews[i] };
            VkFramebufferCreateInfo info = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = vulkanRenderPass.RenderPass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = static_cast<uint32_t>(vulkanRenderPass.RenderPassResolution.x),
                .height = static_cast<uint32_t>(vulkanRenderPass.RenderPassResolution.y),
                .layers = 1
            };
            VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkanSystem.Device, &info, nullptr, &vulkanRenderPass.FrameBufferList[i]));
        }
    }
    else if (!frameBufferAttachment.empty() &&
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
    vulkanSystem.DestroyFrameBuffers(vulkanSystem.Device, &renderPass.FrameBufferList[0], vulkanSystem.SwapChainImageCount);

    renderPass.RenderPassId = VkGuid();
    renderPass.SubPassCount = UINT32_MAX;
    renderPass.SampleCount = VK_SAMPLE_COUNT_1_BIT;
    renderPass.RenderPass = VK_NULL_HANDLE;
    renderPass.InputTextureIdList.clear();
    renderPass.FrameBufferList.clear();
    renderPass.ClearValueList.clear();
    renderPass.RenderPassResolution = ivec2();
    renderPass.MaxPushConstantSize = 0;
    renderPass.UseDefaultSwapChainResolution = false;
    renderPass.IsRenderedToSwapchain = false;
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
    for (auto& renderPass : renderSystem.RenderPassMap)
    {
        DestroyRenderPass(renderPass.second);
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

VulkanRenderPass RenderSystem::FindRenderPass(const RenderPassGuid& renderPassGuid)
{
    return RenderPassMap.at(renderPassGuid);
}

const Vector<VulkanPipeline> RenderSystem::FindRenderPipelineList(const RenderPassGuid& renderPassGuid)
{
    return RenderPipelineMap.at(renderPassGuid);
}