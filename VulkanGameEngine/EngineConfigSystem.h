#pragma once
#include <nlohmann/json.hpp>
#include "Typedef.h"
#include <vulkan/vulkan.h>
#include "JsonStructs.h"
#include "JsonStruct.h"
#include "json.h"
#include <fstream>

class ConfigSystem
{
private:
    ivec2 ParseWindowResolution(const nlohmann::json& j);

public:
	const ivec2 WindowResolution;
	const String CompiledDebugPathCPP;
	const String CompiledReleasePathCPP;
	const String CompilerLocation;

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
const ConfigSystem& configSystem();

