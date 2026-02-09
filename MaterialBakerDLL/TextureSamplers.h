#pragma once
#include <Platform.h>
#include <VulkanSystem.h>

class TextureSamplers
{
public:
	static VkSampler GetImportAlbedoMapSamplerSettings();
    static VkSampler GetImportNormalMapSamplerSettings();
    static VkSampler GetImportPackedORMMapSamplerSettings();
    static VkSampler GetImportParallaxMapSamplerSettings();
    static VkSampler GetImportAlphaMapSamplerSettings();
    static VkSampler GetImportThicknessMapSamplerSettings() { return GetImportPackedORMMapSamplerSettings(); }
    static VkSampler GetImportSubSurfaceScatteringMapSamplerSettings() { return GetImportPackedORMMapSamplerSettings(); }
    static VkSampler GetImportSheenMapSamplerSettings() { return GetImportPackedORMMapSamplerSettings(); }
    static VkSampler GetImportClearCoatMapSamplerSettings() { return GetImportPackedORMMapSamplerSettings(); }
    static VkSampler GetImportEmissionMapSamplerSettings() { return GetImportAlbedoMapSamplerSettings(); }

    static nlohmann::json GetAlbedoMaterialSamplerSettings(nlohmann::json& json);
    static nlohmann::json GetNormalMaterialSamplerSettings(nlohmann::json& json);
    static nlohmann::json GetMROMaterialSamplerSettings(nlohmann::json& json);
    static nlohmann::json GetSheenSSSSamplerSettings(nlohmann::json& json) { return GetMROMaterialSamplerSettings(json); }
    static nlohmann::json GetUnusedSamplerSettings(nlohmann::json& json) { return GetMROMaterialSamplerSettings(json); }
    static nlohmann::json GetEmissionSamplerSettings(nlohmann::json& json);
};

