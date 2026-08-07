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

RenderPassGuid RenderSystem::LoadRenderPass(const String& jsonPath)
{
    RenderPassLoader renderPassLoader = fileSystem.LoadJsonFile(jsonPath).get<RenderPassLoader>();
    return LoadRenderPass(renderPassLoader);
}

RenderPassGuid RenderSystem::LoadRenderPass(RenderPassLoader& renderPassLoader)
{
    auto BuildPipelineLoader = [&](VulkanPipelinePackageLoader& pipelinePackageLoader, VulkanPipelineLoader& pipelineLoader)
        {
            for (auto& shader : pipelineLoader.ShaderLoaderList)
            {
                Vector<byte> vertexShaderCode = fileSystem.LoadAssetFile(shader.ShaderFile.c_str());
                VulkanShader shader = VulkanShader(vertexShaderCode);
                pipelineLoader.VulkanShaderList.emplace_back(shader);
            }
        };

    for (auto& pipelinePackageLoader : renderPassLoader.PipelinePackageList)
    {
        pipelinePackageLoader.GlobalBindlessPool = memoryPoolSystem.GlobalBindlessPool;
        pipelinePackageLoader.GlobalBindlessDescriptorSet = memoryPoolSystem.GlobalBindlessDescriptorSet;
        pipelinePackageLoader.GlobalBindlessDescriptorSetLayout = memoryPoolSystem.GlobalBindlessDescriptorSetLayout;
        for (auto& pipelineLoader : pipelinePackageLoader.PipelineMap)
        {
            BuildPipelineLoader(pipelinePackageLoader, pipelineLoader.second);
        }
    }

    VulkanRenderPass vulkanRenderPass = VulkanRenderPass();
    vulkanRenderPass.LoadRenderPass(renderPassLoader);
    RenderPassMap[renderPassLoader.RenderPassId] = vulkanRenderPass;

    for (auto& pipelinePackage : vulkanRenderPass.PipelinePackageList())
    {
        RenderPipelinePackageMap[pipelinePackage.PipelinePackageId] = pipelinePackage;
    }
    for (auto& vulkanPipeline : vulkanRenderPass.PipelineList())
    {
        RenderPipelineMap[vulkanPipeline.PipelineId()] = vulkanPipeline;
        for (auto& pushConstant : vulkanPipeline.ShaderPushConstantList())
        {
            if (!pushConstant.PushConstantName.empty() &&
                !shaderSystem.ShaderPushConstantExists(pushConstant.PushConstantName))
            {
                shaderSystem.ShaderPushConstantMap[pushConstant.PushConstantName] = pushConstant;
            }
        }
    }

    Texture depthTexture;
    Vector<Texture> renderedTextureList;
    Vector<Texture> frameBufferTextureList;
    VulkanTexture vulkanTexture = VulkanTexture();
    for (int x = 0; x < vulkanRenderPass.AttachmentList().size(); x++)
    {
        VulkanTexture attachment = vulkanRenderPass.AttachmentList()[x];
        ivec2 renderPassSize = ivec2(attachment.TextureSize().x, attachment.TextureSize().y);
        Texture texture = Texture
        {
           .textureGuid = renderPassLoader.AttachmentList[x].RenderedTextureId,
           .texture = attachment,
           .textureType = attachment.m_textureType,
           .textureUsageType = TextureUsageTypeEnum::kUsageType_Undefined,
           .imGuiDescriptorSet = VK_NULL_HANDLE
        };
        renderedTextureList.emplace_back(texture);
        frameBufferTextureList.emplace_back(texture);
        textureSystem.AddToMemoryPool(texture);
        if (texture.textureType == TextureTypeEnum::kTextureType_CubeMap) memoryPoolSystem.UpdateTextureDescriptorSet(texture, memoryPoolSystem.CubeMapDescriptorBinding);
        else memoryPoolSystem.UpdateTextureDescriptorSet(texture, memoryPoolSystem.Texture2DBinding);
    }
    if (!renderedTextureList.empty()) textureSystem.AddRenderedTexture(vulkanRenderPass.RenderPassId(), renderedTextureList);
    if (depthTexture.texture.TextureImage() != VK_NULL_HANDLE) textureSystem.AddDepthTexture(vulkanRenderPass.RenderPassId(), depthTexture);
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

const VulkanRenderPass& RenderSystem::FindRenderPass(const RenderPassGuid& renderPassGuid)
{
    auto it = RenderPassMap.find(renderPassGuid);
    if (it == RenderPassMap.end())
    {
        throw std::runtime_error("RenderPass not found: " + renderPassGuid.ToString());
    }
    return it->second;
}

const VulkanPipelinePackage& RenderSystem::FindPipelinePackage(const VkGuid& pipelinePackageGuid)
{
    auto it = RenderPipelinePackageMap.find(pipelinePackageGuid);
    if (it == RenderPipelinePackageMap.end())
    {
        throw std::runtime_error("PipelinePackage not found: " + pipelinePackageGuid.ToString());
    }
    return it->second;
}

const VulkanPipeline& RenderSystem::FindRenderPipeline(const VkGuid& pipelineGuid)
{
    auto it = RenderPipelineMap.find(pipelineGuid);
    if (it == RenderPipelineMap.end())
    {
        throw std::runtime_error("Pipeline not found: " + pipelineGuid.ToString());
    }
    return it->second;
}

bool RenderSystem::FindPipelinePackageByPipelineType(const VkGuid& pipelinePackageGuid, PipelineType pipelineType)
{
    return RenderPipelinePackageMap[pipelinePackageGuid].PipelineMap.contains(pipelineType);
}

void RenderSystem::BindPushConstants(VkCommandBuffer& commandBuffer, VulkanDrawMessage& drawMessage, uint32 drawIndex, uint32 mip, uint32 mipCount, VkShaderStageFlags stages)
{
    if (drawMessage.PushConstant.has_value())
    {
        const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(drawMessage.RenderPassGuid);
        VulkanPipelinePackage pipelinePackage = FindPipelinePackage(drawMessage.PipelinePackageGuid);
        VulkanPipeline pipeline = FindRenderPipeline(pipelinePackage.PipelineMap[PipelineType::DefaultPipeline]);
     /*   if (FindPipelinePackageByPipelineType(pipelinePackage.PipelinePackageId, PipelineType::WireFramePipeline))
        {
            pipeline = FindRenderPipeline(pipelinePackage.PipelineMap[PipelineType::WireFramePipeline]);
        }*/
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
                    VulkanPipeline pipeline = FindRenderPipeline(pipelinePackage.PipelineMap[PipelineType::DefaultPipeline]);
               /*     if (FindPipelinePackageByPipelineType(pipelinePackage.PipelinePackageId, PipelineType::WireFramePipeline))
                    {
                        pipeline = FindRenderPipeline(pipelinePackage.PipelineMap[PipelineType::WireFramePipeline]);
                    }*/
                    renderPass.BindRenderPassPipeline(commandBuffer, pipeline, 0);

                    if (!renderPassLayer.RenderPassInputs.empty()) inputTexture = textureSystem.FindRenderedTexture(renderPassLayer.RenderPassInputs[0]);
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
    Texture* texture = &textureSystem.FindRenderedTexture(textureGuid);
    if (!texture || texture->texture.TextureImage() == VK_NULL_HANDLE)
    {
        std::cout << "[SamplePixel] Texture not found" << std::endl;
        return UINT32_MAX;
    }

    int x = std::clamp(mousePosition.x, 0, texture->texture.TextureSize().x - 1);
    int y = std::clamp(mousePosition.y, 0, texture->texture.TextureSize().y - 1);

    VkCommandBuffer cmd = vulkan.CommandBuffer().BeginSingleUseCommand();

    VkImageMemoryBarrier barrier =
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
        .oldLayout = texture->texture.TextureImageLayout(),
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = texture->texture.TextureImage(),
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
    };
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(texture->texture.TextureSize().x) * texture->texture.TextureSize().y * 4;
    VkBufferCreateInfo bufferInfo =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT
    };

    VmaAllocationCreateInfo allocInfo =
    {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VmaAllocationInfo allocOut = {};
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VmaAllocation stagingAlloc = VK_NULL_HANDLE;
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
        .imageExtent = { static_cast<uint32>(texture->texture.TextureSize().x), static_cast<uint32>(texture->texture.TextureSize().y), 1 }
    };
    vkCmdCopyImageToBuffer(cmd, texture->texture.TextureImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);

    vulkan.CommandBuffer().EndSingleUseCommand(cmd);
    vkDeviceWaitIdle(vulkan.LogicalDevice());

    const uint32* pData = static_cast<const uint32*>(allocOut.pMappedData);
    uint32 pickedId = pData[y * texture->texture.TextureSize().x + x];

    vmaDestroyBuffer(bufferSystem.VmaAllocatorHandle(), stagingBuffer, stagingAlloc);

    return pickedId;
}

void RenderSystem::DestoryRenderPassSwapChainTextures(Texture& renderedTextureListPtr, size_t& renderedTextureCount, Texture& depthTexture)
{
    Vector<Texture> renderedTextureList = Vector<Texture>(&renderedTextureListPtr, &renderedTextureListPtr + renderedTextureCount);
    for (auto& renderedTexture : renderedTextureList)
    {
        // textureSystem.DestroyTexture(renderedTexture);
    }
    std::memset(static_cast<void*>(&renderedTextureListPtr), 0x00, sizeof(Texture) * renderedTextureCount);
    renderedTextureCount = 0;
    renderedTextureList.clear();
}