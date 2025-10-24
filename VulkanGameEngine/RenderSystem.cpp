#include "renderSystem.h"
#include "TextureSystem.h"
#include <VulkanShaderSystem.h>
#include <BufferSystem.h>
#include <MeshSystem.h>
#include "GameObjectSystem.h"
#include "SpriteSystem.h"
#include "FileSystem.h"
#include "LevelSystem.h"

VkCommandBuffer RenderBloomPass(VkGuid& renderPassId)
{
    const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
    const VulkanPipeline& pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
    VkCommandBuffer commandBuffer = renderPass.CommandBuffer;
    Texture blurTexture = textureSystem.FindRenderedTextureList(levelSystem.spriteRenderPass2DId)[0];
    ShaderPushConstant pushConstant = *shaderSystem.GetGlobalShaderPushConstant("bloomSettings");

    uint mipWidth = renderer.SwapChainResolution.width;
    uint mipHeight = renderer.SwapChainResolution.height;
    for (uint x = 0; x < blurTexture.mipMapLevels; x++)
    {
        VkDescriptorImageInfo imageInfo =
        {
            .sampler = blurTexture.textureSampler,
            .imageView = blurTexture.textureView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = pipeline.DescriptorSetList[0],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo
        };
        vkUpdateDescriptorSets(renderer.Device, 1, &descriptorWrite, 0, nullptr);

        VkViewport viewport
        {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(mipWidth > 1 ? mipWidth : 1),
            .height = static_cast<float>(mipHeight > 1 ? mipHeight : 1),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        VkRect2D scissor = VkRect2D
        {
            .offset = VkOffset2D {.x = 0, .y = 0},
            .extent = {mipWidth > 1 ? mipWidth : 1, mipHeight > 1 ? mipHeight : 1}
        };

        VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = renderPass.RenderPass,
            .framebuffer = renderPass.FrameBufferList[renderer.ImageIndex],
            .renderArea = scissor,
            .clearValueCount = static_cast<uint32>(renderPass.ClearValueCount),
            .pClearValues = renderPass.ClearValueList
        };

        float blurStrength = 1.0f + x * 0.5f;
        float lodLevel = static_cast<float>(x);
        memcpy(shaderSystem.SearchGlobalShaderConstantVar(&pushConstant, "blurScale")->Value, &lodLevel, sizeof(lodLevel));
        memcpy(shaderSystem.SearchGlobalShaderConstantVar(&pushConstant, "blurStrength")->Value, &blurStrength, sizeof(blurStrength));

        VULKAN_RESULT(vkBeginCommandBuffer(commandBuffer, &renderSystem.CommandBufferBeginInfo));
        //vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        //vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        //vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        //vkCmdPushConstants(commandBuffer, pipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer);
        //vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
        //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetCount, pipeline.DescriptorSetList, 0, nullptr);
        //vkCmdDraw(commandBuffer, 6, 1, 0, 0);
        //vkCmdEndRenderPass(commandBuffer);
        VULKAN_RESULT(vkEndCommandBuffer(commandBuffer));
    }
    return commandBuffer;
}


VkCommandBuffer RenderFrameBuffer(VkGuid& renderPassId)
{
    const VulkanRenderPass renderPass = renderSystem.FindRenderPass(renderPassId);
    const VulkanPipeline& pipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
    VkCommandBuffer commandBuffer = renderPass.CommandBuffer;
    Vector renderPassTexture = textureSystem.FindRenderedTextureList(levelSystem.spriteRenderPass2DId);

    VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(renderer.SwapChainResolution.width),
        .height = static_cast<float>(renderer.SwapChainResolution.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = VkRect2D
    {
        .offset = VkOffset2D {.x = 0, .y = 0},
        .extent = VkExtent2D {.width = (uint32)renderer.SwapChainResolution.width, .height = (uint32)renderer.SwapChainResolution.height}
    };

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList[renderer.ImageIndex],
        .renderArea = scissor,
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueCount),
        .pClearValues = renderPass.ClearValueList
    };

    VULKAN_RESULT(vkBeginCommandBuffer(commandBuffer, &renderSystem.CommandBufferBeginInfo));
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.Pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.PipelineLayout, 0, pipeline.DescriptorSetCount, pipeline.DescriptorSetList, 0, nullptr);
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);
    VULKAN_RESULT(vkEndCommandBuffer(commandBuffer));
    return commandBuffer;
}

VkCommandBuffer RenderLevel(VkGuid& renderPassId, VkGuid& levelId, const float deltaTime)
{
    const VulkanRenderPass& renderPass = renderSystem.FindRenderPass(renderPassId);
    const VulkanPipeline& spritePipeline = renderSystem.FindRenderPipelineList(renderPassId)[0];
    const VulkanPipeline& levelPipeline = renderSystem.FindRenderPipelineList(renderPassId)[1];
    const Vector<Mesh>& levelLayerList = meshSystem.FindMeshByMeshType(MeshTypeEnum::Mesh_LevelMesh);
    const VkCommandBuffer& commandBuffer = renderPass.CommandBuffer;
    ShaderPushConstant pushConstant = *shaderSystem.GetGlobalShaderPushConstant("sceneData");

    VkRenderPassBeginInfo renderPassBeginInfo = VkRenderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.RenderPass,
        .framebuffer = renderPass.FrameBufferList[renderer.ImageIndex],
        .renderArea = renderPass.RenderArea,
        .clearValueCount = static_cast<uint32>(renderPass.ClearValueCount),
        .pClearValues = renderPass.ClearValueList
    };

    VkDeviceSize offsets[] = { 0 };
    VULKAN_RESULT(vkBeginCommandBuffer(commandBuffer, &renderSystem.CommandBufferBeginInfo));
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    for (auto& levelLayer : levelLayerList)
    {
        const Vector<uint32>& indiceList = meshSystem.IndexList[levelLayer.IndexIndex];
        const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshVertexBufferId).Buffer;
        const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(levelLayer.MeshIndexBufferId).Buffer;

        // memcpy(shaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "MeshBufferIndex")->Value, &meshIndex, sizeof(meshIndex));
        vkCmdPushConstants(commandBuffer, levelPipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.Pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, levelPipeline.PipelineLayout, 0, levelPipeline.DescriptorSetCount, levelPipeline.DescriptorSetList, 0, nullptr);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &meshVertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, indiceList.size(), 1, 0, 0, 0);
    }
    for (auto& spriteLayer : spriteSystem.SpriteLayerList)
    {
        const Mesh& spriteMesh = Mesh_FindMesh(spriteLayer.second.SpriteLayerMeshId);
        const VkBuffer& meshVertexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshVertexBufferId).Buffer;
        const VkBuffer& meshIndexBuffer = bufferSystem.FindVulkanBuffer(spriteMesh.MeshIndexBufferId).Buffer;
        const Vector<SpriteInstance>& spriteInstanceList = spriteSystem.FindSpriteInstancesByLayer(spriteLayer.second);
        const VkBuffer& spriteInstanceBuffer = bufferSystem.FindVulkanBuffer(spriteLayer.second.SpriteLayerBufferId).Buffer;
        const Vector<uint32>& indiceList = meshSystem.IndexList[spriteMesh.IndexIndex];

        // memcpy(shaderSystem.SearchGlobalShaderConstantVar(&sceneDataBuffer, "MeshBufferIndex")->Value, &spriteLayer.SpriteLayerMeshId, sizeof(spriteLayer.SpriteLayerMeshId));
        vkCmdPushConstants(commandBuffer, spritePipeline.PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushConstant.PushConstantSize, pushConstant.PushConstantBuffer);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.Pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline.PipelineLayout, 0, spritePipeline.DescriptorSetCount, spritePipeline.DescriptorSetList, 0, nullptr);
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &spriteInstanceBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, meshIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, indiceList.size(), spriteInstanceList.size(), 0, 0, 0);
    }
    vkCmdEndRenderPass(commandBuffer);
    VULKAN_RESULT(vkEndCommandBuffer(commandBuffer));
    return commandBuffer;
}
