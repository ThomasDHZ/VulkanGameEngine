#include "LevelSystemDLL.h"

void LevelSystem_LoadLevel(const char* levelPath)
{
	levelSystem.LoadLevel(levelPath);
}

void LevelSystem_Update(const float& deltaTime)
{
	levelSystem.Update(deltaTime);
}

void LevelSystem_LevelEditorRenderPass(const char* levelPath)
{
    levelSystem.LevelEditorRenderPass(levelPath);
}

RenderPassNodeDLL* LevelSystem_CreateDrawCommands(VkCommandBuffer& commandBuffer, const float& deltaTime, size_t* renderPassNodeCount)
{
    Vector<RenderPassNode> renderPassNodeList = levelSystem.CreateDrawCommands(commandBuffer, deltaTime);
    if (renderPassNodeList.empty())
    {
        if (renderPassNodeCount) *renderPassNodeCount = 0;
        return nullptr;
    }
    return ToDLL_RenderPassNodeDLL(renderPassNodeList, commandBuffer, deltaTime, renderPassNodeCount);
}

void LevelSystem_FreeDrawCommands(RenderPassNodeDLL* dllList, size_t renderPassNodeCount)
{
    if (!dllList) return;
    for (size_t x = 0; x < renderPassNodeCount; ++x)
    {
        RenderPassNodeDLL& node = dllList[x];

        if (node.PreRenderPassCmd)
        {
            auto* fn = static_cast<PreRenderPassCmdFunction*>(node.PreRenderPassCmd);
            fn->~PreRenderPassCmdFunction();
            memorySystem.DeletePtr(fn);
            node.PreRenderPassCmd = nullptr;
        }

        if (node.PostRenderPassCmd)
        {
            auto* fn = static_cast<PostRenderPassCmdFunction*>(node.PostRenderPassCmd);
            fn->~PostRenderPassCmdFunction();
            memorySystem.DeletePtr(fn);
            node.PostRenderPassCmd = nullptr;
        }

        VulkanDrawMessageDLL** subPassPtrs = node.SubPassDrawMessage;
        size_t* subPassCounts = node.SubPassDrawMessage_SubPassCounts;
        const size_t subPassCount = node.SubPassDrawMessage_RenderPassCount;

        if (subPassPtrs && subPassCounts)
        {
            for (size_t subPass = 0; subPass < subPassCount; ++subPass)
            {
                VulkanDrawMessageDLL* drawArray = subPassPtrs[subPass];
                const size_t drawCount = subPassCounts[subPass];
                if (!drawArray || drawCount == 0)
                    continue;

                for (size_t d = 0; d < drawCount; ++d)
                {
                    VulkanDrawMessageDLL& msg = drawArray[d];

                    if (msg.PushConstantsCmd)
                    {
                        auto* fn = static_cast<PushConstantsCmdFunction*>(msg.PushConstantsCmd);
                        fn->~PushConstantsCmdFunction();
                        memorySystem.DeletePtr(fn);
                    }
                    if (msg.PreDrawCmd)
                    {
                        auto* fn = static_cast<PreDrawCmdFunction*>(msg.PreDrawCmd);
                        fn->~PreDrawCmdFunction();
                        memorySystem.DeletePtr(fn);
                    }
                    if (msg.CustomDrawCmd)
                    {
                        auto* fn = static_cast<CustomDrawCmdFunction*>(msg.CustomDrawCmd);
                        fn->~CustomDrawCmdFunction();
                        memorySystem.DeletePtr(fn);
                    }
                    if (msg.PostDrawCmd)
                    {
                        auto* fn = static_cast<PostDrawCmdFunction*>(msg.PostDrawCmd);
                        fn->~PostDrawCmdFunction();
                        memorySystem.DeletePtr(fn);
                    }

                    if (msg.PushConstantUpdateRules) memorySystem.DeletePtr(msg.PushConstantUpdateRules);
                    if (msg.DrawMeshList) memorySystem.DeletePtr(msg.DrawMeshList);
                    if (msg.RenderPassInputs) memorySystem.DeletePtr(msg.RenderPassInputs);
                    if (msg.RenderPassOutputs) memorySystem.DeletePtr(msg.RenderPassOutputs);
                    if (msg.PushConstant) memorySystem.DeletePtr(const_cast<char*>(msg.PushConstant));
                }
                memorySystem.DeletePtr(drawArray);
            }
        }
        if (subPassPtrs) memorySystem.DeletePtr(subPassPtrs);
        if (subPassCounts) memorySystem.DeletePtr(subPassCounts);
    }

    memorySystem.DeletePtr(dllList);
}