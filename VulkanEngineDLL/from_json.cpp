#include "from_json.h"

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

    void from_json(const nlohmann::json& j, VkImageCreateInfo& info) {
        info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
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

    void from_json(const nlohmann::json& j, VkSamplerCreateInfo& info) {
        info = Json_LoadVulkanSamplerCreateInfo(j);
    }

    void from_json(const nlohmann::json& j, VkAttachmentDescription& desc) {
        desc = Json_LoadAttachmentDescription(j);
    }

    void from_json(const nlohmann::json& j, VkSubpassDependency& dep) {
        j.at("srcSubpass").get_to(dep.srcSubpass);
        j.at("dstSubpass").get_to(dep.dstSubpass);
        j.at("srcStageMask").get_to(dep.srcStageMask);
        j.at("dstStageMask").get_to(dep.dstStageMask);
        j.at("srcAccessMask").get_to(dep.srcAccessMask);
        j.at("dstAccessMask").get_to(dep.dstAccessMask);
        j.at("dependencyFlags").get_to(dep.dependencyFlags);
    }

    void from_json(const nlohmann::json& j, VkClearValue& clearValue) {
        if (j.contains("Color")) {
            auto& color = j.at("Color");
            color.at("Int32_0").get_to(clearValue.color.int32[0]);
            color.at("Int32_1").get_to(clearValue.color.int32[1]);
            color.at("Int32_2").get_to(clearValue.color.int32[2]);
            color.at("Int32_3").get_to(clearValue.color.int32[3]);
            color.at("Float32_0").get_to(clearValue.color.float32[0]);
            color.at("Float32_1").get_to(clearValue.color.float32[1]);
            color.at("Float32_2").get_to(clearValue.color.float32[2]);
            color.at("Float32_3").get_to(clearValue.color.float32[3]);
            color.at("Uint32_0").get_to(clearValue.color.uint32[0]);
            color.at("Uint32_1").get_to(clearValue.color.uint32[1]);
            color.at("Uint32_2").get_to(clearValue.color.uint32[2]);
            color.at("Uint32_3").get_to(clearValue.color.uint32[3]);
        }
        else if (j.contains("DepthStencil")) {
            auto& ds = j.at("DepthStencil");
            ds.at("DepthStencil").get_to(clearValue.depthStencil.depth);
            ds.at("Stencil").get_to(clearValue.depthStencil.stencil);
        }
        else {
            throw std::runtime_error("Invalid VkClearValue: must contain 'color' or 'depthStencil'");
        }
    }

    void from_json(const nlohmann::json& j, VkRect2D& rect) {
        j.at("offset").at("x").get_to(rect.offset.x);
        j.at("offset").at("y").get_to(rect.offset.y);
        j.at("extent").at("width").get_to(rect.extent.width);
        j.at("extent").at("height").get_to(rect.extent.height);
    }

    void from_json(const nlohmann::json& j, RenderAreaModel& area) {
        j.at("RenderArea").get_to(area.RenderArea);
        j.at("UseDefaultRenderArea").get_to(area.UseDefaultRenderArea);
    }

    void from_json(const nlohmann::json& j, VkGuid& guid) {
        std::string temp;
        j.get_to(temp);
        guid = VkGuid(temp.c_str());
    }

    void from_json(const nlohmann::json& j, RenderedTextureLoader& model) {
        j.at("RenderedTextureInfoName").get_to(model.RenderedTextureInfoName);
        j.at("TextureType").get_to(model.TextureType);
        j.at("ImageCreateInfo").get_to(model.ImageCreateInfo);
        j.at("SamplerCreateInfo").get_to(model.SamplerCreateInfo);
        j.at("AttachmentDescription").get_to(model.AttachmentDescription);
    }
}