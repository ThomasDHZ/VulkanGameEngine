#pragma once
#include "Platform.h"

class ConfigSystem
{
private:
    ivec2 ParseWindowResolution(const nlohmann::json& j);

public:
    const ivec2 WindowResolution;
    const String EngineBasePath;
    const String ShaderSourceDirectory;
    const String CompilerLocation;
    const String CompilerBuildParams;
    const String CompiledShaderOutputDirectory;

    ConfigSystem();
    ConfigSystem(const nlohmann::json& j);
    ~ConfigSystem();

    static ConfigSystem LoadConfig(const String& configPath) {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open " + configPath);
        }
        nlohmann::json j;
        file >> j;
        return ConfigSystem(j);
    }
};
extern DLL_EXPORT ConfigSystem configSystem;

