#include "ShaderSystemDLL.h"

VulkanShaderDLL ShaderSystem_LoadShader(VkGuid& shaderGuid, const char* shaderFile)
{
	VulkanShader vulkanShader = shaderSystem.LoadShader(shaderGuid, shaderFile);

	ShaderPushConstant pushConstant = vulkanShader.PushConstant();

	Vector<ShaderVariableDLL> pushConstantVariableListDLL;
	for (auto& pushConstantVariable : pushConstant.PushConstantVariableList)
	{
		size_t size = std::visit([](auto&& arg) -> size_t 
			{
				return sizeof(std::decay_t<decltype(arg)>);
			}, pushConstantVariable.Value);

		void* buffer = memorySystem.AddPtrBuffer<void*>(size, __FILE__, __LINE__, __func__);
		std::visit([&](auto&& arg) 
			{
				memcpy(buffer, &arg, sizeof(std::decay_t<decltype(arg)>));
			}, pushConstantVariable.Value);

		pushConstantVariableListDLL.emplace_back(ShaderVariableDLL
			{
				.Name = memorySystem.AddStringPtrBuffer(pushConstantVariable.Name, __FILE__, __LINE__, __func__),
				.MemberTypeEnum = static_cast<ShaderMemberTypeEnum>(pushConstantVariable.Value.index()),
				.Variable = buffer
			});
	}

	ShaderPushConstantDLL pushConstantDLL =
	{
		.PushConstantName = memorySystem.AddStringPtrBuffer(pushConstant.PushConstantName, __FILE__, __LINE__, __func__),
		.PushConstantSize = pushConstant.PushConstantSize,
		.ShaderStageFlags = pushConstant.ShaderStageFlags,
		.PushConstantVariableList = memorySystem.AddPtrBuffer<ShaderVariableDLL>(pushConstantVariableListDLL.data(), pushConstantVariableListDLL.size(), __FILE__, __LINE__, __func__),
		.PushConstantVariableCount = pushConstantVariableListDLL.size()
	};

	Vector<ShaderDescriptorBindingDLL> shaderDescriptorBindingDLLList;
	for (auto& descriptorBinding : vulkanShader.DescriptorBindingList())
	{
		shaderDescriptorBindingDLLList.emplace_back(ShaderDescriptorBindingDLL
			{
				.Name = memorySystem.AddStringPtrBuffer(descriptorBinding.Name, __FILE__, __LINE__, __func__),
				.DescriptorSet = descriptorBinding.DescriptorSet,
				.Binding = descriptorBinding.Binding,
				.ShaderStageFlags = descriptorBinding.ShaderStageFlags,
				.DescriptorBindingType = descriptorBinding.DescriptorBindingType,
				.DescripterType = descriptorBinding.DescripterType
			});
	}

	return VulkanShaderDLL
	{
		.ShaderModule = vulkanShader.ShaderModule(),
		.ShaderStages = vulkanShader.ShaderStages(),
		.PushConstant = pushConstantDLL,
		.InputVertexAttributeList = memorySystem.AddPtrBuffer<VkVertexInputAttributeDescription>(vulkanShader.InputVertexAttributeList().data(), vulkanShader.InputVertexAttributeList().size(), __FILE__, __LINE__, __func__),
		.OutputVertexAttributeList = memorySystem.AddPtrBuffer<VkVertexInputAttributeDescription>(vulkanShader.OutputVertexAttributeList().data(), vulkanShader.OutputVertexAttributeList().size(), __FILE__, __LINE__, __func__),
		.VertexInputBindingList = memorySystem.AddPtrBuffer<VkVertexInputBindingDescription>(vulkanShader.VertexInputBindingList().data(), vulkanShader.VertexInputBindingList().size(), __FILE__, __LINE__, __func__),
		.DescriptorBindingList = memorySystem.AddPtrBuffer<ShaderDescriptorBindingDLL>(shaderDescriptorBindingDLLList.data(), shaderDescriptorBindingDLLList.size(), __FILE__, __LINE__, __func__),
		.InputVertexAttributeCount = vulkanShader.InputVertexAttributeList().size(),
		.OutputVertexAttributeCount = vulkanShader.OutputVertexAttributeList().size(),
		.VertexInputBindingCount = vulkanShader.VertexInputBindingList().size(),
		.DescriptorBindingCount = shaderDescriptorBindingDLLList.size()
	};
}

size_t ShaderSystem_GetSpivByteAlignment(ShaderMemberTypeEnum shaderMemberEnum)
{
	return shaderSystem.GetSpivByteAlignment(shaderMemberEnum);
}
