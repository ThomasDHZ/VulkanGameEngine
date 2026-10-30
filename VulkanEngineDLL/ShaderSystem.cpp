#include "ShaderSystem.h"
#include <Platform.h>
#include "MemorySystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include <cctype>
#include <ranges>

ShaderSystem& shaderSystem = ShaderSystem::Get();

VkGuid ShaderSystem::LoadShader(ShaderLoader& shaderLoader)
{
    if (VulkanShaderExists(shaderLoader.ShaderId)) return shaderLoader.ShaderId;

    Vector<byte> vertexShaderCode = fileSystem.LoadAssetFile(shaderLoader.ShaderFile.c_str());
    VulkanShader shader = VulkanShader(shaderLoader.ShaderId, vertexShaderCode);
    if (!shader.PushConstant().PushConstantName.empty() &&
        !shaderSystem.ShaderPushConstantExists(shader.PushConstant().PushConstantName))
    {
        shaderSystem.ShaderPushConstantMap[shader.PushConstant().PushConstantName] = shader.PushConstant();
    }
    shaderSystem.RenderShaderMap[shader.ShaderId()] = shader;
}

VulkanShader ShaderSystem::LoadShader(VkGuid& shaderGuid, const String& fileName)
{
    Vector<byte> shaderData = fileSystem.LoadAssetFile(fileName.c_str());
    return VulkanShader(shaderGuid, shaderData);
}

bool ShaderSystem::CompileShaders(const String& fileDirectory, const String& outputDirectory)
 {
     Vector<String> extensions = {
         "vert", "frag", "tesc", "tese", "geom", "comp",
         "rgen", "rchit", "rmiss", "rahit", "rint", "rcall"
     };

     Vector<String> shaderFiles = fileSystem.GetFilesFromDirectory(fileDirectory, extensions);
     if (shaderFiles.empty()) {
         throw std::runtime_error("No shader files found in: " + fileDirectory);
     }

     String glslc = "glslc";
     std::filesystem::path outDir = outputDirectory;
     std::filesystem::create_directories(outDir);
#if defined(_WIN32)
     if (const char* sdk = std::getenv("VULKAN_SDK"))
     {
         glslc = String(sdk) + "/Bin/glslc.exe";
     }
#endif

     const String baseCmd = glslc + " --target-env=vulkan1.3" + " --target-spv=spv1.6" + " -g -O0";
     for (const auto& srcPath : shaderFiles)
     {
         String extention = fileSystem.GetFileExtention(srcPath.c_str());
         extention[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(extention[0])));

         std::filesystem::path src = srcPath;
         std::filesystem::path dst = outDir / (src.filename().stem().string() + extention + ".spv");
         if (std::filesystem::exists(dst) &&
             std::filesystem::last_write_time(dst) >= std::filesystem::last_write_time(src))
         {
             continue;
         }

         String cmd = baseCmd + " \"" + std::filesystem::absolute(src).string() + "\"" + " -o \"" + dst.string() + "\"";
         int result = std::system(cmd.c_str());
         if (result == -1)
         {
             fprintf(stderr, "Failed to execute: %s\n", cmd.c_str());
             return false;
         }
     }
     return true;
 }

 void ShaderSystem::UpdatePushConstantBuffer(ShaderPushConstant& pushConstantStruct)
 {
     size_t offset = 0;
     for (const auto& pushConstantVar : pushConstantStruct.PushConstantVariableList)
     {
         size_t byteAlignment = GetSpivByteAlignment(pushConstantVar.Value);
         size_t size = std::visit([](auto&& arg)->size_t
                     {
                         return sizeof(std::decay_t<decltype(arg)>);
                     }, pushConstantVar.Value);

         offset = (offset + byteAlignment - 1) & ~(byteAlignment - 1);
         void* dest = static_cast<byte*>(pushConstantStruct.PushConstantBuffer.data()) + offset;
         std::visit([&](auto&& arg) 
             {
                memcpy(dest, &arg, sizeof(std::decay_t<decltype(arg)>));
             }, pushConstantVar.Value);
         offset += byteAlignment;
     }
 }
  
 void ShaderSystem::UpdatePushConstantBuffer(const String& pushConstantName)
 {
     if (!ShaderPushConstantExists(pushConstantName))
     {
         std::cerr << "Error: Push constant '" << pushConstantName << "' does not exist!" << std::endl;
         return;
     }
     UpdatePushConstantBuffer(ShaderPushConstantMap[pushConstantName]);
 }

 ShaderPushConstant& ShaderSystem::FindShaderPushConstant(const String& pushConstantName)
 {
     return shaderSystem.ShaderPushConstantMap.at(pushConstantName);
 }

 ShaderVariable& ShaderSystem::FindShaderPipelineStructVariable(ShaderStruct& shaderStruct, const String& variableName)
 {
     auto it = std::ranges::find_if(shaderStruct.ShaderBufferVariableList, [&](const ShaderVariable& var)
         {
             return var.Name == variableName;
         });
     if (it == shaderStruct.ShaderBufferVariableList.end())
     {
         throw std::runtime_error("Shader variable not found: " + variableName);
     }

     return *it;
 }

 ShaderVariable& ShaderSystem::FindShaderPushConstantStructVariable(ShaderPushConstant& shaderPushConstant, const String& variableName)
 {
     auto it = std::ranges::find_if(shaderPushConstant.PushConstantVariableList, [&](const ShaderVariable& var)
         {
             return var.Name == variableName; 
         });
     if (it == shaderPushConstant.PushConstantVariableList.end())
     {
         throw std::runtime_error("Shader variable not found: " + variableName);
     }
     return *it;
 }

 bool ShaderSystem::SearchShaderConstantBufferExists(const Vector<ShaderPushConstant>& shaderPushConstantList, const String& constBufferName)
 {
     auto it = std::ranges::find_if(shaderPushConstantList, [&](const ShaderPushConstant& var)
         {
             return var.PushConstantName == constBufferName;
         });
     return it != shaderPushConstantList.end();
 }

 bool ShaderSystem::SearchShaderDescriptorBindingExists(const Vector<ShaderDescriptorBinding>& shaderDescriptorBindingList, const String& descriptorBindingName)
 {
     auto it = std::ranges::find_if(shaderDescriptorBindingList, [&](const ShaderDescriptorBinding& var)
         {
             return var.Name == descriptorBindingName;
         });
     return it != shaderDescriptorBindingList.end();
 }

 bool ShaderSystem::SearchShaderPipelineStructExists(const Vector<ShaderStruct>& shaderStructList, const String& structName)
 {
     auto it = std::ranges::find_if(shaderStructList, [&](const ShaderStruct& var)
         {
             return var.Name == structName;
         });
     return it != shaderStructList.end();
 }

 bool ShaderSystem::SearchPushConstantForVariableExists(const String& structKey, const String& variableName)
 {
     auto itMap = ShaderPushConstantMap.find(structKey);
     if (itMap == ShaderPushConstantMap.end())
     {
         return false;
     }

     const auto& pushConstant = itMap->second;
     auto itVar = std::find_if(pushConstant.PushConstantVariableList.begin(), pushConstant.PushConstantVariableList.end(), [&](const ShaderVariable& var)
         {
             return var.Name == variableName;
         });

     return itVar != pushConstant.PushConstantVariableList.end();
 }

 bool ShaderSystem::VulkanShaderExists(const VkGuid& vulkanShaderId)
 {
     return RenderShaderMap.contains(vulkanShaderId);
 }

 size_t ShaderSystem::GetSpivByteAlignment(ShaderMemberTypeEnum shaderMemberEnum)
 {
     switch (shaderMemberEnum)
     {
         case kShaderMember_Int:     return 4;
         case kShaderMember_Int64:   return 8;
         case kShaderMember_Uint:    return 4;
         case kShaderMember_Uint64:  return 8;
         case kShaderMember_Float:   return 4;  
         case kShaderMember_Double:  return 8;
         case kShaderMember_bool:    return 4;
         case kShaderMember_Ivec2:   return 8;
         case kShaderMember_Ivec3:   return 16;
         case kShaderMember_Ivec4:   return 16;
         case kShaderMember_Vec2:    return 8;
         case kShaderMember_Vec3:    return 16;
         case kShaderMember_Vec4:    return 16;
         case kShaderMember_Mat2:    return 8;
         case kShaderMember_Mat3:    return 16;
         case kShaderMember_Mat4:    return 16;
         default: throw std::runtime_error("Unknown ShaderMemberTypeEnum in GetSpivByteAlignment");
     }
 }

 size_t ShaderSystem::GetSpivByteAlignment(const PushConstantVariable& pushConstantVariable)
 {
     return std::visit([this](auto&& arg) -> size_t {
         using T = std::decay_t<decltype(arg)>;

         if      constexpr (std::is_same_v<T, int>)     return GetSpivByteAlignment(kShaderMember_Int);
         else if constexpr (std::is_same_v<T, int64>)   return GetSpivByteAlignment(kShaderMember_Int64);
         else if constexpr (std::is_same_v<T, uint32>)  return GetSpivByteAlignment(kShaderMember_Uint);
         else if constexpr (std::is_same_v<T, uint64>)  return GetSpivByteAlignment(kShaderMember_Uint64);
         else if constexpr (std::is_same_v<T, float>)   return GetSpivByteAlignment(kShaderMember_Float);
         else if constexpr (std::is_same_v<T, double>)  return GetSpivByteAlignment(kShaderMember_Double);
         else if constexpr (std::is_same_v<T, bool>)    return GetSpivByteAlignment(kShaderMember_bool);
         else if constexpr (std::is_same_v<T, ivec2>)   return GetSpivByteAlignment(kShaderMember_Ivec2);
         else if constexpr (std::is_same_v<T, ivec3>)   return GetSpivByteAlignment(kShaderMember_Ivec3);
         else if constexpr (std::is_same_v<T, ivec4>)   return GetSpivByteAlignment(kShaderMember_Ivec4);
         else if constexpr (std::is_same_v<T, vec2>)    return GetSpivByteAlignment(kShaderMember_Vec2);
         else if constexpr (std::is_same_v<T, vec3>)    return GetSpivByteAlignment(kShaderMember_Vec3);
         else if constexpr (std::is_same_v<T, vec4>)    return GetSpivByteAlignment(kShaderMember_Vec4);
         else if constexpr (std::is_same_v<T, mat2>)    return GetSpivByteAlignment(kShaderMember_Mat2);
         else if constexpr (std::is_same_v<T, mat3>)    return GetSpivByteAlignment(kShaderMember_Mat3);
         else if constexpr (std::is_same_v<T, mat4>)    return GetSpivByteAlignment(kShaderMember_Mat4);
         else {
             static_assert(false, "Unknown PushConstantVariable type");
             return 0;
         }
         }, pushConstantVariable);
 }

 bool ShaderSystem::ShaderPushConstantExists(const String& pushConstantName)
 {
     return shaderSystem.ShaderPushConstantMap.contains(pushConstantName);
 }

 const VulkanShader& ShaderSystem::FindVulkanShader(const VkGuid& shaderGuid)
 {
     return RenderShaderMap[shaderGuid];
 }

 void ShaderSystem::Destroy()
 {
     for (auto& shader : RenderShaderMap)
     {
         shader.second.Destroy();
     }
 }

