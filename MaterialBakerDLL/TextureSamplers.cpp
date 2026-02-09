#include "TextureSamplers.h"

VkSampler TextureSamplers::GetImportAlbedoMapSamplerSettings()
{
    VkSamplerCreateInfo samplerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = -0.5f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 16.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &samplerCreateInfo, nullptr, &sampler));
    return sampler;
}

VkSampler TextureSamplers::GetImportNormalMapSamplerSettings()
{
    VkSamplerCreateInfo samplerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = -0.75f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 16.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &samplerCreateInfo, nullptr, &sampler));
    return sampler;
}

VkSampler TextureSamplers::GetImportPackedORMMapSamplerSettings()
{
    VkSamplerCreateInfo samplerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 8.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &samplerCreateInfo, nullptr, &sampler));
    return sampler;
}

VkSampler TextureSamplers::GetImportParallaxMapSamplerSettings()
{
    VkSamplerCreateInfo samplerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = -0.5f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 12.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &samplerCreateInfo, nullptr, &sampler));
    return sampler;
}

VkSampler TextureSamplers::GetImportAlphaMapSamplerSettings()
{
    VkSamplerCreateInfo samplerCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 8.0f,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler = VK_NULL_HANDLE;
    VULKAN_THROW_IF_FAIL(vkCreateSampler(vulkanSystem.Device, &samplerCreateInfo, nullptr, &sampler));
    return sampler;
}

nlohmann::json TextureSamplers::GetAlbedoMaterialSamplerSettings(nlohmann::json& j)
{
    j["SType"] = static_cast<int>(VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
    j["MagFilter"] = static_cast<int>(VK_FILTER_LINEAR);
    j["MinFilter"] = static_cast<int>(VK_FILTER_LINEAR);
    j["MipmapMode"] = static_cast<int>(VK_SAMPLER_MIPMAP_MODE_LINEAR);
    j["AddressModeU"] = static_cast<int>(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    j["AddressModeV"] = static_cast<int>(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    j["AddressModeW"] = static_cast<int>(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    j["MipLodBias"] = -0.25f;   // Slight sharpness boost for detail
    j["AnisotropyEnable"] = true;
    j["MaxAnisotropy"] = 16.0f;   // Max desktop quality for oblique angles
    j["CompareEnable"] = false;
    j["CompareOp"] = static_cast<int>(VK_COMPARE_OP_ALWAYS);
    j["MinLod"] = 0.0f;
    j["MaxLod"] = 1000.0f;
    j["BorderColor"] = static_cast<int>(VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK);
    j["UnnormalizedCoordinates"] = false;
    return j;
}

nlohmann::json TextureSamplers::GetNormalMaterialSamplerSettings(nlohmann::json& j) 
{
    j["SType"] = static_cast<int>(VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
    j["MagFilter"] = static_cast<int>(VK_FILTER_LINEAR);
    j["MinFilter"] = static_cast<int>(VK_FILTER_LINEAR);
    j["MipmapMode"] = static_cast<int>(VK_SAMPLER_MIPMAP_MODE_LINEAR);
    j["AddressModeU"] = static_cast<int>(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    j["AddressModeV"] = static_cast<int>(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    j["AddressModeW"] = static_cast<int>(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    j["MipLodBias"] = -0.75f;
    j["AnisotropyEnable"] = true;
    j["MaxAnisotropy"] = 8.0f;
    j["CompareEnable"] = false;
    j["CompareOp"] = static_cast<int>(VK_COMPARE_OP_ALWAYS);
    j["MinLod"] = 0.0f;
    j["MaxLod"] = 1000.0f;
    j["BorderColor"] = static_cast<int>(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);
    j["UnnormalizedCoordinates"] = false;
    return j;
}

nlohmann::json TextureSamplers::GetMROMaterialSamplerSettings(nlohmann::json& j) {
    j["SType"] = static_cast<int>(VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
    j["MagFilter"] = static_cast<int>(VK_FILTER_NEAREST);
    j["MinFilter"] = static_cast<int>(VK_FILTER_NEAREST);
    j["MipmapMode"] = static_cast<int>(VK_SAMPLER_MIPMAP_MODE_NEAREST);
    j["AddressModeU"] = static_cast<int>(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    j["AddressModeV"] = static_cast<int>(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    j["AddressModeW"] = static_cast<int>(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    j["MipLodBias"] = 0.0f;
    j["AnisotropyEnable"] = false;
    j["CompareEnable"] = false;
    j["CompareOp"] = static_cast<int>(VK_COMPARE_OP_ALWAYS);
    j["MinLod"] = 0.0f;
    j["MaxLod"] = 1000.0f;
    j["BorderColor"] = static_cast<int>(VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK);
    j["UnnormalizedCoordinates"] = false;
    return j;
}

nlohmann::json TextureSamplers::GetEmissionSamplerSettings(nlohmann::json& j) 
{
    j["SType"] = static_cast<int>(VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
    j["MagFilter"] = static_cast<int>(VK_FILTER_LINEAR);
    j["MinFilter"] = static_cast<int>(VK_FILTER_LINEAR);
    j["MipmapMode"] = static_cast<int>(VK_SAMPLER_MIPMAP_MODE_LINEAR);
    j["AddressModeU"] = static_cast<int>(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    j["AddressModeV"] = static_cast<int>(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    j["AddressModeW"] = static_cast<int>(VK_SAMPLER_ADDRESS_MODE_REPEAT);
    j["MipLodBias"] = 0.0f; 
    j["AnisotropyEnable"] = true;
    j["MaxAnisotropy"] = 8.0f;    
    j["CompareEnable"] = false;
    j["CompareOp"] = static_cast<int>(VK_COMPARE_OP_ALWAYS);
    j["MinLod"] = 0.0f;
    j["MaxLod"] = 1000.0f;
    j["BorderColor"] = static_cast<int>(VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK);
    j["UnnormalizedCoordinates"] = false;
    return j;
}