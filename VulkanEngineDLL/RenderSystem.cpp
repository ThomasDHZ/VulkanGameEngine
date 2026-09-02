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
#include <ImGuiSystem.h>
#include <VulkanWindow.h>

RenderSystem& renderSystem = RenderSystem::Get();

void RenderSystem::Update(void* windowHandle, const float& deltaTime)
{
    RecreateSwapchain(windowHandle, deltaTime);
}

RenderPassGuid RenderSystem::LoadRenderPass(const String& jsonPath)
{
    RenderPassLoader renderPassLoader = fileSystem.LoadJsonFile(jsonPath).get<RenderPassLoader>();
    return LoadRenderPass(renderPassLoader);
}

RenderPassGuid RenderSystem::LoadRenderPass(RenderPassLoader& renderPassLoader)
{
    VulkanRenderPass vulkanRenderPass = VulkanRenderPass();
    vulkanRenderPass.LoadRenderPass(renderPassLoader);
    RenderPassMap[renderPassLoader.RenderPassId] = vulkanRenderPass;
    Texture depthTexture;
    Vector<Texture> renderedTextureList;
    VulkanTexture vulkanTexture = VulkanTexture();
    for (int x = 0; x < vulkanRenderPass.AttachmentList().size(); x++)
    {
        VulkanTexture attachment = vulkanRenderPass.AttachmentList()[x];
        ivec2 renderPassSize = ivec2(attachment.TextureSize().x, attachment.TextureSize().y);
        Texture texture = Texture
        {
           .textureGuid = renderPassLoader.AttachmentList[x].RenderedTextureId,
           .gpuTextureBufferIndex = memoryPoolSystem.AddToMemoryPool(attachment),
           .texture = attachment,
           .imGuiDescriptorSet = VK_NULL_HANDLE
        };

        SceneDataBuffer& sceneData = memoryPoolSystem.UpdateSceneDataBuffer();
        switch (renderPassLoader.AttachmentList[x].TextureUsageType)
        {
            case kUsageType_CubeMap:            sceneData.CubeMapId = texture.gpuTextureBufferIndex; break;
            case kUsageType_IrradianceTexture:  sceneData.IrradianceMapId = texture.gpuTextureBufferIndex; break;
            case kUsageType_PrefilterTexture:   sceneData.PrefilterMapId = texture.gpuTextureBufferIndex; break;
            case kUsageType_BRDFTexture:        sceneData.BRDFMapId = texture.gpuTextureBufferIndex; break;
            default: break;
        }
        renderedTextureList.emplace_back(texture);
    }
    if (!renderedTextureList.empty()) AddRenderedTexture(vulkanRenderPass.RenderPassId(), renderedTextureList);

    for (auto& shader : renderPassLoader.ShaderList)
    {
        LoadShader(shader);
    }
    for (auto& pipelinePackage : renderPassLoader.PipelinePackageList)
    {
        if (RenderPipelinePackageExists(pipelinePackage.PipelinePackageId)) continue;
        RenderPipelinePackageMap[pipelinePackage.PipelinePackageId] = pipelinePackage;
    }
    for (auto& pipelineLoaderJsonPath : renderPassLoader.PipelineList)
    {
        VulkanPipelineLoader pipelineLoader = fileSystem.LoadJsonFile<VulkanPipelineLoader>(pipelineLoaderJsonPath);
        LoadPipeline(renderPassLoader, pipelineLoader);
    }
    return renderPassLoader.RenderPassId;
}

VkGuid RenderSystem::LoadShader(ShaderLoader& shaderLoader)
{
    if (VulkanShaderExists(shaderLoader.ShaderId)) return shaderLoader.ShaderId;

    Vector<byte> vertexShaderCode = fileSystem.LoadAssetFile(shaderLoader.ShaderFile.c_str());
    VulkanShader shader = VulkanShader(shaderLoader.ShaderId, vertexShaderCode);
    if (!shader.PushConstant().PushConstantName.empty() &&
        !shaderSystem.ShaderPushConstantExists(shader.PushConstant().PushConstantName))
    {
        shaderSystem.ShaderPushConstantMap[shader.PushConstant().PushConstantName] = shader.PushConstant();
    }
    RenderShaderMap[shader.ShaderId()] = shader;
}

VkGuid RenderSystem::LoadPipeline(RenderPassLoader& renderPassLoader, VulkanPipelineLoader& pipelineLoader)
{
    if (RenderPipelineExists(pipelineLoader.PipelineId)) return pipelineLoader.PipelineId;

    Vector<VkDescriptorImageInfo> descriptorSetInfoList;
    for (auto& attachment : RenderPassMap[renderPassLoader.RenderPassId].AttachmentList())
    {
        descriptorSetInfoList.emplace_back(VkDescriptorImageInfo
            {
                .sampler = attachment.m_textureSampler,
                .imageView = attachment.m_textureViewList.front(),
                .imageLayout = attachment.m_textureImageLayout
            });
    }

    auto CreateShaderList = [&](VulkanPipelineLoader& pipelineLoader)
        {
            Vector<VulkanShader> shaderList;
            for (auto& shaderId : pipelineLoader.ShaderIdList)
            {
                shaderList.emplace_back(RenderShaderMap[shaderId]);
            }
            return shaderList;
        };

    pipelineLoader.VulkanShaderList = CreateShaderList(pipelineLoader);
    pipelineLoader.RenderPassId = renderPassLoader.RenderPassId;
    pipelineLoader.RenderPass = RenderPassMap[renderPassLoader.RenderPassId].RenderPassHandle();
    pipelineLoader.RenderPassResolution = RenderPassMap[renderPassLoader.RenderPassId].RenderPassResolution();
    pipelineLoader.RenderPassInputTextures = descriptorSetInfoList;
    pipelineLoader.BindlessDescriptorSetIndex = pipelineLoader.BindlessDescriptorSetIndex;
    pipelineLoader.UseGlobalBindlessSet = renderPassLoader.UseGlobalBindlessSet;
    pipelineLoader.GlobalBindlessPool = memoryPoolSystem.GlobalBindlessPool;
    pipelineLoader.GlobalBindlessDescriptorSet = memoryPoolSystem.GlobalBindlessDescriptorSet;
    pipelineLoader.GlobalBindlessDescriptorSetLayout = memoryPoolSystem.GlobalBindlessDescriptorSetLayout;
    pipelineLoader.PipelineMultisampleStateCreateInfo.rasterizationSamples = RenderPassMap[renderPassLoader.RenderPassId].SampleCount();
    pipelineLoader.PipelineMultisampleStateCreateInfo.sampleShadingEnable = (RenderPassMap[renderPassLoader.RenderPassId].SampleCount() > VK_SAMPLE_COUNT_1_BIT);

    VulkanPipeline vulkanPipeline(pipelineLoader);
    RenderPipelineMap[vulkanPipeline.PipelineId()] = vulkanPipeline;
    RenderPassMap[renderPassLoader.RenderPassId].AddRenderPipeline(vulkanPipeline.PipelineId());
    return vulkanPipeline.PipelineId();
}

void RenderSystem::RecreateSwapchain(void* windowHandle, const float& deltaTime)
{
    if (!vulkan.WasFramebufferResized()) return;

    ivec2 size;
    if (vulkan.CustomSurface())
    {
        size = vulkan.WindowResolution();
    }
    else
    {
        size = vulkanWindow.GetFramebufferSize();
        while (size.x == 0 || size.y == 0)
        {
            glfwWaitEvents();
            size = vulkanWindow.GetFramebufferSize();
        }
    }
    if (size.x == 0 || size.y == 0) return;

    vkDeviceWaitIdle(vulkan.LogicalDevice());
    vulkan.Swapchain().RebuildSwapChain(windowHandle);
    for (auto& [id, renderPass] : RenderPassMap)
    {
        const ivec2 resolutionBeforeRebuild = renderPass.RenderPassResolution();
        renderPass.RebuildRenderPass();

        auto attachmentIt = RenderAttachmentMap.find(renderPass.RenderPassId());
        if (attachmentIt == RenderAttachmentMap.end())
            continue;

        const bool attachmentsRebuilt = renderPass.RenderPassResolution() != resolutionBeforeRebuild ||
            (!renderPass.AttachmentList().empty() &&
             (renderPass.AttachmentList()[0].TextureSize().x != resolutionBeforeRebuild.x ||
              renderPass.AttachmentList()[0].TextureSize().y != resolutionBeforeRebuild.y));

        if (!attachmentsRebuilt)
            continue;

        Vector<Texture>& renderedTextures = attachmentIt->second;
        const size_t attachmentCount = std::min(renderedTextures.size(), renderPass.AttachmentList().size());
        for (size_t x = 0; x < attachmentCount; x++)
        {
            renderedTextures[x].texture = renderPass.AttachmentList()[x];
            const uint32 binding = renderedTextures[x].texture.m_textureType == TextureTypeEnum::kTextureType_CubeMap ? memoryPoolSystem.CubeMapDescriptorBinding: memoryPoolSystem.Texture2DBinding;
            memoryPoolSystem.UpdateTextureDescriptorSet(renderedTextures[x].gpuTextureBufferIndex, renderedTextures[x].texture, binding);
        }
    }
    imGuiSystem.RebuildSwapChain();
    vulkan.ResetFramebufferResized();
}

void RenderSystem::PresentToSwapChain(VkCommandBuffer& commandBuffer, const VkGuid& renderPassTextureGuid)
{
    Texture srcTexture = renderSystem.FindRenderPassAttachment(renderPassTextureGuid);
    VkImage srcImage = srcTexture.texture.TextureImage();
    VkImage dstImage = vulkan.Swapchain().SwapChainImages()[vulkan.Swapchain().ImageIndex()];

    Vector<VkImageMemoryBarrier> barriers = Vector<VkImageMemoryBarrier>
    {
        VkImageMemoryBarrier
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = srcImage,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        },
        VkImageMemoryBarrier
        {
             .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = dstImage,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        }
    };
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, barriers.size(), barriers.data());

    VkImageBlit blitRegion
    {
        .srcSubresource =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .srcOffsets =
        {
            VkOffset3D
            {
                .x = 0,
                .y = 0,
                .z = 0
            },
            VkOffset3D
            {
                .x = srcTexture.texture.TextureSize().x,
                .y = srcTexture.texture.TextureSize().y,
                .z = 1
            }
        },
        .dstSubresource =
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .dstOffsets =
        {
            VkOffset3D
            {
                .x = 0,
                .y = 0,
                .z = 0
            },
            VkOffset3D
            {
                .x = static_cast<int>(vulkan.SwapChainResolution().width),
                .y = static_cast<int>(vulkan.SwapChainResolution().height),
                .z = 1
            }
        }
    };
    vkCmdBlitImage(commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_LINEAR);

    VkImageMemoryBarrier presentBarrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = dstImage,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentBarrier);
}

void RenderSystem::BindPushConstants(VkCommandBuffer& commandBuffer, VulkanDrawMessage& drawMessage, uint32 drawIndex, uint32 mip, uint32 mipCount, VkShaderStageFlags stages)
{
    if (drawMessage.PushConstant.has_value())
    {
        const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(drawMessage.RenderPassGuid);
        VulkanPipelinePackage pipelinePackage = FindPipelinePackage(drawMessage.PipelinePackageGuid);
        VulkanPipeline pipeline = FindRenderPipeline(pipelinePackage.PipelineMap[PipelineTypeEnum::DefaultPipeline]);
        if (FindPipelinePackageByPipelineType(pipelinePackage.PipelinePackageId, PipelineTypeEnum::WireFramePipeline))
        {
            pipeline = FindRenderPipeline(pipelinePackage.PipelineMap[PipelineTypeEnum::WireFramePipeline]);
        }
        PushConstantContext pushConstantContext = PushConstantContext
        {
            .RenderPassGuid = drawMessage.RenderPassGuid,
            .MeshId = drawMessage.DrawMeshList[drawIndex].MeshId,
            .DrawIndex = static_cast<uint32>(drawIndex),
            .MipLevel = mip,
            .MipCount = mipCount,
            .RenderPassResolution = renderPass.RenderPassResolution()
        };

        ShaderPushConstant shaderPushConstant = shaderSystem.FindShaderPushConstant(drawMessage.PushConstant.value());
        pushConstantRegistry.ApplyPushConstantRules(shaderPushConstant, pushConstantContext);
        vkCmdPushConstants(commandBuffer, pipeline.PipelineLayout(), stages, 0, shaderPushConstant.PushConstantSize, shaderPushConstant.PushConstantBuffer.data());
    }
}

void RenderSystem::Draw(VkCommandBuffer& commandBuffer, Vector<RenderPassNode>& renderPassNodeList)
{
    for (auto& renderPassNode : renderPassNodeList)
    {
        VulkanRenderPass renderPass = FindRenderPass(renderPassNode.RenderPassGuid);

        uint32 mipCount = std::max(1u, renderPassNode.MipCount);
        if (renderPassNode.PreRenderPassCmd) renderPassNode.PreRenderPassCmd(commandBuffer, renderPassNode);
        for (uint32 mip = 0; mip < mipCount; mip++)
        {
            uint subPassIndex = 0;
            renderPass.BeginRenderPass(commandBuffer, mip);
            renderPass.BindViewPort(commandBuffer, mip);
            for (auto& subPass : renderPassNode.SubPassDrawMessage)
            {
                if (subPassIndex != 0) renderPass.NextSubpass(commandBuffer);
                for (auto& renderPassLayer : subPass)
                {
                    Texture inputTexture;
                    VulkanPipelinePackage pipelinePackage = FindPipelinePackage(renderPassLayer.PipelinePackageGuid);
                    VulkanPipeline pipeline = FindRenderPipeline(pipelinePackage.PipelineMap[PipelineTypeEnum::DefaultPipeline]);
                 /*   if (FindPipelinePackageByPipelineType(pipelinePackage.PipelinePackageId, PipelineType::WireFramePipeline))
                    {
                        pipeline = FindRenderPipeline(pipelinePackage.PipelineMap[PipelineType::WireFramePipeline]);
                    }*/
                    renderPass.BindRenderPassPipeline(commandBuffer, pipeline, 0);

                    if (!renderPassLayer.RenderPassInputs.empty()) inputTexture = renderSystem.FindRenderPassAttachment(renderPassLayer.RenderPassInputs[0]);
                    if (renderPassLayer.PreDrawCmd) renderPassLayer.PreDrawCmd(commandBuffer, renderPassLayer);
                    if (renderPassLayer.OffScreenRenderPass && renderPassLayer.DrawMeshList.empty())
                    {
                        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
                    }
                    else
                    {
                        for (int x = 0; x < renderPassLayer.DrawMeshList.size(); x++)
                        {
                            BindPushConstants(commandBuffer, renderPassLayer, x, mip, mipCount);
                            renderPass.DrawMesh(commandBuffer, renderPassLayer.DrawMeshList[x]);
                        }
                    }
                    if (renderPassLayer.PostDrawCmd) renderPassLayer.PostDrawCmd(commandBuffer, renderPassLayer);
                }
                subPassIndex++;
            }
            if (renderPassNode.PostRenderPassCmd) renderPassNode.PostRenderPassCmd(commandBuffer, renderPassNode);
            renderPass.EndRenderPass(commandBuffer);
        }
    }
}

uint32 RenderSystem::SampleRenderPassPixel(const TextureGuid& textureGuid, ivec2 mousePosition)
{
    Texture* texture = &FindRenderPassAttachment(textureGuid);
    if (!texture || texture->texture.TextureImage() == VK_NULL_HANDLE)
        return 0;

    const int w = texture->texture.TextureSize().x;
    const int h = texture->texture.TextureSize().y;
    const int x = std::clamp(mousePosition.x, 0, w - 1);
    const int y = std::clamp(mousePosition.y, 0, h - 1);

    const VkImageLayout oldLayout = texture->texture.TextureImageLayout();

    // 1) GPU must finish the pass that WROTE this image
    vkDeviceWaitIdle(vulkan.LogicalDevice());

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingAlloc = VK_NULL_HANDLE;
    VmaAllocationInfo allocOut{};

    VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = 4;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT
        | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    if (vmaCreateBuffer(bufferSystem.VmaAllocatorHandle(), &bufferInfo, &allocInfo,
        &stagingBuffer, &stagingAlloc, &allocOut) != VK_SUCCESS)
        return 0;

    VkCommandBuffer cmd = vulkan.CommandBuffer().BeginSingleUseCommand();

    VkImageMemoryBarrier toSrc{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    toSrc.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_SHADER_WRITE_BIT
        | VK_ACCESS_SHADER_READ_BIT;
    toSrc.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    toSrc.oldLayout = oldLayout;
    toSrc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    toSrc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toSrc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toSrc.image = texture->texture.TextureImage();
    toSrc.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &toSrc);

    VkBufferImageCopy region{};
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageOffset = { x, y, 0 };
    region.imageExtent = { 1, 1, 1 };

    vkCmdCopyImageToBuffer(cmd,
        texture->texture.TextureImage(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        stagingBuffer, 1, &region);

    VkImageMemoryBarrier toOld = toSrc;
    toOld.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    toOld.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_SHADER_READ_BIT;
    toOld.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    toOld.newLayout = oldLayout;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &toOld);

    vulkan.CommandBuffer().EndSingleUseCommand(cmd);
    vkDeviceWaitIdle(vulkan.LogicalDevice());

    vmaInvalidateAllocation(bufferSystem.VmaAllocatorHandle(),
        stagingAlloc, 0, VK_WHOLE_SIZE);

    const uint32 pickedId = *static_cast<const uint32*>(allocOut.pMappedData);
    vmaDestroyBuffer(bufferSystem.VmaAllocatorHandle(), stagingBuffer, stagingAlloc);
    return pickedId;
}

const VulkanRenderPass& RenderSystem::FindRenderPass(const RenderPassGuid& renderPassGuid)
{
    return RenderPassMap[renderPassGuid];
}

const VulkanPipelinePackage& RenderSystem::FindPipelinePackage(const VkGuid& pipelinePackageGuid)
{
    return RenderPipelinePackageMap[pipelinePackageGuid];
}

const VulkanPipeline& RenderSystem::FindRenderPipeline(const VkGuid& pipelineGuid)
{
    return RenderPipelineMap[pipelineGuid];
}

const VulkanShader& RenderSystem::FindVulkanShader(const VkGuid& shaderGuid)
{
    return RenderShaderMap[shaderGuid];
}

bool RenderSystem::FindPipelinePackageByPipelineType(const VkGuid& pipelinePackageGuid, PipelineTypeEnum pipelineType)
{
    return RenderPipelinePackageMap[pipelinePackageGuid].PipelineMap.contains(pipelineType);
}

void RenderSystem::AddRenderedTexture(RenderPassGuid renderPassGuid, Vector<Texture>& renderedTextureList)
{
    RenderAttachmentMap[renderPassGuid] = renderedTextureList;
}

Texture& RenderSystem::FindRenderPassAttachment(const TextureGuid& textureGuid)
{
    for (auto& pair : RenderAttachmentMap)
    {
        auto& textureList = pair.second;
        auto it = std::find_if(textureList.begin(), textureList.end(),
            [&textureGuid](const Texture& texture)
            {
                return texture.textureGuid == textureGuid;
            });
        if (it != textureList.end())
            return *it;
    }
    throw std::out_of_range("Texture with Id: " + textureGuid.ToString() + " not found");
}

Vector<Texture>& RenderSystem::FindRenderPassAttachmentList(const RenderPassGuid& renderPassGuid)
{
    return RenderAttachmentMap.at(renderPassGuid);
}

bool RenderSystem::RenderPassExists(const VkGuid& renderPassId)
{
    return RenderPassMap.contains(renderPassId);
}

bool RenderSystem::RenderPipelinePackageExists(const VkGuid& pipelinePackageId)
{
    return RenderPipelinePackageMap.contains(pipelinePackageId);
}

bool RenderSystem::RenderPipelineExists(const VkGuid& pipelineId)
{
    return RenderPipelineMap.contains(pipelineId);
}

bool RenderSystem::VulkanShaderExists(const VkGuid& vulkanShaderId)
{
    return RenderShaderMap.contains(vulkanShaderId);
}

void RenderSystem::Destroy()
{
    for (auto& shader : RenderShaderMap)
    {
        shader.second.Destroy();
    }
    for (auto& renderPipeline : RenderPipelineMap)
    {
        renderPipeline.second.Destroy();
    }
    for (auto& renderPass : RenderPassMap)
    {
        renderPass.second.Destroy();
    }
}