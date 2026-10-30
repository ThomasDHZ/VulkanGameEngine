#pragma once
#include "DLL.h"
#include <Platform.h>
#include <VulkanShader.h>
#include <VulkanPipeline.h>
#include "JsonStruct.h"
#include "MemorySystem.h"
#include "BufferSystem.h"
#include <cstdlib>
#include "RenderSystem.h"

class ENGINE_DLL_EXPORT ShaderSystem
{
public:
    static ShaderSystem& Get();

private:
    ShaderSystem() = default;
    ~ShaderSystem() = default;
    ShaderSystem(const ShaderSystem&) = delete;
    ShaderSystem& operator=(const ShaderSystem&) = delete;
    ShaderSystem(ShaderSystem&&) = delete;
    ShaderSystem& operator=(ShaderSystem&&) = delete;

public:
    UnorderedMap<VkGuid, VulkanShader>                      RenderShaderMap;
    UnorderedMap<String, ShaderPushConstant>                ShaderPushConstantMap;

    VkGuid                                       LoadShader(ShaderLoader& shaderLoader);
    VulkanShader                                 LoadShader(VkGuid& shaderGuid, const String& fileName);
    bool                                         CompileShaders(const String& fileDirectory, const String& outputDirectory);
    void                                         UpdatePushConstantBuffer(const String& pushConstantName);
    void                                         UpdatePushConstantBuffer(ShaderPushConstant& pushConstantStruct);
    ShaderPushConstant&                          FindShaderPushConstant(const String& pushConstantName);
    ShaderVariable&                              FindShaderPipelineStructVariable(ShaderStruct& shaderStruct, const String& variableName);
    ShaderVariable&                              FindShaderPushConstantStructVariable(ShaderPushConstant& shaderPushConstant, const String& variableName);
    const VulkanShader&                          FindVulkanShader(const VkGuid& shaderGuid);
    bool                                         ShaderPushConstantExists(const String& pushConstantName);
    bool                                         SearchShaderConstantBufferExists(const Vector<ShaderPushConstant>& shaderPushConstantList, const String& constBufferName);
    bool                                         SearchShaderDescriptorBindingExists(const Vector<ShaderDescriptorBinding>& shaderDescriptorBindingList, const String& descriptorBindingName);
    bool                                         SearchShaderPipelineStructExists(const Vector<ShaderStruct>& shaderStructList, const String& structName);
    bool                                         SearchPushConstantForVariableExists(const String& structKey, const String& variableName);
    bool                                         VulkanShaderExists(const VkGuid& vulkanShaderId);
    size_t                                       GetSpivByteAlignment(ShaderMemberTypeEnum shaderMemberEnum);
    size_t                                       GetSpivByteAlignment(const PushConstantVariable& pushConstantVariable);
    void                                         Destroy();

    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };

    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
    void UpdatePushConstantValue(ShaderPushConstant& pushConst, const PushConstantUpdateRule& pushConstantVariable)
    {
        ShaderVariable& variable = FindShaderPushConstantStructVariable(pushConst, pushConstantVariable.Variable);
        std::visit(overloaded{
            [&](const PushConstantConstVariable& c) 
            {
                   variable.Value = c.Value;
            },
            [&](const PushConstantRef& r)
            {
                const Texture& tex = renderSystem.FindRenderPassAttachment(r.AssetId);
                if (r.TextureVar == TextureVarTypeEnum::kVar_TextureSize) variable.Value = tex.texture.m_textureSize.x;
            }
            }, pushConstantVariable.PushConstant);
    }

    template<typename T>
    void UpdatePushConstantValue(ShaderPushConstant& pushConst, const String& valueName, const T& value)
    {
        ShaderVariable& variable = FindShaderPushConstantStructVariable(pushConst, valueName);
        static_assert(std::is_trivially_copyable_v<T>, "Push constant type must be trivially copyable");
        size_t size = std::visit([](auto&& arg) -> size_t
            {
                return sizeof(std::decay_t<decltype(arg)>);
            }, variable.Value);

        if (size != sizeof(T))
        {
            throw std::runtime_error(
                "Push constant size mismatch for '" + valueName + "': "
                "expected " + std::to_string(size) + " bytes, "
                "got " + std::to_string(sizeof(T)) + " bytes (type: " + typeid(T).name() + ")"
            );
        }
        variable.Value = value;
    }
};
ENGINE_DLL_EXPORT extern ShaderSystem& shaderSystem;
inline ShaderSystem& ShaderSystem::Get()
{
    static ShaderSystem instance;
    return instance;
}