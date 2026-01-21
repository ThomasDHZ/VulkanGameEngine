#include "pch.h"
#include "AssetCreatorSystem.h"
#include <ShaderSystem.h>
#include <FileSystem.h>
#include <EngineConfigSystem.h>
#include <TextureSystem.h>

AssetCreatorSystem& assetCreatorSystem = AssetCreatorSystem::Get();

void AssetCreatorSystem::Run()
{
    const String inDir = configSystem.MaterialSourceDirectory.c_str();
    std::filesystem::path outDir = configSystem.MaterialDstDirectory.c_str();
    std::filesystem::create_directories(outDir);

    Vector<Material> materialList;
    Vector<String> ext = { ".json" };
    const VkGuid dummyGuid = VkGuid();
    Vector<String> materialFiles = fileSystem.GetFilesFromDirectory(configSystem.MaterialSourceDirectory.c_str(), ext);
    for (auto& materialPath : materialFiles)
    {
        std::filesystem::path src = materialPath;
        std::filesystem::path dst = outDir / (src.filename().stem().string() + fileSystem.GetFileExtention(materialPath.c_str()) + ".json");
        if (std::filesystem::exists(dst) &&
            std::filesystem::last_write_time(dst) >= std::filesystem::last_write_time(src))
        {
            nlohmann::json json = fileSystem.LoadJsonFile(materialPath.c_str());
          //  RenderPassLoader renderPipelineLoader = json.get<RenderPassLoader>();


            Material material;
            material.Albedo = vec3(json["Albedo"][0], json["Albedo"][1], json["Albedo"][2]);
            material.SheenColor = vec3(json["SheenColor"][0], json["SheenColor"][1], json["SheenColor"][2]);
            material.SubSurfaceScatteringColor = vec3(json["SubSurfaceScatteringColor"][0], json["SubSurfaceScatteringColor"][1], json["SubSurfaceScatteringColor"][2]);
            material.Emission = vec3(json["Emission"][0], json["Emission"][1], json["Emission"][2]);
            material.ClearcoatTint = json["ClearcoatTint"];
            material.Metallic = json["Metallic"];
            material.Roughness = json["Roughness"];
            material.AmbientOcclusion = json["AmbientOcclusion"];
            material.ClearcoatStrength = json["ClearcoatStrength"];
            material.ClearcoatRoughness = json["ClearcoatRoughness"];
            material.Thickness = json["Thickness"];
            material.SheenIntensity = json["SheenIntensity"];
            material.NormalStrength = json["NormalStrength"];
            material.HeightScale = json["HeightScale"];
            material.Height = json["Height"];
            material.Alpha = json["Alpha"];

            material.AlbedoMapId = VkGuid(json["AlbedoMapId"].get<std::string>());
            material.MetallicMapId = VkGuid(json["MetallicMapId"].get<std::string>());
            material.RoughnessMapId = VkGuid(json["RoughnessMapId"].get<std::string>());
            material.ThicknessMapId = VkGuid(json["ThicknessMapId"].get<std::string>());
            material.SubSurfaceScatteringMapId = VkGuid(json["SubSurfaceScatteringColorMapId"].get<std::string>());
            material.SheenMapId = VkGuid(json["SheenMapId"].get<std::string>());
            material.ClearCoatMapId = VkGuid(json["ClearCoatMapId"].get<std::string>());
            material.AmbientOcclusionMapId = VkGuid(json["AmbientOcclusionMapId"].get<std::string>());
            material.NormalMapId = VkGuid(json["NormalMapId"].get<std::string>());
            material.AlphaMapId = VkGuid(json["AlphaMapId"].get<std::string>());
            material.EmissionMapId = VkGuid(json["EmissionMapId"].get<std::string>());
            material.HeightMapId = VkGuid(json["HeightMapId"].get<std::string>());


           // Vector<Texture> texture = textureSystem.FindTexture(material.AlbedoMapId, 0);
           // VkGuid AssetCreatorRenderPassId = renderSystem.LoadRenderPass(dummyGuid, "RenderPass/AssetCreatorRenderPass.json",);

        }
    }
}
