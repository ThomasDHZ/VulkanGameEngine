#include "from_json.h"
#include "VulkanSystem.h"

namespace nlohmann
{
    void from_json(const json& j, VkExtent2D& extent) {
        j.at("width").get_to(extent.width);
        j.at("height").get_to(extent.height);
    }

    void from_json(const json& j, VkExtent3D& extent) {
        j.at("width").get_to(extent.width);
        j.at("height").get_to(extent.height);
        j.at("depth").get_to(extent.depth);
    }

    void from_json(const json& j, VkOffset2D& offset) {
        j.at("x").get_to(offset.x);
        j.at("y").get_to(offset.y);
    }

    void from_json(const json& j, VkOffset3D& offset) {
        j.at("x").get_to(offset.x);
        j.at("y").get_to(offset.y);
        j.at("z").get_to(offset.z);
    }

    void from_json(const json& j, VkImageCreateInfo& info) 
    {
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        j.at("ImageType").get_to(info.imageType);
        j.at("Format").get_to(info.format);
        j.at("MipLevels").get_to(info.mipLevels);
        j.at("ArrayLayers").get_to(info.arrayLayers);
        j.at("Samples").get_to(info.samples);
        j.at("Tiling").get_to(info.tiling);
        j.at("Usage").get_to(info.usage);
        j.at("SharingMode").get_to(info.sharingMode);
        j.at("InitialLayout").get_to(info.initialLayout);
    }

    void from_json(const json& j, VkSamplerCreateInfo& info)
    {
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        j.at("MagFilter").get_to(info.magFilter);
        j.at("MinFilter").get_to(info.minFilter);
        j.at("MipmapMode").get_to(info.mipmapMode);
        j.at("AddressModeU").get_to(info.addressModeU);
        j.at("AddressModeV").get_to(info.addressModeV);
        j.at("AddressModeW").get_to(info.addressModeW);
        j.at("MipLodBias").get_to(info.mipLodBias);
        j.at("AnisotropyEnable").get_to(info.anisotropyEnable);
        j.at("MaxAnisotropy").get_to(info.maxAnisotropy);
        j.at("CompareEnable").get_to(info.compareEnable);
        j.at("CompareOp").get_to(info.compareOp);
        j.at("MinLod").get_to(info.minLod);
        j.at("MaxLod").get_to(info.maxLod);
        j.at("BorderColor").get_to(info.borderColor);
        j.at("UnnormalizedCoordinates").get_to(info.unnormalizedCoordinates);
    }

    void from_json(const json& j, VkAttachmentDescription& desc) {
        j.at("Flags").get_to(desc.flags);
        j.at("Format").get_to(desc.format);
        j.at("Samples").get_to(desc.samples);
        j.at("LoadOp").get_to(desc.loadOp);
        j.at("StoreOp").get_to(desc.storeOp);
        j.at("StencilLoadOp").get_to(desc.stencilLoadOp);
        j.at("StencilStoreOp").get_to(desc.stencilStoreOp);
        j.at("InitialLayout").get_to(desc.initialLayout);
        j.at("FinalLayout").get_to(desc.finalLayout);
    }

    void from_json(const json& j, VkSubpassDependency& dep) {
        j.at("SrcSubpass").get_to(dep.srcSubpass);
        j.at("DstSubpass").get_to(dep.dstSubpass);
        j.at("SrcStageMask").get_to(dep.srcStageMask);
        j.at("DstStageMask").get_to(dep.dstStageMask);
        j.at("SrcAccessMask").get_to(dep.srcAccessMask);
        j.at("DstAccessMask").get_to(dep.dstAccessMask);
        j.at("DependencyFlags").get_to(dep.dependencyFlags);
    }

    void from_json(const json& j, VkClearValue& clearValue) {
        if (j.contains("Color")) {
            auto& color = j.at("Color");
            if (j.contains("Int32_0")) 
            {
                color.at("Int32_0").get_to(clearValue.color.int32[0]);
                color.at("Int32_1").get_to(clearValue.color.int32[1]);
                color.at("Int32_2").get_to(clearValue.color.int32[2]);
                color.at("Int32_3").get_to(clearValue.color.int32[3]);
            }
            if (j.contains("Float32_0"))
            {
                color.at("Float32_0").get_to(clearValue.color.float32[0]);
                color.at("Float32_1").get_to(clearValue.color.float32[1]);
                color.at("Float32_2").get_to(clearValue.color.float32[2]);
                color.at("Float32_3").get_to(clearValue.color.float32[3]);
            }
            if (j.contains("Uint32_0"))
            {
                color.at("Uint32_0").get_to(clearValue.color.uint32[0]);
                color.at("Uint32_1").get_to(clearValue.color.uint32[1]);
                color.at("Uint32_2").get_to(clearValue.color.uint32[2]);
                color.at("Uint32_3").get_to(clearValue.color.uint32[3]);
            }
        }
        else if (j.contains("DepthStencil")) 
        {
            auto& depthStencil = j.at("DepthStencil");
            depthStencil.at("depth").get_to(clearValue.depthStencil.depth);
            depthStencil.at("stencil").get_to(clearValue.depthStencil.stencil);
        }
        else {
            throw std::runtime_error("Invalid VkClearValue: must contain 'color' or 'depthStencil'");
        }
    }

    void from_json(const json& j, VkRect2D& rect) {
        j.at("offset").at("x").get_to(rect.offset.x);
        j.at("offset").at("y").get_to(rect.offset.y);
        j.at("extent").at("width").get_to(rect.extent.width);
        j.at("extent").at("height").get_to(rect.extent.height);
    }

    void from_json(const json& j, VkGuid& guid) {
        std::string temp;
        j.get_to(temp);
        guid = VkGuid(temp.c_str());
    }

    void from_json(const json& j, VkViewport& viewPort)
    {
        j.at("x").get_to(viewPort.x);
        j.at("y").get_to(viewPort.y);
        j.at("width").get_to(viewPort.width);
        j.at("height").get_to(viewPort.height);
        j.at("minDepth").get_to(viewPort.minDepth);
        j.at("maxDepth").get_to(viewPort.maxDepth);
    }

    void from_json(const json& j, RenderPassAttachmentTexture& model) {
        j.at("RenderedTextureId").get_to(model.RenderedTextureId);
        j.at("RenderTextureType").get_to(model.RenderTextureType);
        j.at("RenderAttachmentTypes").get_to(model.RenderAttachmentTypes);
        j.at("Format").get_to(model.Format);
        j.at("LoadOp").get_to(model.LoadOp);
        j.at("StoreOp").get_to(model.StoreOp);
        j.at("FinalLayout").get_to(model.FinalLayout);
        j.at("UseSampler").get_to(model.UseSampler);
        j.at("UseMipMaps").get_to(model.UseMipMaps);
        j.at("IsCubeMapAttachment").get_to(model.IsCubeMapAttachment);
        j.at("IsTextureToExport").get_to(model.IsTextureToExport);

        if (model.UseMipMaps &&
            j.contains("MipMapCount"))
        {
            j.at("MipMapCount").get_to(model.MipMapCount);
        }
        else
        {
            model.MipMapCount = 1;
        }

        if (model.UseSampler && j.contains("SamplerCreateInfo"))
        {
            j.at("SamplerCreateInfo").get_to(model.SamplerCreateInfo);
        }
    }

    void from_json(const json& j, RenderedTextureInfoModel& model)
    {
        model.RenderedTextureInfoName = j.at("RenderedTextureInfoName");
        model.TextureType = j.at("TextureType");
        model.ImageCreateInfo = j.at("ImageCreateInfo");
        model.SamplerCreateInfo = j.at("SamplerCreateInfo");
        model.AttachmentDescription = j.at("AttachmentDescription");
    }

    void from_json(const json& j, PipelineDescriptorModel& model)
    {
        model.BindingNumber = j.at("BindingNumber");
        model.DstArrayElement = j.at("DstArrayElement");
        model.BindingPropertiesList = j.at("BindingPropertiesList");
        model.DescriptorType = j.at("DescriptorType");
        model.StageFlags = j.at("StageFlags");
    }

    void from_json(const json& j, RenderPassBuildInfoModel& model)
    {
        model.IsRenderedToSwapchain = j.at("IsRenderedToSwapchain").get<bool>();
        for (int x = 0; x < j.at("RenderPipelineList").size(); x++)
        {
            model.RenderPipelineList.emplace_back(j.at("RenderPipelineList")[x]["Path"]);
        }
        for (int x = 0; x < j.at("RenderedTextureInfoModelList").size(); x++)
        {
            model.RenderedTextureInfoModelList.emplace_back(j.at("RenderedTextureInfoModelList")[x]);
        }
        for (int x = 0; x < j.at("SubpassDependencyList").size(); x++)
        {
            model.SubpassDependencyModelList.emplace_back(j.at("SubpassDependencyList")[x]);
        }
        for (int x = 0; x < j.at("ClearValueList").size(); x++)
        {
            model.ClearValueList.emplace_back(j.at("ClearValueList")[x]);
        }
    }

    void from_json(const nlohmann::json& j, VkVertexInputBindingDescription& model)
    {
        j.at("binding").get_to(model.binding);
        j.at("stride").get_to(model.stride);
        j.at("inputRate").get_to(model.inputRate);
    }

    void from_json(const nlohmann::json& j, VkVertexInputAttributeDescription& model)
    {
        j.at("location").get_to(model.location);
        j.at("binding").get_to(model.binding);
        j.at("format").get_to(model.format);
        j.at("offset").get_to(model.offset);
    }

    void from_json(const nlohmann::json& j, VkPipelineColorBlendAttachmentState& model)
    {
        j.at("BlendEnable").get_to(model.blendEnable);
        j.at("SrcColorBlendFactor").get_to(model.srcColorBlendFactor);
        j.at("DstColorBlendFactor").get_to(model.dstColorBlendFactor);
        j.at("ColorBlendOp").get_to(model.colorBlendOp);
        j.at("SrcAlphaBlendFactor").get_to(model.srcAlphaBlendFactor);
        j.at("DstAlphaBlendFactor").get_to(model.dstAlphaBlendFactor);
        j.at("AlphaBlendOp").get_to(model.alphaBlendOp);
        j.at("ColorWriteMask").get_to(model.colorWriteMask);
    }

    void from_json(const nlohmann::json& j, VkPipelineColorBlendStateCreateInfo& model)
    {
        model.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        model.pNext = nullptr;
        model.flags = 0;
        j.at("LogicOpEnable").get_to(model.logicOpEnable);
        if (model.logicOpEnable)
        {
            j.at("LogicOp").get_to(model.logicOp);
            if (j.contains("BlendConstants"))
            {
                auto bc = j["BlendConstants"];
                j.at("BlendConstants").at("Red").get_to(model.blendConstants[0]);
                j.at("BlendConstants").at("Green").get_to(model.blendConstants[1]);
                j.at("BlendConstants").at("Blue").get_to(model.blendConstants[2]);
                j.at("BlendConstants").at("Alpha").get_to(model.blendConstants[3]);
            }
        }
    }

    void from_json(const nlohmann::json& j, VkPipelineRasterizationStateCreateInfo& model)
    {
        model.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        model.pNext = nullptr;
        model.flags = 0;
        j.at("depthClampEnable").get_to(model.depthClampEnable);
        j.at("rasterizerDiscardEnable").get_to(model.rasterizerDiscardEnable);
        j.at("polygonMode").get_to(model.polygonMode);
        j.at("cullMode").get_to(model.cullMode);
        j.at("frontFace").get_to(model.frontFace);
        j.at("depthBiasEnable").get_to(model.depthBiasEnable);
        j.at("depthBiasConstantFactor").get_to(model.depthBiasConstantFactor);
        j.at("depthBiasClamp").get_to(model.depthBiasClamp);
        j.at("depthBiasSlopeFactor").get_to(model.depthBiasSlopeFactor);
        j.at("lineWidth").get_to(model.lineWidth);
    }

    void from_json(const nlohmann::json& j, VkPipelineMultisampleStateCreateInfo& model)
    {
        model.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        model.pNext = nullptr;
        model.flags = 0;
        j.at("rasterizationSamples").get_to(model.rasterizationSamples);
        j.at("sampleShadingEnable").get_to(model.sampleShadingEnable);
        j.at("minSampleShading").get_to(model.minSampleShading);
        j.at("alphaToCoverageEnable").get_to(model.alphaToCoverageEnable);
        j.at("alphaToOneEnable").get_to(model.alphaToOneEnable);
    }

    void from_json(const nlohmann::json& j, VkPipelineDepthStencilStateCreateInfo& model)
    {
        model.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        model.pNext = nullptr;
        model.flags = 0;
        j.at("depthTestEnable").get_to(model.depthTestEnable);
        j.at("depthWriteEnable").get_to(model.depthWriteEnable);
        j.at("depthCompareOp").get_to(model.depthCompareOp);
        j.at("depthBoundsTestEnable").get_to(model.depthBoundsTestEnable);
        j.at("stencilTestEnable").get_to(model.stencilTestEnable);
        j.at("minDepthBounds").get_to(model.minDepthBounds);
        j.at("maxDepthBounds").get_to(model.maxDepthBounds);

        if (model.stencilTestEnable)
        {
            if (j.contains("front"))
            {
                j.at("front").at("failOp").get_to(model.front.failOp);
                j.at("front").at("passOp").get_to(model.front.passOp);
                j.at("front").at("depthFailOp").get_to(model.front.depthFailOp);
                j.at("front").at("compareOp").get_to(model.front.compareOp);
                j.at("front").at("compareMask").get_to(model.front.compareMask);
                j.at("front").at("writeMask").get_to(model.front.writeMask);
                j.at("front").at("reference").get_to(model.front.reference);
            }
            if (j.contains("back"))
            {
                j.at("back").at("failOp").get_to(model.back.failOp);
                j.at("back").at("passOp").get_to(model.back.passOp);
                j.at("back").at("depthFailOp").get_to(model.back.depthFailOp);
                j.at("back").at("compareOp").get_to(model.back.compareOp);
                j.at("back").at("compareMask").get_to(model.back.compareMask);
                j.at("back").at("writeMask").get_to(model.back.writeMask);
                j.at("back").at("reference").get_to(model.back.reference);
            }
        }
    }

    void from_json(const nlohmann::json& j, VkPipelineInputAssemblyStateCreateInfo& model)
    {
        model.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        model.pNext = nullptr;
        model.flags = 0;
        j.at("topology").get_to(model.topology);
        j.at("primitiveRestartEnable").get_to(model.primitiveRestartEnable);
    }

    void from_json(const nlohmann::json& j, BlendConstantsModel& model)
    {
        j.at("Red").get_to(model.Red);
        j.at("Green").get_to(model.Green);
        j.at("Blue").get_to(model.Blue);
        j.at("Alpha").get_to(model.Alpha);
    }

    void from_json(const nlohmann::json& j, VkDescriptorSetLayoutBinding& model)
    {
        j.at("binding").get_to(model.binding);
        j.at("descriptorType").get_to(model.descriptorType);
        j.at("descriptorCount").get_to(model.descriptorCount);
        j.at("stageFlags").get_to(model.stageFlags);
    }
    
    void from_json(const json& j, RenderPassLoader& model)
    {
        j.at("RenderPassId").get_to(model.RenderPassId);
        j.at("SubPassCount").get_to(model.SubPassCount);
        j.at("IsRenderedToSwapchain").get_to(model.IsRenderedToSwapchain);
        j.at("RenderPipelineList").get_to(model.RenderPipelineList);
        j.at("RenderAttachmentList").get_to(model.RenderAttachmentList);
        j.at("SubpassDependencyList").get_to(model.SubpassDependencyModelList);
        j.at("ClearValueList").get_to(model.ClearValueList);
        j.at("UseDefaultSwapChainResolution").get_to(model.UseDefaultSwapChainResolution);
        j.at("UseCubeMapMultiView").get_to(model.UseCubeMapMultiView);

        if (model.UseDefaultSwapChainResolution)
        {
            model.RenderPassResolution = ivec2(static_cast<int>(vulkanSystem.SwapChainResolution.width, static_cast<int>(vulkanSystem.SwapChainResolution.height)));
        }
        else
        {
            model.RenderPassResolution = ivec2(j.at("RenderPassResolution")[0], j.at("RenderPassResolution")[1]);
        }

        if (j.contains("InputTextureList"))
        {
            for (int x = 0; x < j.at("InputTextureList").size(); x++)
            {
                model.InputTextureList.emplace_back(VkGuid(j["InputTextureList"][x].get<String>().c_str()));
            }
        }
    }

  void from_json(const json& j, RenderPipelineLoader& model)
    {
        j.at("PipelineId").get_to(model.PipelineId);
        j.at("SubPassId").get_to(model.SubPassId);
        j.at("UseDynamicColorWrite").get_to(model.UseDynamicColorWrite);
        j.at("PipelineRasterizationStateCreateInfo").get_to(model.PipelineRasterizationStateCreateInfo);
        j.at("PipelineMultisampleStateCreateInfo").get_to(model.PipelineMultisampleStateCreateInfo);
        j.at("PipelineDepthStencilStateCreateInfo").get_to(model.PipelineDepthStencilStateCreateInfo);
        j.at("PipelineInputAssemblyStateCreateInfo").get_to(model.PipelineInputAssemblyStateCreateInfo);
        j.at("PipelineColorBlendStateCreateInfoModel").get_to(model.PipelineColorBlendStateCreateInfoModel);
        for (auto& pipelineColorBlendAttachmentState : j.at("PipelineColorBlendAttachmentStateList"))
        {
            model.PipelineColorBlendAttachmentStateList.emplace_back(pipelineColorBlendAttachmentState);
        }
        for (auto& viewPort : j.at("ViewportList"))
        {
            model.ViewportList.emplace_back(viewPort);
        }
        for (auto& scissor : j.at("ScissorList"))
        {
            model.ScissorList.emplace_back(scissor);
        }
    }

    void from_json(const json& j, TextureLoader& model)
    {
        for (int x = 0; x < j.at("TextureFilePath").size(); x++)
        {
            model.TextureFilePath.emplace_back(j["TextureFilePath"][x].get<String>().c_str());
        }

        j.at("TextureId").get_to(model.TextureId);
        j.at("ImageType").get_to(model.ImageType);
        j.at("TextureByteFormat").get_to(model.TextureByteFormat);
        j.at("UsingSRGBFormat").get_to(model.UsingSRGBFormat);
        j.at("TextureType").get_to(model.TextureType);
        j.at("UseMipMaps").get_to(model.UseMipMaps);
        j.at("IsSkyBox").get_to(model.IsSkyBox);
        j.at("SamplerCreateInfo").get_to(model.SamplerCreateInfo);
    }
}

