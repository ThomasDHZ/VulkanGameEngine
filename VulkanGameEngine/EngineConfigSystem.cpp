#include "EngineConfigSystem.h"


ConfigSystem::ConfigSystem() : WindowResolution(glm::ivec2(1280, 720)),
							   CompiledDebugPathCPP("bin/debug"),
							   CompiledReleasePathCPP("bin/release"),
							   CompilerLocation("C:/tools/compiler") 
{
}

ConfigSystem::ConfigSystem(const nlohmann::json& j) : WindowResolution(ParseWindowResolution(j)),
													  CompiledDebugPathCPP(j.at("CompiledDebugPathCPP").get<String>()),
													  CompiledReleasePathCPP(j.at("CompiledReleasePathCPP").get<String>()),
													  CompilerLocation(j.at("CompilerLocation").get<String>())
{
}

ConfigSystem::~ConfigSystem()
{
}

ivec2 ConfigSystem::ParseWindowResolution(const nlohmann::json& j)
{
    const auto& arr = j.at("WindowResolution");
    if (!arr.is_array() || 
        arr.size() != 2 || 
        !arr[0].is_number_integer() || 
        !arr[1].is_number_integer()) 
    {
        throw std::runtime_error("WindowResolution must be an array of 2 integers");
    }

    int x = arr[0].get<int>();
    int y = arr[1].get<int>();
    if (x <= 0 || y <= 0) 
    {
        throw std::runtime_error("WindowResolution must have positive dimensions");
    }
    return ivec2(x, y);
}

const ConfigSystem& configSystem()
{
	return ConfigSystem::LoadConfig("C:/Users/dotha/Documents/GitHub/VulkanGameEngine/EngineConfig.json");
}
