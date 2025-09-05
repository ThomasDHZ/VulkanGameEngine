#include "VulkanShader.h"
#include <regex>
#include "MemorySystem.h"
#include "json.h"
#include "CHelper.h"

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

    ShaderPipelineData pipelineData = ShaderPipelineData
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
    return pipelineData;
}

void Shader_ShaderDestroy(ShaderPipelineData& shader)
{
    Shader_DestroyShaderBindingData(shader);
    memorySystem.RemovePtrBuffer<ShaderPushConstant>(shader.PushConstantList);
    memorySystem.RemovePtrBuffer<ShaderDescriptorBinding>(shader.DescriptorBindingsList);
    memorySystem.RemovePtrBuffer<VkVertexInputBindingDescription>(shader.VertexInputBindingList);
    memorySystem.RemovePtrBuffer<VkVertexInputAttributeDescription>(shader.VertexInputAttributeList);
}

void Shader_DestroyShaderStructData(ShaderStruct* shaderStruct)
{
    if (!shaderStruct)
    {
        return;
    }

    Span<ShaderVariable> shaderVarList(shaderStruct->ShaderBufferVariableList, shaderStruct->ShaderBufferVariableListCount);
    for (auto& shaderVar : shaderVarList)
    {
        if (shaderVar.Value)
        {
            shaderVar.ByteAlignment = 0;
            shaderVar.MemberTypeEnum = ShaderMemberType::shaderUnknown;
            shaderVar.Name = "";
            shaderVar.Size = 0;
            memorySystem.RemovePtrBuffer(shaderVar.Value);
        }
    }

    shaderStruct->Name = "";
    shaderStruct->ShaderBufferSize = 0;
    shaderStruct->ShaderBufferVariableListCount = 0;
    shaderStruct->ShaderStructBufferId = 0;
    if (shaderStruct->ShaderBufferVariableList)
    {
        memorySystem.RemovePtrBuffer<ShaderVariable>(shaderStruct->ShaderBufferVariableList);
    }
    if (shaderStruct->ShaderStructBuffer)
    {
        memorySystem.RemovePtrBuffer(shaderStruct->ShaderStructBuffer);
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

void Shader_DestroyPushConstantBufferData(ShaderPushConstant* pushConstant)
{
    if (!pushConstant)
    {
        return;
    }

    Span<ShaderVariable> shaderVarList(pushConstant->PushConstantVariableList, pushConstant->PushConstantVariableListCount);
    for (int x = 0; x < pushConstant->PushConstantVariableListCount; x++)
    {
        if (pushConstant->PushConstantVariableList[x].Value)
        {
            pushConstant->PushConstantVariableList[x].ByteAlignment = 0;
            pushConstant->PushConstantVariableList[x].MemberTypeEnum = ShaderMemberType::shaderUnknown;
            pushConstant->PushConstantVariableList[x].Name = "";
            pushConstant->PushConstantVariableList[x].Size = 0;
            memorySystem.RemovePtrBuffer(pushConstant->PushConstantVariableList[x].Value);
        }
    }

    pushConstant->GlobalPushContant = false;
    pushConstant->PushConstantName = "";
    pushConstant->PushConstantSize = 0;
    pushConstant->PushConstantVariableListCount = 0;
    pushConstant->ShaderStageFlags = 0;
    if (pushConstant->PushConstantVariableList)
    {
        memorySystem.RemovePtrBuffer<ShaderVariable>(pushConstant->PushConstantVariableList);
    }
    if (pushConstant->PushConstantBuffer)
    {
        memorySystem.RemovePtrBuffer(pushConstant->PushConstantBuffer);
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

VkPipelineShaderStageCreateInfo Shader_CreateShader(VkDevice device, const char* filename, VkShaderStageFlagBits shaderStages)
{
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (File_GetFileExtention(filename) == "hlsl")
    {
      //  shaderModule = Shader_BuildHLSLShader(device, filename.c_str(), shaderStages);
    }
    else
    {
        shaderModule = Shader_BuildGLSLShaderFile(device, filename);
    }
    return Shader_CreateShader(shaderModule, shaderStages);
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
    Span<ShaderVariable> pushConstantVarList(pushConstantStruct.PushConstantVariableList, pushConstantStruct.PushConstantVariableListCount);
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
    }

    outProtoTypeStructCount = shaderStructs.size();
    ShaderStruct* shaderStructPtr = memorySystem.AddPtrBuffer<ShaderStruct>(shaderStructs.data(), shaderStructs.size(), __FILE__, __LINE__, __func__);
    return shaderStructPtr;
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
    for (size_t i = 0; i < sourceVarList.size(); ++i)
    {
        ShaderVariable& destVar = destVarList[i];
        const ShaderVariable& srcVar = sourceVarList[i];

        destVar.Name = memorySystem.AddPtrBuffer(srcVar.Name ? srcVar.Name : "", __FILE__, __LINE__, __func__, "ShaderVariable.Name copy");
        destVar.Size = srcVar.Size;
        destVar.ByteAlignment = srcVar.ByteAlignment;
        destVar.MemberTypeEnum = srcVar.MemberTypeEnum;

        destVar.Value = memorySystem.AddPtrBuffer<byte>(destVar.Size, __FILE__, __LINE__, __func__, destVar.Name ? destVar.Name : "Unnamed ShaderVariable");
        Shader_SetVariableDefaults(destVar);
    }

    return shaderStruct;
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

VkShaderModule Shader_BuildGLSLShader(VkDevice device, const char* path, VkShaderStageFlagBits stage)
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

Microsoft::WRL::ComPtr<IDxcBlob> Shader_CompileHLSLShader(VkDevice device, const String& path, Microsoft::WRL::ComPtr<IDxcCompiler3>& dxc_compiler, Microsoft::WRL::ComPtr<IDxcIncludeHandler>& defaultIncludeHandler, VkShaderStageFlagBits stage)
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
    args.emplace_back(L"-fspv-target-env=vulkan1.3");
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
        if (!Shader_SearchDescriptorBindingExists(shaderDescriptorSetBinding.data(), shaderDescriptorSetBinding.size(), descriptorBinding->name))
        {
            String searchString(String("DescriptorBindingType" + std::to_string(descriptorBinding->binding)));
            Vector<SpvReflectSpecializationConstant*> DescriptorBindingAttributeTypeResult = Shader_SearchShaderSpecializationConstant(specializationConstantList, searchString.c_str());
            const char* copiedStr = memorySystem.AddPtrBuffer(descriptorBinding->name, __FILE__, __LINE__, __func__, "descriptorBinding->name copy");
            shaderDescriptorSetBinding.emplace_back(ShaderDescriptorBinding
                {
                    .Name = copiedStr,
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

        const char* copiedStr = memorySystem.AddPtrBuffer(variable.struct_member_name, __FILE__, __LINE__, __func__, "variable.struct_member_name copy");
        shaderVariables.emplace_back(ShaderVariable
            {
                .Name = copiedStr,
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
        .Name = shaderInfo.type_name,
        .ShaderBufferSize = bufferSize,
        .ShaderBufferVariableListCount = shaderVariables.size(),
        .ShaderBufferVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(shaderVariables.data(), shaderVariables.size(), __FILE__, __LINE__, __func__, ("Struct Name: " + (shaderInfo.type_name ? std::string(shaderInfo.type_name) : "")).c_str()),
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
        if (!Shader_SearchShaderConstBufferExists(shaderPushConstantList.data(), shaderPushConstantList.size(), pushConstant->name))
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

                const char* copiedStr = memorySystem.AddPtrBuffer(variable.struct_member_name, __FILE__, __LINE__, __func__, "variable.struct_member_name copy");
                shaderVariables.emplace_back(ShaderVariable
                    {
                        .Name = copiedStr,
                        .Size = memberSize,
                        .ByteAlignment = byteAlignment,
                        .Value = nullptr,
                        .MemberTypeEnum = memberType,
                    });
                size_t alignment = byteAlignment;
                bufferSize = (bufferSize + alignment - 1) & ~(alignment - 1);
                bufferSize += memberSize;
            }

            const char* copiedStr = memorySystem.AddPtrBuffer(pushConstant->name, __FILE__, __LINE__, __func__, "pushConstant->name copy");
            shaderPushConstantList.emplace_back(ShaderPushConstant
                {
                    .PushConstantName = copiedStr,
                    .PushConstantSize = bufferSize,
                    .PushConstantVariableListCount = shaderVariables.size(),
                    .ShaderStageFlags = static_cast<VkShaderStageFlags>(module.shader_stage),
                    .PushConstantVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(shaderVariables.data(), shaderVariables.size(), __FILE__, __LINE__, __func__),
                });
        }
        else
        {
            const char* copiedStr = memorySystem.AddPtrBuffer(pushConstant->name, __FILE__, __LINE__, __func__, "pushConstant->name copy");
            auto it = std::find_if(shaderPushConstantList.data(), shaderPushConstantList.data() + shaderPushConstantList.size(),
                [&](ShaderPushConstant& var) {
                    var.ShaderStageFlags |= static_cast<VkShaderStageFlags>(module.shader_stage);
                    return var.PushConstantName == copiedStr;
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

ShaderPushConstant* Shader_SearchShaderConstBuffer(ShaderPushConstant* shaderPushConstantList, size_t shaderPushConstantCount, const char* constBufferName)
{
    if (shaderPushConstantList == nullptr ||
        shaderPushConstantList->PushConstantName == nullptr ||
        constBufferName == nullptr)
    {
        return nullptr;
    }

    Span span = Span(shaderPushConstantList, shaderPushConstantCount);
    auto it = std::ranges::find_if(span, [&](const ShaderPushConstant& var)
        {
            return var.PushConstantName != nullptr && 
                   std::strcmp(var.PushConstantName, constBufferName) == 0;
        });

    return (it != span.end()) ? &(*it) : nullptr;
}

ShaderVariable* Shader_SearchShaderConstStructVar(ShaderPushConstant* pushConstant, const char* varName)
{
    if (pushConstant == nullptr)
    {
        return nullptr;
    }

    auto it = std::ranges::find_if(
        std::span(pushConstant->PushConstantVariableList, pushConstant->PushConstantVariableListCount),
        [&](const ShaderVariable& var)
        {
            return var.Name != nullptr &&
                std::strcmp(var.Name, varName) == 0;
        }
    );

    return (it != std::span(pushConstant->PushConstantVariableList, pushConstant->PushConstantVariableListCount).end()) ? &(*it) : nullptr;
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

bool Shader_SearchShaderConstBufferExists(ShaderPushConstant* shaderPushConstantList, size_t shaderPushConstantCount, const char* constBufferName)
{
    if (shaderPushConstantList == nullptr ||
        shaderPushConstantList->PushConstantName == nullptr ||
        constBufferName == nullptr)
    {
        return false;
    }

    Span span = Span(shaderPushConstantList, shaderPushConstantCount);
    auto it = std::ranges::find_if(span, [&](const ShaderPushConstant& var)
        {
            return var.PushConstantName != nullptr && std::strcmp(var.PushConstantName, constBufferName) == 0;
        });

    return (it != span.end()) ? true : false;
}

bool Shader_SearchDescriptorBindingExists(ShaderDescriptorBinding* shaderDescriptorBindingList, size_t shaderDescriptorBindingsCount, const char* descriptorBindingName)
{
    if (shaderDescriptorBindingList == nullptr ||
        shaderDescriptorBindingList->Name == nullptr ||
        descriptorBindingName == nullptr)
    {
        return false;
    }

    Span span = Span(shaderDescriptorBindingList, shaderDescriptorBindingsCount);
    auto it = std::ranges::find_if(span, [&](const ShaderDescriptorBinding& var)
        {
            return var.Name != nullptr && std::strcmp(var.Name, descriptorBindingName) == 0;
        });

    return (it != span.end()) ? true : false;
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