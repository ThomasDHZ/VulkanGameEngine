#include "VulkanShader.h"
#include <regex>
#include "MemorySystem.h"


void Shader_StartUp()
{
    //DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(dxc_utils.ReleaseAndGetAddressOf()));
    //DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler));
    //dxc_utils->CreateDefaultIncludeHandler(&defaultIncludeHandler);
}
ShaderModule Shader_GetShaderData(const String& spvPath)
{ 
    SpvReflectShaderModule spvModule;
    Vector<VkVertexInputBindingDescription> vertexInputBindingList;
    Vector<VkVertexInputAttributeDescription> vertexInputAttributeList;
    FileState file = File_Read(spvPath.c_str());
    SpvReflectResult result = spvReflectCreateShaderModule(file.Size * sizeof(byte), file.Data, &spvModule);
    Vector<ShaderPushConstant> constBuffers = Shader_GetShaderConstBuffer(spvModule);
    Vector<ShaderStruct> shaderStructs = Shader_GetShaderDescriptorSetInfo(spvModule);
    Vector<ShaderVertexVariable> outputVariables = Shader_GetShaderOutputVertexVariables(spvModule);
    Vector<ShaderDescriptorBinding> descriptorBindings = Shader_GetShaderDescriptorBindings(spvModule);
    Shader_GetShaderInputVertexVariables(spvModule, vertexInputBindingList, vertexInputAttributeList);
   
    ShaderModule modulea = ShaderModule
    {
        .ShaderPath = spvPath,
        .ShaderStage = spvModule.shader_stage,
        .DescriptorBindingCount = descriptorBindings.size(),
        .ShaderStructCount = shaderStructs.size(),
        .VertexInputBindingCount = vertexInputBindingList.size(),
        .VertexInputAttributeListCount = vertexInputAttributeList.size(),
        .ShaderOutputCount = 0,
        .PushConstantCount = constBuffers.size(),
        .DescriptorBindingsList = memorySystem.AddPtrBuffer<ShaderDescriptorBinding>(descriptorBindings.data(), descriptorBindings.size(), __FILE__, __LINE__, __func__),
        .ShaderStructList = memorySystem.AddPtrBuffer<ShaderStruct>(shaderStructs.data(), shaderStructs.size(), __FILE__, __LINE__, __func__),
        .VertexInputBindingList = memorySystem.AddPtrBuffer<VkVertexInputBindingDescription>(vertexInputBindingList.data(), vertexInputBindingList.size(), __FILE__, __LINE__, __func__),
        .VertexInputAttributeList = memorySystem.AddPtrBuffer<VkVertexInputAttributeDescription>(vertexInputAttributeList.data(), vertexInputAttributeList.size(), __FILE__, __LINE__, __func__),
      //  .ShaderOutputList = memorySystem.AddPtrBuffer<ShaderVariable>(outputVariables.data(), outputVariables.size(), __FILE__, __LINE__, __func__),
        .PushConstantList = memorySystem.AddPtrBuffer<ShaderPushConstant>(constBuffers.data(), constBuffers.size(), __FILE__, __LINE__, __func__)
    };
    return modulea;
}

void Shader_ShaderDestroy(ShaderModule& shader)
{
    Shader_DestroyShaderStructData(shader);
    Shader_DestroyShaderBindingData(shader);
    memorySystem.RemovePtrBuffer<ShaderDescriptorBinding>(shader.DescriptorBindingsList);
    memorySystem.RemovePtrBuffer<ShaderStruct>(shader.ShaderStructList);
    memorySystem.RemovePtrBuffer<VkVertexInputBindingDescription>(shader.VertexInputBindingList);
    memorySystem.RemovePtrBuffer<VkVertexInputAttributeDescription>(shader.VertexInputAttributeList);
    memorySystem.RemovePtrBuffer<ShaderVariable>(shader.ShaderOutputList);
    memorySystem.RemovePtrBuffer<ShaderPushConstant>(shader.PushConstantList);
}

void Shader_DestroyShaderStructData(ShaderModule& shader)
{
    for (int x = 0; x < shader.ShaderStructCount; x++)
    {
        memorySystem.RemovePtrBuffer(shader.ShaderStructList[x].ShaderStructBuffer);
    }
}

void Shader_DestroyShaderBindingData(ShaderModule& shader)
{
    for (int x = 0; x < shader.DescriptorBindingCount; x++)
    {
        memorySystem.RemovePtrBuffer<VkDescriptorBufferInfo>(shader.DescriptorBindingsList[x].DescriptorBufferInfo);
        memorySystem.RemovePtrBuffer<VkDescriptorImageInfo>(shader.DescriptorBindingsList[x].DescriptorImageInfo);
    }
}

void Shader_DestroyConstantBufferVariableData(ShaderPushConstant* pushConstant, size_t pushConstantCount)
{
    for (int x = 0; x < pushConstantCount; x++)
    {
        memorySystem.RemovePtrBuffer<ShaderPushConstant>(pushConstant);
    }
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

    Vector<SpvReflectSpecializationConstant*> specializationConstantList = Shader_GetShaderSpecializationConstant(module);

    Vector<ShaderDescriptorBinding> descriptorBindingList;
    for (auto& descriptorBinding : descriptorSetBindings)
    {
        String searchString(String("DescriptorBindingType" + std::to_string(descriptorBinding->binding)));
        Vector<SpvReflectSpecializationConstant*> DescriptorBindingAttributeTypeResult = Shader_SearchShaderSpecializationConstant(specializationConstantList, searchString.c_str());
        
        descriptorBindingList.emplace_back(ShaderDescriptorBinding
            {
                .Name = descriptorBinding->name,
                .Binding = descriptorBinding->binding,
                .DescriptorBindingType = static_cast<DescriptorBindingPropertiesEnum>(*DescriptorBindingAttributeTypeResult[0]->default_literals),
                .DescripterType = static_cast<VkDescriptorType>(descriptorBinding->descriptor_type)
            });
    }
    return descriptorBindingList;
}

Vector<ShaderStruct> Shader_GetShaderDescriptorSetInfo(const SpvReflectShaderModule& module)
{
    uint descriptorSetCount = 0;
    SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, nullptr));
    Vector<SpvReflectDescriptorSet*> descriptorSets(descriptorSetCount);
    SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorSets(&module, &descriptorSetCount, descriptorSets.data()));

    Vector<ShaderStruct> ShaderStructList;
    for (auto& descriptorSet : descriptorSets)
    {
        Span<SpvReflectDescriptorBinding> descriptorBindingList(*descriptorSet->bindings, descriptorSet->binding_count);
        for (auto& descriptorBinding : descriptorBindingList)
        {
            SpvReflectTypeDescription bindingType = *descriptorBinding.type_description;
            Span<SpvReflectTypeDescription> structList(bindingType.struct_type_description, bindingType.member_count);
            for (auto& shaderInfo : structList)
            {
                if (shaderInfo.op == SpvOp::SpvOpTypeStruct)
                {
                    ShaderStructList.emplace_back(Shader_GetShaderStruct(shaderInfo));
                }
            }
        }
    }
    return ShaderStructList;
}

ShaderStruct Shader_GetShaderStruct(SpvReflectTypeDescription& shaderInfo)
{
    size_t bufferSize = 0;
    Vector<ShaderVariable> shaderVariables;
    Span<SpvReflectTypeDescription> structMembers(shaderInfo.members, shaderInfo.member_count);
    for (auto& members : structMembers)
    {
        Vector<SpvReflectTypeDescription> shaderVariableList = Vector<SpvReflectTypeDescription>(members.members, members.members + members.member_count);
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
                    .Name = variable.struct_member_name,
                    .Size = memberSize,
                    .ByteAlignment = byteAlignment,
                    .Value = nullptr,
                    .MemberTypeEnum = memberType,
                });
            size_t alignment = byteAlignment;
            bufferSize = (bufferSize + alignment - 1) & ~(alignment - 1);
            bufferSize += memberSize;
        }
    }

    return ShaderStruct
    {
        .Name = shaderInfo.type_name ? std::string(shaderInfo.type_name) : "",
        .ShaderBufferSize = bufferSize,
        .ShaderBufferVariableListCount = shaderVariables.size(),
        .ShaderBufferVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(shaderVariables.data(), shaderVariables.size(), __FILE__, __LINE__, __func__, ("Struct Name: " + (shaderInfo.type_name ? std::string(shaderInfo.type_name) : "")).c_str()),
        .ShaderStructBuffer =  memorySystem.AddPtrBuffer<byte>(bufferSize, __FILE__, __LINE__, __func__,  ("Struct Name: " + (shaderInfo.type_name ? std::string(shaderInfo.type_name) : "")).c_str())
    };
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
                    .Name = variable.struct_member_name,
                    .Size = memberSize,
                    .ByteAlignment = byteAlignment,
                    .Value = nullptr,
                    .MemberTypeEnum = memberType,
                });
            size_t alignment = byteAlignment;
            bufferSize = (bufferSize + alignment - 1) & ~(alignment - 1);
            bufferSize += memberSize;
        }

        pushConstantList.emplace_back(ShaderPushConstant
            {
                .PushConstantName = String(pushConstant->name),
                .PushConstantSize = bufferSize,
                .PushConstantVariableListCount = shaderVariables.size(),
                .PushConstantVariableList = memorySystem.AddPtrBuffer<ShaderVariable>(shaderVariables.data(), shaderVariables.size(), __FILE__, __LINE__, __func__),
            });
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

ShaderVariable* Shader_SearchShaderStructhVar(const ShaderStruct& shaderStruct, const String& varName)
{
    auto it = std::find_if(shaderStruct.ShaderBufferVariableList, shaderStruct.ShaderBufferVariableList + shaderStruct.ShaderBufferVariableListCount,
        [&](const ShaderVariable& var) {
            return var.Name == varName;
        }
    );

    if (it != shaderStruct.ShaderBufferVariableList + shaderStruct.ShaderBufferVariableListCount) {
        ShaderVariable& foundVar = *it;
    }

    return it;
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