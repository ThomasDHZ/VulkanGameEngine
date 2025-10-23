#pragma once
#include "VulkanRenderPass.h"
#include "VulkanPipeline.h"

struct RenderArchive
{
    UnorderedMap<RenderPassGuid, VulkanRenderPass>                RenderPassMap;
    UnorderedMap<RenderPassGuid, Vector<VulkanPipeline>>          RenderPipelineMap;
    UnorderedMap<RenderPassGuid, String>                          RenderPassLoaderJsonMap;
};
DLL_EXPORT RenderArchive renderArchive;

