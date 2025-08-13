#pragma once
#include <nlohmann/json.hpp>
#include "Typedef.h"
#include <vulkan/vulkan.h>
#include "JsonStructs.h"
#include "JsonStruct.h"

namespace nlohmann
{
    DLL_EXPORT void from_json(const json& j, VkExtent2D& extent);
    DLL_EXPORT void from_json(const json& j, VkExtent3D& extent);
    DLL_EXPORT void from_json(const json& j, VkOffset2D& offset);
    DLL_EXPORT void from_json(const json& j, VkOffset3D& offset);
    DLL_EXPORT void from_json(const nlohmann::json& j, VkImageCreateInfo& info);
    DLL_EXPORT void from_json(const nlohmann::json& j, VkSamplerCreateInfo& info);
    DLL_EXPORT void from_json(const nlohmann::json& j, VkAttachmentDescription& desc);
    DLL_EXPORT void from_json(const nlohmann::json& j, VkSubpassDependency& dep);
    DLL_EXPORT void from_json(const nlohmann::json& j, VkClearValue& clearValue);
    DLL_EXPORT void from_json(const nlohmann::json& j, VkRect2D& rect);
    DLL_EXPORT void from_json(const nlohmann::json& j, RenderAreaModel& area);
    DLL_EXPORT void from_json(const nlohmann::json& j, VkGuid& guid);
    DLL_EXPORT void from_json(const nlohmann::json& j, RenderedTextureLoader& model);
}