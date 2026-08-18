#pragma once
#include "DLL.h"
#include <LevelSystem.h>

struct VulkanDrawMessageDLL
{
    VkGuid                            RenderPassGuid;
    VkGuid                            PipelinePackageGuid;
    const char*                       PushConstant = nullptr;
    PushConstantUpdateRule*           PushConstantUpdateRules = nullptr;
    MeshDrawMessage*                  DrawMeshList = nullptr;
    VkGuid*                           RenderPassInputs = nullptr;
    VkGuid*                           RenderPassOutputs = nullptr;
    size_t                            PushConstantUpdateRulesCount = 0;
    size_t                            DrawMeshListCount = 0;
    size_t                            RenderPassInputsCount = 0;
    size_t                            RenderPassOutputsCount = 0;
    bool                              OffScreenRenderPass = false;

    void*                             PushConstantsCmd = nullptr;
    void*                             PreDrawCmd = nullptr;
    void*                             CustomDrawCmd = nullptr;
    void*                             PostDrawCmd = nullptr;
};

struct RenderPassNodeDLL
{
    VkGuid                 RenderPassGuid;
    VulkanDrawMessageDLL** SubPassDrawMessage;
    size_t                 SubPassDrawMessage_RenderPassCount; //number of subpasses (outer size);
    size_t* SubPassDrawMessage_SubPassCounts;   //array of sizes for each subpass (inner size);
    void* PreRenderPassCmd = nullptr;
    void* PostRenderPassCmd = nullptr;
    uint                   MipCount = 0;
};

using PushConstantsCmdFunction = std::function<void(VkCommandBuffer, VulkanDrawMessage&, uint32, ivec2, uint32)>;
using PreDrawCmdFunction = std::function<void(VkCommandBuffer, VulkanDrawMessage)>;

using CustomDrawCmdFunction     = std::function<void(VkCommandBuffer, VulkanDrawMessage)>;
using PostDrawCmdFunction       = std::function<void(VkCommandBuffer, VulkanDrawMessage)>;
using PreRenderPassCmdFunction  = std::function<void(VkCommandBuffer, RenderPassNode&)>;
using PostRenderPassCmdFunction = std::function<void(VkCommandBuffer, RenderPassNode&)>;

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT void                           LevelSystem_LoadLevel(const char* levelPath);
    DLL_EXPORT void                           LevelSystem_Update(const float& deltaTime);
    DLL_EXPORT void                           LevelSystem_RenderFrameBuffer(VkCommandBuffer& commandBuffer, VkGuid& renderPassId);
    DLL_EXPORT RenderPassNodeDLL*             LevelSystem_CreateDrawCommands(VkCommandBuffer& commandBuffer, const float& deltaTime, size_t* renderPassNodeCount);
#ifdef __cplusplus
}
#endif