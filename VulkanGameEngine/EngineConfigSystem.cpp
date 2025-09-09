#include "EngineConfigSystem.h"

ConfigSystem configSystem = ConfigSystem::LoadConfig("../EngineConfig.json");

ConfigSystem::ConfigSystem() : WindowResolution(glm::ivec2(1280, 720)),
                               ShaderSourceDirectory("..\\Shaders"),
                               CompilerLocation("C:/VulkanSDK/1.4.313.0/Bin/glslc.exe"),
                               CompilerBuildParams("--target-env=vulkan1.4 --target-spv=spv1.6"),
                               CompiledShaderOutputDirectory("..\\Assets\\Shaders\\")
{
}

ConfigSystem::ConfigSystem(const nlohmann::json& j) : WindowResolution(ParseWindowResolution(j)),
                                                      ShaderSourceDirectory(j.at("ShaderSourceDirectory").get<String>()),
                                                      CompilerLocation(j.at("CompilerLocation").get<String>()),
                                                      CompilerBuildParams(j.at("CompilerBuildParams").get<String>()),
                                                      CompiledShaderOutputDirectory(j.at("CompiledShaderOutputDirectory").get<String>())
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