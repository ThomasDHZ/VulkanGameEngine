#include "LevelSystemDLL.h"

void LevelSystem_LoadLevel(const char* levelPath)
{
	levelSystem.LoadLevel(levelPath);
}

void LevelSystem_Update(const float& deltaTime)
{
	levelSystem.Update(deltaTime);
}

RenderPassNodeDLL* LevelSystem_CreateDrawCommands(VkCommandBuffer& commandBuffer, const float& deltaTime, size_t* renderPassNodeCount)
{
    Vector<RenderPassNode> renderPassNodeList = levelSystem.CreateDrawCommands(commandBuffer, deltaTime);
    RenderPassNodeDLL* dllList = memorySystem.AddPtrBuffer<RenderPassNodeDLL>(renderPassNodeList.size(), __FILE__, __LINE__, __func__);

    if (!dllList && !renderPassNodeList.empty()) return nullptr;
    for (size_t x = 0; x < renderPassNodeList.size(); ++x)
    {
        RenderPassNode& srcNode = renderPassNodeList[x];
        RenderPassNodeDLL& dstNode = dllList[x];
        PreRenderPassCmdFunction* preFn = memorySystem.AddPtrBuffer<PreRenderPassCmdFunction>(1, __FILE__, __LINE__, __func__);
        if (preFn) new (preFn) PreRenderPassCmdFunction(std::move(srcNode.PreRenderPassCmd));

        PostRenderPassCmdFunction* postFn = memorySystem.AddPtrBuffer<PostRenderPassCmdFunction>(1, __FILE__, __LINE__, __func__);
        if (postFn)  new (postFn) PostRenderPassCmdFunction(std::move(srcNode.PostRenderPassCmd));

        const size_t subPassCount = srcNode.SubPassDrawMessage.size();
        size_t* subPassCounts = memorySystem.AddPtrBuffer<size_t>(subPassCount, __FILE__, __LINE__, __func__);
        VulkanDrawMessageDLL** subPassPtrs = memorySystem.AddPtrBuffer<VulkanDrawMessageDLL*>(subPassCount, __FILE__, __LINE__, __func__);
        for (size_t subPass = 0; subPass < subPassCount; ++subPass)
        {
            auto& srcDrawList = srcNode.SubPassDrawMessage[subPass];
            const size_t drawCount = srcDrawList.size();

            subPassCounts[subPass] = drawCount;

            if (drawCount == 0)
            {
                subPassPtrs[subPass] = nullptr;
                continue;
            }

            VulkanDrawMessageDLL* drawArray = memorySystem.AddPtrBuffer<VulkanDrawMessageDLL>(drawCount, __FILE__, __LINE__, __func__);
            subPassPtrs[subPass] = drawArray;

            for (size_t d = 0; d < drawCount; ++d)
            {
                VulkanDrawMessage& srcMsg = srcDrawList[d];
                VulkanDrawMessageDLL& dstMsg = drawArray[d];

                PushConstantsCmdFunction* pushFn = memorySystem.AddPtrBuffer<PushConstantsCmdFunction>(1, __FILE__, __LINE__, __func__);
                if (pushFn) new (pushFn) PushConstantsCmdFunction(std::move(srcMsg.PushConstantsCmd));

                PreDrawCmdFunction* preDrawFn = memorySystem.AddPtrBuffer<PreDrawCmdFunction>(1, __FILE__, __LINE__, __func__);
                if (preDrawFn) new (preDrawFn) PreDrawCmdFunction(std::move(srcMsg.PreDrawCmd));

                CustomDrawCmdFunction* customFn = memorySystem.AddPtrBuffer<CustomDrawCmdFunction>(1, __FILE__, __LINE__, __func__);
                if (customFn) new (customFn) CustomDrawCmdFunction(std::move(srcMsg.CustomDrawCmd));

                PostDrawCmdFunction* postDrawFn = memorySystem.AddPtrBuffer<PostDrawCmdFunction>(1, __FILE__, __LINE__, __func__);
                if (postDrawFn) new (postDrawFn) PostDrawCmdFunction(std::move(srcMsg.PostDrawCmd));

                PushConstantUpdateRule* rulesCopy = nullptr;
                if (!srcMsg.PushConstantUpdateRules.empty())
                {
                    rulesCopy = memorySystem.AddPtrBuffer<PushConstantUpdateRule>(srcMsg.PushConstantUpdateRules.size(), __FILE__, __LINE__, __func__);
                    if (rulesCopy) memcpy(rulesCopy, srcMsg.PushConstantUpdateRules.data(), srcMsg.PushConstantUpdateRules.size() * sizeof(PushConstantUpdateRule));
                }

                MeshDrawMessage* drawMeshCopy = nullptr;
                if (!srcMsg.DrawMeshList.empty())
                {
                    drawMeshCopy = memorySystem.AddPtrBuffer<MeshDrawMessage>(srcMsg.DrawMeshList.size(), __FILE__, __LINE__, __func__);
                    if (drawMeshCopy) memcpy(drawMeshCopy, srcMsg.DrawMeshList.data(), srcMsg.DrawMeshList.size() * sizeof(MeshDrawMessage));
                }

                VkGuid* renderPassInputsCopy = nullptr;
                if (!srcMsg.RenderPassInputs.empty())
                {
                    renderPassInputsCopy = memorySystem.AddPtrBuffer<VkGuid>(srcMsg.RenderPassInputs.size(), __FILE__, __LINE__, __func__);
                    if (renderPassInputsCopy) memcpy(renderPassInputsCopy, srcMsg.RenderPassInputs.data(), srcMsg.RenderPassInputs.size() * sizeof(VkGuid));
                }

                VkGuid* renderPassOutputsCopy = nullptr;
                if (!srcMsg.RenderPassOutputs.empty())
                {
                    renderPassOutputsCopy = memorySystem.AddPtrBuffer<VkGuid>(srcMsg.RenderPassOutputs.size(), __FILE__, __LINE__, __func__);
                    if (renderPassOutputsCopy) memcpy(renderPassOutputsCopy, srcMsg.RenderPassOutputs.data(), srcMsg.RenderPassOutputs.size() * sizeof(VkGuid));
                }

                char* pushConstantNameCopy = nullptr;
                if (srcMsg.PushConstant.has_value())
                {
                    const std::string& str = srcMsg.PushConstant.value();
                    pushConstantNameCopy = memorySystem.AddPtrBuffer<char>(str.size() + 1, __FILE__, __LINE__, __func__);
                    if (pushConstantNameCopy)
                    {
                        memcpy(pushConstantNameCopy, str.data(), str.size());
                        pushConstantNameCopy[str.size()] = '\0';
                    }
                }

                dstMsg = VulkanDrawMessageDLL
                {
                    .RenderPassGuid = srcMsg.RenderPassGuid,
                    .PipelinePackageGuid = srcMsg.PipelinePackageGuid,
                    .PushConstant = pushConstantNameCopy,
                    .PushConstantUpdateRules = rulesCopy,
                    .DrawMeshList = drawMeshCopy,
                    .RenderPassInputs = renderPassInputsCopy,
                    .RenderPassOutputs = renderPassOutputsCopy,
                    .PushConstantUpdateRulesCount = srcMsg.PushConstantUpdateRules.size(),
                    .DrawMeshListCount = srcMsg.DrawMeshList.size(),
                    .RenderPassInputsCount = srcMsg.RenderPassInputs.size(),
                    .RenderPassOutputsCount = srcMsg.RenderPassOutputs.size(),
                    .OffScreenRenderPass = srcMsg.OffScreenRenderPass,
                    .PushConstantsCmd = pushFn,
                    .PreDrawCmd = preDrawFn,
                    .CustomDrawCmd = customFn,
                    .PostDrawCmd = postDrawFn,
                };
            }
        }

        dstNode = RenderPassNodeDLL
        {
            .RenderPassGuid = srcNode.RenderPassGuid,
            .SubPassDrawMessage = subPassPtrs,
            .SubPassDrawMessage_RenderPassCount = subPassCount,
            .SubPassDrawMessage_SubPassCounts = subPassCounts,
            .PreRenderPassCmd = preFn,
            .PostRenderPassCmd = postFn,
            .MipCount = srcNode.MipCount,
        };
    }
    *renderPassNodeCount = renderPassNodeList.size();
    return dllList;
}
