#pragma once
#include "Platform.h"
#include "FileSystem.h"

class ConfigSystem
{
private:
    ivec2 ParseWindowResolution(const nlohmann::json& j);
    ivec2 ParseRenderResolution(const nlohmann::json& j);

public:
#ifndef PLATFORM_ANDROID
    const ivec2 WindowResolution;
    const ivec2 RenderResolution;
    const String EngineBasePath;
    const String AssetDirectory;
    const String ShaderSourceDirectory;
    const String MaterialSourceDirectory;
    const String MaterialDstDirectory;
    const String TextureAssetRenderer;
    const String CompilerLocation;
    const String CompilerBuildParams;
    const String CompiledShaderOutputDirectory;
    const String NvidiaTextureTool;
#else
    ivec2 WindowResolution;
    String EngineBasePath;
    String ShaderSourceDirectory;
    String CompilerLocation;
    String CompilerBuildParams;
    String CompiledShaderOutputDirectory;
#endif

    ConfigSystem();
#ifndef PLATFORM_ANDROID
    ConfigSystem(const nlohmann::json& j);
#endif
    ~ConfigSystem();

#ifndef PLATFORM_ANDROID
    static ConfigSystem LoadConfig(const String& configPath = "../EngineConfig.json")
    {
        std::ifstream file(configPath);
        if (!file.is_open()) 
        {
            throw std::runtime_error("Failed to open EngineConfig.json. Make sure it exists in the bin folder next to the executable.");
        }

        nlohmann::json j;
        file >> j;
        return ConfigSystem(j);
    }
#else
    ConfigSystem LoadConfig(const String& configPath);
#endif
};

extern DLL_EXPORT ConfigSystem configSystem;