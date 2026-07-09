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
#include "PushConstantRegistry.h"

RenderSystem& renderSystem = RenderSystem::Get();

void RenderSystem::Update(void* windowHandle, const float& deltaTime)
{
    //if (vulkan.RebuildRendererFlag)
    //{
    //    RecreateSwapchain(windowHandle, deltaTime);
    //    vulkan.RebuildRendererFlag = false;
    //}
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
        .RenderPassResolution = ivec2(INT32_MAX, INT32_MAX) == renderPassLoader.RenderPassResolution  || ivec2(0) == renderPassLoader.RenderPassResolution ? vulkan.RenderPassResolution() : renderPassLoader.RenderPassResolution,
        .RenderPass = VK_NULL_HANDLE,
        .FrameBufferList = Vector<VkFramebuffer>(),
        .VulkanSubPassList = Vector<Vector<VulkanSubPass>>(),
        .ClearValueList = renderPassLoader.ClearValueList,
        .SampleCount = renderPassLoader.SampleCount >= vulkan.MaxSampleCount() ? vulkan.MaxSampleCount() : renderPassLoader.SampleCount,
        .UseCubeMapMultiView = renderPassLoader.UseCubeMapMultiView,
        .IsCubeMapRenderPass = renderPassLoader.IsCubeMapRenderPass
    };
    RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId] = renderPassLoader.RenderAttachmentList; 

    BuildRenderPass(vulkanRenderPass, renderPassLoader);
    for (auto& renderPass : renderPassLoader.SubPassList)
    {
        Vector<VulkanSubPass> subPassList;
        for (auto& subPass : renderPass)
        {
            BuildPipelines(vulkanRenderPass, subPass, renderPassLoader.UseGlobalBindlessSet);
            subPassList.emplace_back(BuildSubpasses(renderPassLoader.RenderPassId, subPass));
        }
        vulkanRenderPass.VulkanSubPassList.emplace_back(subPassList);
    }
    BuildFrameBuffer(vulkanRenderPass);

    RenderPassMap[renderPassLoader.RenderPassId] = vulkanRenderPass;
    return renderPassLoader.RenderPassId;
}

void RenderSystem::RecreateSwapchain(void* windowHandle, const float& deltaTime)
{
    //vkDeviceWaitIdle(vulkan.LogicalDevice());
    //for (auto& renderPass : renderSystem.RenderPassList()) vulkanSystem.DestroyFrameBuffers(vulkan.LogicalDevice(), renderPass.FrameBufferList);
    //vulkanSystem.DestroySwapChainImageView(vulkan.LogicalDevice(), vulkanSystem.SwapChainImageViews);
    //vulkanSystem.DestroySwapChain(vulkan.LogicalDevice(), &vulkanSystem.Swapchain);

    //vulkanSystem.SetUpSwapChain(windowHandle);
    //for (auto& renderPass : renderSystem.RenderPassList())
    //{
    //    BuildFrameBuffer(renderPass);
    //}
    // ImGui_RebuildSwapChain(renderer, imGuiRenderer);
}

void RenderSystem::BuildRenderPass(VulkanRenderPass& vulkanRenderPass, const RenderPassLoader& renderPassJsonLoader)
{
    VkAttachmentReference unusedRef = {};
    VkAttachmentReference depthReference = VkAttachmentReference();
    Vector<bool> useDepthReferences(renderPassJsonLoader.SubPassList.size(), false);
    Vector<VkAttachmentReference> depthReferences(renderPassJsonLoader.SubPassList.size());
    Vector<VkSubpassDescription> subPassDescriptionList;
    Vector<Vector<VkAttachmentReference>> inputAttachmentReferenceList(renderPassJsonLoader.SubPassList.size());
    Vector<Vector<VkAttachmentReference>> colorAttachmentReferenceList(renderPassJsonLoader.SubPassList.size());
    Vector<Vector<VkAttachmentReference>> resolveAttachmentReferenceList(renderPassJsonLoader.SubPassList.size());
    Vector<Vector<VkSubpassDescription>> preserveAttachmentReferenceList(renderPassJsonLoader.SubPassList.size());
    Vector<RenderPassAttachmentTexture> renderPassAttachmentTextureInfoMap = RenderPassAttachmentTextureInfoMap[vulkanRenderPass.RenderPassId];
    for (int x = 0; x < renderPassJsonLoader.SubPassList.size(); x++)
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
                bool is_depth = (renderAttachment.Format >= VK_FORMAT_D16_UNORM && renderAttachment.Format <= VK_FORMAT_D32_SFLOAT_S8_UINT);
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

    Vector<VkSubpassDependency> subpassDependencies = renderPassJsonLoader.SubpassDependencyList;
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
    VULKAN_THROW_IF_FAIL(vkCreateRenderPass(vulkan.LogicalDevice(), &renderPassInfo, nullptr, &vulkanRenderPass.RenderPass));
}

VulkanSubPass RenderSystem::BuildSubpasses(VkGuid& renderPassId, const VulkanSubPassLoader& subPassLoader)
{
    return VulkanSubPass
    {
        .RenderPassGuid = renderPassId,
        .PipelineGuid = fileSystem.LoadJsonFile(subPassLoader.Pipeline.c_str()).get<RenderPipelineLoader>().PipelineId,
        .MeshType = subPassLoader.MeshType,
        .ShaderPushConstant = subPassLoader.ShaderPushConstant,
        .InputTextureList = subPassLoader.InputTextureList,
        .OutputTextureList = subPassLoader.OutputTextureList,
        .OffScreenFrameBuffer = subPassLoader.OffScreenRenderPass,
    };
}

void RenderSystem::BuildPipelines(VulkanRenderPass& renderPass, const VulkanSubPassLoader& subPassLoader, bool useGlobalBindlessSet)
{
    nlohmann::json pipelineJson = fileSystem.LoadJsonFile(subPassLoader.Pipeline.c_str());
    RenderPipelineLoader renderPipelineLoader = pipelineJson.get<RenderPipelineLoader>();
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = renderPass.SampleCount;
    renderPipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = (renderPass.SampleCount > VK_SAMPLE_COUNT_1_BIT);
    renderPipelineLoader.RenderPassId = renderPass.RenderPassId;
    renderPipelineLoader.RenderPass = renderPass.RenderPass;
    renderPipelineLoader.RenderPassResolution = renderPass.RenderPassResolution;
    renderPipelineLoader.UseGlobalBindlessSet = useGlobalBindlessSet;
    renderPipelineLoader.GlobalBindlessPool = memoryPoolSystem.GlobalBindlessPool;
    renderPipelineLoader.GlobalBindlessDescriptorSet = memoryPoolSystem.GlobalBindlessDescriptorSet;
    renderPipelineLoader.GlobalBindlessDescriptorSetLayout = memoryPoolSystem.GlobalBindlessDescriptorSetLayout;
    renderPipelineLoader.RenderPassInputTextures = memoryPoolSystem.GetSubPassInputTextureDescriptor(renderPass.RenderPassId);

    ShaderLoader vertexShaderLoader = pipelineJson["ShaderList"][0].get<ShaderLoader>();
    ShaderLoader pixelShaderLoader = pipelineJson["ShaderList"][1].get<ShaderLoader>();

    Vector<byte> vertexShaderCode = fileSystem.LoadAssetFile(vertexShaderLoader.ShaderFile.c_str());
    Vector<byte> pixelShaderCode = fileSystem.LoadAssetFile(pixelShaderLoader.ShaderFile.c_str());

    VulkanShader renderVertexShader = VulkanShader(vertexShaderCode);
    VulkanShader renderPixelShader = VulkanShader(pixelShaderCode);

    
    if (!renderVertexShader.PushConstant().PushConstantName.empty() &&
        !shaderSystem.ShaderPushConstantExists(renderVertexShader.PushConstant().PushConstantName))
    {
        shaderSystem.ShaderPushConstantMap[renderVertexShader.PushConstant().PushConstantName] = renderVertexShader.PushConstant();
    }
    if (!renderPixelShader.PushConstant().PushConstantName.empty() &&
        !shaderSystem.ShaderPushConstantExists(renderPixelShader.PushConstant().PushConstantName))
    {
        shaderSystem.ShaderPushConstantMap[renderPixelShader.PushConstant().PushConstantName] = renderPixelShader.PushConstant();
    }

    renderPipelineLoader.VulkanShaderList = Vector<VulkanShader>
    {
        renderVertexShader,
        renderPixelShader
    };

    VulkanPipeline pipeline;
    pipeline.BuildPipelines(renderPipelineLoader);
    RenderPipelineMap[renderPipelineLoader.PipelineId] = pipeline;
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
        switch (renderAttachment.TextureUsageType)
        {
            case kUsageType_SwapChainTexture:      initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;         finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                  break;
            case kUsageType_OffscreenColorTexture: initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_DepthBufferTexture:    initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;  break;
            case kUsageType_GBufferTexture:        initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_IrradianceTexture:     initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_PrefilterTexture:      initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;         finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_CubeMap:               initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            case kUsageType_BRDFTexture:           initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                        finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;         break;
            default: throw std::runtime_error("Unknown TextureUsageType");
        }

        attachmentDescriptionList.emplace_back(VkAttachmentDescription
            {
            .format = renderAttachment.Format,
            .samples = vulkanRenderPass.SampleCount >= vulkan.MaxSampleCount() ? vulkan.MaxSampleCount() : vulkanRenderPass.SampleCount,
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
        Texture texture = textureSystem.CreateRenderPassTexture(vulkanRenderPass, x, renderPassAttachmentTextureInfoList[x].TextureType);
        renderedTextureList.emplace_back(texture);
        frameBufferTextureList.emplace_back(texture);

        if (texture.textureType == TextureTypeEnum::kTextureType_DepthTexture)
        {
            depthTexture = texture;
        }

        if (!renderSystem.UsingMaterialBaker)
        {
            if (texture.textureType == TextureTypeEnum::kTextureType_CubeMap)
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
    Vector<Texture> frameBufferAttachment = textureSystem.FindRenderedTextureList(vulkanRenderPass.RenderPassId);
    if (frameBufferAttachment.empty()) return;

    const Texture& firstTex = frameBufferAttachment[0];
    bool isCubeMap = (firstTex.textureType == TextureTypeEnum::kTextureType_CubeMap);
    if (isCubeMap && !firstTex.textureViewList.empty())
    {
        uint32 mipLevels = firstTex.mipMapLevels;
        uint32 baseSize = firstTex.width;
        vulkanRenderPass.FrameBufferList.resize(mipLevels);
        for (uint32_t mip = 0; mip < mipLevels; ++mip)
        {
            uint32 mipWidth = std::max(1u, baseSize >> mip);
            uint32 mipHeight = std::max(1u, baseSize >> mip);
            if (mip >= firstTex.textureViewList.size())
            {
                std::cerr << "Error: Missing mip view " << mip << " for cubemap\n";
                break;
            }

            Vector<VkImageView> attachments{ firstTex.textureViewList[mip] };
            VkFramebufferCreateInfo info
            {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = vulkanRenderPass.RenderPass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = mipWidth,
                .height = mipHeight,
                .layers = 1u 
            };
            VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkan.LogicalDevice(), &info, nullptr, &vulkanRenderPass.FrameBufferList[mip]));
        }
    }
    else
    {
        vulkanRenderPass.FrameBufferList.resize(1);

        Vector<VkImageView> attachments;
        attachments.reserve(frameBufferAttachment.size());
        for (const auto& tex : frameBufferAttachment)
        {
            if (!tex.textureViewList.empty()) attachments.push_back(tex.textureViewList.front());
        }

        VkFramebufferCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = vulkanRenderPass.RenderPass,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = static_cast<uint32_t>(vulkanRenderPass.RenderPassResolution.x),
            .height = static_cast<uint32_t>(vulkanRenderPass.RenderPassResolution.y),
            .layers = 1
        };
        VULKAN_THROW_IF_FAIL(vkCreateFramebuffer(vulkan.LogicalDevice(), &info, nullptr, &vulkanRenderPass.FrameBufferList[0]));
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
    //vulkanSystem.DestroyRenderPass(vulkan.LogicalDevice(), &renderPass.RenderPass);
//    vulkanSystem.DestroyCommandBuffers(vulkan.LogicalDevice(), &vulkanSystem.CommandPool, &renderPass.com, 1);
   // vulkanSystem.DestroyFrameBuffers(vulkan.LogicalDevice(), renderPass.FrameBufferList);
    renderPass = VulkanRenderPass();
}

void RenderSystem::Destroy()
{
    DestroyRenderPipelines();
    DestroyRenderPasses();
}

void RenderSystem::DestroyRenderPasses()
{/*
    for (auto& renderPass : renderSystem.RenderPassList())
    {
        DestroyRenderPass(renderPass);
    }*/
    //renderSystem.RenderPassMap.clear();
}

void RenderSystem::DestroyRenderPipelines()
{
    //for (auto& renderPipelineList : renderSystem.RenderPipelineMap)
    //{
    //    for (auto& renderPipeline : renderPipelineList.second)
    //    {
    //        DestroyPipeline(renderPipeline);
    //    }
    //}
    //renderSystem.RenderPipelineMap.clear();
}

void RenderSystem::DestroyPipeline(VulkanPipeline& vulkanPipeline)
{
    //vulkanPipeline.RenderPipelineId = VkGuid();
    //vulkanSystem.DestroyPipeline(vulkan.LogicalDevice(), &vulkanPipeline.Pipeline);
    //vulkanSystem.DestroyPipelineLayout(vulkan.LogicalDevice(), &vulkanPipeline.PipelineLayout);
    //vulkanSystem.DestroyPipelineCache(vulkan.LogicalDevice(), &vulkanPipeline.PipelineCache);
}

void RenderSystem::DestroyFrameBuffers(Vector<VkFramebuffer>& frameBufferList)
{
    //vulkanSystem.DestroyFrameBuffers(vulkan.LogicalDevice(), frameBufferList);
}

void RenderSystem::DestroyCommandBuffers(Vector<VkCommandBuffer>& commandBuffer)
{
 //   vulkanSystem.DestroyCommandBuffers(vulkan.LogicalDevice(), &vulkanSystem.CommandPool, commandBuffer);
}

void RenderSystem::DestroyBuffer(VkBuffer& buffer)
{
   //vulkanSystem.DestroyBuffer(vulkan.LogicalDevice(), &buffer);
}

Vector<VkDescriptorImageInfo> RenderSystem::GetTexturePropertiesBuffer(const RenderPassGuid& renderPassGuid)
{
    Vector<Texture> textureList;
    const VulkanRenderPass& renderPass = FindRenderPass(renderPassGuid);
    for (auto& subpass : renderPass.VulkanSubPassList)
    {
        for (auto& renderPass : subpass)
        {
            for (auto& inputTexture : renderPass.InputTextureList)
            {
                Texture texture = textureSystem.FindTexture(inputTexture);
                if (texture.textureImageLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                {
                    continue;
                }
                textureList.emplace_back(texture);
            }
        }
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
        if (vkCreateSampler(vulkan.LogicalDevice(), &NullSamplerInfo, nullptr, &nullSampler))
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
        if (vkCreateSampler(vulkan.LogicalDevice(), &NullSamplerInfo, nullptr, &nullSampler))
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

const VulkanRenderPass& RenderSystem::FindRenderPass(const RenderPassGuid& renderPassGuid)
{
    auto it = RenderPassMap.find(renderPassGuid);
    if (it == RenderPassMap.end())
    {
        throw std::runtime_error("RenderPass not found: " + renderPassGuid.ToString());
    }
    return it->second;
}

//const Vector<VulkanPipeline> RenderSystem::FindRenderPipelineList(const RenderPassGuid& renderPassGuid)
//{
//    return RenderPipelineMap.at(renderPassGuid);
//}

const VulkanPipeline& RenderSystem::FindRenderPipeline(const VkGuid& pipelineGuid)
{
    auto it = RenderPipelineMap.find(pipelineGuid);
    if (it == RenderPipelineMap.end())
    {
        throw std::runtime_error("Pipeline not found: " + pipelineGuid.ToString());
    }
    return it->second;
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

    VkCommandBuffer cmd = vulkan.CommandBuffer().BeginSingleUseCommand();

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

    if (vmaCreateBuffer(bufferSystem.VmaAllocatorHandle(), &bufferInfo, &allocInfo, &stagingBuffer, &stagingAlloc, &allocOut) != VK_SUCCESS)
    {
        std::cout << "[SamplePixel] Failed to create staging buffer" << std::endl;
        vulkan.CommandBuffer().EndSingleUseCommand(cmd);
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

    vulkan.CommandBuffer().EndSingleUseCommand(cmd);
    vkDeviceWaitIdle(vulkan.LogicalDevice());

    const uint32* pData = static_cast<const uint32*>(allocOut.pMappedData);
    uint32 pickedId = pData[y * texture->width + x];

    vmaDestroyBuffer(bufferSystem.VmaAllocatorHandle(), stagingBuffer, stagingAlloc);

    return pickedId;
}

void RenderSystem::AddRenderNode(RenderPassNode renderPassNode)
{
    RenderPassNodeList.emplace_back(renderPassNode);
}

void RenderSystem::BeginRenderPass(VkCommandBuffer& commandBuffer, const VulkanRenderPass& renderPass, ivec2 renderPassResolution, uint mipLevel)
{

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
                .width = static_cast<uint32>(renderPassResolution.x),
                .height = static_cast<uint32>(renderPassResolution.y)
            }
        },
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueList.size()),
        .pClearValues = renderPass.ClearValueList.data()
    };
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
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
    if (renderPass.RenderPassResolution == ivec2(INT32_MAX, INT32_MAX))
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

void RenderSystem::BindViewPort(VkCommandBuffer& commandBuffer, ivec2 renderPassResolution, uint mipLevel)
{
    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderPassResolution.x),
        .height = static_cast<float>(renderPassResolution.y),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D rect2D = VkRect2D
    {
       .offset = VkOffset2D {.x = 0, .y = 0 },
       .extent = VkExtent2D {.width = static_cast<uint32>(renderPassResolution.x), .height = static_cast<uint32>(renderPassResolution.y) }
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &rect2D);
}

void RenderSystem::BindPushConstants(VkCommandBuffer& commandBuffer, VulkanDrawMessage& drawMessage, uint32 drawIndex, uint32 mip, uint32 mipCount, VkShaderStageFlags stages)
{
    if (drawMessage.PushConstant.has_value())
    {
        const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(drawMessage.RenderPassGuid);
        const VulkanPipeline&   pipeline = renderSystem.FindRenderPipeline(drawMessage.PipelineGuid);
        PushConstantContext pushConstantContext = PushConstantContext
        {
            .RenderPassGuid = drawMessage.RenderPassGuid,
            .MeshId = drawMessage.DrawMeshList[drawIndex].MeshId,
            .DrawIndex = static_cast<uint32>(drawIndex),
            .MipLevel = mip,
            .MipCount = mipCount,
            .RenderPassResolution = renderPass.RenderPassResolution
        };

        ShaderPushConstant shaderPushConstant = shaderSystem.FindShaderPushConstant(drawMessage.PushConstant.value());
        pushConstantRegistry.ApplyPushConstantRules(shaderPushConstant, pushConstantContext);
        vkCmdPushConstants(commandBuffer, pipeline.PipelineLayout(), stages, 0, shaderPushConstant.PushConstantSize, shaderPushConstant.PushConstantBuffer.data());
    }
}

void RenderSystem::BindRenderPassPipeline(VkCommandBuffer& commandBuffer, const VulkanPipeline& pipeline, uint32 firstSet)
{
    if (pipeline.Pipeline() == nullptr)
    {
        std::cout << "Pipeline not set" << std::endl;
        return;
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline());
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout(), firstSet, pipeline.DescriptorSetList().size(), pipeline.DescriptorSetList().data(), 0, nullptr);
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
    for (auto& renderPassNode : RenderPassNodeList)
    {
        const VulkanRenderPass& renderPass = FindRenderPass(renderPassNode.RenderPassGuid);

        uint32 mipCount = std::max(1u, renderPassNode.MipCount);
        if (renderPassNode.PreRenderPassCmd) renderPassNode.PreRenderPassCmd(commandBuffer, renderPassNode);
        for (uint32 mip = 0; mip < mipCount; mip++)
        {
            bool firstSubPass = true;
            const ivec2 renderPassResolution = ivec2(std::max(1, renderPass.RenderPassResolution.x >> mip),
                                                     std::max(1, renderPass.RenderPassResolution.y >> mip));

            BeginRenderPass(commandBuffer, renderPass, renderPassResolution, mip);
            BindViewPort(commandBuffer, renderPassResolution, mip);
            for (auto& subPass : renderPassNode.SubPassDrawMessage)
            {
                if (!firstSubPass)
                {
                    NextSubpass(commandBuffer);
                }

                for (auto& renderPassLayer : subPass)
                {
                    const VulkanPipeline& pipeline = FindRenderPipeline(renderPassLayer.PipelineGuid);

                    Texture inputTexture;
                    if (!renderPassLayer.RenderPassInputs.empty()) inputTexture = textureSystem.FindRenderedTexture(renderPassLayer.RenderPassInputs[0]);
                    if (renderPassLayer.PreDrawCmd)
                    {
                        renderPassLayer.PreDrawCmd(commandBuffer, renderPassLayer);
                    }

                    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline());
                    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout(), 0, pipeline.DescriptorSetList().size(), pipeline.DescriptorSetList().data(), 0, nullptr);
                    if (renderPassLayer.OffScreenRenderPass)
                    {
                        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
                    }
                    else
                    {
                        for (int x = 0; x < renderPassLayer.DrawMeshList.size(); x++)
                        {
                            const MeshDrawMessage mesh = renderPassLayer.DrawMeshList[x];
                            BindPushConstants(commandBuffer, renderPassLayer, x, mip, mipCount);
                            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.VertexBuffer, &mesh.VertexOffset);
                            if (mesh.IndexBuffer)
                            {
                                vkCmdBindIndexBuffer(commandBuffer, mesh.IndexBuffer, mesh.FirstIndex * sizeof(uint32), VK_INDEX_TYPE_UINT32);
                                vkCmdDrawIndexed(commandBuffer, mesh.IndexCount, mesh.InstanceCount, mesh.FirstIndex, 0, mesh.StartInstanceIndex);
                            }
                            else
                            {
                                vkCmdDraw(commandBuffer, mesh.VertexCount, mesh.InstanceCount, mesh.FirstVertex, mesh.StartInstanceIndex);
                            }
                        }
                    }

                    if (renderPassLayer.PostDrawCmd)
                    {
                        renderPassLayer.PostDrawCmd(commandBuffer, renderPassLayer);
                    }
                }

                firstSubPass = false;
            }
            if (renderPassNode.PostRenderPassCmd) renderPassNode.PostRenderPassCmd(commandBuffer, renderPassNode);
            EndRenderPass(commandBuffer);
        }
    }
}
