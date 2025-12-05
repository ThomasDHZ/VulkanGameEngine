#pragma once
#include "Platform.h"
#include "FileSystem.h"

class ConfigSystem
{
private:
    ivec2 ParseWindowResolution(const nlohmann::json& j);

public:
#ifndef PLATFORM_ANDROID
    const ivec2 WindowResolution;
    const String EngineBasePath;
    const String ShaderSourceDirectory;
    const String CompilerLocation;
    const String CompilerBuildParams;
    const String CompiledShaderOutputDirectory;

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
    static ConfigSystem LoadConfig(const String& configPath) 
    {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open " + configPath);
        }
        nlohmann::json j;
        file >> j;
        return ConfigSystem(j);
    }
#else
ConfigSystem LoadConfig(const String& configPath)
    {
        nlohmann::json jsonText = fileSystem.LoadConfig(configPath);
        WindowResolution.x = ParseWindowResolution(jsonText).x;
        WindowResolution.y = ParseWindowResolution(jsonText).y;
        ShaderSourceDirectory = jsonText.at("ShaderSourceDirectory").get<String>();
        CompilerLocation = jsonText.at("CompilerLocation").get<String>();
        CompilerBuildParams = jsonText.at("CompilerBuildParams").get<String>();
        CompiledShaderOutputDirectory = jsonText.at("CompiledShaderOutputDirectory").get<String>();
        return *this;
    }
#endif
};
extern DLL_EXPORT ConfigSystem configSystem;

