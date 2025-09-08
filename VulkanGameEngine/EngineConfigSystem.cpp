#include "EngineConfigSystem.h"

ConfigSystem configSystem = ConfigSystem::LoadConfig("../EngineConfig.json");

ConfigSystem::ConfigSystem() : WindowResolution(glm::ivec2(1280, 720)),
                               ShaderSourceDirectory("..\\Shaders"),
							   CompiledDebugPathCPP("..\\x64\\Debug"),
							   CompiledReleasePathCPP("..\\x64\\Release"),
							   CompilerLocation("C:/VulkanSDK/1.4.313.0/Bin/glslangValidator.exe")
{
}

ConfigSystem::ConfigSystem(const nlohmann::json& j) : WindowResolution(ParseWindowResolution(j)),
                                                      ShaderSourceDirectory(j.at("ShaderSourceDirectory").get<String>()),
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