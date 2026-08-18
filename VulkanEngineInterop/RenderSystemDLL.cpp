#include "RenderSystemDLL.h"
#include "DllHelper.h"

RenderPassGuid RenderSystem_LoadRenderPass(const char* jsonPath)
{
    return renderSystem.LoadRenderPass(jsonPath);
}

void RenderSystem_Update(void* windowHandle, const float& deltaTime)
{
    renderSystem.Update(windowHandle, deltaTime);
}

//void RenderSystem_Draw(VkCommandBuffer& commandBuffer, RenderPassNodeDLL* renderPassNodeListPtr, size_t renderPassNodeCount)
//{
//    Vector<RenderPassNode> renderPassNodeL;
//    Span<RenderPassNodeDLL> renderPassNodeList = Span<RenderPassNodeDLL>(renderPassNodeListPtr, renderPassNodeCount);
//    for (auto& renderPassNode : renderPassNodeList)
//    {
//        Vector<Vector<VulkanDrawMessage>> vulkanDrawMessageList;
//        Vector<Vector<VulkanDrawMessageDLL>> renderPassListList = Vector<Vector<VulkanDrawMessageDLL>>(renderPassNode.SubPassDrawMessage, renderPassNode.SubPassDrawMessage + renderPassNode.SubPassDrawMessage_RenderPassCount);
//        for (int x = 0; x < renderPassListList.size(); x++)
//        {
//            Vector<VulkanDrawMessage> vulkanDrawMessage;
//            Vector<size_t> subPassCountList = Vector<size_t>(renderPassNode.SubPassDrawMessage[x], renderPassNode.SubPassDrawMessage[x] + renderPassNode.SubPassDrawMessage_SubPassCounts[x]);
//            Vector<VulkanDrawMessageDLL> subPassList = Vector<VulkanDrawMessageDLL>(renderPassListList[x], renderPassListList[x] + subPassCountList[x]);
//            for (auto& subPass : subPassList)
//            {
//                size_t subPassCountList = subPass;
//
//            }
//        }
//        VulkanDrawMessageDLL** SubPassDrawMessage;
//
//        renderPassNodeL.emplace_back(RenderPassNode
//            {
//                .RenderPassGuid = renderPassNode.RenderPassGuid,
//                .SubPassDrawMessage = vulkanDrawMessageList,
//                .PreRenderPassCmd = DllHelper::ExtractDllFunction<PostRenderPassCmdFunction>(renderPassNode.PreRenderPassCmd),
//                .PostRenderPassCmd = DllHelper::ExtractDllFunction<PostRenderPassCmdFunction>(renderPassNode.PostRenderPassCmd),
//                .MipCount = renderPassNode.MipCount
//            });
//    }
//}
void RenderSystem_Draw(VkCommandBuffer& commandBuffer, RenderPassNodeDLL* renderPassNodeListPtr, size_t renderPassNodeCount)
{
    if (!renderPassNodeListPtr || renderPassNodeCount == 0)
        return;

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

//const VulkanRenderPass& RenderSystem_FindRenderPass(const RenderPassGuid& renderPassGuid)
//{
//    // TODO: insert return statement here
//}
//
//const VulkanPipelinePackage& RenderSystem_FindPipelinePackage(const VkGuid& pipelinePackageGuid)
//{
//    // TODO: insert return statement here
//}
//
//const VulkanPipeline& RenderSystem_FindRenderPipeline(const VkGuid& pipelineGuid)
//{
//    // TODO: insert return statement here
//}
//
//bool RenderSystem_FindPipelinePackageByPipelineType(const VkGuid& pipelinePackageGuid, PipelineType pipelineType)
//{
//    return false;
//}
//
//uint32 RenderSystem_SampleRenderPassPixel(const TextureGuid& textureGuid, ivec2 mousePosition)
//{
//    return uint32();
//}
