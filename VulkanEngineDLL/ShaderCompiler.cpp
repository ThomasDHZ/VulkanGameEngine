#include "ShaderCompiler.h"

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

void Shader_VertexDataFromSpirv(const Vector<byte>& spirvCode)
{
    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(spirvCode.size() * sizeof(byte), spirvCode.data(), &module);
    Vector<SpvReflectDescriptorBinding> descriptorBindings = Vector<SpvReflectDescriptorBinding>(module.descriptor_bindings, module.descriptor_bindings + module.descriptor_binding_count);
    Vector<SpvReflectDescriptorSet> descriptorSets = Vector<SpvReflectDescriptorSet>(module.descriptor_sets, module.descriptor_sets + module.descriptor_set_count);
    //Vector<SpvReflectInterfaceVariable> inputVariables = Vector<SpvReflectInterfaceVariable>(module.input_variables, module.input_variables + module.input_variable_count);
 //   Vector<SpvReflectInterfaceVariable> outputVariables = Vector<SpvReflectInterfaceVariable>(module.output_variables, module.output_variables + module.output_variable_count);
    for (uint32_t i = 0; i < module.input_variable_count; ++i) {
        SpvReflectInterfaceVariable* var = module.input_variables[i];

        // Access variable properties, for example:
        printf("Input Variable %u:\n", i);
        printf("  Name: %s\n", var->name);
        printf("  Type: %u\n", var->type_description->type_flags); // example
        // Add other properties as needed
    }
    //Vector<SpvReflectInterfaceVariable> interfaceVariables = Vector<SpvReflectInterfaceVariable>(module.interface_variables, module.interface_variables + module.interface_variable_count);
    //Vector<SpvReflectBlockVariable> pushConstantBlocks = Vector<SpvReflectBlockVariable>(module.push_constant_blocks, module.push_constant_blocks + module.push_constant_block_count);
    //Vector<SpvReflectSpecializationConstant> specConstants = Vector<SpvReflectSpecializationConstant>(module.spec_constants, module.spec_constants + module.spec_constant_count);
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
