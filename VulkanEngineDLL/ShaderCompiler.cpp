#include "ShaderCompiler.h"
#include <regex>
#include "MemorySystem.h"


void Shader_StartUp()
{
    //DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(dxc_utils.ReleaseAndGetAddressOf()));
    //DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler));
    //dxc_utils->CreateDefaultIncludeHandler(&defaultIncludeHandler);
}
SpvReflectShaderModule Shader_GetShaderData(const String& spvPath)
{
    SpvReflectShaderModule module;
    FileState file = File_Read(spvPath.c_str());
    SpvReflectResult result = spvReflectCreateShaderModule(file.Size * sizeof(byte), file.Data, &module);

    return module;
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

//SpvReflectShaderModule Shader_ShaderDataFromSpirv(const String& path)
//{
//    SpvReflectShaderModule module;
//    FileState file = File_Read(path.c_str());
//    SpvReflectResult result = spvReflectCreateShaderModule(file.Size * sizeof(byte), file.Data, &module);
//
//   auto asdf =  Shader_GetShaderConstBuffer(module);
//    return module;
//   
//        Vector<SpvReflectTypeDescription> interfaceVariables3 = Vector<SpvReflectTypeDescription>(module._internal->type_descriptions, module._internal->type_descriptions + module._internal->type_description_count);
//    Vector<ShaderPushConstant> constBufferList = Shader_GetShaderConstBuffer(module);
//
//    Vector<SpvReflectDescriptorSet> descriptorSets = Vector<SpvReflectDescriptorSet>(module.descriptor_sets, module.descriptor_sets + module.descriptor_set_count);
//  //  Vector<ShaderVertexVariable> inputVertexVariables = Shader_GetShaderInputVertexVariables(module);
//    Vector<ShaderVertexVariable> outputVertexVariables = Shader_GetShaderOutputVertexVariables(module);
//    Vector<ShaderDescriptorBinding> descriptorBindings = Shader_GetShaderDescriptorBindings(module);
//
//    Vector<SpvReflectInterfaceVariable> interfaceVariables = Vector<SpvReflectInterfaceVariable>(module.interface_variables, module.interface_variables + module.interface_variable_count);
//    Vector<SpvReflectBlockVariable> pushConstantBlocks = Vector<SpvReflectBlockVariable>(module.push_constant_blocks, module.push_constant_blocks + module.push_constant_block_count);
//    Vector<SpvReflectSpecializationConstant> specConstants = Vector<SpvReflectSpecializationConstant>(module.spec_constants, module.spec_constants + module.spec_constant_count);
//
//}
        

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

        ShaderPushConstant shaderStruct;
        shaderStruct.PushConstantName = pushConstant->name;
        shaderStruct.PushConstantSize = static_cast<size_t>(pushConstant->size);
        shaderStruct.PushConstantVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(shaderVariables.size(), __FILE__, __LINE__, __func__);
        shaderStruct.PushConstantVariableListCount = pushConstant->member_count;
        std::memcpy(shaderStruct.PushConstantVariableList, shaderVariables.data(), shaderVariables.size() * sizeof(ShaderVariable));
        pushConstantList.emplace_back(shaderStruct);
    }

    return pushConstantList;
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

void printSpecializationConstants(SpvReflectShaderModule& module) {
    // Step 1: Get the count of specialization constants
    uint32_t count = 0;
    SpvReflectResult result = spvReflectEnumerateSpecializationConstants(&module, &count, nullptr);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        std::cerr << "Failed to get specialization constant count: " << result << std::endl;
        return;
    }

    // Step 2: Allocate vector for constant pointers
    std::vector<SpvReflectSpecializationConstant*> constants(count);

    // Step 3: Enumerate specialization constants
    result = spvReflectEnumerateSpecializationConstants(&module, &count, constants.data());
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        std::cerr << "Failed to enumerate specialization constants: " << result << std::endl;
        return;
    }

    // Step 4: Process each constant
    for (uint32_t i = 0; i < count; ++i) {
        const SpvReflectSpecializationConstant* constant = constants[i];
        std::cout << "Constant #" << i << ":\n";
        std::cout << "  Name: " << (constant->name ? constant->name : "Unnamed") << "\n";
        std::cout << "  SPIR-V ID: " << constant->spirv_id << "\n";
        std::cout << "  Constant ID: " << constant->constant_id << "\n";

        // Step 5: Get type details
        const SpvReflectTypeDescription* type = constant->type;
        if (!type) {
            std::cout << "  No type description available\n";
            continue;
        }

       auto afs = getSpecializationConstantOp(module, constant->spirv_id);
        std::cout << "  Type Opcode: " << afs << " (e.g., OpTypeInt, OpTypeFloat, etc.)\n";
        std::cout << "  Type: ";

        SpvReflectTypeFlags type_flags = type->type_flags;
        if (type_flags & SPV_REFLECT_TYPE_FLAG_BOOL) {
            std::cout << "Bool";
        }
        else if (type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
            uint32_t width = type->traits.numeric.scalar.width;
            std::cout << "Float (" << width << "-bit)";
        }
        else if (type_flags & SPV_REFLECT_TYPE_FLAG_INT) {
            uint32_t width = type->traits.numeric.scalar.width;
            uint32_t signedness = type->traits.numeric.scalar.signedness;
            std::cout << (signedness ? "Signed Int" : "Unsigned Int") << " (" << width << "-bit)";
        }
        else {
            std::cout << "Unsupported type (flags: " << type_flags << ")";
            continue;
        }

        // Handle vector components if applicable
        if (type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR) {
            std::cout << " Vector (" << type->traits.numeric.vector.component_count << " components)";
        }

        std::cout << "\n";

        // Step 6: Print default value if present
        if (constant->has_default_value) {
            std::cout << "  Default Value (as " << constant->default_literal_count << " literals): ";
            for (uint32_t lit = 0; lit < constant->default_literal_count; ++lit) {
                std::cout << "0x" << std::hex << constant->default_literals[lit] << " ";
            }
            std::cout << "\n";

            // Interpret based on type (example for common cases)
            if (type_flags & SPV_REFLECT_TYPE_FLAG_BOOL) {
                bool value = (constant->default_literals[0] != 0);
                std::cout << "    Interpreted: " << (value ? "true" : "false") << "\n";
            }
            else if (type_flags & SPV_REFLECT_TYPE_FLAG_INT) {
                uint32_t width = type->traits.numeric.scalar.width;
                uint32_t signedness = type->traits.numeric.scalar.signedness;
                if (width == 32) {
                    if (signedness) {
                        int32_t value = static_cast<int32_t>(constant->default_literals[0]);
                        std::cout << "    Interpreted: " << value << "\n";
                    }
                    else {
                        uint32_t value = constant->default_literals[0];
                        std::cout << "    Interpreted: " << value << "\n";
                    }
                }
                else if (width == 64) {
                    // For 64-bit, assume 2 literals (low/high word)
                    if (constant->default_literal_count >= 2) {
                        uint64_t value = (static_cast<uint64_t>(constant->default_literals[1]) << 32) |
                            constant->default_literals[0];
                        if (signedness) {
                            std::cout << "    Interpreted: " << static_cast<int64_t>(value) << "\n";
                        }
                        else {
                            std::cout << "    Interpreted: " << value << "\n";
                        }
                    }
                }
            }
            else if (type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
                uint32_t width = type->traits.numeric.scalar.width;
                if (width == 32) {
                    float value = *reinterpret_cast<const float*>(&constant->default_literals[0]);
                    std::cout << "    Interpreted: " << value << "\n";
                }
                else if (width == 64) {
                    // For 64-bit float, assume 2 literals
                    if (constant->default_literal_count >= 2) {
                        uint64_t raw = (static_cast<uint64_t>(constant->default_literals[1]) << 32) |
                            constant->default_literals[0];
                        double value = *reinterpret_cast<const double*>(&raw);
                        std::cout << "    Interpreted: " << value << "\n";
                    }
                }
            }
            // Add handling for vectors/composites as needed
        }
        else {
            std::cout << "  No default value\n";
        }
    }
}

SpvOp getSpecializationConstantOp(const SpvReflectShaderModule& module, uint32_t spirv_id) {
    const uint32_t* code = spvReflectGetCode(&module);
    uint32_t size = spvReflectGetCodeSize(&module) / sizeof(uint32_t);
    uint32_t index = 5;  // Skip header (magic, version, generator, bound, schema)

    while (index < size) {
        uint32_t word = code[index];
        SpvOp op = static_cast<SpvOp>(word & 0xFFFF);
        uint32_t word_count = (word >> 16) & 0xFFFF;

        if (word_count == 0 || index + word_count > size) {
            return SpvOpNop;  // Invalid
        }

        // Check if this instruction defines the spirv_id
        bool has_result_id = (op == SpvOpSpecConstantTrue || op == SpvOpSpecConstantFalse ||
            op == SpvOpSpecConstant || op == SpvOpSpecConstantComposite ||
            op == SpvOpSpecConstantOp);
        if (has_result_id && word_count > 2 && code[index + 2] == spirv_id) {
            return op;
        }

        index += word_count;
    }
    return SpvOpNop;  // Not found
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