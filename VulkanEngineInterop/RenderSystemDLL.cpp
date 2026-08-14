#include "RenderSystemDLL.h"

RenderPassGuid RenderSystem_LoadRenderPass(const char* jsonPath)
{
    return renderSystem.LoadRenderPass(jsonPath);
}

void RenderSystem_Update(void* windowHandle, const float& deltaTime)
{
    renderSystem.Update(windowHandle, deltaTime);
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
