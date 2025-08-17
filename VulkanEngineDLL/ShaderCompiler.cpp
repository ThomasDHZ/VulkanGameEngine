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

   
    Vector<SpvReflectDescriptorSet> descriptorSets = Vector<SpvReflectDescriptorSet>(module.descriptor_sets, module.descriptor_sets + module.descriptor_set_count);
   // Vector<ShaderVertexVariable> inputVertexVariables = Shader_GetShaderInputVertexVariables(module);
    Vector<ShaderVertexVariable> outputVertexVariables = Shader_GetShaderOutputVertexVariables(module);
    Vector<ShaderDescriptorBinding> descriptorBindings = Shader_GetShaderDescriptorBindings(module);
    Vector<ShaderPushConstant> constBufferList = Shader_GetShaderConstBuffer(module);

    Vector<SpvReflectInterfaceVariable> interfaceVariables = Vector<SpvReflectInterfaceVariable>(module.interface_variables, module.interface_variables + module.interface_variable_count);
    Vector<SpvReflectBlockVariable> pushConstantBlocks = Vector<SpvReflectBlockVariable>(module.push_constant_blocks, module.push_constant_blocks + module.push_constant_block_count);
    Vector<SpvReflectSpecializationConstant> specConstants = Vector<SpvReflectSpecializationConstant>(module.spec_constants, module.spec_constants + module.spec_constant_count);

    uint32_t inputCount = 0;
    spvReflectEnumerateSpecializationConstants(&module, &inputCount, nullptr);
    std::vector<SpvReflectSpecializationConstant*> inputs(inputCount);
    spvReflectEnumerateSpecializationConstants(&module, &inputCount, inputs.data());

    // New code to extract and print default ivec2 values (assuming ivec2 type):
    const uint32_t* spirv_code = spvReflectGetCode(&module);
    uint32_t spirv_word_count = spvReflectGetCodeSize(&module) / sizeof(uint32_t);

    for (const auto* spec : inputs) {
        if (!spec) continue;

        // Skip type check or hardcode for ivec2 (vector of 2 int32)

        // Search for OpSpecConstantComposite matching spec->spirv_id.
        bool found = false;
        std::vector<uint32_t> constituent_ids(2);
        size_t pos = 0;
        while (pos < spirv_word_count) {
            uint32_t word0 = spirv_code[pos];
            uint32_t opcode = word0 & 0xFFFF;
            uint32_t word_count = word0 >> 16;
            if (word_count == 0 || pos + word_count > spirv_word_count) {
                break;  // Invalid module.
            }

            if (opcode == 44 && word_count == 5) {  // OpSpecConstantComposite for vec2: header(1) + type(1) + result(1) + 2 constituents.
                uint32_t result_id = spirv_code[pos + 2];
                if (result_id == spec->spirv_id) {
                    constituent_ids[0] = spirv_code[pos + 3];
                    constituent_ids[1] = spirv_code[pos + 4];
                    found = true;
                    break;
                }
            }
            pos += word_count;
        }

        if (!found) {
            std::cerr << "OpSpecConstantComposite not found for spirv_id " << spec->spirv_id << std::endl;
            continue;
        }

        // Now extract values for each constituent (expected to be OpConstant int32).
        int32_t values[2] = { 0, 0 };
        for (int i = 0; i < 2; ++i) {
            uint32_t const_id = constituent_ids[i];
            pos = 0;  // Reset search for each.
            bool const_found = false;
            while (pos < spirv_word_count) {
                uint32_t word0 = spirv_code[pos];
                uint32_t opcode = word0 & 0xFFFF;
                uint32_t word_count = word0 >> 16;
                if (word_count == 0 || pos + word_count > spirv_word_count) {
                    break;
                }

                if (opcode == 43 && word_count == 4) {  // OpSpecConstant for 32-bit scalar: header(1) + type(1) + result(1) + default_value(1). Note: opcode 43 for OpSpecConstant.
                    uint32_t result_id = spirv_code[pos + 2];
                    if (result_id == const_id) {
                        uint32_t raw_value = spirv_code[pos + 3];
                        values[i] = *reinterpret_cast<int32_t*>(&raw_value);  // Signed int32.
                        const_found = true;
                        break;
                    }
                }
                else if (opcode == 32 && word_count == 4) {  // Fallback to OpConstant if not specialized.
                    uint32_t result_id = spirv_code[pos + 2];
                    if (result_id == const_id) {
                        uint32_t raw_value = spirv_code[pos + 3];
                        values[i] = *reinterpret_cast<int32_t*>(&raw_value);  // Signed int32.
                        const_found = true;
                        break;
                    }
                }
                pos += word_count;
            }

            if (!const_found) {
                std::cerr << "Op(Spec)Constant not found for constituent_id " << const_id << std::endl;
                continue;
            }
        }

        // Output the vec2 value (replace with your storage/usage).
        std::cout << "Constant ID " << spec->constant_id << " (" << spec->name << "): ivec2("
            << values[0] << ", " << values[1] << ")" << std::endl;
    }
    return module;
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

    uint32 offset = 0;
    for (int x = 0; x < inputs.size(); x++)
    {
        if (inputs[x]->built_in != -1) {
            continue;
        }

     /*   if (inputs[x]->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
            std::cerr << "Skipping built-in variable at location " << var->location << std::endl;
            continue;
        }*/

        VkVertexInputRate inputRate = (inputs[x]->decoration_flags & SPV_REFLECT_DECORATION_PER_VERTEX)
            ? VK_VERTEX_INPUT_RATE_VERTEX
            : (inputs[x]->decoration_flags & SPV_REFLECT_DECORATION_PER_TASK)
            ? VK_VERTEX_INPUT_RATE_INSTANCE
            : VK_VERTEX_INPUT_RATE_VERTEX; // Default to vertex if unspecified

        switch (inputs[x]->format)
        {
            case SPV_REFLECT_FORMAT_R32_UINT: offset += 4; break;
            case SPV_REFLECT_FORMAT_R32G32_UINT: offset += 8; break;
            case SPV_REFLECT_FORMAT_R32G32B32_UINT: offset += 12; break;
            case SPV_REFLECT_FORMAT_R32G32B32A32_UINT: offset += 16;  break;
            case SPV_REFLECT_FORMAT_R32_SINT: offset += 4; break;
            case SPV_REFLECT_FORMAT_R32G32_SINT: offset += 8; break;
            case SPV_REFLECT_FORMAT_R32G32B32_SINT: offset += 12; break;
            case SPV_REFLECT_FORMAT_R32G32B32A32_SINT: offset += 16; break;
            case SPV_REFLECT_FORMAT_R32_SFLOAT: offset += 4; break;
            case SPV_REFLECT_FORMAT_R32G32_SFLOAT: offset += 8; break;
            case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT: offset += 12; break;
            case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: offset += 16; break;
        }

        vertexInputAttributeList.emplace_back(VkVertexInputAttributeDescription
            {
                .location = inputs[x]->location,
                .binding = 0,
                .format = static_cast<VkFormat>(inputs[x]->format),
                .offset = offset
            });
    }
    int a = 234;
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
