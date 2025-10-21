//#include "ShaderSystem.h"
//#include "RenderSystem.h"
//#include "MeshSystem.h"
//#include "TextureSystem.h"
//#include "BufferSystem.h"
//#include <algorithm>
//#include <iostream>
//#include <string_view>
//#include <CHelper.h>
//#include "EngineConfigSystem.h"
//#include <FileSystem.h>
//
//ShaderSystem shaderSystem = ShaderSystem();
//
//ShaderSystem::ShaderSystem()
//{
//}
//
//ShaderSystem::~ShaderSystem()
//{
//}
//
//void ShaderSystem::StartUp()
//{
//    Shader_StartUp();
//}
//
//void ShaderSystem::Destroy()
//{
//    Shader_Destroy();
//}
//
//ShaderPipelineData ShaderSystem::LoadShaderPipelineData(Vector<String> shaderPathList)
//{
//    return Shader_LoadShaderPipelineData(shaderPathList);
//}
//
//const ShaderVariable* ShaderSystem::SearchGlobalShaderConstantVar(ShaderPushConstant* pushConstant, const char* varName)
//{
//    return Shader_SearchShaderConstStructVar(pushConstant, varName);
//}
//
//ShaderVariable* ShaderSystem::SearchShaderStruct(ShaderStruct& shaderStruct, const String& varName)
//{
//    return Shader_SearchShaderStructVar(&shaderStruct, varName.c_str());
//}
//
//void ShaderSystem::UpdateGlobalShaderBuffer(const String& pushConstantName)
//{
//    Shader_UpdateGlobalShaderBuffer(renderer, pushConstantName);
//}
//
//void ShaderSystem::UpdateShaderBuffer(uint vulkanBufferId)
//{
//    Shader_UpdateShaderBuffer(renderer, vulkanBufferId);
//}
//
//ShaderPushConstant* ShaderSystem::GetGlobalShaderPushConstant(const String& pushConstantName)
//{
//    return Shader_GetGlobalShaderPushConstant(pushConstantName);
//}
//
//void ShaderSystem::LoadShaderPipelineStructPrototypes(const Vector<String>& renderPassJsonList)
//{
//    Shader_LoadShaderPipelineStructPrototypes(renderPassJsonList);
//}
//
//ShaderPipelineData& ShaderSystem::FindShaderModule(const String& shaderFile)
//{
//    return Shader_FindShaderModule(shaderFile);
//}
//
//ShaderPushConstant& ShaderSystem::FindShaderPushConstant(const String& shaderFile)
//{
//    return Shader_FindShaderPushConstant(shaderFile);
//}
//
//ShaderStruct ShaderSystem::FindShaderProtoTypeStruct(const String& shaderKey)
//{
//    return Shader_FindShaderProtoTypeStruct(shaderKey);
//}
//
//ShaderStruct& ShaderSystem::FindShaderStruct(int vulkanBufferId)
//{
//    return Shader_FindShaderStruct(vulkanBufferId);
//}
//
//ShaderStruct ShaderSystem::CopyShaderStructProtoType(const String& structName)
//{
//    ShaderStruct shaderStructCopy = FindShaderProtoTypeStruct(structName);
//    return Shader_CopyShaderStructPrototype(shaderStructCopy);
//}
//
//const bool ShaderSystem::ShaderPushConstantExists(const String& pushConstantName) const
//{
//    return Shader_ShaderPushConstantExists(pushConstantName);
//}
//
//const bool ShaderSystem::ShaderModuleExists(const String& shaderFile) const
//{
//    return Shader_ShaderModuleExists(shaderFile);
//}
//
//const bool ShaderSystem::ShaderStructPrototypeExists(const String& structKey) const
//{
//    return Shader_ShaderStructPrototypeExists(structKey);
//}
//
//const bool ShaderSystem::ShaderStructExists(uint vulkanBufferKey) const
//{
//    return Shader_ShaderStructExists(vulkanBufferKey);
//}
//
//void ShaderSystem::CompileShaders(const char* shaderFilePath)
//{
//    Shader_CompileShaders(renderer.Device, shaderFilePath, configSystem.CompiledShaderOutputDirectory.c_str());
//}