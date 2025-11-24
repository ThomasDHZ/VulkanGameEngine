#include "ShaderSystem.h"
#include "Platform.h"
#include "MemorySystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"

ShaderSystem shaderSystem = ShaderSystem();

 ShaderSystem::ShaderSystem()
 {
 }

 ShaderSystem::~ShaderSystem()
 {
 }

 VkPipelineShaderStageCreateInfo ShaderSystem::LoadShader(const char* filename, VkShaderStageFlagBits shaderStages)
 {
     FileState file = File_Read(filename);
     VkShaderModuleCreateInfo shaderModuleCreateInfo =
     {
         .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
         .codeSize = file.Size,
         .pCode = (const uint32*)file.Data
     };

     VkShaderModule shaderModule = VK_NULL_HANDLE;
     VULKAN_RESULT(vkCreateShaderModule(renderer.Device, &shaderModuleCreateInfo, NULL, &shaderModule));
     free(file.Data);
     file.Data = nullptr;

     return VkPipelineShaderStageCreateInfo
     {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = shaderStages,
         .module = shaderModule,
         .pName = "main"
     };
 }

 ShaderPipelineDataDLL ShaderSystem::LoadPipelineShaderData(const Vector<String>& pipelineShaderPathList)
 {
     SpvReflectShaderModule spvModule;
     Vector<VkVertexInputBindingDescription> vertexInputBindingList;
     Vector<VkVertexInputAttributeDescription> vertexInputAttributeList;
     Vector<ShaderPushConstantDLL> constBuffers;
     Vector<ShaderStruct> shaderStructs;
     Vector<ShaderDescriptorBindingDLL> descriptorBindings;

     for (auto& pipelineShaderPath : pipelineShaderPathList)
     {
         FileState file = File_Read(pipelineShaderPath.c_str());
         SPV_VULKAN_RESULT(spvReflectCreateShaderModule(file.Size * sizeof(byte), file.Data, &spvModule));
         LoadShaderConstantBufferData(spvModule, constBuffers);
         LoadShaderDescriptorBindings(spvModule, descriptorBindings);
         if (spvModule.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
         {
             LoadShaderVertexInputVariables(spvModule, vertexInputBindingList, vertexInputAttributeList);
         }
         spvReflectDestroyShaderModule(&spvModule);
         free(file.Data);
         file.Data = nullptr;
     }

     return ShaderPipelineDataDLL
     {
          .ShaderList = pipelineShaderPathList,
          .DescriptorBindingsList = descriptorBindings,
          .VertexInputBindingList = vertexInputBindingList,
          .VertexInputAttributeList = vertexInputAttributeList,
          .PushConstantList = constBuffers
     };
 }

 void ShaderSystem::LoadShaderPipelineStructPrototypes(const Vector<String>& shaderPathList)
 {
     for (size_t x = 0; x < shaderPathList.size(); ++x)
     {
         nlohmann::json renderPassJson = File_LoadJsonFile(shaderPathList[x].c_str());
         for (size_t y = 0; y < renderPassJson["RenderPipelineList"].size(); ++y)
         {
             nlohmann::json pipelineJson = File_LoadJsonFile(renderPassJson["RenderPipelineList"][y].get<String>().c_str());
             Vector<String> shaderJsonList = Vector<String>{ pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] };
             Vector<ShaderStructDLL> shaderStructList = LoadProtoTypeStructs(shaderJsonList);
             for (auto& shaderStruct : shaderStructList)
             {
                 if (!ShaderStructPrototypeExists(shaderStruct.Name))
                 {
                     String name = shaderStruct.Name;
                     shaderSystem.PipelineShaderStructPrototypeMap[name] = ShaderStructDLL
                     {
                         .Name = name,
                         .ShaderBufferSize = shaderStruct.ShaderBufferSize,
                         .ShaderBufferVariableList = shaderStruct.ShaderBufferVariableList,
                         .ShaderStructBufferId = shaderStruct.ShaderStructBufferId,
                         .ShaderStructBuffer = memorySystem.AddPtrBuffer<byte>(shaderStruct.ShaderBufferSize, __FILE__, __LINE__, __func__, name.c_str())
                     };
                 }
             }
         }
     }
 }

 void ShaderSystem::LoadShaderVertexInputVariables(const SpvReflectShaderModule& module, Vector<VkVertexInputBindingDescription>& vertexInputBindingList, Vector<VkVertexInputAttributeDescription>& vertexInputAttributeList)
 {
     uint32 offset = 0;
     Vector<SpvReflectSpecializationConstant*> specializationConstantList = LoadShaderSpecializationConstants(module);
     Vector<SpvReflectSpecializationConstant*> vertexInputRateLocationConstantResult = FindShaderSpecializationConstant(specializationConstantList, "VertexInputRateLocation");
     Vector<SpvReflectSpecializationConstant*> vertexAttributeLocationpecializationConstantResult = FindShaderSpecializationConstant(specializationConstantList, "VertexAttributeLocation");

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

 Vector<SpvReflectInterfaceVariable*> ShaderSystem::LoadShaderVertexOutputVariables(const SpvReflectShaderModule& module)
 {
     uint32 outputCount = 0;
     SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&module, &outputCount, nullptr));
     Vector<SpvReflectInterfaceVariable*> outputs(outputCount);
     SPV_VULKAN_RESULT(spvReflectEnumerateOutputVariables(&module, &outputCount, outputs.data()));
     return outputs;
 }

 void ShaderSystem::LoadShaderConstantBufferData(const SpvReflectShaderModule& module, Vector<ShaderPushConstantDLL>& shaderPushConstantList)
 {
     uint32 pushConstCount = 0;
     SPV_VULKAN_RESULT(spvReflectEnumeratePushConstantBlocks(&module, &pushConstCount, nullptr));
     Vector<SpvReflectBlockVariable*> pushConstants(pushConstCount);
     SPV_VULKAN_RESULT(spvReflectEnumeratePushConstantBlocks(&module, &pushConstCount, pushConstants.data()));

     for (auto pushConstant : pushConstants)
     {
         String pushConstantName(pushConstant->name);
         if (!SearchShaderConstantBufferExists(shaderPushConstantList, pushConstantName))
         {
             size_t bufferSize = 0;
             Vector<ShaderVariableDLL> shaderStructVariableList = LoadShaderStructVariables(*pushConstant->type_description, bufferSize);
             shaderSystem.ShaderPushConstantMap[pushConstantName] = ShaderPushConstantDLL
             {
                .PushConstantName = pushConstantName,
                .PushConstantSize = bufferSize,
                .ShaderStageFlags = static_cast<VkShaderStageFlags>(module.shader_stage),
                .PushConstantVariableList = shaderStructVariableList,
                .PushConstantBuffer = memorySystem.AddPtrBuffer<byte>(bufferSize, __FILE__, __LINE__, __func__, pushConstantName.c_str())
             };
             for (auto& shaderVariable : shaderSystem.ShaderPushConstantMap[pushConstantName].PushConstantVariableList)
             {
                 shaderVariable.Value = memorySystem.AddPtrBuffer<byte>(shaderVariable.Size, __FILE__, __LINE__, __func__, shaderVariable.Name.c_str());
                 SetVariableDefaults(shaderVariable);
             }
             shaderPushConstantList.emplace_back(shaderSystem.ShaderPushConstantMap[pushConstantName]);
         }
         else
         {
             auto it = std::find_if(shaderPushConstantList.data(), shaderPushConstantList.data() + shaderPushConstantList.size(),
                 [&](ShaderPushConstantDLL& var) {
                     var.ShaderStageFlags |= static_cast<VkShaderStageFlags>(module.shader_stage);
                     return var.PushConstantName == pushConstant->name;
                 }
             );
         }
     }
 }

 void ShaderSystem::LoadShaderDescriptorBindings(const SpvReflectShaderModule& module, Vector<ShaderDescriptorBindingDLL>& shaderDescriptorSetBinding)
 {
     uint32_t descriptorBindingsCount = 0;
     SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorBindings(&module, &descriptorBindingsCount, nullptr));
     Vector<SpvReflectDescriptorBinding*> descriptorSetBindings(descriptorBindingsCount);
     SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorBindings(&module, &descriptorBindingsCount, descriptorSetBindings.data()));
     Vector<SpvReflectSpecializationConstant*> specializationConstantList = LoadShaderSpecializationConstants(module);

     for (auto& descriptorBinding : descriptorSetBindings)
     {
         String name(descriptorBinding->name);
         if (!SearchShaderDescriptorBindingExists(shaderDescriptorSetBinding, name))
         {
             String searchString(String("DescriptorBindingType" + std::to_string(descriptorBinding->binding)));
             Vector<SpvReflectSpecializationConstant*> DescriptorBindingAttributeTypeResult = FindShaderSpecializationConstant(specializationConstantList, searchString.c_str());
             shaderDescriptorSetBinding.emplace_back(ShaderDescriptorBindingDLL
                 {
                     .Name = name,
                     .Binding = descriptorBinding->binding,
                     .ShaderStageFlags = static_cast<VkShaderStageFlags>(module.shader_stage),
                     .DescriptorBindingType = static_cast<DescriptorBindingPropertiesEnum>(*DescriptorBindingAttributeTypeResult[0]->default_literals),
                     .DescripterType = static_cast<VkDescriptorType>(descriptorBinding->descriptor_type)
                 });
         }
         else
         {
             auto it = std::find_if(shaderDescriptorSetBinding.data(), shaderDescriptorSetBinding.data() + shaderDescriptorSetBinding.size(),
                 [&](ShaderDescriptorBindingDLL& var) {
                     var.ShaderStageFlags |= static_cast<VkShaderStageFlags>(module.shader_stage);
                     return var.Name == descriptorBinding->name;
                 }
             );
         }
     }
 }

 void ShaderSystem::LoadShaderDescriptorSets(const SpvReflectShaderModule& module, Vector<ShaderStructDLL>& shaderStructList)
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
                     !SearchShaderPipelineStructExists(shaderStructList, shaderInfo.type_name))
                 {
                     shaderStructList.emplace_back(LoadShaderPipelineStruct(shaderInfo));
                 }
             }
         }
     }
 }

 Vector<SpvReflectSpecializationConstant*> ShaderSystem::LoadShaderSpecializationConstants(const SpvReflectShaderModule& module)
 {
     uint32 specializationConstantCount = 0;
     spvReflectEnumerateSpecializationConstants(&module, &specializationConstantCount, nullptr);
     std::vector<SpvReflectSpecializationConstant*> specializationConstantList(specializationConstantCount);
     spvReflectEnumerateSpecializationConstants(&module, &specializationConstantCount, specializationConstantList.data());
     return specializationConstantList;
 }

 ShaderStructDLL ShaderSystem::LoadShaderPipelineStruct(const SpvReflectTypeDescription& shaderInfo)
 {
     size_t bufferSize = 0;
     Vector<ShaderVariableDLL> structVariableList = LoadShaderStructVariables(shaderInfo, bufferSize);
     ShaderStructDLL shaderStruct =
     {
         .Name = String(shaderInfo.type_name),
         .ShaderBufferSize = bufferSize,
         .ShaderBufferVariableList = structVariableList,
         .ShaderStructBuffer = nullptr
     };
     return shaderStruct;
 }

 Vector<ShaderVariableDLL> ShaderSystem::LoadShaderStructVariables(const SpvReflectTypeDescription& shaderInfo, size_t& returnBufferSize)
 {
     Vector<ShaderVariableDLL> shaderVariables;
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

         shaderVariables.emplace_back(ShaderVariableDLL
             {
                 .Name = String(variable.struct_member_name),
                 .Size = memberSize,
                 .ByteAlignment = byteAlignment,
                 .Value = nullptr,
                 .MemberTypeEnum = memberType,
             });
         size_t alignment = byteAlignment;
         returnBufferSize = (returnBufferSize + alignment - 1) & ~(alignment - 1);
         returnBufferSize += memberSize;
     }
     return shaderVariables;
 }

 Vector<ShaderStructDLL> ShaderSystem::LoadProtoTypeStructs(const Vector<String>& pipelineShaderList)
 {
     Vector<ShaderStructDLL> shaderStructs;
     SpvReflectShaderModule spvModule{};

     for (const auto& filePath : pipelineShaderList)
     {
         // We are already chdir'ed into Assets/ ? this is correct
         std::cout << "=== SHADER LOAD DEBUG ===" << std::endl;
         std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
         std::cout << "Trying to open: [" << filePath << "]" << std::endl;
         std::cout << "Loading shader: " << filePath << std::endl;

         std::ifstream file(filePath, std::ios::binary | std::ios::ate);
         if (!file) {
             std::cerr << "ERROR: Cannot open shader file: " << filePath << std::endl;
             continue;
         }

         size_t size = file.tellg();
         file.seekg(0);

         std::vector<std::byte> buffer(size);
         file.read(reinterpret_cast<char*>(buffer.data()), size);

         if (!file) {
             std::cerr << "ERROR: Failed to read full shader file: " << filePath << std::endl;
             continue;
         }

         SpvReflectResult result = spvReflectCreateShaderModule(
             size, buffer.data(), &spvModule);

         if (result != SPV_REFLECT_RESULT_SUCCESS) {
             std::cerr << "SPIRV-Reflect failed for " << filePath << std::endl;
             continue;
         }

         LoadShaderDescriptorSetInfo(spvModule, shaderStructs);
         spvReflectDestroyShaderModule(&spvModule);
     }

     return shaderStructs;
 }

 void ShaderSystem::LoadShaderDescriptorSetInfo(const SpvReflectShaderModule& module, Vector<ShaderStructDLL>& shaderStructList)
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
                     !SearchShaderPipelineStructExists(shaderStructList, shaderInfo.type_name))
                 {
                     shaderStructList.emplace_back(LoadShaderPipelineStruct(shaderInfo));
                 }
             }
         }
     }
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
         std::filesystem::path src = srcPath;
         std::filesystem::path dst = outDir / (src.filename().stem().string() + fileSystem.GetFileExtention(srcPath.c_str()) + ".spv");
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

 void ShaderSystem::UpdatePushConstantBuffer(ShaderPushConstantDLL& pushConstantStruct)
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
  
 void ShaderSystem::UpdateGlobalShaderBuffer(const String& pushConstantName)
 {
     if (!ShaderPushConstantExists(pushConstantName))
     {
         std::cerr << "Error: Push constant '" << pushConstantName << "' does not exist!" << std::endl;
         return;
     }
     UpdatePushConstantBuffer(ShaderPushConstantMap[pushConstantName]);
 }

 void ShaderSystem::UpdateShaderBuffer(uint vulkanBufferId)
 {
     if (!ShaderPipelineStructExists(vulkanBufferId))
     {
         return;
     }

     ShaderStructDLL shaderStruct = PipelineShaderStructMap[vulkanBufferId];
     VulkanBuffer vulkanBuffer = bufferSystem.FindVulkanBuffer(vulkanBufferId);
     size_t offset = 0;
     for (const auto& shaderStrucVar : shaderStruct.ShaderBufferVariableList)
     {
         offset = (offset + shaderStrucVar.ByteAlignment - 1) & ~(shaderStrucVar.ByteAlignment - 1);
         void* dest = static_cast<byte*>(shaderStruct.ShaderStructBuffer) + offset;
         memcpy(dest, shaderStrucVar.Value, shaderStrucVar.Size);
         offset += shaderStrucVar.Size;
     }
     bufferSystem.UpdateBufferMemory(vulkanBuffer, shaderStruct.ShaderStructBuffer, shaderStruct.ShaderBufferSize, 1);
 }

 ShaderStructDLL ShaderSystem::CopyShaderStructProtoType(const String& structName)
 {
     ShaderStructDLL shaderStructCopy = FindShaderProtoTypeStruct(structName);
     ShaderStructDLL shaderStruct = ShaderStructDLL
     {
         .Name = shaderStructCopy.Name,
         .ShaderBufferSize = shaderStructCopy.ShaderBufferSize,
         .ShaderBufferVariableList = shaderStructCopy.ShaderBufferVariableList,
         .ShaderStructBufferId = shaderStructCopy.ShaderStructBufferId,
         .ShaderStructBuffer = memorySystem.AddPtrBuffer<byte>(shaderStructCopy.ShaderBufferSize, __FILE__, __LINE__, __func__, shaderStructCopy.Name.c_str()),
     };
     for (size_t x = 0; x < shaderStructCopy.ShaderBufferVariableList.size(); ++x)
     {
         ShaderVariableDLL& destVar = shaderStruct.ShaderBufferVariableList[x];
         const ShaderVariableDLL& srcVar = shaderStructCopy.ShaderBufferVariableList[x];

         destVar.Name = srcVar.Name;
         destVar.Size = srcVar.Size;
         destVar.ByteAlignment = srcVar.ByteAlignment;
         destVar.MemberTypeEnum = srcVar.MemberTypeEnum;
         destVar.Value = memorySystem.AddPtrBuffer<byte>(destVar.Size, __FILE__, __LINE__, __func__, destVar.Name.c_str());
         SetVariableDefaults(destVar);
     }
     return shaderStruct;
 }

 Vector<SpvReflectSpecializationConstant*> ShaderSystem::FindShaderSpecializationConstant(const Vector<SpvReflectSpecializationConstant*>& specializationConstantList, const String& searchString)
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

 ShaderPipelineDataDLL ShaderSystem::FindShaderModule(const String& shaderFile)
 {
     return shaderSystem.ShaderModuleMap.at(shaderFile);
 }

 ShaderPushConstantDLL ShaderSystem::FindShaderPushConstant(const String& shaderFile)
 {
     return shaderSystem.ShaderPushConstantMap.at(shaderFile);
 }

 ShaderStructDLL ShaderSystem::FindShaderProtoTypeStruct(const String& shaderKey)
 {
     return shaderSystem.PipelineShaderStructPrototypeMap.at(shaderKey);
 }

 ShaderStructDLL ShaderSystem::FindShaderStruct(int vulkanBufferId)
 {
     return shaderSystem.PipelineShaderStructMap.at(vulkanBufferId);
 }

 ShaderVariableDLL ShaderSystem::FindShaderPipelineStructVariable(ShaderStructDLL& shaderStruct, const String& variableName)
 {
     auto it = std::ranges::find_if(shaderStruct.ShaderBufferVariableList, [&](const ShaderVariableDLL& var)
         {
             return var.Name == variableName;
         });
     if (it == shaderStruct.ShaderBufferVariableList.end())
     {
         throw std::runtime_error("Shader variable not found: " + variableName);
     }

     return *it;
 }

 ShaderVariableDLL ShaderSystem::FindShaderPushConstantStructVariable(ShaderPushConstantDLL& shaderPushConstant, const String& variableName)
 {
     auto it = std::ranges::find_if(shaderPushConstant.PushConstantVariableList, [&](const ShaderVariableDLL& var)
         {
             return var.Name == variableName; 
         });
     if (it == shaderPushConstant.PushConstantVariableList.end())
     {
         throw std::runtime_error("Shader variable not found: " + variableName);
     }
     return *it;
 }

 bool ShaderSystem::SearchShaderConstantBufferExists(const Vector<ShaderPushConstantDLL>& shaderPushConstantList, const String& constBufferName)
 {
     auto it = std::ranges::find_if(shaderPushConstantList, [&](const ShaderPushConstantDLL& var)
         {
             return var.PushConstantName == constBufferName;
         });
     return it != shaderPushConstantList.end();
 }

 bool ShaderSystem::SearchShaderDescriptorBindingExists(const Vector<ShaderDescriptorBindingDLL>& shaderDescriptorBindingList, const String& descriptorBindingName)
 {
     auto it = std::ranges::find_if(shaderDescriptorBindingList, [&](const ShaderDescriptorBindingDLL& var)
         {
             return var.Name == descriptorBindingName;
         });
     return it != shaderDescriptorBindingList.end();
 }

 bool ShaderSystem::SearchShaderPipelineStructExists(const Vector<ShaderStructDLL>& shaderStructList, const String& structName)
 {
     auto it = std::ranges::find_if(shaderStructList, [&](const ShaderStructDLL& var)
         {
             return var.Name == structName;
         });
     return it != shaderStructList.end();
 }

 bool ShaderSystem::ShaderPushConstantExists(const String& pushConstantName)
 {
     return shaderSystem.ShaderPushConstantMap.contains(pushConstantName);
 }

 bool ShaderSystem::ShaderModuleExists(const String& shaderFile)
 {
     return shaderSystem.ShaderModuleMap.contains(shaderFile);
 }

 bool ShaderSystem::ShaderStructPrototypeExists(const String& structKey)
 {
     return shaderSystem.PipelineShaderStructPrototypeMap.contains(structKey);
 }

 bool ShaderSystem::ShaderPipelineStructExists(uint vulkanBufferKey)
 {
     return shaderSystem.PipelineShaderStructMap.contains(vulkanBufferKey);
 }

 void ShaderSystem::DestroyShaderStructData(Vector<ShaderStructDLL>& shaderStructList)
 {
     for (auto& shaderStruct : shaderStructList)
     {
         if (!shaderStruct.ShaderBufferVariableList.empty())
         {
             for (auto& shaderVar : shaderStruct.ShaderBufferVariableList)
             {
                 if (shaderVar.Value != nullptr)
                 {
                     byte* value = static_cast<byte*>(shaderVar.Value);
                     memorySystem.DeletePtr(value);
                 }
             }
         }
         if (shaderStruct.ShaderStructBuffer != nullptr)
         {
             byte* value = static_cast<byte*>(shaderStruct.ShaderStructBuffer);
             memorySystem.DeletePtr(value);
         }
     }
 }

 void ShaderSystem::DestroyPushConstantBufferData(Vector<ShaderPushConstantDLL>& pushConstantList)
 {
     for (auto& shaderStruct : pushConstantList)
     {
         if (!shaderStruct.PushConstantVariableList.empty())
         {
             for (auto& shaderVar : shaderStruct.PushConstantVariableList)
             {
                 if (shaderVar.Value != nullptr)
                 {
                     byte* value = static_cast<byte*>(shaderVar.Value);
                     memorySystem.DeletePtr(value);
                 }
             }
         }
         if (shaderStruct.PushConstantBuffer != nullptr)
         {
             byte* value = static_cast<byte*>(shaderStruct.PushConstantBuffer);
             memorySystem.DeletePtr(value);
         }
     }
 }

 void ShaderSystem::Destroy()
 {
     Vector<ShaderPushConstantDLL> shaderPushConstantList;
     shaderPushConstantList.reserve(shaderSystem.ShaderPushConstantMap.size());
     for (const auto& pair : shaderSystem.ShaderPushConstantMap) {
         shaderPushConstantList.push_back(pair.second);
     }
     DestroyPushConstantBufferData(shaderPushConstantList);

     Vector<ShaderStructDLL> shaderStructProtoList;
     shaderStructProtoList.reserve(shaderSystem.PipelineShaderStructPrototypeMap.size());
     for (const auto& pair : shaderSystem.PipelineShaderStructPrototypeMap) {
         shaderStructProtoList.push_back(pair.second);
     }
     DestroyShaderStructData(shaderStructProtoList);

     Vector<ShaderStructDLL> shaderStructList;
     shaderStructList.reserve(shaderSystem.PipelineShaderStructMap.size());
     for (const auto& pair : shaderSystem.PipelineShaderStructMap) {
         shaderStructList.push_back(pair.second);
     }
     DestroyShaderStructData(shaderStructList);
     shaderSystem.ShaderModuleMap.clear();
 }

 void ShaderSystem::SetVariableDefaults(ShaderVariableDLL& shaderVariable)
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