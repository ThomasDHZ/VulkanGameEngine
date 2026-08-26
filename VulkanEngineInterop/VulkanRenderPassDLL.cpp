#pragma once
#include "VulkanRenderPassDLL.h"
#include <MemorySystem.h>
#include <RenderSystem.h>

VulkanRenderPass* VulkanRenderPass_Create()
{
    return memorySystem.AddPtrBuffer<VulkanRenderPass>(1, __FILE__, __LINE__, __func__);
}

void VulkanRenderPass_LoadRenderPass(VulkanRenderPass* renderPass, RenderPassLoader& renderPassLoader)
{
    if (renderPass) renderPass->LoadRenderPass(renderPassLoader);
    else throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");
}

void VulkanRenderPass_BeginRenderPass(VulkanRenderPass* renderPass, VkCommandBuffer& commandBuffer, uint mipLevel)
{
    if (renderPass) renderPass->BeginRenderPass(commandBuffer, mipLevel);
    else throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");
}

void VulkanRenderPass_NextSubpass(VulkanRenderPass* renderPass, VkCommandBuffer& commandBuffer)
{
    if (renderPass) renderPass->NextSubpass(commandBuffer);
    else throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");
}

void VulkanRenderPass_BindViewPort(VulkanRenderPass* renderPass, VkCommandBuffer& commandBuffer, uint drawMipLevel)
{
    if (renderPass) renderPass->BindViewPort(commandBuffer, drawMipLevel);
    else throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");
}

void VulkanRenderPass_BindRenderPassPipeline(VulkanRenderPass* renderPass, VkCommandBuffer& commandBuffer, const VulkanPipeline& pipeline, uint32 firstSet)
{
    if (renderPass) renderPass->BindRenderPassPipeline(commandBuffer, pipeline, firstSet);
    else throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");
}

void VulkanRenderPass_DrawMesh(VulkanRenderPass* renderPass, VkCommandBuffer commandBuffer, MeshDrawMessage& mesh)
{
    if (renderPass) renderPass->DrawMesh(commandBuffer, mesh);
    else throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");
}

void VulkanRenderPass_EndRenderPass(VulkanRenderPass* renderPass, VkCommandBuffer& commandBuffer)
{
    if (renderPass) renderPass->EndRenderPass(commandBuffer);
    else throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");
}

void VulkanRenderPass_Destroy(VulkanRenderPass* renderPass)
{
    if (renderPass)
    {
        renderPass->Destroy();
        memorySystem.DeletePtr(renderPass);
    }
    else throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");
}

VkSampleCountFlagBits VulkanRenderPass_SampleCount(VulkanRenderPass* renderPass)
{
    if (renderPass) renderPass->SampleCount();
    else throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");
}

VkGuid VulkanRenderPass_RenderPassId(VulkanRenderPass* renderPass)
{
    if (renderPass) return renderPass->RenderPassId();
    else throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");
}

ivec2 VulkanRenderPass_RenderPassResolution(VulkanRenderPass* renderPass)
{
    if (renderPass) return renderPass->RenderPassResolution();
    else throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");
}

[[nodiscard]] VulkanTexture* VulkanRenderPass_AttachmentList(VulkanRenderPass* renderPass, size_t& outCount)
{
    if (!renderPass) throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");

    outCount = renderPass->AttachmentList().size();
    VulkanTexture* texturePtr = memorySystem.AddPtrBuffer<VulkanTexture>(outCount, __FILE__, __LINE__, __func__);
    for (size_t x = 0; x < outCount; ++x)
    {
        texturePtr[x] = renderPass->AttachmentList().data()[x];
    }
    return texturePtr;
}

[[nodiscard]] VulkanPipeline* VulkanRenderPass_PipelineList(VulkanRenderPass* renderPass, size_t& outCount)
{
    if (!renderPass) throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");

    outCount = renderPass->PipelineList().size();
    VulkanPipeline* pipelinePtr = memorySystem.AddPtrBuffer<VulkanPipeline>(outCount, __FILE__, __LINE__, __func__);
    for (size_t x = 0; x < outCount; ++x)
    {
        pipelinePtr[x] = renderSystem.FindRenderPipeline(renderPass->PipelineList().data()[x]);
    }
    return pipelinePtr;
}

[[nodiscard]] VulkanSubPass** VulkanRenderPass_SubPassList(VulkanRenderPass* renderPass, size_t* outOuterCount, size_t** outInnerCounts)
{
    if (!renderPass) throw std::runtime_error(String(__func__) + ": RenderPass pointer is null");

    const auto& subPassList = renderPass->SubPassList();
    *outOuterCount = subPassList.size();
    if (*outOuterCount == 0) *outInnerCounts = nullptr; return nullptr;

    *outInnerCounts = memorySystem.AddPtrBuffer<size_t>(*outOuterCount, __FILE__, __LINE__, __func__);
    VulkanSubPass** outerPtr = memorySystem.AddPtrBuffer<VulkanSubPass*>(*outOuterCount, __FILE__, __LINE__, __func__);
    for (size_t x = 0; x < *outOuterCount; ++x)
    {
        const auto& innerList = subPassList[x];
        size_t innerCount = innerList.size();
        (*outInnerCounts)[x] = innerCount;

        if (innerCount == 0) outerPtr[x] = nullptr; continue;
        outerPtr[x] = memorySystem.AddPtrBuffer<VulkanSubPass>(innerCount, __FILE__, __LINE__, __func__);
        for (size_t y = 0; y < innerCount; ++y)
        {
            outerPtr[x][y] = innerList[y];
        }
    }
    return outerPtr;
}
