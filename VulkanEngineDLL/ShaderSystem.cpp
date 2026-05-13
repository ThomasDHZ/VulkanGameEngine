#include "ShaderSystem.h"
#include "Platform.h"
#include "MemorySystem.h"
#include "FileSystem.h"
#include "BufferSystem.h"
#include <cctype>
#include <ranges>

ShaderSystem& shaderSystem = ShaderSystem::Get();

 VkPipelineShaderStageCreateInfo ShaderSystem::LoadShader(const char* filename, VkShaderStageFlagBits shaderStages)
 {
     Vector<byte> file = fileSystem.LoadAssetFile(filename);
     VkShaderModuleCreateInfo shaderModuleCreateInfo =
     {
         .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
         .codeSize = file.size(),
         .pCode = (const uint32*)file.data()
     };

     VkShaderModule shaderModule = VK_NULL_HANDLE;
     VULKAN_THROW_IF_FAIL(vkCreateShaderModule(vulkanSystem.Device, &shaderModuleCreateInfo, NULL, &shaderModule));

     return VkPipelineShaderStageCreateInfo
     {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = shaderStages,
         .module = shaderModule,
         .pName = "main"
     };
 }

 ShaderPipelineData ShaderSystem::LoadPipelineShaderData(const Vector<String>& pipelineShaderPathList)
 {
     SpvReflectShaderModule spvModule;
     Vector<VkVertexInputBindingDescription> vertexInputBindingList;
     Vector<VkVertexInputAttributeDescription> vertexInputAttributeList;
     Vector<ShaderPushConstant> constBuffers;
     Vector<ShaderStruct> shaderStructs;
     Vector<ShaderDescriptorBinding> descriptorBindings;

     for (auto& pipelineShaderPath : pipelineShaderPathList)
     {
         Vector<byte> file = fileSystem.LoadAssetFile(pipelineShaderPath.c_str());
         SPV_VULKAN_RESULT(spvReflectCreateShaderModule(file.size(), file.data(), &spvModule));
         LoadShaderConstantBufferData(spvModule, constBuffers);
         LoadShaderDescriptorBindings(spvModule, descriptorBindings);
         if (spvModule.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
         {
             LoadShaderVertexInputVariables(spvModule, vertexInputBindingList, vertexInputAttributeList);
         }
         spvReflectDestroyShaderModule(&spvModule);
     }

     return ShaderPipelineData
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
         nlohmann::json renderPassJson = fileSystem.LoadJsonFile(shaderPathList[x].c_str());
         for (size_t y = 0; y < renderPassJson["RenderPipelineList"].size(); ++y)
         {
             nlohmann::json pipelineJson = fileSystem.LoadJsonFile(renderPassJson["RenderPipelineList"][y].get<String>().c_str());
             Vector<String> shaderJsonList = Vector<String>{ pipelineJson["ShaderList"][0], pipelineJson["ShaderList"][1] };
             Vector<ShaderStruct> shaderStructList = LoadProtoTypeStructs(shaderJsonList);
             for (auto& shaderStruct : shaderStructList)
             {
                 if (!ShaderStructPrototypeExists(shaderStruct.Name))
                 {
                     String name = shaderStruct.Name;
                     shaderSystem.PipelineShaderStructPrototypeMap[name] = ShaderStruct
                     {
                         .Name = name,
                         .ShaderBufferSize = shaderStruct.ShaderBufferSize,
                         .ShaderBufferVariableList = shaderStruct.ShaderBufferVariableList,
                         .ShaderStructBufferId = shaderStruct.ShaderStructBufferId,
                         .ShaderStructBuffer = Vector<byte>(shaderStruct.ShaderBufferSize, 0x00)
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
                 binding = *static_cast<DescriptorBindingPropertiesEnum*>(vertexAttributeLocationpecializationConstantResult[0]->default_value);
             }
         }

         if (vertexAttributeLocationpecializationConstantResult.size())
         {
             String vertexInputRateLocationString(vertexInputRateLocationConstantResult[x]->name);
             if (vertexInputRateLocationString.find("VertexInputRateLocation" + std::to_string(inputs[x]->location)) != String::npos)
             {
                 inputRate = *static_cast<DescriptorBindingPropertiesEnum*>(vertexInputRateLocationConstantResult[0]->default_value);
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

 void ShaderSystem::LoadShaderConstantBufferData(const SpvReflectShaderModule& module, Vector<ShaderPushConstant>& shaderPushConstantList)
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
             Vector<ShaderVariable> shaderStructVariableList = LoadShaderStructVariables(*pushConstant->type_description, bufferSize);
             shaderSystem.ShaderPushConstantMap[pushConstantName] = ShaderPushConstant
             {
                .PushConstantName = pushConstantName,
                .PushConstantSize = bufferSize,
                .ShaderStageFlags = static_cast<VkShaderStageFlags>(module.shader_stage),
                .PushConstantVariableList = shaderStructVariableList,
                .PushConstantBuffer = Vector<byte>(bufferSize, 0x00)
             };
             for (auto& shaderVariable : shaderSystem.ShaderPushConstantMap[pushConstantName].PushConstantVariableList)
             {
                 shaderVariable.Value = Vector<byte>(shaderVariable.Size, 0x00);
             }
             shaderPushConstantList.emplace_back(shaderSystem.ShaderPushConstantMap[pushConstantName]);
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

 void ShaderSystem::LoadShaderDescriptorBindings(const SpvReflectShaderModule& module, Vector<ShaderDescriptorBinding>& shaderDescriptorSetBinding)
 {
     uint32_t descriptorBindingsCount = 0;
     SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorBindings(&module, &descriptorBindingsCount, nullptr));
     Vector<SpvReflectDescriptorBinding*> descriptorSetBindings(descriptorBindingsCount);
     SPV_VULKAN_RESULT(spvReflectEnumerateDescriptorBindings(&module, &descriptorBindingsCount, descriptorSetBindings.data()));
     Vector<SpvReflectSpecializationConstant*> specializationConstantList = LoadShaderSpecializationConstants(module);

     for (auto& descriptorBinding : descriptorSetBindings)
     {
         String name(descriptorBinding->name);
         auto it = std::ranges::find(shaderDescriptorSetBinding, name, &ShaderDescriptorBinding::Name);
         if(it == shaderDescriptorSetBinding.end())
         {
             shaderDescriptorSetBinding.emplace_back(ShaderDescriptorBinding
                 {
                     .Name = name,
                     .DescriptorSet = descriptorBinding->set,
                     .Binding = descriptorBinding->binding,
                     .ShaderStageFlags = static_cast<VkShaderStageFlags>(module.shader_stage),
                     .DescriptorBindingType = kSubpassInputDescriptor,
                     .DescripterType = static_cast<VkDescriptorType>(descriptorBinding->descriptor_type)
                 });
         }
         else
         {
             it->ShaderStageFlags |= static_cast<VkShaderStageFlags>(module.shader_stage);
         }
     }
     std::sort(shaderDescriptorSetBinding.begin(), shaderDescriptorSetBinding.end(), [](const ShaderDescriptorBinding& a, const ShaderDescriptorBinding& b) 
         {
             return a.Binding < b.Binding;
         });
 }

 void ShaderSystem::LoadShaderDescriptorSets(const SpvReflectShaderModule& module, Vector<ShaderStruct>& shaderStructList)
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

 ShaderStruct ShaderSystem::LoadShaderPipelineStruct(const SpvReflectTypeDescription& shaderInfo)
 {
     size_t bufferSize = 0;
     Vector<ShaderVariable> structVariableList = LoadShaderStructVariables(shaderInfo, bufferSize);
     ShaderStruct shaderStruct =
     {
         .Name = String(shaderInfo.type_name),
         .ShaderBufferSize = bufferSize,
         .ShaderBufferVariableList = structVariableList,
         .ShaderStructBuffer = Vector<byte>()
     };
     return shaderStruct;
 }

 Vector<ShaderVariable> ShaderSystem::LoadShaderStructVariables(const SpvReflectTypeDescription& shaderInfo, size_t& returnBufferSize)
 {
     Vector<ShaderVariable> shaderVariables;
     Vector<SpvReflectTypeDescription> shaderVariableList = Vector<SpvReflectTypeDescription>(shaderInfo.members, shaderInfo.members + shaderInfo.member_count);
     for (auto& variable : shaderVariableList)
     {
         uint memberSize = 0;
         size_t byteAlignment = 0;
         size_t arraySize = variable.traits.array.dims[0];
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
                 uint32 rowCount = variable.traits.numeric.matrix.row_count;
                 uint32 colCount = variable.traits.numeric.matrix.column_count;
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
             case SpvOpTypeArray:
             {
                 uint32 rowCount = variable.traits.numeric.matrix.row_count;
                 uint32 colCount = variable.traits.numeric.matrix.column_count;
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
                 .Size = arraySize == 0 ? memberSize : memberSize * arraySize,
                 .ByteAlignment = byteAlignment,
                 .Value = Vector<byte>(),
                 .MemberTypeEnum = memberType,
             });
         size_t alignment = byteAlignment;
         returnBufferSize = (returnBufferSize + alignment - 1) & ~(alignment - 1);
         returnBufferSize += memberSize;
     }
     return shaderVariables;
 }

 Vector<ShaderStruct> ShaderSystem::LoadProtoTypeStructs(const Vector<String>& pipelineShaderList)
 {
     Vector<ShaderStruct> shaderStructs;
     SpvReflectShaderModule spvModule{};

     for (const auto& filePath : pipelineShaderList)
     {
         Vector<byte> buffer = fileSystem.LoadAssetFile(filePath);
         if (spvReflectCreateShaderModule(buffer.size(), buffer.data(), &spvModule) != SPV_REFLECT_RESULT_SUCCESS)
         {
             std::cerr << "SPIRV-Reflect failed for " << filePath << std::endl;
             continue;
         }

         LoadShaderDescriptorSetInfo(spvModule, shaderStructs);
         spvReflectDestroyShaderModule(&spvModule);
     }

     return shaderStructs;
 }

 void ShaderSystem::LoadShaderDescriptorSetInfo(const SpvReflectShaderModule& module, Vector<ShaderStruct>& shaderStructList)
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
         offset = (offset + pushConstantVar.ByteAlignment - 1) & ~(pushConstantVar.ByteAlignment - 1);
         void* dest = static_cast<byte*>(pushConstantStruct.PushConstantBuffer.data()) + offset;
         memcpy(dest, pushConstantVar.Value.data(), pushConstantVar.Size);
         offset += pushConstantVar.Size;
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

 void ShaderSystem::UpdateShaderBuffer(ShaderStruct& shaderStruct, uint vulkanBufferId)
 {
     if (!ShaderPipelineStructExists(vulkanBufferId))
     {
         return;
     }

     size_t offset = 0;
     VulkanBuffer vulkanBuffer = bufferSystem.FindVulkanBuffer(vulkanBufferId);
     for (const auto& shaderStrucVar : shaderStruct.ShaderBufferVariableList)
     {
         offset = (offset + shaderStrucVar.ByteAlignment - 1) & ~(shaderStrucVar.ByteAlignment - 1);
         void* dest = static_cast<byte*>(shaderStruct.ShaderStructBuffer.data()) + offset;
         memcpy(dest, shaderStrucVar.Value.data(), shaderStrucVar.Size);
         offset += shaderStrucVar.Size;
     }
     bufferSystem.UpdateDynamicBuffer(vulkanBuffer.BufferId, shaderStruct.ShaderStructBuffer.data(), shaderStruct.ShaderBufferSize);
 }

 void ShaderSystem::UpdateShaderVariableValue(ShaderVariable& shaderVariable, const String& newShaderVariableValue)
 {
     if (newShaderVariableValue.empty()) return;

     const char* first = newShaderVariableValue.data();
     const char* last = first + newShaderVariableValue.size();
     auto parse_and_copy = [&](auto& value)
         {
             auto res = std::from_chars(first, last, value);
             if (res.ec == std::errc{}) memcpy(shaderVariable.Value.data(), &value, sizeof(value));
             else std::cerr << "Failed to parse: " << shaderVariable.Name << std::endl;
         };

     switch (shaderVariable.MemberTypeEnum)
     {
         case shaderInt:
         {
             int   data;
             parse_and_copy(data);
             break;
         }
         case shaderUint: 
         {
             uint  data; 
             parse_and_copy(data); 
             break;
         }
         case shaderFloat: 
         {
             float data; 
             parse_and_copy(data); 
             break;
         }
         case shaderIvec2: ParseVector<2, int>(shaderVariable, newShaderVariableValue); break;
         case shaderIvec3: ParseVector<3, int>(shaderVariable, newShaderVariableValue); break;
         case shaderIvec4: ParseVector<4, int>(shaderVariable, newShaderVariableValue); break;
         case shaderVec2:  ParseVector<2, float>(shaderVariable, newShaderVariableValue); break;
         case shaderVec3:  ParseVector<3, float>(shaderVariable, newShaderVariableValue); break;
         case shaderVec4:  ParseVector<4, float>(shaderVariable, newShaderVariableValue); break;
         case shaderMat2:  ParseMatrix<2, 2, float>(shaderVariable, newShaderVariableValue); break;
         case shaderMat3:  ParseMatrix<3, 3, float>(shaderVariable, newShaderVariableValue); break;
         case shaderMat4:  ParseMatrix<4, 4, float>(shaderVariable, newShaderVariableValue); break;
         case shaderbool: 
         {
             bool data = false;
             if (newShaderVariableValue == "true" || newShaderVariableValue == "1") data = true;
             else if (newShaderVariableValue == "false" || newShaderVariableValue == "0") data = false;
             else std::cerr << "Failed to parse bool: " << newShaderVariableValue << "\n";

             memcpy(shaderVariable.Value.data(), &data, sizeof(bool));
             break;
         }
     }
 }

 ShaderStruct ShaderSystem::CopyShaderStructProtoType(const String& structName)
 {
     ShaderStruct shaderStructCopy = FindShaderProtoTypeStruct(structName);
     ShaderStruct shaderStruct = ShaderStruct
     {
         .Name = shaderStructCopy.Name,
         .ShaderBufferSize = shaderStructCopy.ShaderBufferSize,
         .ShaderBufferVariableList = shaderStructCopy.ShaderBufferVariableList,
         .ShaderStructBufferId = shaderStructCopy.ShaderStructBufferId,
         .ShaderStructBuffer = Vector<byte>(shaderStructCopy.ShaderBufferSize, 0x00)
     };
     for (size_t x = 0; x < shaderStructCopy.ShaderBufferVariableList.size(); ++x)
     {
         ShaderVariable& destVar = shaderStruct.ShaderBufferVariableList[x];
         const ShaderVariable& srcVar = shaderStructCopy.ShaderBufferVariableList[x];

         destVar.Name = srcVar.Name;
         destVar.Size = srcVar.Size;
         destVar.ByteAlignment = srcVar.ByteAlignment;
         destVar.MemberTypeEnum = srcVar.MemberTypeEnum;
         destVar.Value = Vector<byte>(destVar.Size, 0x00);
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

 ShaderPipelineData ShaderSystem::FindShaderModule(const String& shaderFile)
 {
     return shaderSystem.ShaderModuleMap.at(shaderFile);
 }

 ShaderPushConstant& ShaderSystem::FindShaderPushConstant(const String& pushConstantName)
 {
     return shaderSystem.ShaderPushConstantMap.at(pushConstantName);
 }

 ShaderStruct ShaderSystem::FindShaderProtoTypeStruct(const String& shaderKey)
 {
     return shaderSystem.PipelineShaderStructPrototypeMap.at(shaderKey);
 }

 ShaderStruct& ShaderSystem::FindShaderStruct(int vulkanBufferId)
 {
     return shaderSystem.PipelineShaderStructMap.at(vulkanBufferId);
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