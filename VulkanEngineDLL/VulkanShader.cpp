#include "VulkanShader.h"
#include <regex>
#include "MemorySystem.h"
#include "json.h"
#include "CHelper.h"
#include "File.h"
#include "CFile.h"

void Shader_StartUp()
{
    //DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(dxc_utils.ReleaseAndGetAddressOf()));
    //DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler));
    //dxc_utils->CreateDefaultIncludeHandler(&defaultIncludeHandler);
}

ShaderPipelineData Shader_LoadPipelineShaderData(const char** pipelineShaderPaths, size_t pipelineShaderCount)
{
    SpvReflectShaderModule spvModule;
    Vector<VkVertexInputBindingDescription> vertexInputBindingList;
    Vector<VkVertexInputAttributeDescription> vertexInputAttributeList;
    Vector<ShaderPushConstant> constBuffers;
    Vector<ShaderStruct> shaderStructs;
    Vector<ShaderDescriptorBinding> descriptorBindings;
    
    Vector<String> pipelineShaderPathList(pipelineShaderPaths, pipelineShaderPaths + pipelineShaderCount);
    for (auto& pipelineShaderPath : pipelineShaderPathList)
    {
        FileState file = File_Read(pipelineShaderPath.c_str());
        SPV_VULKAN_RESULT(spvReflectCreateShaderModule(file.Size * sizeof(byte), file.Data, &spvModule));
        Shader_GetShaderConstBuffer(spvModule, constBuffers);
        Shader_GetShaderDescriptorBindings(spvModule, descriptorBindings);
        if (spvModule.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
        {
            Shader_GetShaderInputVertexVariables(spvModule, vertexInputBindingList, vertexInputAttributeList);
        }
        spvReflectDestroyShaderModule(&spvModule);
        free(file.Data);
        file.Data = nullptr;
    }

    Vector<const char*> cPipelineShaderPathList;
    cPipelineShaderPathList.reserve(pipelineShaderPathList.size());
    for (const auto& pipelineShaderPath : pipelineShaderPathList)
    {
        const char* copiedStr = memorySystem.AddPtrBuffer(pipelineShaderPath.c_str(), __FILE__, __LINE__, __func__, "Shader path string");
        if (copiedStr)
        {
            cPipelineShaderPathList.push_back(copiedStr);
        }
        else
        {
            std::cerr << "Failed to allocate memory for shader path: " << pipelineShaderPath << std::endl;
        }
    }

    return ShaderPipelineData
    {
         .ShaderCount = pipelineShaderCount,
         .DescriptorBindingCount = descriptorBindings.size(),
         .VertexInputBindingCount = vertexInputBindingList.size(),
         .VertexInputAttributeListCount = vertexInputAttributeList.size(),
         .PushConstantCount = constBuffers.size(),
         .ShaderList = memorySystem.AddPtrBuffer<const char*>(cPipelineShaderPathList.data(), cPipelineShaderPathList.size(), __FILE__, __LINE__, __func__),
         .DescriptorBindingsList = memorySystem.AddPtrBuffer<ShaderDescriptorBinding>(descriptorBindings.data(), descriptorBindings.size(), __FILE__, __LINE__, __func__),
         .VertexInputBindingList = memorySystem.AddPtrBuffer<VkVertexInputBindingDescription>(vertexInputBindingList.data(), vertexInputBindingList.size(), __FILE__, __LINE__, __func__),
         .VertexInputAttributeList = memorySystem.AddPtrBuffer<VkVertexInputAttributeDescription>(vertexInputAttributeList.data(), vertexInputAttributeList.size(), __FILE__, __LINE__, __func__),
         .PushConstantList = memorySystem.AddPtrBuffer<ShaderPushConstant>(constBuffers.data(), constBuffers.size(), __FILE__, __LINE__, __func__)
    };
}

void Shader_ShaderDestroy(ShaderPipelineData& shader)
{
    Shader_DestroyShaderBindingData(shader);
    memorySystem.RemovePtrBuffer<ShaderPushConstant>(shader.PushConstantList);
    memorySystem.RemovePtrBuffer<ShaderDescriptorBinding>(shader.DescriptorBindingsList);
    memorySystem.RemovePtrBuffer<VkVertexInputBindingDescription>(shader.VertexInputBindingList);
    memorySystem.RemovePtrBuffer<VkVertexInputAttributeDescription>(shader.VertexInputAttributeList);
}

void Shader_DestroyShaderStructData(ShaderStruct* shaderStructList, size_t shaderStrucCount)
{
    Span<ShaderStruct> shaderStructSpan(shaderStructList, shaderStrucCount);
    for (auto& shaderStruct : shaderStructSpan)
    {
        for (auto& shaderStructVar : shaderStruct.ShaderBufferVariableList)
        {
            shaderStructVar.Name = shaderStructVar.Name.empty();
            shaderStructVar.ByteAlignment = 0;
            shaderStructVar.MemberTypeEnum = ShaderMemberType::shaderUnknown;
            shaderStructVar.Size = 0;
            memorySystem.RemovePtrBuffer(shaderStructVar.Value);
        }

        shaderStruct.Name = shaderStruct.Name.empty();
        shaderStruct.ShaderBufferSize = 0;
        shaderStruct.ShaderStructBufferId = 0;
        if (shaderStruct.ShaderStructBuffer)
        {
            memorySystem.RemovePtrBuffer(shaderStruct.ShaderStructBuffer);
        }
    }
}

void Shader_DestroyShaderBindingData(ShaderPipelineData& shader)
{
    for (int x = 0; x < shader.DescriptorBindingCount; x++)
    {
        if (shader.DescriptorBindingsList[x].DescriptorBufferInfo != nullptr)
        {
            memorySystem.RemovePtrBuffer<VkDescriptorBufferInfo>(shader.DescriptorBindingsList[x].DescriptorBufferInfo);
        }
        if (shader.DescriptorBindingsList[x].DescriptorImageInfo != nullptr)
        {
            memorySystem.RemovePtrBuffer<VkDescriptorImageInfo>(shader.DescriptorBindingsList[x].DescriptorImageInfo);
        }
    }
}

void Shader_DestroyPushConstantBufferData(ShaderPushConstant* pushConstantListArray, size_t pushConstantCount)
{
    Span<ShaderPushConstant> pushConstantSpan(pushConstantListArray, pushConstantCount);
    for (auto& pushConstant : pushConstantSpan)
    {
        for (auto& shaderVar : pushConstant.PushConstantVariableList)
        {
            shaderVar.Name = shaderVar.Name.empty();
            shaderVar.ByteAlignment = 0;
            shaderVar.MemberTypeEnum = ShaderMemberType::shaderUnknown;
            shaderVar.Size = 0;
            memorySystem.RemovePtrBuffer(shaderVar.Value);
        }

        pushConstant.PushConstantName = pushConstant.PushConstantName.empty();
        pushConstant.GlobalPushContant = false;
        pushConstant.PushConstantSize = 0;
        pushConstant.ShaderStageFlags = 0;
        if (pushConstant.PushConstantBuffer)
        {
            memorySystem.RemovePtrBuffer(pushConstant.PushConstantBuffer);
        }
    }
}

void Shader_SetVariableDefaults(ShaderVariable& shaderVariable)
{
    switch (shaderVariable.MemberTypeEnum)
    {
        case shaderInt:
        {
            int varType = 0;
            memcpy(shaderVariable.Value, &varType, shaderVariable.Size);
            break;
        }
        case shaderUint:
        {
            int varType = 0;
            memcpy(shaderVariable.Value, &varType, shaderVariable.Size);
            break;
        }
        case shaderFloat:
        {
            int varType = 0.0f;
            memcpy(shaderVariable.Value, &varType, shaderVariable.Size);
            break;
        }
        case shaderIvec2:
        {
            ivec2 varType = ivec2(0.0f);
            memcpy(shaderVariable.Value, &varType, shaderVariable.Size);
            break;
        }
        case shaderIvec3:
        {
            ivec3 varType = ivec3(0.0f);
            memcpy(shaderVariable.Value, &varType, shaderVariable.Size);
            break;
        }
        case shaderIvec4:
        {
            ivec4 varType = ivec4(0.0f);
            memcpy(shaderVariable.Value, &varType, shaderVariable.Size);
            break;
        }
        case shaderVec2:
        {
            vec2 varType = vec2(0.0);
            memcpy(shaderVariable.Value, &varType, shaderVariable.Size);
            break;
        }
        case shaderVec3:
        {
            vec3 varType = vec3(0.0f);
            memcpy(shaderVariable.Value, &varType, shaderVariable.Size);
            break;
        }
        case shaderVec4:
        {
            vec4 varType = vec4(0.0f);
            memcpy(shaderVariable.Value, &varType, shaderVariable.Size);
            break;
        }
        case shaderMat2:
        {
            mat2 varType = mat2(1.0f);
            memcpy(shaderVariable.Value, &varType, shaderVariable.Size);
            break;
        }
        case shaderMat3:
        {
            mat3 varType = mat3(1.0f);
            memcpy(shaderVariable.Value, &varType, shaderVariable.Size);
            break;
        }
        case shaderMat4:
        {
            mat4 varType = mat4(1.0f);
            memcpy(shaderVariable.Value, &varType, shaderVariable.Size);
            break;
        }
    }
}

VkPipelineShaderStageCreateInfo Shader_CreateShader(VkDevice device, const char* filepath, VkShaderStageFlagBits shaderStages)
{
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (File_GetFileExtention(filepath) == "spv")
    {
        shaderModule = Shader_BuildGLSLShaderFile(device, filepath);

    }
    else if (File_GetFileExtention(filepath) == "hlsl")
    {
        //  shaderModule = Shader_BuildHLSLShader(device, filename.c_str(), shaderStages);
    }
    else
    {
        throw std::runtime_error("File extention not a accepted shader type.");
    }

    return VkPipelineShaderStageCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = shaderStages,
        .module = shaderModule,
        .pName = "main"
    };
}

void Shader_UpdateShaderBuffer(const GraphicsRenderer& renderer, VulkanBuffer& vulkanBuffer, ShaderStruct* shaderStruct, size_t shaderCount)
{
    size_t offset = 0;
    for (const auto& shaderStrucVar : shaderStruct->ShaderBufferVariableList)
    {
        offset = (offset + shaderStrucVar.ByteAlignment - 1) & ~(shaderStrucVar.ByteAlignment - 1);
        void* dest = static_cast<byte*>(shaderStruct->ShaderStructBuffer) + offset;
        memcpy(dest, shaderStrucVar.Value, shaderStrucVar.Size);
        offset += shaderStrucVar.Size;
    }
    VulkanBuffer_UpdateBufferMemory(renderer, vulkanBuffer, shaderStruct->ShaderStructBuffer, shaderStruct->ShaderBufferSize, shaderCount);
}

void Shader_UpdatePushConstantBuffer(const GraphicsRenderer& renderer, ShaderPushConstant& pushConstantStruct)
{
    size_t offset = 0;
    for (const auto& pushConstantVar : pushConstantStruct.PushConstantVariableList)
    {
        offset = (offset + pushConstantVar.ByteAlignment - 1) & ~(pushConstantVar.ByteAlignment - 1);
        void* dest = static_cast<byte*>(pushConstantStruct.PushConstantBuffer) + offset;
        memcpy(dest, pushConstantVar.Value, pushConstantVar.Size);
        offset += pushConstantVar.Size;
    }
}

ShaderStruct* Shader_LoadProtoTypeStructs(const char** pipelineShaderPaths, size_t pipelineShaderCount, size_t& outProtoTypeStructCount)
{
    SpvReflectShaderModule spvModule;
    Vector<ShaderStruct> shaderStructs;
    Vector<String> pipelineShaderPathList = CHelper_ConstCharPtrPtrToVector(pipelineShaderPaths, pipelineShaderCount);
    for (auto& pipelineShaderPath : pipelineShaderPathList)
    {
        FileState file = File_Read(pipelineShaderPath.c_str());
        SPV_VULKAN_RESULT(spvReflectCreateShaderModule(file.Size * sizeof(byte), file.Data, &spvModule));
        Shader_GetShaderDescriptorSetInfo(spvModule, shaderStructs);
        spvReflectDestroyShaderModule(&spvModule);
    }

    outProtoTypeStructCount = shaderStructs.size();
    return memorySystem.AddPtrBuffer<ShaderStruct>(shaderStructs.data(), shaderStructs.size(), __FILE__, __LINE__, __func__);
}

ShaderStruct Shader_CopyShaderStructPrototype(const ShaderStruct& shaderStructToCopy)
{
    ShaderStruct shaderStruct = ShaderStruct
    {
        .Name = shaderStructToCopy.Name,
        .ShaderBufferSize = shaderStructToCopy.ShaderBufferSize,
        .ShaderBufferVariableList = shaderStructToCopy.ShaderBufferVariableList,
        .ShaderStructBufferId = shaderStructToCopy.ShaderStructBufferId,
        .ShaderStructBuffer = memorySystem.AddPtrBuffer<byte>(shaderStructToCopy.ShaderBufferSize, __FILE__, __LINE__, __func__, shaderStructToCopy.Name.c_str()),
    };

    for (size_t x = 0; x < shaderStruct.ShaderBufferVariableList.size(); ++x)
    {
        shaderStruct.ShaderBufferVariableList[x].Value = memorySystem.AddPtrBuffer<byte>(shaderStruct.ShaderBufferVariableList[x].Size, __FILE__, __LINE__, __func__, shaderStruct.ShaderBufferVariableList[x].Name.c_str());
        Shader_SetVariableDefaults(shaderStruct.ShaderBufferVariableList[x]);
    }

    return shaderStruct;
}

LPWSTR Shader_StringToLPWSTR(const String& str)
{
    std::wstring wstr(str.begin(), str.end());
    LPWSTR result = new wchar_t[wstr.length() + 1];
    wcscpy_s(result, wstr.length() + 1, wstr.c_str());
    return result;
}

String Shader_ConvertLPCWSTRToString(LPCWSTR lpcwszStr)
{
    int strLength = WideCharToMultiByte(CP_UTF8, 0, lpcwszStr, -1, nullptr, 0, nullptr, nullptr);
    String str(strLength, 0);
    WideCharToMultiByte(CP_UTF8, 0, lpcwszStr, -1, &str[0], strLength, nullptr, nullptr);
    return str;
}

void Shader_uint32ToUnsignedCharString(uint32 value, String& string)
{
    string += static_cast<unsigned char>((value >> 24) & 0xFF);
    string += static_cast<unsigned char>((value >> 16) & 0xFF);
    string += static_cast<unsigned char>((value >> 8) & 0xFF);
    string += static_cast<unsigned char>(value & 0xFF);
}

void Shader_GetShaderInputVertexVariables(const SpvReflectShaderModule& module, Vector<VkVertexInputBindingDescription>& vertexInputBindingList, Vector<VkVertexInputAttributeDescription>& vertexInputAttributeList)
{
    uint32 offset = 0;
    Vector<SpvReflectInterfaceVariable*> inputs = Shader_GetShaderVertexInputVariables(module);
    Vector<SpvReflectSpecializationConstant*> specializationConstantList = Shader_GetShaderSpecializationConstant(module);
    Vector<SpvReflectSpecializationConstant*> vertexInputRateLocationConstantResult = Shader_SearchShaderSpecializationConstant(specializationConstantList, "VertexInputRateLocation");
    Vector<SpvReflectSpecializationConstant*> vertexAttributeLocationpecializationConstantResult = Shader_SearchShaderSpecializationConstant(specializationConstantList, "VertexAttributeLocation");
    for (int x = 0; x < inputs.size(); x++)
    {
        uint32 binding = 0;
        uint32 inputRate = 0;
        if (vertexAttributeLocationpecializationConstantResult.size())
        {
            String vertexAttributeLocationString(vertexAttributeLocationpecializationConstantResult[x]->name);
            if (vertexAttributeLocationString.find("VertexAttributeLocation" + std::to_string(inputs[x]->location)) != String::npos)
            {
                binding = *vertexAttributeLocationpecializationConstantResult[x]->default_literals;
            }
        }

        if (vertexAttributeLocationpecializationConstantResult.size())
        {
            String vertexInputRateLocationString(vertexInputRateLocationConstantResult[x]->name);
            if (vertexInputRateLocationString.find("VertexInputRateLocation" + std::to_string(inputs[x]->location)) != String::npos)
            {
                inputRate = *vertexInputRateLocationConstantResult[x]->default_literals;
            }
        }

        switch (inputs[x]->type_description->op)
        {
            case SpvOpTypeInt:
            {
                vertexInputAttributeList.emplace_back(VkVertexInputAttributeDescription
                    {
                        .location = inputs[x]->location,
                        .binding = binding,
                        .format = static_cast<VkFormat>(inputs[x]->format),
                        .offset = offset
                    });
                offset += inputs[x]->type_description->traits.numeric.scalar.width / 8;
                break;
            }
            case SpvOpTypeFloat:
            {
                vertexInputAttributeList.emplace_back(VkVertexInputAttributeDescription
                    {
                        .location = inputs[x]->location,
                        .binding = binding,
                        .format = static_cast<VkFormat>(inputs[x]->format),
                        .offset = offset
                    });
                offset += inputs[x]->type_description->traits.numeric.scalar.width / 8;
                break;
            }
            case SpvOpTypeVector:
            {
                vertexInputAttributeList.emplace_back(VkVertexInputAttributeDescription
                    {
                        .location = inputs[x]->location,
                        .binding = binding,
                        .format = static_cast<VkFormat>(inputs[x]->format),
                        .offset = offset
                    });
                offset += (inputs[x]->type_description->traits.numeric.scalar.width / 8) * inputs[x]->type_description->traits.numeric.vector.component_count;
                break;
            }
            case SpvOpTypeMatrix:
            {
                for (int y = 0; y < inputs[x]->type_description->traits.numeric.vector.component_count; y++)
                {
                    vertexInputAttributeList.emplace_back(VkVertexInputAttributeDescription
                        {
                            .location = inputs[x]->location,
                            .binding = binding,
                            .format = static_cast<VkFormat>(inputs[x]->format),
                            .offset = offset
                        });
                    inputs[x]->location += 1;
                    offset += (inputs[x]->type_description->traits.numeric.scalar.width / 8) * inputs[x]->type_description->traits.numeric.vector.component_count;
                }
                break;
            }
        }

        if (inputs.size() == 0 ||
            inputs.size() == 1)
        {
            vertexInputBindingList.emplace_back(VkVertexInputBindingDescription{
                   .binding = vertexInputAttributeList[x].binding,
                   .stride = offset,
                   .inputRate = static_cast<VkVertexInputRate>(inputRate)
                });
        }
        else
        {
            if (x + 1 == inputs.size() ||
               (x > 0 &&
                vertexInputAttributeList[x - 1].binding != binding))
            {
                vertexInputBindingList.emplace_back(VkVertexInputBindingDescription{
                    .binding = vertexInputAttributeList[x - 1].binding,
                    .stride = offset,
                    .inputRate = static_cast<VkVertexInputRate>(inputRate)
                    });
                offset = 0;
            }
        }
    }
}

void Shader_GetShaderDescriptorBindings(const SpvReflectShaderModule& module, Vector<ShaderDescriptorBinding>& shaderDescriptorSetBinding)
{
    uint32_t descriptorBindingsCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorBindings(&module, &descriptorBindingsCount, nullptr));
    Vector<SpvReflectDescriptorBinding*> descriptorSetBindings(descriptorBindingsCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorBindings(&module, &descriptorBindingsCount, descriptorSetBindings.data()));

    Vector<SpvReflectSpecializationConstant*> specializationConstantList = Shader_GetShaderSpecializationConstant(module);

    for (auto& descriptorBinding : descriptorSetBindings)
    {
        String name(descriptorBinding->name);
        if (!Shader_SearchDescriptorBindingExists(shaderDescriptorSetBinding, name))
        {
            String searchString(String("DescriptorBindingType" + std::to_string(descriptorBinding->binding)));
            Vector<SpvReflectSpecializationConstant*> DescriptorBindingAttributeTypeResult = Shader_SearchShaderSpecializationConstant(specializationConstantList, searchString.c_str());
            shaderDescriptorSetBinding.emplace_back(ShaderDescriptorBinding
                {
                    .Name = String(descriptorBinding->name),
                    .Binding = descriptorBinding->binding,
                    .ShaderStageFlags = static_cast<VkShaderStageFlags>(module.shader_stage),
                    .DescriptorBindingType = static_cast<DescriptorBindingPropertiesEnum>(*DescriptorBindingAttributeTypeResult[0]->default_literals),
                    .DescripterType = static_cast<VkDescriptorType>(descriptorBinding->descriptor_type)
                });
        }
        else
        {
            auto it = std::find_if(shaderDescriptorSetBinding.data(), shaderDescriptorSetBinding.data() + shaderDescriptorSetBinding.size(),
                [&](ShaderDescriptorBinding& var) {
                    var.ShaderStageFlags |= static_cast<VkShaderStageFlags>(module.shader_stage);
                    return var.Name == descriptorBinding->name;
                }
            );
        }
    }
}

void Shader_GetShaderDescriptorSetInfo(const SpvReflectShaderModule& module, Vector<ShaderStruct>& shaderStructList)
{
    uint descriptorSetCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, nullptr));
    Vector<SpvReflectDescriptorSet*> descriptorSets(descriptorSetCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, descriptorSets.data()));

    for (auto& descriptorSet : descriptorSets)
    {
        Span<SpvReflectDescriptorBinding> descriptorBindingList(*descriptorSet->bindings, descriptorSet->binding_count);
        for (auto& descriptorBinding : descriptorBindingList)
        {
            SpvReflectTypeDescription bindingType = *descriptorBinding.type_description;
            Span<SpvReflectTypeDescription> structList(bindingType.members, bindingType.member_count);
            for (auto& shaderInfo : structList)
            {
                if (shaderInfo.op == SpvOp::SpvOpTypeStruct &&
                    !Shader_SearchShaderStructExists(shaderStructList, shaderInfo.type_name))
                {
                    shaderStructList.emplace_back(Shader_GetShaderStruct(shaderInfo));
                }
            }
        }
    }
}

VkPipelineShaderStageCreateInfo Shader_LoadShader(VkDevice device, const char* filename, VkShaderStageFlagBits shaderStages)
{
    VkShaderModule shaderModule =Shader_ReadGLSLShader(device, filename, shaderStages);
    //VkShaderModule shaderModule = File_GetFileExtention(filename) == "spv" ? Shader_ReadGLSLShader(device, filename, shaderStages) : VkShaderModule(); //Shader_BuildHLSLShader(device, filename.c_str(), shaderStages);
    return VkPipelineShaderStageCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = shaderStages,
        .module = shaderModule,
        .pName = "main"
    };
}

void Shader_CompileShaders(VkDevice device, const char* fileDirectory, const char* outputDirectory)
{
        Shader_CompileGLSLShaders(device, fileDirectory, outputDirectory);
}

void Shader_CompileGLSLShaders(VkDevice device, const char* fileDirectory, const char* outputDirectory)
{
    Vector<const char*> fileExtenstionList
    {
        "vert",
        "frag",
        "tesc",
        "tess",
        "geom",
        "comp",
        "rgen",
        "rchit",
        "rmiss",
        "rahit"
    };

    size_t returnFileCount = 0;
    size_t extenstionListCount = fileExtenstionList.size();
    const char** extenstionList = memorySystem.AddPtrBuffer<const char*>(fileExtenstionList.data(), fileExtenstionList.size(), __FILE__, __LINE__, __func__, "Directory List String");
    const char** fileList = File_GetFilesFromDirectory(fileDirectory, extenstionList, extenstionListCount, returnFileCount);
    if (!fileList || returnFileCount == 0)
    {
        throw std::runtime_error("No shader files found in directory: " + String(fileDirectory));
    }

    Vector<String> shaderSourceFileList(fileList, fileList + returnFileCount);

    String command = "";
    for (const auto& shaderSource : shaderSourceFileList)
    {
        String shaderExtension = File_GetFileExtention(shaderSource.c_str());
        String shaderSourceFile = File_GetFileNameFromPath(shaderSource.c_str());
        String inputFile = std::filesystem::absolute(shaderSource).string();
        String outputFile = shaderSourceFile + shaderExtension;
        outputFile[shaderSourceFile.size()] = std::toupper(outputFile[shaderSourceFile.size()]);
        outputFile += ".spv";
      
        if (!std::filesystem::exists(inputFile.c_str()) ||
            std::filesystem::last_write_time(("..\\Assets\\Shaders\\" + outputFile).c_str()) < std::filesystem::last_write_time(inputFile.c_str()))
        {
            command += "C:/VulkanSDK/1.4.313.0/Bin/glslc.exe --target-env=vulkan1.4 --target-spv=spv1.6 " + inputFile + " -o " + outputDirectory + outputFile + "\n";
        }
    }
    String tempBatchPath = std::filesystem::temp_directory_path().append("temp_glslc.bat").string();
    std::ofstream tempBatch(tempBatchPath);
    if (!tempBatch.is_open()) 
    {
        throw std::runtime_error("Failed to create temporary batch file: " + tempBatchPath);
    }
    tempBatch << command;
    tempBatch.close();

    STARTUPINFO startUpInfo = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION processInformation;
    startUpInfo.dwFlags = STARTF_USESTDHANDLES;
    HANDLE hStdOutRead, hStdOutWrite;
    SECURITY_ATTRIBUTES securityAttributes = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &securityAttributes, 0))
    {
        std::filesystem::remove(tempBatchPath);
        throw std::runtime_error("Failed to create pipe for batch file output");
    }
    SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);
    startUpInfo.hStdOutput = hStdOutWrite;
    startUpInfo.hStdError = hStdOutWrite;

    String batchCommand = "cmd.exe /C \"" + tempBatchPath + "\"";
    LPWSTR batchCommandW = Shader_StringToLPWSTR(batchCommand);
    std::wstring workingDir = std::filesystem::path(fileDirectory).wstring();
    if (!CreateProcessW(nullptr, batchCommandW, nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, workingDir.c_str(), &startUpInfo, &processInformation)) 
    {
        DWORD errorCode = GetLastError();
        std::stringstream errorMsg;
        errorMsg << "Failed to start batch file: " << tempBatchPath << "\nWindows Error Code: " << errorCode;
        delete[] batchCommandW;
        CloseHandle(hStdOutRead);
        CloseHandle(hStdOutWrite);
        std::filesystem::remove(tempBatchPath);
        throw std::runtime_error(errorMsg.str());
    }
    delete[] batchCommandW;
    CloseHandle(hStdOutWrite);

    String output;
    char buffer[4096];
    DWORD bytesRead;
    while (true)
    {
        bool readSuccess = ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr);
        if (!readSuccess || 
            bytesRead == 0) 
        {
            break;
        }
        buffer[bytesRead] = '\0';
        output += buffer;
        std::cout << buffer;
        std::cout.flush();
    }
    CloseHandle(hStdOutRead);

    DWORD waitResult = WaitForSingleObject(processInformation.hProcess, INFINITE);
    if (waitResult == WAIT_TIMEOUT) {
        std::cerr << "Batch file process timed out: " << tempBatchPath << "\n";
        TerminateProcess(processInformation.hProcess, 1);
    }

    DWORD exitCode;
    GetExitCodeProcess(processInformation.hProcess, &exitCode);
    CloseHandle(processInformation.hProcess);
    CloseHandle(processInformation.hThread);
    std::filesystem::remove(tempBatchPath);

    for (int x = 0; x < returnFileCount; x++)
    {
        memorySystem.RemovePtrBuffer(fileList[x]);
    }
    memorySystem.RemovePtrBuffer(fileList);
    memorySystem.RemovePtrBuffer(extenstionList);
}

VkShaderModule Shader_ReadGLSLShader(VkDevice device, const char* path, VkShaderStageFlagBits stage)
{
    FileState file = File_Read(path);

    VkShaderModuleCreateInfo shaderModuleCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = file.Size,
        .pCode = (const uint32*)file.Data
    };

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VULKAN_RESULT(vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, &shaderModule));

    return shaderModule;
}

Microsoft::WRL::ComPtr<IDxcBlob> Shader_CompileHLSLShaders(VkDevice device, const String& path, Microsoft::WRL::ComPtr<IDxcCompiler3>& dxc_compiler, Microsoft::WRL::ComPtr<IDxcIncludeHandler>& defaultIncludeHandler, VkShaderStageFlagBits stage)
{
    const char* cShaderCode = File_Read(path.c_str()).Data;
    if (!cShaderCode) {
        std::cerr << "Failed to read file: " << String(path) << std::endl;
        return nullptr;
    }
    String shaderCode(cShaderCode);

    if (shaderCode.size() >= 3 &&
        static_cast<unsigned char>(shaderCode[0]) == 0xEF &&
        static_cast<unsigned char>(shaderCode[1]) == 0xBB &&
        static_cast<unsigned char>(shaderCode[2]) == 0xBF) {
        shaderCode = shaderCode.substr(3);
    }

    DxcBuffer src_buffer = {
        .Ptr = shaderCode.c_str(),
        .Size = static_cast<uint32_t>(shaderCode.size()),
        .Encoding = 0
    };

    std::vector<LPCWSTR> args;
    args.emplace_back(L"-spirv");
    args.emplace_back(L"-fspv-target-env=vulkan1.4");
    switch (stage) {
    case VK_SHADER_STAGE_VERTEX_BIT: args.emplace_back(L"-T vs_6_5"); break;
    case VK_SHADER_STAGE_FRAGMENT_BIT: args.emplace_back(L"-T ps_6_5"); break;
    case VK_SHADER_STAGE_COMPUTE_BIT: args.emplace_back(L"-T cs_6_5"); break;
    default: args.emplace_back(L"-T lib_6_5"); break;
    }

    Microsoft::WRL::ComPtr<IDxcResult> result;
    dxc_compiler->Compile(&src_buffer, args.data(), static_cast<uint32_t>(args.size()), defaultIncludeHandler.Get(), IID_PPV_ARGS(&result));

    Microsoft::WRL::ComPtr<IDxcBlob> shader_obj;
    result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader_obj), nullptr);

    Microsoft::WRL::ComPtr<IDxcBlobUtf8> error_message;
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&error_message), nullptr);
    if (error_message && error_message->GetStringLength() > 0) {
        std::cout << "Compiler error: " << error_message->GetStringPointer() << std::endl;
        std::cout << "Error in file: " << path << std::endl;
    }

    if (shader_obj) {
        String spvFileName = String(File_GetFileNameFromPath(path.c_str())) + ".spv";
        std::ofstream spvFile(spvFileName, std::ios::binary);
        spvFile.write(static_cast<const char*>(shader_obj->GetBufferPointer()), shader_obj->GetBufferSize());
        spvFile.close();
    }

    return shader_obj;
}

ShaderStruct Shader_GetShaderStruct(SpvReflectTypeDescription& shaderInfo)
{
    size_t bufferSize = 0;
    Vector<ShaderVariable> shaderVariables;
    Vector<SpvReflectTypeDescription> shaderVariableList = Vector<SpvReflectTypeDescription>(shaderInfo.members, shaderInfo.members + shaderInfo.member_count);
    for (auto& variable : shaderVariableList)
    {
        uint memberSize = 0;
        size_t byteAlignment = 0;
        ShaderMemberType memberType = shaderUnknown;
        switch (variable.op)
        {
        case SpvOpTypeInt:
        {
            memberSize = variable.traits.numeric.scalar.width / 8;
            memberType = variable.traits.numeric.scalar.signedness ? shaderUint : shaderInt;
            byteAlignment = 4;
            break;
        }
        case SpvOpTypeFloat:
        {
            memberSize = variable.traits.numeric.scalar.width / 8;
            memberType = shaderFloat;
            byteAlignment = 4;
            break;
        }
        case SpvOpTypeVector:
        {
            memberSize = (variable.traits.numeric.scalar.width / 8) * variable.traits.numeric.vector.component_count;
            switch (variable.traits.numeric.vector.component_count)
            {
            case 2:
                memberType = shaderVec2;
                byteAlignment = 8;
                break;
            case 3:
                memberType = shaderVec3;
                byteAlignment = 16;
                break;
            case 4:
                memberType = shaderVec4;
                byteAlignment = 16;
                break;
            }
            break;
        }
        case SpvOpTypeMatrix:
        {
            uint32_t rowCount = variable.traits.numeric.matrix.row_count;
            uint32_t colCount = variable.traits.numeric.matrix.column_count;
            memberSize = (variable.traits.numeric.scalar.width / 8) * rowCount * colCount;
            if (rowCount == 2 && colCount == 2)
            {
                memberType = shaderMat2;
                byteAlignment = 8;
            }
            else if (rowCount == 3 && colCount == 3)
            {
                memberType = shaderMat3;
                byteAlignment = 16;
            }
            else if (rowCount == 4 && colCount == 4)
            {
                memberType = shaderMat4;
                byteAlignment = 16;
            }
            else
            {
                std::cerr << "Unsupported matrix size: " << rowCount << "x" << colCount << std::endl;
                byteAlignment = -1;
            }
            break;
        }
        }

        shaderVariables.emplace_back(ShaderVariable
            {
                .Name = String(variable.struct_member_name),
                .Size = memberSize,
                .ByteAlignment = byteAlignment,
                .Value = nullptr,
                .MemberTypeEnum = memberType,
            });
        size_t alignment = byteAlignment;
        bufferSize = (bufferSize + alignment - 1) & ~(alignment - 1);
        bufferSize += memberSize;
    }

    return ShaderStruct
    {
        .Name = String(shaderInfo.type_name),
        .ShaderBufferSize = bufferSize,
        .ShaderBufferVariableList = shaderVariables,
        .ShaderStructBuffer = nullptr
    };;
}

void Shader_GetShaderConstBuffer(const SpvReflectShaderModule& module, Vector<ShaderPushConstant>& shaderPushConstantList)
{
    uint32_t pushConstCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumeratePushConstantBlocks(&module, &pushConstCount, nullptr));
    Vector<SpvReflectBlockVariable*> pushConstants(pushConstCount);
    SPV_VULKAN_RESULT(spvReflectEnumeratePushConstantBlocks(&module, &pushConstCount, pushConstants.data()));

    for (auto pushConstant : pushConstants)
    {
        String pushConstantName(pushConstant->name);
        if (!Shader_SearchShaderConstBufferExists(shaderPushConstantList, pushConstantName))
        {
            size_t bufferSize = 0;
            Vector<ShaderVariable> shaderVariables;
            SpvReflectTypeDescription shaderBufferMembers = *pushConstant->type_description;

            Span<SpvReflectTypeDescription> shaderVariableList(shaderBufferMembers.members, shaderBufferMembers.member_count);
            for (auto& variable : shaderVariableList)
            {
                size_t memberSize = 0;
                size_t byteAlignment = 0;
                ShaderMemberType memberType = shaderUnknown;
                switch (variable.op)
                {
                    case SpvOpTypeInt:
                    {
                        memberSize = variable.traits.numeric.scalar.width / 8;
                        memberType = variable.traits.numeric.scalar.signedness ? shaderUint : shaderInt;
                        byteAlignment = 4;
                        break;
                    }
                    case SpvOpTypeFloat:
                    {
                        memberSize = variable.traits.numeric.scalar.width / 8;
                        memberType = shaderFloat;
                        byteAlignment = 4;
                        break;
                    }
                    case SpvOpTypeVector:
                    {
                        memberSize = (variable.traits.numeric.scalar.width / 8) * variable.traits.numeric.vector.component_count;
                        switch (variable.traits.numeric.vector.component_count)
                        {
                        case 2:
                            memberType = shaderVec2;
                            byteAlignment = 8;
                            break;
                        case 3:
                            memberType = shaderVec3;
                            byteAlignment = 16;
                            break;
                        case 4:
                            memberType = shaderVec4;
                            byteAlignment = 16;
                            break;
                        }
                        break;
                    }
                    case SpvOpTypeMatrix:
                    {
                        uint rowCount = variable.traits.numeric.matrix.row_count;
                        uint colCount = variable.traits.numeric.matrix.column_count;
                        memberSize = (variable.traits.numeric.scalar.width / 8) * rowCount * colCount;
                        if (rowCount == 2 && colCount == 2)
                        {
                            memberType = shaderMat2;
                            byteAlignment = 8;
                        }
                        else if (rowCount == 3 && colCount == 3)
                        {
                            memberType = shaderMat3;
                            byteAlignment = 16;
                        }
                        else if (rowCount == 4 && colCount == 4)
                        {
                            memberType = shaderMat4;
                            byteAlignment = 16;
                        }
                        else
                        {
                            std::cerr << "Unsupported matrix size: " << rowCount << "x" << colCount << std::endl;
                            byteAlignment = -1;
                        }
                        break;
                    }
                }

                shaderVariables.emplace_back(ShaderVariable
                    {
                        .Name = String(variable.struct_member_name),
                        .Size = memberSize,
                        .ByteAlignment = byteAlignment,
                        .Value = nullptr,
                        .MemberTypeEnum = memberType,
                    });
                size_t alignment = byteAlignment;
                bufferSize = (bufferSize + alignment - 1) & ~(alignment - 1);
                bufferSize += memberSize;
            }

            shaderPushConstantList.emplace_back(ShaderPushConstant
                {
                    .PushConstantName = pushConstantName,
                    .PushConstantSize = bufferSize,
                    .ShaderStageFlags = static_cast<VkShaderStageFlags>(module.shader_stage),
                    .PushConstantVariableList = shaderVariables,
                });
        }
        else
        {

            auto it = std::find_if(shaderPushConstantList.data(), shaderPushConstantList.data() + shaderPushConstantList.size(),
                [&](ShaderPushConstant& var) {
                    var.ShaderStageFlags |= static_cast<VkShaderStageFlags>(module.shader_stage);
                    return var.PushConstantName == String(pushConstant->name);
                }
            );
        }
    }
}

Vector<SpvReflectInterfaceVariable*> Shader_GetShaderVertexInputVariables(const SpvReflectShaderModule& module)
{
    uint32 inputCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateInputVariables(&module, &inputCount, nullptr));
    Vector<SpvReflectInterfaceVariable*> inputs(inputCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateInputVariables(&module, &inputCount, inputs.data()));

    inputs.erase(std::remove_if(inputs.begin(), inputs.end(), [](SpvReflectInterfaceVariable* input) { return input->built_in != -1; }),
        inputs.end()
    );
    inputs.shrink_to_fit();

    std::sort(inputs.begin(), inputs.end(), [](SpvReflectInterfaceVariable* a, SpvReflectInterfaceVariable* b)
        {
            return a->location < b->location;
        });
    return inputs;
}

Vector<SpvReflectInterfaceVariable*> Shader_GetShaderVertexOutputVariables(const SpvReflectShaderModule& module)
{
    uint32 outputCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&module, &outputCount, nullptr));
    Vector<SpvReflectInterfaceVariable*> outputs(outputCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&module, &outputCount, outputs.data()));
    return outputs;
}

Vector<SpvReflectSpecializationConstant*> Shader_GetShaderSpecializationConstant(const SpvReflectShaderModule& module)
{
    uint32 specializationConstantCount = 0;
    spvReflectEnumerateSpecializationConstants(&module, &specializationConstantCount, nullptr);
    std::vector<SpvReflectSpecializationConstant*> specializationConstantList(specializationConstantCount);
    spvReflectEnumerateSpecializationConstants(&module, &specializationConstantCount, specializationConstantList.data());
    return specializationConstantList;
}

Vector<SpvReflectSpecializationConstant*> Shader_SearchShaderSpecializationConstant(Vector<SpvReflectSpecializationConstant*>& specializationConstantList, const char* searchString)
{
    Vector<SpvReflectSpecializationConstant*> results;
    for (auto* constant : specializationConstantList)
    {
        if (constant &&
            constant->name)
        {
            String nameStr(constant->name);
            if (nameStr.find(searchString) != String::npos)
            {
                results.push_back(constant);
            }
        }
    }
    return results;
}

const ShaderPushConstant* Shader_SearchShaderConstBuffer(const Vector<ShaderPushConstant>& shaderPushConstantList, const String& constBufferName)
{
    auto it = std::ranges::find_if(shaderPushConstantList, [&](const ShaderPushConstant& var)
        {
            return var.PushConstantName == constBufferName;
        });
    return (it != shaderPushConstantList.end()) ? &(*it) : nullptr;
}

 const ShaderVariable* Shader_SearchShaderConstStructVar(const ShaderPushConstant* pushConstant, const String& varName)
{
    auto& variableList = pushConstant->PushConstantVariableList;
    auto it = std::ranges::find_if(variableList, [&](const ShaderVariable& var)
        {
            return std::strcmp(var.Name.c_str(), varName.c_str()) == 0;
        });
    return (it != variableList.end()) ? &(*it) : nullptr;
}

const ShaderStruct* Shader_SearchShaderStructs(const Vector<ShaderStruct>& shaderStructList, const String& structName)
{
    auto it = std::ranges::find_if(shaderStructList, [&](const ShaderStruct& var)
        {
            return var.Name.c_str() != nullptr && std::strcmp(var.Name.c_str(), structName.c_str()) == 0;
        });
    return (it != shaderStructList.end()) ? &(*it) : nullptr;
}

ShaderVariable* Shader_SearchShaderStructVar(ShaderStruct* shaderStruct, const char* varName)
{
    if (shaderStruct == nullptr ||
        varName == nullptr)
    {
        return nullptr;
    }

    auto it = std::ranges::find_if(shaderStruct->ShaderBufferVariableList, [&](const ShaderVariable& var)
        {
            return var.Name.c_str() != nullptr && std::strcmp(var.Name.c_str(), varName) == 0;
        });

    return (it != shaderStruct->ShaderBufferVariableList.end()) ? &(*it) : nullptr;
}

bool Shader_SearchShaderConstBufferExists(const Vector<ShaderPushConstant>& shaderPushConstantList, const String& constBufferName)
{
    auto it = std::ranges::find_if(shaderPushConstantList, [&](const ShaderPushConstant& var)
        {
            return var.PushConstantName == constBufferName;
        });
    return it != shaderPushConstantList.end();
}

bool Shader_SearchDescriptorBindingExists(const Vector<ShaderDescriptorBinding>& shaderDescriptorBindingList, const String& descriptorBindingName)
{
    auto it = std::ranges::find_if(shaderDescriptorBindingList, [&](const ShaderDescriptorBinding& var)
        {
            return var.Name == descriptorBindingName;
        });
    return it != shaderDescriptorBindingList.end();
}

bool Shader_SearchShaderStructExists(const Vector<ShaderStruct>& shaderStructList, const String& structName)
{
    auto it = std::ranges::find_if(shaderStructList, [&](const ShaderStruct& var)
        {
            return var.Name.c_str() != nullptr && std::strcmp(var.Name.c_str(), structName.c_str()) == 0;
        });
    return (it != shaderStructList.end()) ? true : false;
}

bool Shader_SearchShaderStructVarExists(const ShaderStruct* shaderStruct, const char* varName)
{
    if (shaderStruct == nullptr ||
        varName == nullptr)
    {
        return false;
    }

    auto it = std::ranges::find_if(shaderStruct->ShaderBufferVariableList, [&](const ShaderVariable& var)
        {
            return var.Name.c_str() != nullptr && std::strcmp(var.Name.c_str(), varName) == 0;
        });
    return (it != shaderStruct->ShaderBufferVariableList.end()) ? true : false;
}

const char* Renderer_GetShaderReflectError(SpvReflectResult result)
{
    switch (result)
    {
        case SPV_REFLECT_RESULT_SUCCESS: return "SPV_REFLECT_RESULT_SUCCESS";
        case SPV_REFLECT_RESULT_NOT_READY: return "SPV_REFLECT_RESULT_NOT_READY";
        case SPV_REFLECT_RESULT_ERROR_PARSE_FAILED: return "SPV_REFLECT_RESULT_ERROR_PARSE_FAILED";
        case SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED: return "SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED";
        case SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED: return "SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED";
        case SPV_REFLECT_RESULT_ERROR_NULL_POINTER: return "SPV_REFLECT_RESULT_ERROR_NULL_POINTER";
        case SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR: return "SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR";
        case SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH: return "SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH";
        case SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND: return "SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF: return "SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW: return "SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION: return "SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA: return "SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE: return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_MAX_RECURSIVE_EXCEEDED: return "SPV_REFLECT_RESULT_ERROR_SPIRV_MAX_RECURSIVE_EXCEEDED";
        default: return "Unknown Result";
    }
}