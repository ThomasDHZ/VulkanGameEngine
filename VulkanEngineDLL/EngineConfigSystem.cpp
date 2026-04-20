#include "EngineConfigSystem.h"

#if defined(_WIN32)
    ConfigSystem configSystem = ConfigSystem::LoadConfig("..//EngineConfig.json");
#elif defined(__ANDROID__)
    ConfigSystem configSystem;
#elif defined(__linux__) && !defined(__ANDROID__)
    ConfigSystem configSystem = ConfigSystem::LoadConfig("EngineConfig.json");
#endif

#ifndef PLATFORM_ANDROID
ConfigSystem::ConfigSystem() :
    WindowResolution(ivec2(1280, 720)),
    RenderResolution(ivec2(1920, 1080)),
    EngineBasePath(),
    AssetDirectory("../Assets/"),
    ShaderSourceDirectory("../Assets/Shaders"),
    MaterialSourceDirectory("../Assets/Materials"),
    MaterialDstDirectory("../Assets/Material2"),
    TextureAssetRenderer("../Assets/RenderPass/AssetCreatorRenderPass.json"),
    CompilerLocation("C:/VulkanSDK/1.4.318.0/Bin/glslc.exe"),
    CompilerBuildParams("--target-env=vulkan1.4 --target-spv=spv1.6"),
    CompiledShaderOutputDirectory("../Assets/Shaders/"),
    NvidiaTextureTool("C:\\Program Files\\NVIDIA Corporation\\NVIDIA Texture Tools\\nvtt_export.exe")
{
}

ConfigSystem::ConfigSystem(const nlohmann::json& j) :
    WindowResolution(ParseWindowResolution(j)),
    RenderResolution(ParseRenderResolution(j)),
    AssetDirectory(j.at("AssetDirectory").get<String>()),
    ShaderSourceDirectory(j.at("ShaderSourceDirectory").get<String>()),
    MaterialSourceDirectory(j.at("MaterialSourceDirectory").get<String>()),
    MaterialDstDirectory(j.at("MaterialDstDirectory").get<String>()),
    TextureAssetRenderer(j.at("TextureAssetRenderer").get<String>()),
    CompilerLocation(j.at("CompilerLocation").get<String>()),
    CompilerBuildParams(j.at("CompilerBuildParams").get<String>()),
    CompiledShaderOutputDirectory(j.at("CompiledShaderOutputDirectory").get<String>()),
    NvidiaTextureTool(j.at("NvidiaTextureTool").get<String>())
{
}
#else
ConfigSystem::ConfigSystem()
{
}

ConfigSystem ConfigSystem::LoadConfig(const String& configPath)
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

ConfigSystem::~ConfigSystem() = default;

ivec2 ConfigSystem::ParseWindowResolution(const nlohmann::json& j)
{
    const auto& arr = j.at("WindowResolution");
    if (!arr.is_array() || arr.size() != 2 || !arr[0].is_number_integer() || !arr[1].is_number_integer()) {
        throw std::runtime_error("WindowResolution must be an array of 2 integers");
    }

    int x = arr[0].get<int>();
    int y = arr[1].get<int>();
    if (x <= 0 || y <= 0) {
        throw std::runtime_error("WindowResolution must have positive dimensions");
    }
    return ivec2(x, y);
}

ivec2 ConfigSystem::ParseRenderResolution(const nlohmann::json& j)
{
    const auto& arr = j.at("RenderResolution");
    if (!arr.is_array() || arr.size() != 2 || !arr[0].is_number_integer() || !arr[1].is_number_integer()) {
        throw std::runtime_error("RenderResolution must be an array of 2 integers");
    }

    int x = arr[0].get<int>();
    int y = arr[1].get<int>();
    if (x <= 0 || y <= 0) {
        throw std::runtime_error("RenderResolution must have positive dimensions");
    }
    return ivec2(x, y);
}