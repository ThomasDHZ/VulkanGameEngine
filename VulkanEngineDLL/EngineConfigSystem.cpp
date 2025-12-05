#include "EngineConfigSystem.h"

#if defined(_WIN32)
    ConfigSystem configSystem = ConfigSystem::LoadConfig("C:/Users/dotha/Documents/GitHub/VulkanGameEngine/EngineConfig.json");
#elif defined(__ANDROID__)
    ConfigSystem configSystem;
#elif defined(__linux__) && !defined(__ANDROID__)
    ConfigSystem configSystem = ConfigSystem::LoadConfig("/home/dothackzero/.vs/VulkanGameEngine/c7fad58a-e4c4-4409-80d1-05d1bf970194/out/build/Linux-Clang-Debug/EngineConfig.json");
#endif

#ifndef PLATFORM_ANDROID
    ConfigSystem::ConfigSystem() : WindowResolution(glm::ivec2(1280, 720)),
    EngineBasePath(),
    ShaderSourceDirectory(EngineBasePath + "../Assets/Shaders"),
    CompilerLocation("C:/VulkanSDK/1.4.318.0/Bin/glslc.exe"),
    CompilerBuildParams("--target-env=vulkan1.4 --target-spv=spv1.6"),
    CompiledShaderOutputDirectory("../Assets/Shaders/")
    {

    }

    ConfigSystem::ConfigSystem(const nlohmann::json& j) : WindowResolution(ParseWindowResolution(j)),
    ShaderSourceDirectory(j.at("ShaderSourceDirectory").get<String>()),
    CompilerLocation(j.at("CompilerLocation").get<String>()),
    CompilerBuildParams(j.at("CompilerBuildParams").get<String>()),
    CompiledShaderOutputDirectory(j.at("CompiledShaderOutputDirectory").get<String>())
    {

    }
#else
    ConfigSystem::ConfigSystem()
    {

    }
#endif


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