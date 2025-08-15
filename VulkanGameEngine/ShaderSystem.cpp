#include "ShaderSystem.h"
#include "RenderSystem.h"

ShaderSystem shaderSystem = ShaderSystem();


ShaderSystem::ShaderSystem()
{
}

ShaderSystem::~ShaderSystem()
{
}

void ShaderSystem::StartUp()
{
    Shader_StartUp();
}

void ShaderSystem::VertexDataFromSpirv(const String& path)
{
    FileState file = File_Read(path.c_str());
    Vector<byte> shaderCode = Vector<byte>(file.Data, file.Data + file.Size);
    Shader_VertexDataFromSpirv(shaderCode);
}

VkPipelineShaderStageCreateInfo ShaderSystem::CreateShader(VkDevice device, const String& path, VkShaderStageFlagBits shaderStages)
{
    return Shader_CreateShader(device, path, shaderStages);
}