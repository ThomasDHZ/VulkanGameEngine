#pragma once
#include "Typedef.h"
#include "JsonStructs.h"

struct  RenderedTextureLoader
{
    VkGuid RenderedTextureId;
    String RenderedTextureInfoName;
    VkImageCreateInfo ImageCreateInfo;
    VkSamplerCreateInfo SamplerCreateInfo;
    VkAttachmentDescription AttachmentDescription;
    RenderedTextureType TextureType;
};

struct RenderPassLoader
{
    VkGuid RenderPassId;
    bool IsRenderedToSwapchain;
    Vector<String> RenderPipelineList;
    Vector<VkGuid> InputTextureList;
    Vector<RenderedTextureLoader> RenderedTextureInfoModelList;
    Vector<VkSubpassDependency> SubpassDependencyModelList;
    Vector<VkClearValue> ClearValueList;
    RenderAreaModel RenderArea;
};