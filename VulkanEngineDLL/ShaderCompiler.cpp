#include "ShaderCompiler.h"
#include "MemorySystem.h"


void Shader_StartUp()
{
    //DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(dxc_utils.ReleaseAndGetAddressOf()));
    //DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler));
    //dxc_utils->CreateDefaultIncludeHandler(&defaultIncludeHandler);
}

VkPipelineShaderStageCreateInfo Shader_CreateShader(VkDevice device, const String& filename, VkShaderStageFlagBits shaderStages)
{
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (File_GetFileExtention(filename.c_str()) == "hlsl")
    {
      //  shaderModule = Shader_BuildHLSLShader(device, filename.c_str(), shaderStages);
    }
    else
    {
        shaderModule = Shader_BuildGLSLShaderFile(device, filename.c_str());
    }
    return Shader_CreateShader(shaderModule, shaderStages);
}

SpvReflectShaderModule Shader_ShaderDataFromSpirv(const String& path)
{
    SpvReflectShaderModule module;
    FileState file = File_Read(path.c_str());
    SpvReflectResult result = spvReflectCreateShaderModule(file.Size * sizeof(byte), file.Data, &module);

    return module;
   
   // Vector<SpvReflectDescriptorSet> descriptorSets = Vector<SpvReflectDescriptorSet>(module.descriptor_sets, module.descriptor_sets + module.descriptor_set_count);
   /* Vector<ShaderVertexVariable> inputVertexVariables = Shader_GetShaderInputVertexVariables(module);
    Vector<ShaderVertexVariable> outputVertexVariables = Shader_GetShaderOutputVertexVariables(module);
    Vector<ShaderDescriptorBinding> descriptorBindings = Shader_GetShaderDescriptorBindings(module);
    Vector<ShaderPushConstant> constBufferList = Shader_GetShaderConstBuffer(module);

    Vector<SpvReflectInterfaceVariable> interfaceVariables = Vector<SpvReflectInterfaceVariable>(module.interface_variables, module.interface_variables + module.interface_variable_count);
    Vector<SpvReflectBlockVariable> pushConstantBlocks = Vector<SpvReflectBlockVariable>(module.push_constant_blocks, module.push_constant_blocks + module.push_constant_block_count);
    Vector<SpvReflectSpecializationConstant> specConstants = Vector<SpvReflectSpecializationConstant>(module.spec_constants, module.spec_constants + module.spec_constant_count);*/
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
    uint32_t inputCount = 0;
    vertexInputBindingList.clear();
    vertexInputAttributeList.clear();
    SPV_VULKAN_RESULT(spvReflectEnumerateInputVariables(&module, &inputCount, nullptr));
    Vector<SpvReflectInterfaceVariable*> inputs(inputCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateInputVariables(&module, &inputCount, inputs.data()));

    Vector<SpvReflectInterfaceVariable> interfaceVariables = Vector<SpvReflectInterfaceVariable>(module.interface_variables, module.interface_variables + module.interface_variable_count);

    for (int x = 0; x < inputs.size(); x++)
    {
  /*      vertexInputBindingList.emplace_back(VkVertexInputBindingDescription
        {
                .Name = inputs[x]->name,
                .Location = inputs[x]->location,
                .Format = static_cast<VkFormat>(inputs[x]->format)
        });*/

        vertexInputAttributeList.emplace_back(VkVertexInputAttributeDescription
            {
                .location = inputs[x]->location,
                //.binding = inputs[x]->bin,
                .format = static_cast<VkFormat>(inputs[x]->format),

            });
    }
}

Vector<ShaderVertexVariable> Shader_GetShaderOutputVertexVariables(const SpvReflectShaderModule& module)
{
    uint32_t outputCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&module, &outputCount, nullptr));
    Vector<SpvReflectInterfaceVariable*> outputs(outputCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&module, &outputCount, outputs.data()));
   
    Vector<ShaderVertexVariable> vertexAttributeList;
    for (int x = 0; x < outputs.size(); x++)
    {
        vertexAttributeList.emplace_back(ShaderVertexVariable
            {
                    .Name = outputs[x]->name,
                    .Location = outputs[x]->location,
                    .Format = static_cast<VkFormat>(outputs[x]->format)
            });
    }
    return vertexAttributeList;
}

Vector<ShaderDescriptorBinding> Shader_GetShaderDescriptorBindings(const SpvReflectShaderModule& module)
{
    uint32_t descriptorBindingsCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorBindings(&module, &descriptorBindingsCount, nullptr));
    Vector<SpvReflectDescriptorBinding*> descriptorSetBindings(descriptorBindingsCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorBindings(&module, &descriptorBindingsCount, descriptorSetBindings.data()));

    Vector<ShaderDescriptorBinding> ShaderDescriptorBindingList;
    for (auto& shaderBuffer : descriptorSetBindings)
    {
        Vector<ShaderStruct> shaderStructList;
        SpvReflectTypeDescription shaderBufferMembers = *shaderBuffer->type_description;
        Vector<SpvReflectTypeDescription> shaderBufferMembersList = Vector<SpvReflectTypeDescription>(shaderBufferMembers.members, shaderBufferMembers.members + shaderBufferMembers.member_count);
        if (shaderBufferMembersList.size())
        {
            for (auto& members : shaderBufferMembersList)
            {
                Vector<ShaderVariable> shaderVariables;
                Vector<SpvReflectTypeDescription> shaderVariableList = Vector<SpvReflectTypeDescription>(members.members, members.members + members.member_count);
                for (auto& variable : shaderVariableList)
                {
                    ShaderMemberType memberType;
                    uint memberSize = 0;
                    switch (variable.op)
                    {
                        case SpvOpTypeInt:
                        {
                            memberSize = variable.traits.numeric.scalar.width / 8;
                            memberType = variable.traits.numeric.scalar.signedness ? shaderUint : shaderInt;
                            break;
                        }
                        case SpvOpTypeFloat:
                        {
                            memberSize = variable.traits.numeric.scalar.width / 8;
                            memberType = shaderFloat;
                            break;
                        }
                        case SpvOpTypeVector:
                        {
                            memberSize = (variable.traits.numeric.scalar.width / 8) * variable.traits.numeric.vector.component_count;
                            switch (variable.traits.numeric.vector.component_count)
                            {
                                case 2: memberType = shaderVec2; break;
                                case 3: memberType = shaderVec3; break;
                                case 4: memberType = shaderVec4; break;
                            }
                            break;
                        }
                        case SpvOpTypeMatrix:
                        {
                            uint32_t rowCount = variable.traits.numeric.matrix.row_count;
                            uint32_t colCount = variable.traits.numeric.matrix.column_count;
                            if (rowCount == 2 && colCount == 2)
                            {
                                memberSize = (variable.traits.numeric.scalar.width / 8) * rowCount * colCount;
                                memberType = shaderMat2;
                            }
                            if (rowCount == 3 && colCount == 3)
                            {
                                memberSize = (variable.traits.numeric.scalar.width / 8) * rowCount * colCount;
                                memberType = shaderMat3;
                            }
                            if (rowCount == 4 && colCount == 4)
                            {
                                memberSize = (variable.traits.numeric.scalar.width / 8) * rowCount * colCount;
                                memberType = shaderMat4;
                            }
                            break;
                        }
                    }
                   
                    shaderVariables.emplace_back(ShaderVariable
                        {
                            .Name = variable.struct_member_name,
                            .VarSize = memberSize,
                            .Value = nullptr,
                            .MemberTypeEnum = memberType,
                        });
                }

                ShaderStruct shaderStruct;
                shaderStruct.Name = members.type_name;
                shaderStruct.ShaderBufferMemberName = members.struct_member_name;
                shaderStruct.ShaderStructOp = members.op;
                shaderStruct.ShaderBufferVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(shaderVariables.size(), __FILE__, __LINE__, __func__);
                shaderStruct.ShaderBufferVariableListCount = members.member_count;
                std::memcpy(shaderStruct.ShaderBufferVariableList, shaderVariables.data(), shaderVariables.size() * sizeof(ShaderVariable));
                shaderStructList.emplace_back(shaderStruct);
            }
        }
        ShaderDescriptorBinding shaderDescriptorBinding;
        shaderDescriptorBinding.Name = shaderBuffer->name;
        shaderDescriptorBinding.Binding = shaderBuffer->binding;
        shaderDescriptorBinding.DescripterType = static_cast<VkDescriptorType>(shaderBuffer->descriptor_type);
        if (shaderBuffer->descriptor_type != SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
        {
            shaderDescriptorBinding.ShaderStructListCount = shaderBuffer->count;
            shaderDescriptorBinding.ShaderStructList = memorySystem.AddPtrBuffer<ShaderStruct>(shaderStructList.size(), __FILE__, __LINE__, __func__);
            std::memcpy(shaderDescriptorBinding.ShaderStructList, shaderStructList.data(), shaderStructList.size() * sizeof(ShaderStruct));
        }
        ShaderDescriptorBindingList.emplace_back(shaderDescriptorBinding);
    }
    return ShaderDescriptorBindingList;
}

Vector<ShaderPushConstant> Shader_GetShaderConstBuffer(const SpvReflectShaderModule& module)
{
    uint32_t pushConstCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumeratePushConstantBlocks(&module, &pushConstCount, nullptr));
    Vector<SpvReflectBlockVariable*> pushConstants(pushConstCount);
    SPV_VULKAN_RESULT(spvReflectEnumeratePushConstantBlocks(&module, &pushConstCount, pushConstants.data()));

    Vector<ShaderPushConstant> pushConstantList;
    for (auto pushConstant : pushConstants)
    {
        Vector<ShaderVariable> shaderVariables;
        SpvReflectTypeDescription shaderBufferMembers = *pushConstant->type_description;
        Vector<SpvReflectTypeDescription> shaderVariableList = Vector<SpvReflectTypeDescription>(shaderBufferMembers.members, shaderBufferMembers.members + shaderBufferMembers.member_count);
        for (auto& variable : shaderVariableList)
        {
            shaderVariables.emplace_back(ShaderVariable
                {
                    .Name = variable.struct_member_name,
      /*              .ShaderVarOp = variable.op,
                    .VectorCount = variable.traits.numeric.vector.component_count > 0 ? 1 : variable.traits.numeric.vector.component_count,
                    .ColumnCount = variable.traits.numeric.matrix.column_count,
                    .RowCount = variable.traits.numeric.matrix.row_count,*/
         /*           .MatrixStride = variable.traits.numeric.matrix.stride,
                    .IsSigned = static_cast<bool>(variable.traits.numeric.scalar.signedness)*/
                });
        }

        ShaderPushConstant shaderStruct;
        shaderStruct.Name = pushConstant->name;
        shaderStruct.BufferSize = static_cast<size_t>(pushConstant->size);
        shaderStruct.ShaderBufferVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(shaderVariables.size(), __FILE__, __LINE__, __func__);
        shaderStruct.ShaderBufferVariableListCount = pushConstant->member_count;
        std::memcpy(shaderStruct.ShaderBufferVariableList, shaderVariables.data(), shaderVariables.size() * sizeof(ShaderVariable));
        pushConstantList.emplace_back(shaderStruct);
    }

    return pushConstantList;
}
