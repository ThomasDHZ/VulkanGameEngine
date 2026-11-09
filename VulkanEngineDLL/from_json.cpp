#include "from_json.h"
#include <VulkanSystem.h>

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
        j.at("imageType").get_to(info.imageType);
        j.at("format").get_to(info.format);
        j.at("mipLevels").get_to(info.mipLevels);
        j.at("arrayLayers").get_to(info.arrayLayers);
        j.at("samples").get_to(info.samples);
        j.at("tiling").get_to(info.tiling);
        j.at("usage").get_to(info.usage);
        j.at("sharingMode").get_to(info.sharingMode);
        j.at("initialLayout").get_to(info.initialLayout);
    }

    void from_json(const json& j, VkSamplerCreateInfo& info)
    {
        auto a = j.dump();
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        j.at("magFilter").get_to(info.magFilter);
        j.at("minFilter").get_to(info.minFilter);
        j.at("mipmapMode").get_to(info.mipmapMode);
        j.at("addressModeU").get_to(info.addressModeU);
        j.at("addressModeV").get_to(info.addressModeV);
        j.at("addressModeW").get_to(info.addressModeW);
        j.at("mipLodBias").get_to(info.mipLodBias);
        info.anisotropyEnable = j.at("anisotropyEnable").get<VkBool32>();
        j.at("maxAnisotropy").get_to(info.maxAnisotropy);
        info.compareEnable = j.at("compareEnable").get<VkBool32>();
        j.at("compareOp").get_to(info.compareOp);
        j.at("minLod").get_to(info.minLod);
        j.at("maxLod").get_to(info.maxLod);
        j.at("borderColor").get_to(info.borderColor);
        j.at("unnormalizedCoordinates").get_to(info.unnormalizedCoordinates);
    }

    void from_json(const json& j, VkAttachmentDescription& desc) {
        j.at("flags").get_to(desc.flags);
        j.at("format").get_to(desc.format);
        j.at("samples").get_to(desc.samples);
        j.at("loadOp").get_to(desc.loadOp);
        j.at("storeOp").get_to(desc.storeOp);
        j.at("stencilLoadOp").get_to(desc.stencilLoadOp);
        j.at("stencilStoreOp").get_to(desc.stencilStoreOp);
        j.at("initialLayout").get_to(desc.initialLayout);
        j.at("finalLayout").get_to(desc.finalLayout);
    }

    void from_json(const json& j, VkSubpassDependency& dep) {
        j.at("srcSubpass").get_to(dep.srcSubpass);
        j.at("dstSubpass").get_to(dep.dstSubpass);
        j.at("srcStageMask").get_to(dep.srcStageMask);
        j.at("dstStageMask").get_to(dep.dstStageMask);
        j.at("srcAccessMask").get_to(dep.srcAccessMask);
        j.at("dstAccessMask").get_to(dep.dstAccessMask);
        j.at("dependencyFlags").get_to(dep.dependencyFlags);
    }

    void from_json(const json& j, VkClearValue& clearValue)
    {
        if (j["ClearColorType"] == 0)
        {
            auto& color = j["ClearValue"].at("Color");
            if (color.contains("Int32_0"))   color.at("Int32_0").get_to(clearValue.color.int32[0]);
            if (color.contains("Int32_1"))   color.at("Int32_1").get_to(clearValue.color.int32[1]);
            if (color.contains("Int32_2"))   color.at("Int32_2").get_to(clearValue.color.int32[2]);
            if (color.contains("Int32_3"))   color.at("Int32_3").get_to(clearValue.color.int32[3]);
            if (color.contains("Float32_0")) color.at("Float32_0").get_to(clearValue.color.float32[0]);
            if (color.contains("Float32_1")) color.at("Float32_1").get_to(clearValue.color.float32[1]);
            if (color.contains("Float32_2")) color.at("Float32_2").get_to(clearValue.color.float32[2]);
            if (color.contains("Float32_3")) color.at("Float32_3").get_to(clearValue.color.float32[3]);
            if (color.contains("Uint32_0"))  color.at("Uint32_0").get_to(clearValue.color.uint32[0]);
            if (color.contains("Uint32_1"))  color.at("Uint32_1").get_to(clearValue.color.uint32[1]);
            if (color.contains("Uint32_2"))  color.at("Uint32_2").get_to(clearValue.color.uint32[2]);
            if (color.contains("Uint32_3"))  color.at("Uint32_3").get_to(clearValue.color.uint32[3]);
        }
        else
        {
            auto& depthStencil = j["ClearValue"].at("DepthStencil");
            depthStencil.at("depth").get_to(clearValue.depthStencil.depth);
            depthStencil.at("stencil").get_to(clearValue.depthStencil.stencil);
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

    void from_json(const json& j, RenderPassAttachmentLoader& model) {
        j.at("AttachmentName").get_to(model.AttachmentName);
        j.at("RenderedTextureId").get_to(model.RenderedTextureId);
        j.at("TextureType").get_to(model.TextureType);
        j.at("TextureUsageType").get_to(model.TextureUsageType);
        j.at("RenderAttachmentTypes").get_to(model.RenderAttachmentTypes);
        j.at("TextureByteLayout").get_to(model.TextureByteLayout);
        j.at("LoadOp").get_to(model.LoadOp);
        j.at("StoreOp").get_to(model.StoreOp);
        j.at("FinalLayout").get_to(model.FinalLayout);
        j.at("UseMipMaps").get_to(model.UseMipMaps);
        j.at("SamplerCreateInfo").get_to(model.SamplerCreateInfo);

        if (model.UseMipMaps && j.contains("MipMapCount")) j.at("MipMapCount").get_to(model.MipMapCount);
        else model.MipMapCount = 1;
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
        for (int x = 0; x < j.at("RenderPipelineList").size(); x++)           model.RenderPipelineList.emplace_back(j.at("RenderPipelineList")[x]["Path"]);
        for (int x = 0; x < j.at("RenderedTextureInfoModelList").size(); x++) model.RenderedTextureInfoModelList.emplace_back(j.at("RenderedTextureInfoModelList")[x]);
        for (int x = 0; x < j.at("SubpassDependencyList").size(); x++)        model.SubpassDependencyModelList.emplace_back(j.at("SubpassDependencyList")[x]);
        for (int x = 0; x < j.at("ClearValueList").size(); x++)               model.ClearValueList.emplace_back(j.at("ClearValueList")[x]);
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
        j.at("blendEnable").get_to(model.blendEnable);
        j.at("srcColorBlendFactor").get_to(model.srcColorBlendFactor);
        j.at("dstColorBlendFactor").get_to(model.dstColorBlendFactor);
        j.at("colorBlendOp").get_to(model.colorBlendOp);
        j.at("srcAlphaBlendFactor").get_to(model.srcAlphaBlendFactor);
        j.at("dstAlphaBlendFactor").get_to(model.dstAlphaBlendFactor);
        j.at("alphaBlendOp").get_to(model.alphaBlendOp);
        j.at("colorWriteMask").get_to(model.colorWriteMask);
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

    void from_json(const json& j, PushConstantUpdateRule& model)
    {
        j.at("VariableName").get_to(model.Variable);
        auto type = j["PushConstantType"].get<PushConstantVarTypeEnum>();

        if (type == kPushConstantVarType_PushConstant)
        {
            PushConstantConstVariable c;
            c.Value = j.get<PushConstantVariable>(); 
            model.PushConstant = c;
        }
        else if (type == kPushConstantVarType_TextureRefrence)
        {
            model.PushConstant = PushConstantRef{
                .AssetId = VkGuid(j["PushConstantRef"]["AssetId"].get<String>().c_str()),
                .TextureVar = j["PushConstantRef"]["TextureVar"].get<TextureVarTypeEnum>()
            };
        }
    }

    void from_json(const json& j, VulkanSubPassLoader& model)
    {
        j.at("MeshType").get_to(model.MeshType);
        j.at("RenderPassDrawFlags").get_to(model.RenderPassDrawFlags);
        j.at("PipelinePackageId").get_to(model.PipelinePackageId);
        j.at("ShaderPushConstant").get_to(model.ShaderPushConstant);
        j.at("InputAttachmentList").get_to(model.InputAttachmentList);
        j.at("InputTextureList").get_to(model.InputTextureList);
        j.at("OutputTextureList").get_to(model.OutputTextureList);
        j.at("OffScreenRenderPass").get_to(model.OffScreenRenderPass);
        j.at("AlphaRenderPass").get_to(model.AlphaRenderPass);
        j.at("UseCustomDrawCommand").get_to(model.UseCustomDrawCommand);

        if (j["UseCustomDrawCommand"].get<bool>())
        {
            j.at("CustomDrawCommand").get_to(model.CustomDrawCommand);
            model.CustomDrawCommandContext = j["CustomDrawCommandContext"].get<CustomDrawInitContext>();
        }
        if (j.contains("PushConstantUpdates") &&
            j["PushConstantUpdates"].is_array())
        {
            for (const auto& pushConstantUpdateRules : j["PushConstantUpdates"])
            {
                j.at("PushConstantUpdates").get_to(model.PushConstantUpdates);
            }
        }
    }

    void from_json(const json& j, RenderPassLoader& model)
    {
        auto a = j.dump();
        j.at("Name").get_to(model.Name);
        j.at("RenderPassId").get_to(model.RenderPassId);
        j.at("UseDefaultRenderResolution").get_to(model.UseDefaultRenderResolution);
        j.at("UseGlobalBindlessSet").get_to(model.UseGlobalBindlessSet);
        j.at("UseVkMultiview").get_to(model.UseVkMultiview);
        j.at("RenderAsCubemap").get_to(model.RenderAsCubemap);
        j.at("UseFrameBufferResolution").get_to(model.UseFrameBufferResolution);
        j.at("OneTimeDraw").get_to(model.OneTimeDraw);
        model.RenderPassResolution = model.UseDefaultRenderResolution ? vulkan.RenderPassResolution() : ivec2(j.at("RenderPassResolution")[0], j.at("RenderPassResolution")[1]);
        j.at("AttachmentList").get_to(model.AttachmentList);
        j.at("SubpassDependencyList").get_to(model.SubpassDependencyList);
        j.at("ClearValueList").get_to(model.ClearValueList);
        j.at("SampleCount").get_to(model.SampleCount);

        if (j.contains("ShaderList") && j["ShaderList"].is_array())
        {
            for (const auto& shader : j["ShaderList"])
            {
                model.ShaderList.push_back(shader.get<ShaderLoader>());
            }
        }
        if (j.contains("PipelineList") && j["PipelineList"].is_array())
        {
            for (const auto& pipeline : j["PipelineList"])
            {
                model.PipelineList.push_back(pipeline.get<String>());
            }
        }
        if (j.contains("PipelinePackageList") && j["PipelinePackageList"].is_array())
        {
            for (const auto& pipelinePackage : j["PipelinePackageList"])
            {
                model.PipelinePackageList.push_back(pipelinePackage.get<VulkanPipelinePackage>());
            }
        }
        if (j.contains("SubPassList") && j["SubPassList"].is_array())
        {
            for (const auto& subPass : j["SubPassList"])
            {
                Vector<VulkanSubPassLoader> subPassLoader;
                if (subPass.is_array())
                {
                    for (const auto& item : subPass)
                    {
                        subPassLoader.push_back(item.get<VulkanSubPassLoader>());
                    }
                }
                model.SubPassList.push_back(subPassLoader);
            }
        }
    }

    void from_json(const json& j, VulkanPipelinePackage& model)
    {
        j.at("PipelinePackageId").get_to(model.PipelinePackageId);
        if (j.contains("PipelineList") && j["PipelineList"].is_array())
        {
            for (const auto& pipeline : j["PipelineList"])
            {
                model.PipelineMap[pipeline["PipelineType"].get<PipelineTypeEnum>()] = pipeline["PipelineId"].get<VkGuid>();
            }
        }
    }

    void from_json(const json& j, VulkanPipelineLoader& model)
    {
        j.at("PipelineId").get_to(model.PipelineId);
        j.at("SubPassId").get_to(model.SubPassId);
        j.at("PipelineRasterizationStateCreateInfo").get_to(model.PipelineRasterizationStateCreateInfo);
        j.at("PipelineMultisampleStateCreateInfo").get_to(model.PipelineMultisampleStateCreateInfo);
        j.at("PipelineDepthStencilStateCreateInfo").get_to(model.PipelineDepthStencilStateCreateInfo);
        j.at("PipelineInputAssemblyStateCreateInfo").get_to(model.PipelineInputAssemblyStateCreateInfo);
        j.at("PipelineColorBlendStateCreateInfoModel").get_to(model.PipelineColorBlendStateCreateInfoModel);
        for (auto& shader : j.at("ShaderList"))                                                       model.ShaderIdList.emplace_back(shader);
        for (auto& pipelineColorBlendAttachmentState : j.at("PipelineColorBlendAttachmentStateList")) model.PipelineColorBlendAttachmentStateList.emplace_back(pipelineColorBlendAttachmentState);
        for (auto& viewPort : j.at("ViewportList"))                                                   model.ViewportList.emplace_back(viewPort);
        for (auto& scissor : j.at("ScissorList"))                                                     model.ScissorList.emplace_back(scissor);
    }

    void from_json(const json& j, TextureLoader& model)
    {
        for (int x = 0; x < j.at("TextureFilePath").size(); x++)
        {
            model.TextureFilePath.emplace_back(j["TextureFilePath"][x].get<String>().c_str());
        }

        j.at("TextureId").get_to(model.TextureId);
        j.at("TextureByteLayout").get_to(model.TextureByteLayout);
        j.at("TextureType").get_to(model.TextureType);
        j.at("TextureUsageType").get_to(model.TextureUsageType);
        j.at("MipMapCount").get_to(model.MipMapCount);
        j.at("SamplerCreateInfo").get_to(model.SamplerCreateInfo);
    }

    void from_json(const json& j, Collider2DComponent& model)
    {
        model.Size = ivec2(j.at("ColliderSize")[0], j.at("ColliderSize")[1]);
        model.Offset = ivec2(j.at("ColliderOffset")[0], j.at("ColliderOffset")[1]);
        j.at("ColliderEnabled").get_to(model.Enabled);
        j.at("IsTrigger").get_to(model.IsTrigger);
    }

    void from_json(const json& j, Transform2DComponent& model)
    {
        model.GameObjectPosition = ivec2(j.at("GameObjectPosition")[0], j.at("GameObjectPosition")[1]);
        model.GameObjectRotation = ivec2(j.at("GameObjectRotation")[0], j.at("GameObjectRotation")[1]);
        model.GameObjectScale = ivec2(j.at("GameObjectScale")[0], j.at("GameObjectScale")[1]);
    }

    void from_json(const json& j, ShaderLoader& model)
    {
        j.at("ShaderId").get_to(model.ShaderId);
        j.at("ShaderFile").get_to(model.ShaderFile);
        j.at("ShaderStage").get_to(model.ShaderStage);
    }

    void from_json(const json& j, PushConstantRef& model)
    {
        j.at("AssetId").get_to(model.AssetId);
        j.at("TextureVarnameEnum").get_to(model.TextureVar);
    }

    void from_json(const json& j, PushConstantConstVariable& model)
    {
        j.at("Value").get_to(model.Value);
    }

    void from_json(const json& j, CustomDrawInitContext& model)
    {
        j.at("RenderPassId").get_to(model.RenderPassId);
        j.at("TextureAttachmentId").get_to(model.TextureAttachmentId);
        j.at("TileSize").get_to(model.TileSize);
    }

    void from_json(const json& j, ivec2& v) 
    {
        v.x = j.at(0);
        v.y = j.at(1);
    }

    void from_json(const json& j, ivec3& v) 
    {
        v.x = j.at(0); 
        v.y = j.at(1); 
        v.z = j.at(2);
    }

    void from_json(const json& j, ivec4& v) 
    {
        v.x = j.at(0); 
        v.y = j.at(1); 
        v.z = j.at(2); 
        v.w = j.at(3);
    }

    void from_json(const json& j, vec2& v) 
    {
        v.x = j.at(0); 
        v.y = j.at(1);
    }

    void from_json(const json& j, vec3& v) 
    {
        v.x = j.at(0); 
        v.y = j.at(1); 
        v.z = j.at(2);
    }

    void from_json(const json& j, vec4& v) 
    {
        v.x = j.at(0); 
        v.y = j.at(1); 
        v.z = j.at(2); 
        v.w = j.at(3);
    }

    void from_json(const json& j, mat2& m)
    {
        m[0][0] = j.at(0);
        m[0][1] = j.at(1);

        m[1][0] = j.at(2);
        m[1][1] = j.at(3);
    }

    void from_json(const json& j, mat3& m)
    {
        m[0][0] = j.at(0); 
        m[0][1] = j.at(1); 
        m[0][2] = j.at(2);

        m[1][0] = j.at(3); 
        m[1][1] = j.at(4); 
        m[1][2] = j.at(5);

        m[2][0] = j.at(6); 
        m[2][1] = j.at(7); 
        m[2][2] = j.at(8);
    }

    void from_json(const json& j, mat4& m)
    {
        m[0][0] = j.at(0);
        m[0][1] = j.at(1);
        m[0][2] = j.at(2);
        m[0][3] = j.at(3);

        m[1][0] = j.at(4);
        m[1][1] = j.at(5);
        m[1][2] = j.at(6);
        m[1][3] = j.at(7);

        m[2][0] = j.at(8);
        m[2][1] = j.at(9);
        m[2][2] = j.at(10);
        m[2][3] = j.at(11);

        m[3][0] = j.at(12);
        m[3][1] = j.at(13);
        m[3][2] = j.at(14);
        m[3][3] = j.at(15);
    }
}

