//#pragma once
//#include <ShaderSystem.h>
//
//typedef const char* const* StringListPtr;
//
//struct ShaderVariableDLL
//{
//    const char*                     Name;
//    size_t                          Size = 0;
//    size_t                          ByteAlignment = 0;
//    void*                           ValuePtr;
//    size_t                          ValueSize;
//    ShaderMemberType                MemberTypeEnum = shaderUnknown;
//};
//
//struct ShaderStructDLL
//{
//    const char*                     Name;
//    size_t			                ShaderBufferSize = 0;
//    ShaderVariableDLL*              ShaderBufferVariableListPtr;
//    size_t                          ShaderBufferVariableListSize;
//    int                             ShaderStructBufferId;
//    void*                           ShaderStructBufferPtr;
//    size_t                          ShaderStructBufferSize;
//};
//
//struct ShaderDescriptorSetDLL
//{
//    const char*                     Name;
//    uint32                          Binding;
//    VkDescriptorType                DescripterType;
//    ShaderStruct*                   ShaderStructListPtr;
//    size_t                          ShaderStructCount;
//};
//
//struct ShaderDescriptorBindingDLL
//{
//    const char* Name;
//    uint32                          DescriptorSet = UINT32_MAX;
//    uint32                          Binding = UINT32_MAX;
//    size_t                          DescriptorCount;
//    VkShaderStageFlags              ShaderStageFlags;
//    DescriptorBindingPropertiesEnum DescriptorBindingType;
//    VkDescriptorType                DescripterType;
//    VkDescriptorImageInfo*          DescriptorImageInfoPtr;
//    size_t                          DescriptorImageInfoCount;
//    VkDescriptorBufferInfo*         DescriptorBufferInfoPtr;
//    size_t                          DescriptorBufferInfoCount;
//};
//
//struct ShaderPushConstantDLL
//{
//    const char*                     PushConstantName;
//    size_t			                PushConstantSize = 0;
//    VkShaderStageFlags              ShaderStageFlags;
//    ShaderVariableDLL*              PushConstantVariableListPtr;
//    size_t                          PushConstantVariableListCount;
//    void*                           PushConstantBufferPtr;
//    size_t                          PushConstantBufferSize;
//    bool			                GlobalPushContsant = false;
//};
//
//struct ShaderPipelineDataDLL
//{
//    const char**                         ShaderListPtr;
//    ShaderDescriptorBinding*             DescriptorBindingsListPtr;
//    ShaderStruct*                        ShaderStructListPtr;
//    VkVertexInputBindingDescription*     VertexInputBindingListPtr;
//    VkVertexInputAttributeDescription*   VertexInputAttributeListPtr;
//    ShaderPushConstant*                  PushConstantListPtr;
//    size_t                               ShaderListCount;
//    size_t                               DescriptorBindingsListCount;
//    size_t                               ShaderStructListCount;
//    size_t                               VertexInputBindingListCount;
//    size_t                               VertexInputAttributeListCount;
//    size_t                               PushConstantListCount;
//};
//
//ShaderVariable ShaderSystemDLL_ShaderVariableDLLToCPP(ShaderVariableDLL& shaderVariable);
//ShaderVariableDLL ShaderSystemDLL_ShaderVariableCPPToDLL(ShaderVariable& shaderVariable);
//#ifdef __cplusplus
//extern "C" {
//#endif
//DLL_EXPORT VkPipelineShaderStageCreateInfo              ShaderSystemDLL_LoadShader(const char* filename, VkShaderStageFlagBits shaderStages);
//DLL_EXPORT ShaderPipelineDataDLL                        ShaderSystemDLL_LoadPipelineShaderData(StringListPtr pipelineShaderPathListPtrPtr, size_t listCount);
//DLL_EXPORT void                                         ShaderSystemDLL_LoadShaderPipelineStructPrototypes(StringListPtr shaderPathList);
//DLL_EXPORT SpvReflectSpecializationConstant**           ShaderSystemDLL_LoadShaderSpecializationConstants(const SpvReflectShaderModule& module, size_t& outCount);
//DLL_EXPORT bool                                         ShaderSystemDLL_CompileShaders(const char* fileDirectory, const char* outputDirectory);
//DLL_EXPORT void                                         ShaderSystemDLL_UpdatePushConstantBuffer(const char* pushConstantName);
//DLL_EXPORT void                                         ShaderSystemDLL_UpdatePushConstantBuffer(ShaderPushConstantDLL& pushConstantStruct);
//DLL_EXPORT void                                         ShaderSystemDLL_UpdateShaderBuffer(ShaderStructDLL& shaderStruct, uint vulkanBufferId);
//DLL_EXPORT ShaderStructDLL                              ShaderSystemDLL_CopyShaderStructProtoType(const char* structName);
//DLL_EXPORT ShaderPipelineDataDLL                        ShaderSystemDLL_FindShaderModule(const char* shaderFile);
//DLL_EXPORT ShaderPushConstantDLL&				        ShaderSystemDLL_FindShaderPushConstant(const char* shaderFile);
//DLL_EXPORT ShaderStructDLL                              ShaderSystemDLL_FindShaderProtoTypeStruct(const char* shaderKey);
//DLL_EXPORT ShaderStructDLL&					            ShaderSystemDLL_FindShaderStruct(int vulkanBufferId);
//DLL_EXPORT ShaderVariableDLL&					        ShaderSystemDLL_FindShaderPipelineStructVariable(ShaderStructDLL& shaderStruct, const char* variableName);
//DLL_EXPORT ShaderVariableDLL&					        ShaderSystemDLL_FindShaderPushConstantStructVariable(ShaderPushConstantDLL& shaderPushConstant, const char* variableName);
//DLL_EXPORT SpvReflectSpecializationConstant**           ShaderSystemDLL_FindShaderSpecializationConstant(const SpvReflectSpecializationConstant** specializationConstantListPtr, size_t specializationConstantCount, const char* searchString, size_t& outCount);
//DLL_EXPORT bool                                         ShaderSystemDLL_ShaderModuleExists(const char* shaderFile);
//DLL_EXPORT bool                                         ShaderSystemDLL_ShaderPushConstantExists(const char* pushConstantName);
//DLL_EXPORT bool                                         ShaderSystemDLL_ShaderStructPrototypeExists(const char* structKey);
//DLL_EXPORT bool                                         ShaderSystemDLL_ShaderPipelineStructExists(uint vulkanBufferKey);
//DLL_EXPORT bool                                         ShaderSystemDLL_SearchShaderConstantBufferExists(const ShaderPushConstantDLL* shaderPushConstantListPtr, size_t shaderPushConstantCount, const char* constBufferName);
//DLL_EXPORT bool                                         ShaderSystemDLL_SearchShaderDescriptorBindingExists(const ShaderDescriptorBindingDLL* shaderDescriptorBindingListPtr, size_t shaderDescriptorBindingCount, const char* descriptorBindingName);
//DLL_EXPORT bool                                         ShaderSystemDLL_SearchShaderPipelineStructExists(const ShaderStructDLL* shaderStructListPtr, size_t shaderStructCount, const char* structName);
//#ifdef __cplusplus
//}
//#endif
