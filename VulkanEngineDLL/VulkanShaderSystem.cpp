#include "VulkanShaderSystem.h"
#include <regex>
#include "MemorySystem.h"
#include "CHelper.h"
#include "FileSystem.h"
#include "BufferSystem.h"

ShaderSystem shaderSystem = ShaderSystem();

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
    Shader_DestroyShaderBindingData(shader.DescriptorBindingsList, shader.DescriptorBindingCount);
    memorySystem.RemovePtrBuffer<ShaderPushConstant>(shader.PushConstantList);
    memorySystem.RemovePtrBuffer<ShaderDescriptorBinding>(shader.DescriptorBindingsList);
    memorySystem.RemovePtrBuffer<VkVertexInputBindingDescription>(shader.VertexInputBindingList);
    memorySystem.RemovePtrBuffer<VkVertexInputAttributeDescription>(shader.VertexInputAttributeList);
}

void Shader_DestroyShaderStructData(ShaderStruct* shaderStruct, size_t shaderStrucCount)
{
    Span<ShaderStruct> shaderStructList(shaderStruct, shaderStruct + shaderStrucCount);
    for (auto& shaderStruct : shaderStructList)
    {
        if (shaderStruct.ShaderBufferVariableList != nullptr)
        {
            Span<ShaderVariable> shaderVarList(shaderStruct.ShaderBufferVariableList, shaderStruct.ShaderBufferVariableListCount);
            for (auto& shaderVar : shaderVarList)
            {
                if (shaderVar.Name != nullptr) memorySystem.RemovePtrBuffer(shaderVar.Name);
                if (shaderVar.Value != nullptr) memorySystem.RemovePtrBuffer(shaderVar.Value);
            }
        }
        if (shaderStruct.Name != nullptr) memorySystem.RemovePtrBuffer(shaderStruct.Name);
        if (shaderStruct.ShaderBufferVariableList != nullptr) memorySystem.RemovePtrBuffer(shaderStruct.ShaderBufferVariableList);
        if (shaderStruct.ShaderStructBuffer != nullptr) memorySystem.RemovePtrBuffer(shaderStruct.ShaderStructBuffer);
    }
}

void Shader_DestroyShaderBindingData(ShaderDescriptorBinding* descriptorBinding, size_t descriptorCount)
{
    if (descriptorBinding->Name != nullptr) memorySystem.RemovePtrBuffer(descriptorBinding->Name);
}

void Shader_DestroyPushConstantBufferData(ShaderPushConstant* pushConstant, size_t pushConstantCount)
{
    Span<ShaderPushConstant> shaderPushConstantList(pushConstant, pushConstant + pushConstantCount);
    for (auto& shaderPushConstant : shaderPushConstantList)
    {
        if (shaderPushConstant.PushConstantVariableList != nullptr)
        {
            Span<ShaderVariable> shaderVarList(shaderPushConstant.PushConstantVariableList, shaderPushConstant.PushConstantVariableCount);
            for (auto& shaderVar : shaderVarList)
            {
                if (shaderVar.Name != nullptr) memorySystem.RemovePtrBuffer(shaderVar.Name);
                if (shaderVar.Value != nullptr) memorySystem.RemovePtrBuffer(shaderVar.Value);
            }
        }
        if (shaderPushConstant.PushConstantName != nullptr) memorySystem.RemovePtrBuffer(shaderPushConstant.PushConstantName);
        if (shaderPushConstant.PushConstantVariableList != nullptr) memorySystem.RemovePtrBuffer(shaderPushConstant.PushConstantVariableList);
        if (shaderPushConstant.PushConstantBuffer != nullptr) memorySystem.RemovePtrBuffer(shaderPushConstant.PushConstantBuffer);
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
    Span<ShaderVariable> shaderStructVarList(shaderStruct->ShaderBufferVariableList, shaderStruct->ShaderBufferVariableListCount);
    for (const auto& shaderStrucVar : shaderStructVarList)
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
    Span<ShaderVariable> pushConstantVarList(pushConstantStruct.PushConstantVariableList, pushConstantStruct.PushConstantVariableCount);
    for (const auto& pushConstantVar : pushConstantVarList)
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
    const char* copiedStr = memorySystem.AddPtrBuffer(shaderStructToCopy.Name ? shaderStructToCopy.Name : "", __FILE__, __LINE__, __func__, "shaderStructToCopy.Name copy");
    ShaderStruct shaderStruct = ShaderStruct
    {
        .Name = copiedStr,
        .ShaderBufferSize = shaderStructToCopy.ShaderBufferSize,
        .ShaderBufferVariableListCount = shaderStructToCopy.ShaderBufferVariableListCount,
        .ShaderBufferVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(shaderStructToCopy.ShaderBufferVariableListCount, __FILE__, __LINE__, __func__, copiedStr),
        .ShaderStructBufferId = shaderStructToCopy.ShaderStructBufferId,
        .ShaderStructBuffer = memorySystem.AddPtrBuffer<byte>(shaderStructToCopy.ShaderBufferSize, __FILE__, __LINE__, __func__, copiedStr),
    };

    Span<ShaderVariable> sourceVarList(shaderStructToCopy.ShaderBufferVariableList, shaderStructToCopy.ShaderBufferVariableListCount);
    Span<ShaderVariable> destVarList(shaderStruct.ShaderBufferVariableList, shaderStruct.ShaderBufferVariableListCount);
    assert(sourceVarList.size() == destVarList.size());
    for (size_t x = 0; x < sourceVarList.size(); ++x)
    {
        ShaderVariable& destVar = destVarList[x];
        const ShaderVariable& srcVar = sourceVarList[x];

        destVar.Name = memorySystem.AddPtrBuffer(srcVar.Name ? srcVar.Name : "", __FILE__, __LINE__, __func__, "ShaderVariable.Name copy");
        destVar.Size = srcVar.Size;
        destVar.ByteAlignment = srcVar.ByteAlignment;
        destVar.MemberTypeEnum = srcVar.MemberTypeEnum;

        destVar.Value = memorySystem.AddPtrBuffer<byte>(destVar.Size, __FILE__, __LINE__, __func__, destVar.Name ? destVar.Name : "Unnamed ShaderVariable");
        Shader_SetVariableDefaults(destVar);
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
                    .Name = memorySystem.AddPtrBuffer(descriptorBinding->name, __FILE__, __LINE__, __func__),
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
                    !Shader_SearchShaderStructExists(shaderStructList.data(), shaderStructList.size(), shaderInfo.type_name))
                {
                    shaderStructList.emplace_back(Shader_GetShaderStruct(shaderInfo));
                }
            }
        }
    }
}

VkPipelineShaderStageCreateInfo Shader_LoadShader(VkDevice device, const char* filename, VkShaderStageFlagBits shaderStages)
{
    VkShaderModule shaderModule = Shader_ReadGLSLShader(device, filename, shaderStages);
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

ShaderStruct Shader_GetShaderStruct(SpvReflectTypeDescription& shaderInfo)
{
    size_t bufferSize = 0;
    Vector<ShaderVariable> shaderVariables;
    Vector<SpvReflectTypeDescription> shaderVariableList = Vector<SpvReflectTypeDescription>(shaderInfo.members, shaderInfo.members + shaderInfo.member_count);
    for (auto& variable : shaderVariableList)
    {
        uint memberSize = 0;
        size_t byteAlignment = 0;
        ShaderMemberType memberType;
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
                .Name = memorySystem.AddPtrBuffer(variable.struct_member_name, __FILE__, __LINE__, __func__, ("variable.struct_member_name - " + String(variable.struct_member_name) + "copy 564").c_str()),
                .Size = memberSize,
                .ByteAlignment = byteAlignment,
                .Value = nullptr,
                .MemberTypeEnum = memberType,
            });
        size_t alignment = byteAlignment;
        bufferSize = (bufferSize + alignment - 1) & ~(alignment - 1);
        bufferSize += memberSize;
    }

    ShaderStruct shaderStruct =
    {
        .Name = memorySystem.AddPtrBuffer(shaderInfo.type_name, __FILE__, __LINE__, __func__, "shader struct name copy"),
        .ShaderBufferSize = bufferSize,
        .ShaderBufferVariableListCount = shaderVariables.size(),
        .ShaderBufferVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(shaderVariables.data(), shaderVariables.size(), __FILE__, __LINE__, __func__, ("Struct Name: " + std::string(shaderStruct.Name)).c_str()),
        .ShaderStructBuffer = nullptr
    };
    return shaderStruct;
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
                        .Name = memorySystem.AddPtrBuffer(variable.struct_member_name, __FILE__, __LINE__, __func__, "variable.struct_member_name copy"),
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
                    .PushConstantName = memorySystem.AddPtrBuffer(pushConstantName.c_str(), __FILE__, __LINE__, __func__, pushConstantName.c_str()),
                    .PushConstantSize = bufferSize,
                    .PushConstantVariableCount = shaderVariables.size(),
                    .ShaderStageFlags = static_cast<VkShaderStageFlags>(module.shader_stage),
                    .PushConstantVariableList = memorySystem.AddPtrBuffer(shaderVariables.data(), shaderVariables.size(), __FILE__, __LINE__, __func__, pushConstantName.c_str()),
                });

            for (size_t x = 0; x < shaderVariables.size(); ++x)
            {
                shaderVariables[x].Name = nullptr;
                shaderVariables[x].Value = nullptr;
            }
        }
        else
        {
            auto it = std::find_if(shaderPushConstantList.data(), shaderPushConstantList.data() + shaderPushConstantList.size(),
                [&](ShaderPushConstant& var) {
                    var.ShaderStageFlags |= static_cast<VkShaderStageFlags>(module.shader_stage);
                    return var.PushConstantName == pushConstant->name;
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

const ShaderPushConstant* Shader_SearchShaderConstBuffer(const Vector<ShaderPushConstant>& shaderPushConstantList, const char* constBufferName)
{
    auto it = std::ranges::find_if(shaderPushConstantList, [&](const ShaderPushConstant& var)
        {
            return var.PushConstantName == constBufferName;
        });
    return (it != shaderPushConstantList.end()) ? &(*it) : nullptr;
}

ShaderVariable* Shader_SearchShaderConstStructVar(ShaderPushConstant* pushConstant, const char* varName)
{
    if (pushConstant == nullptr)
    {
        return nullptr;
    }

    Span pushConstantVariableList(pushConstant->PushConstantVariableList, pushConstant->PushConstantVariableCount);
    auto it = std::ranges::find_if(pushConstantVariableList, [&](const ShaderVariable& var)
        {
            return var.Name != nullptr &&
                std::strcmp(var.Name, varName) == 0;
        }
    );
    return (it != pushConstantVariableList.end()) ? &(*it) : nullptr;
}

ShaderStruct* Shader_SearchShaderStructs(ShaderStruct* shaderStructList, size_t shaderStructCount, const char* structName)
{
    if (shaderStructList == nullptr ||
        shaderStructList->ShaderBufferVariableList == nullptr ||
        structName == nullptr)
    {
        return nullptr;
    }

    Span span = Span(shaderStructList, shaderStructCount);
    auto it = std::ranges::find_if(span, [&](const ShaderStruct& var)
        {
            return var.Name != nullptr && std::strcmp(var.Name, structName) == 0;
        });

    return (it != span.end()) ? &(*it) : nullptr;
}

ShaderVariable* Shader_SearchShaderStructVar(ShaderStruct* shaderStruct, const char* varName)
{
    if (shaderStruct == nullptr ||
        shaderStruct->Name == nullptr ||
        varName == nullptr)
    {
        return nullptr;
    }

    Span span = Span(shaderStruct->ShaderBufferVariableList, shaderStruct->ShaderBufferVariableListCount);
    auto it = std::ranges::find_if(span, [&](const ShaderVariable& var)
        {
            return var.Name != nullptr && std::strcmp(var.Name, varName) == 0;
        });

    return (it != span.end()) ? &(*it) : nullptr;
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

bool Shader_SearchShaderStructExists(ShaderStruct* shaderStructList, size_t shaderStructCount, const char* structName)
{
    if (shaderStructList == nullptr ||
        shaderStructList->Name == nullptr ||
        structName == nullptr)
    {
        return false;
    }

    Span span = Span(shaderStructList, shaderStructCount);
    auto it = std::ranges::find_if(span, [&](const ShaderStruct& var)
        {
            return var.Name != nullptr && std::strcmp(var.Name, structName) == 0;
        });

    return (it != span.end()) ? true : false;
}

bool Shader_SearchShaderStructVarExists(const ShaderStruct* shaderStruct, const char* varName)
{
    if (shaderStruct == nullptr ||
        shaderStruct->ShaderBufferVariableList == nullptr ||
        varName == nullptr)
    {
        return false;
    }

    Span span = Span(shaderStruct->ShaderBufferVariableList, shaderStruct->ShaderBufferVariableListCount);
    auto it = std::ranges::find_if(span, [&](const ShaderVariable& var)
        {
            return var.Name != nullptr && std::strcmp(var.Name, varName) == 0;
        });

    return (it != span.end()) ? true : false;
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

VkPipelineShaderStageCreateInfo Shader_CreateShader(VkShaderModule shaderModule, VkShaderStageFlagBits shaderStages)
{
    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = shaderStages,
        .module = shaderModule,
        .pName = "main"
    };

    return pipelineShaderStageCreateInfo;
}

VkShaderModule Shader_BuildGLSLShaderFile(VkDevice device, const char* path)
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

bool Shader_BuildGLSLShaders(const char* command)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, command, -1, NULL, 0);
    std::wstring wcommand(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, command, -1, &wcommand[0], size_needed);

    STARTUPINFOW startUpInfo = { sizeof(startUpInfo) };
    PROCESS_INFORMATION processInfo;
    if (CreateProcessW(NULL, (LPWSTR)wcommand.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &startUpInfo, &processInfo)) {
        WaitForSingleObject(processInfo.hProcess, INFINITE);

        DWORD exitCode;
        GetExitCodeProcess(processInfo.hProcess, &exitCode);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        return exitCode == 0;
    }
    return false;
}

void Shader_Destroy()
{
    Vector<ShaderPushConstant> shaderPushConstantList;
    shaderPushConstantList.reserve(shaderSystem.ShaderPushConstantMap.size());
    for (const auto& pair : shaderSystem.ShaderPushConstantMap) {
        shaderPushConstantList.push_back(pair.second);
    }
    Shader_DestroyPushConstantBufferData(shaderPushConstantList.data(), shaderPushConstantList.size());

    Vector<ShaderStruct> shaderStructProtoList;
    shaderStructProtoList.reserve(shaderSystem.PipelineShaderStructPrototypeMap.size());
    for (const auto& pair : shaderSystem.PipelineShaderStructPrototypeMap) {
        shaderStructProtoList.push_back(pair.second);
    }
    Shader_DestroyShaderStructData(shaderStructProtoList.data(), shaderStructProtoList.size());

    Vector<ShaderStruct> shaderStructList;
    shaderStructList.reserve(shaderSystem.PipelineShaderStructMap.size());
    for (const auto& pair : shaderSystem.PipelineShaderStructMap) {
        shaderStructList.push_back(pair.second);
    }
    Shader_DestroyShaderStructData(shaderStructList.data(), shaderStructList.size());

    Vector<String> shaderModuleKeys;
    for (const auto& pair : shaderSystem.ShaderModuleMap)
    {
        shaderModuleKeys.push_back(pair.first);
    }
    for (const auto& key : shaderModuleKeys)
    {
        auto& pipelineData = shaderSystem.ShaderModuleMap[key];
        Shader_ShaderDestroy(pipelineData);
    }

    shaderSystem.ShaderModuleMap.clear();
}

ShaderPipelineData Shader_LoadShaderPipelineData(Vector<String> shaderPathList)
{
    const char** cShaderList = CHelper_VectorToConstCharPtrPtr(shaderPathList);
    ShaderPipelineData pipelineData = Shader_LoadPipelineShaderData(cShaderList, shaderPathList.size());

    Span<ShaderPushConstant> pushConstantList(pipelineData.PushConstantList, pipelineData.PushConstantCount);
    for (auto& pushConstant : pushConstantList)
    {
        if (!Shader_ShaderPushConstantExists(pushConstant.PushConstantName))
        {
            shaderSystem.ShaderPushConstantMap[pushConstant.PushConstantName] = ShaderPushConstant
            {
                .PushConstantName = memorySystem.AddPtrBuffer(pushConstant.PushConstantName, __FILE__, __LINE__, __func__, pushConstant.PushConstantName),
                .PushConstantSize = pushConstant.PushConstantSize,
                .PushConstantVariableCount = pushConstant.PushConstantVariableCount,
                .ShaderStageFlags = pushConstant.ShaderStageFlags,
                .PushConstantVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(pushConstant.PushConstantVariableList, pushConstant.PushConstantVariableCount, __FILE__, __LINE__, __func__, pushConstant.PushConstantName),
                .PushConstantBuffer = memorySystem.AddPtrBuffer<byte>(pushConstant.PushConstantSize, __FILE__, __LINE__, __func__, pushConstant.PushConstantName),
                .GlobalPushContant = pushConstant.GlobalPushContant
            };

            Span<ShaderVariable> shaderVarList(pushConstant.PushConstantVariableList, pushConstant.PushConstantVariableCount);
            for (int x = 0; x < pushConstant.PushConstantVariableCount; x++)
            {
                shaderSystem.ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Value = memorySystem.AddPtrBuffer<byte>(shaderSystem.ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Size, __FILE__, __LINE__, __func__, shaderSystem.ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x].Name);
                Shader_SetVariableDefaults(shaderSystem.ShaderPushConstantMap[pushConstant.PushConstantName].PushConstantVariableList[x]);
            }

            pushConstant.PushConstantName = nullptr;
            pushConstant.PushConstantVariableList = nullptr;
        }
    }

    Span<ShaderDescriptorBinding> shaderDescriptorBindingList(pipelineData.DescriptorBindingsList, pipelineData.DescriptorBindingCount);
    for (auto& descriptorBinding : shaderDescriptorBindingList)
    {
        if (descriptorBinding.Name != nullptr)
            memorySystem.RemovePtrBuffer(descriptorBinding.Name);
    }

    for (auto& shaderStruct : pushConstantList)
    {
        if (shaderStruct.PushConstantVariableList != nullptr)
        {
            Span<ShaderVariable> shaderVarList(shaderStruct.PushConstantVariableList, shaderStruct.PushConstantVariableCount);
            for (auto& shaderVar : shaderVarList)
            {
                if (shaderVar.Name != nullptr) memorySystem.RemovePtrBuffer(shaderVar.Name);
                if (shaderVar.Value != nullptr) memorySystem.RemovePtrBuffer(shaderVar.Value);
            }
        }
        if (shaderStruct.PushConstantName != nullptr) memorySystem.RemovePtrBuffer(shaderStruct.PushConstantName);
        if (shaderStruct.PushConstantVariableList != nullptr) memorySystem.RemovePtrBuffer(shaderStruct.PushConstantVariableList);
        if (shaderStruct.PushConstantBuffer != nullptr) memorySystem.RemovePtrBuffer(shaderStruct.PushConstantBuffer);
    }
    shaderSystem.ShaderModuleMap[pipelineData.ShaderList[0]] = pipelineData;
    CHelper_DestroyConstCharPtrPtr(cShaderList);
    return pipelineData;
}

const ShaderVariable* Shader_SearchGlobalShaderConstantVar(ShaderPushConstant* pushConstant, const char* varName)
{
    return Shader_SearchShaderConstStructVar(pushConstant, varName);
}

ShaderVariable* Shader_SearchShaderStruct(ShaderStruct& shaderStruct, const String& varName)
{
    return Shader_SearchShaderStructVar(&shaderStruct, varName.c_str());
}

void Shader_UpdateGlobalShaderBuffer(const GraphicsRenderer& renderer, const String& pushConstantName)
{
    if (!Shader_ShaderPushConstantExists(pushConstantName))
    {
        std::cerr << "Error: Push constant '" << pushConstantName << "' does not exist!" << std::endl;
        return;
    }
    Shader_UpdatePushConstantBuffer(renderer, shaderSystem.ShaderPushConstantMap[pushConstantName]);
}

void Shader_UpdateShaderBuffer(const GraphicsRenderer& renderer, uint vulkanBufferId)
{
    if (!Shader_ShaderStructExists(vulkanBufferId))
    {
        return;
    }

    ShaderStruct& shaderStruct = shaderSystem.PipelineShaderStructMap[vulkanBufferId];
    VulkanBuffer& vulkanBuffer = bufferSystem.FindVulkanBuffer(vulkanBufferId);
    Shader_UpdateShaderBuffer(renderer, vulkanBuffer, &shaderStruct, 1);
}

ShaderPushConstant* Shader_GetGlobalShaderPushConstant(const String& pushConstantName)
{
    return Shader_ShaderPushConstantExists(pushConstantName) ? &shaderSystem.ShaderPushConstantMap[pushConstantName] : nullptr;
}

void Shader_LoadShaderPipelineStructPrototypes(const char** renderPassJsonList, size_t pipelineShaderCount)
{
    Vector<String> shaderPaths;
    shaderPaths.reserve(pipelineShaderCount);
    for (size_t x = 0; x < pipelineShaderCount; x++) 
    {
        shaderPaths.emplace_back(renderPassJsonList[x]);
    }

    size_t protoTypeStructCount = 0;
    for (size_t x = 0; x < shaderPaths.size(); ++x)
    {
        nlohmann::json renderPassJson = File_LoadJsonFile(shaderPaths[x].c_str());
        for (size_t y = 0; y < renderPassJson["RenderPipelineList"].size(); ++y)
        {
            nlohmann::json pipelineJson = File_LoadJsonFile(renderPassJson["RenderPipelineList"][y].get<String>().c_str());
            Vector<String> shaderJsonList = Vector<String>{ pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] };
            const char** cShaderList = CHelper_VectorToConstCharPtrPtr(shaderJsonList);
            ShaderStruct* shaderStructArray = Shader_LoadProtoTypeStructs(cShaderList, shaderJsonList.size(), protoTypeStructCount);
            Span<ShaderStruct> shaderStructList(shaderStructArray, protoTypeStructCount);
            for (auto& shaderStruct : shaderStructList)
            {
                if (!Shader_ShaderStructPrototypeExists(shaderStruct.Name))
                {
                    String name = shaderStruct.Name;
                    shaderSystem.PipelineShaderStructPrototypeMap[name] = ShaderStruct
                    {
                        .Name = memorySystem.AddPtrBuffer(shaderStruct.Name, __FILE__, __LINE__, __func__),
                        .ShaderBufferSize = shaderStruct.ShaderBufferSize,
                        .ShaderBufferVariableListCount = shaderStruct.ShaderBufferVariableListCount,
                        .ShaderBufferVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(shaderStruct.ShaderBufferVariableList, shaderStruct.ShaderBufferVariableListCount, __FILE__, __LINE__, __func__, name.c_str()),
                        .ShaderStructBufferId = shaderStruct.ShaderStructBufferId,
                        .ShaderStructBuffer = memorySystem.AddPtrBuffer<byte>(shaderStruct.ShaderBufferSize, __FILE__, __LINE__, __func__, name.c_str())
                    };

                    for (size_t z = 0; z < shaderStruct.ShaderBufferVariableListCount; ++z)
                    {
                        shaderStruct.ShaderBufferVariableList[z].Name = nullptr;
                        shaderStruct.ShaderBufferVariableList[z].Value = nullptr;
                    }
                }
            }

            for (auto& shaderStruct : shaderStructList)
            {
                if (shaderStruct.ShaderBufferVariableList != nullptr)
                {
                    Span<ShaderVariable> shaderVarList(shaderStruct.ShaderBufferVariableList, shaderStruct.ShaderBufferVariableListCount);
                    for (auto& shaderVar : shaderVarList)
                    {
                        if (shaderVar.Name != nullptr) memorySystem.RemovePtrBuffer(shaderVar.Name);
                        if (shaderVar.Value != nullptr) memorySystem.RemovePtrBuffer(shaderVar.Value);
                    }
                }

                if (shaderStruct.Name != nullptr) memorySystem.RemovePtrBuffer(shaderStruct.Name);
                if (shaderStruct.ShaderBufferVariableList != nullptr) memorySystem.RemovePtrBuffer(shaderStruct.ShaderBufferVariableList);
                if (shaderStruct.ShaderStructBuffer != nullptr) memorySystem.RemovePtrBuffer(shaderStruct.ShaderStructBuffer);
            }

            Shader_DestroyShaderStructData(shaderStructList.data(), shaderStructList.size());
            CHelper_DestroyConstCharPtrPtr(cShaderList);
            memorySystem.RemovePtrBuffer(shaderStructArray);
        }
    }
}

ShaderPipelineData& Shader_FindShaderModule(const String& shaderFile)
{
    return shaderSystem.ShaderModuleMap.at(shaderFile);
}

ShaderPushConstant& Shader_FindShaderPushConstant(const String& shaderFile)
{
    return shaderSystem.ShaderPushConstantMap.at(shaderFile);
}

ShaderStruct Shader_FindShaderProtoTypeStruct(const String& shaderKey)
{
    return shaderSystem.PipelineShaderStructPrototypeMap.at(shaderKey);
}

ShaderStruct& Shader_FindShaderStruct(int vulkanBufferId)
{
    return shaderSystem.PipelineShaderStructMap.at(vulkanBufferId);
}

ShaderStruct Shader_CopyShaderStructProtoType(const String& structName)
{
    ShaderStruct shaderStructCopy = Shader_FindShaderProtoTypeStruct(structName);
    return Shader_CopyShaderStructPrototype(shaderStructCopy);
}

 bool Shader_ShaderPushConstantExists(const String& pushConstantName)
{
    return shaderSystem.ShaderPushConstantMap.contains(pushConstantName);
}

 bool Shader_ShaderModuleExists(const String& shaderFile)
{
    return shaderSystem.ShaderModuleMap.contains(shaderFile);

 }

 bool Shader_ShaderStructPrototypeExists(const String& structKey)
{
    return shaderSystem.PipelineShaderStructPrototypeMap.contains(structKey);
 }

 bool Shader_ShaderStructExists(uint vulkanBufferKey)
 {
     return shaderSystem.PipelineShaderStructMap.contains(vulkanBufferKey);
 }