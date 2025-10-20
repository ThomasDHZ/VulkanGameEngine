#pragma once
#include "VulkanRenderer.h"
#include "VulkanRenderPass.h"
#include "VulkanPipeline.h"

struct VulkanRendererArchive
{
    UnorderedMap<RenderPassGuid, VulkanRenderPass>                RenderPassMap;
    UnorderedMap<RenderPassGuid, Vector<VulkanPipeline>>          RenderPipelineMap;
    UnorderedMap<RenderPassGuid, String>                          RenderPassLoaderJsonMap;
};
DLL_EXPORT VulkanRendererArchive rendererArchive;

