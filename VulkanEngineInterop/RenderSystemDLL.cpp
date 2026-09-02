#include "RenderSystemDLL.h"
#include "DllHelper.h"
#include "MemorySystemDLL.h"

RenderPassGuid RenderSystem_LoadRenderPass(const char* jsonPath)
{
    return renderSystem.LoadRenderPass(jsonPath);
}

void RenderSystem_Update(void* windowHandle, const float& deltaTime)
{
    renderSystem.Update(windowHandle, deltaTime);
}

void RenderSystem_Draw(VkCommandBuffer& commandBuffer, RenderPassNodeDLL* renderPassNodeListPtr, size_t renderPassNodeCount)
{
    if (!renderPassNodeListPtr || renderPassNodeCount == 0) return;

    Vector<RenderPassNode> renderPassNodes;
    renderPassNodes.reserve(renderPassNodeCount);
    for (size_t i = 0; i < renderPassNodeCount; ++i)
    {
        const RenderPassNodeDLL& src = renderPassNodeListPtr[i];

        RenderPassNode dst;
        dst.RenderPassGuid = src.RenderPassGuid;
        dst.MipCount = src.MipCount;

        const size_t subPassCount = src.SubPassDrawMessage_RenderPassCount;
        dst.SubPassDrawMessage.resize(subPassCount);
        for (size_t x = 0; x < subPassCount; ++x)
        {
            const size_t drawCount = src.SubPassDrawMessage_SubPassCounts[x];
            VulkanDrawMessageDLL* drawArray = src.SubPassDrawMessage[x];

            auto& dstDrawList = dst.SubPassDrawMessage[x];
            dstDrawList.reserve(drawCount);
            for (size_t y = 0; y < drawCount; ++y)
            {
                dstDrawList.emplace_back(VulkanDrawMessage
                    {
                        .RenderPassGuid = drawArray[y].RenderPassGuid,
                        .PipelinePackageGuid = drawArray[y].PipelinePackageGuid,
                        .PushConstant = drawArray[y].PushConstant ? std::optional<String>(drawArray[y].PushConstant) : std::nullopt,
                        .PushConstantUpdateRules = Vector<PushConstantUpdateRule>(drawArray[y].PushConstantUpdateRules, drawArray[y].PushConstantUpdateRules + drawArray[y].PushConstantUpdateRulesCount),
                        .DrawMeshList = Vector<MeshDrawMessage>(drawArray[y].DrawMeshList, drawArray[y].DrawMeshList + drawArray[y].DrawMeshListCount),
                        .RenderPassInputs = Vector<VkGuid>(drawArray[y].RenderPassInputs, drawArray[y].RenderPassInputs + drawArray[y].RenderPassInputsCount),
                        .RenderPassOutputs = Vector<VkGuid>(drawArray[y].RenderPassOutputs, drawArray[y].RenderPassOutputs + drawArray[y].RenderPassOutputsCount),
                        .OffScreenRenderPass = drawArray[y].OffScreenRenderPass,
      /*                .PushConstantsCmd = DllHelper::ExtractDllFunction<PushConstantsCmdFunction>(drawArray[y].PushConstantsCmd),
                        .PreDrawCmd = DllHelper::ExtractDllFunction<PreDrawCmdFunction>(drawArray[y].PreDrawCmd),
                        .CustomDrawCmd = DllHelper::ExtractDllFunction<CustomDrawCmdFunction>(drawArray[y].CustomDrawCmd),
                        .PostDrawCmd = DllHelper::ExtractDllFunction<PostDrawCmdFunction>(drawArray[y].PostDrawCmd)*/
                    });
            }
        }
        renderPassNodes.emplace_back(std::move(dst));
    }
    renderSystem.Draw(commandBuffer, renderPassNodes);
}

void RenderSystem_PresentToSwapChain(VkCommandBuffer& commandBuffer, const VkGuid* renderPassTextureGuid)
{
    renderSystem.PresentToSwapChain(commandBuffer, *renderPassTextureGuid);
}

VkGuid* RenderSystem_FindRenderPassAttachmentList(const VkGuid& renderPassGuid, uint32* returnTextureCount)
{
    Vector<Texture> attachmentList = renderSystem.FindRenderPassAttachmentList(renderPassGuid);

    Vector<VkGuid> attachmentIdList;
    attachmentIdList.reserve(attachmentList.size());
    for (auto& attachmentTexture : attachmentList)
    {
        attachmentIdList.emplace_back(attachmentTexture.textureGuid);
    }

    *returnTextureCount = attachmentList.size();
    return memorySystem.AddPtrBuffer(attachmentIdList.data(), attachmentIdList.size(), __FILE__, __LINE__, __func__);
}

uint32 RenderSystem_SampleRenderPassPixel(const VkGuid& attachmentGuid, ivec2 mousePosition)
{
    return renderSystem.SampleRenderPassPixel(attachmentGuid, mousePosition);
}

void RenderSystem_GetAttachmentSize(const VkGuid& attachmentGuid, int* outX, int* outY)
{
    ivec2 size = renderSystem.FindRenderPassAttachment(attachmentGuid).texture.TextureSize();
    *outX = size.x;
    *outY = size.y;
}
