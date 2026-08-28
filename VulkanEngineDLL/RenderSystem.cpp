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
    if (!vulkanWindow.WasFramebufferResized()) return;

    vkDeviceWaitIdle(vulkan.LogicalDevice());
    vulkan.Swapchain().RebuildSwapChain(windowHandle);
    for (auto& [id, renderPass] : RenderPassMap)
    {
        renderPass.RebuildRenderPass();
     
        Vector<Texture> renderedTextureList;
        for (int x = 0; x < renderPass.AttachmentList().size(); x++)
        {
            VulkanTexture attachment = renderPass.AttachmentList()[x];
            ivec2 renderPassSize = ivec2(attachment.TextureSize().x, attachment.TextureSize().y);
            Texture texture = Texture
            {
               .textureGuid = id,
               .texture = attachment,
               .imGuiDescriptorSet = VK_NULL_HANDLE
            };
            renderedTextureList.emplace_back(texture);
            if (texture.texture.m_textureType == TextureTypeEnum::kTextureType_CubeMap) memoryPoolSystem.UpdateTextureDescriptorSet(texture.gpuTextureBufferIndex, texture.texture, memoryPoolSystem.CubeMapDescriptorBinding);
            else memoryPoolSystem.UpdateTextureDescriptorSet(texture.gpuTextureBufferIndex, texture.texture, memoryPoolSystem.Texture2DBinding);
        }
    }
    imGuiSystem.RebuildSwapChain();
    vulkanWindow.ResetFramebufferResized();
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

Vector<Texture>& RenderSystem::FindRenderedTextureList(const RenderPassGuid& renderPassGuid)
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