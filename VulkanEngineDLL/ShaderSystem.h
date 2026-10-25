#pragma once
#include "DLL.h"
#include <Platform.h>
#include <VulkanShader.h>
#include <VulkanPipeline.h>
#include "JsonStruct.h"
#include "MemorySystem.h"
#include "BufferSystem.h"
#include <cstdlib>

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
    void                                         Destroy();

    void UpdatePushConstantValue(ShaderPushConstant& pushConst, const PushConstantUpdateRule& pushConstantVariable)
    {
        ShaderVariable& variable = FindShaderPushConstantStructVariable(pushConst, pushConstantVariable.Variable);
        std::visit([&](auto&& value)
            {
                using T = std::decay_t<decltype(value)>;
                static_assert(std::is_trivially_copyable_v<T>);

                if (variable.Value.size() != sizeof(T)) throw std::runtime_error("size mismatch");

                std::memcpy(variable.Value.data(), &value, sizeof(T));
            }, pushConstantVariable.Value);
        //ShaderVariable variable;
        //if (pushConstantVariable.PushConstantVarType == PushConstantVarTypeEnum::kVar_PushConstant) variable = FindShaderPushConstantStructVariable(pushConst, pushConstantVariable.PushConstantVar.value().Variable);
        //if (pushConstantVariable.PushConstantVarType == PushConstantVarTypeEnum::kVar_TextureRefrence)
        //{

        //    const Texture& texture = renderSystem.FindRenderPassAttachment(pushConstantVariable.PushConstantRef.value().AssetId);
        //    variable = ShaderVariable
        //    {
        //        .Name = pushConstantVariable.PushConstantRef.value().Variable,
        //        .Size = sizeof(VulkanTexture::m_textureSize),
        //        .Offset = offsetof(VulkanTexture, m_textureSize)
        //    };
        //}
        //std::visit([&](auto&& value) 
        //    {
        //        using T = std::decay_t<decltype(value)>;
        //        static_assert(std::is_trivially_copyable_v<T>);

        //        if (variable.Value.size() != sizeof(T)) throw std::runtime_error("size mismatch");

        //        std::memcpy(variable.Value.data(), &value, sizeof(T));
        //    }, variable);
    }

    template<typename T>
    void UpdatePushConstantValue(ShaderPushConstant& pushConst, const String& valueName, const T& value)
    {
        ShaderVariable& variable = FindShaderPushConstantStructVariable(pushConst, valueName);
        static_assert(std::is_trivially_copyable_v<T>, "Push constant type must be trivially copyable");
        if (variable.Value.size() != sizeof(T))
        {
            throw std::runtime_error(
                "Push constant size mismatch for '" + valueName + "': "
                "expected " + std::to_string(variable.Value.size()) + " bytes, "
                "got " + std::to_string(sizeof(T)) + " bytes (type: " + typeid(T).name() + ")"
            );
        }
        std::memcpy(variable.Value.data(), &value, variable.Value.size());
    }

    //template <typename T>
    //T GetPushConstantValue(const PushConstantUpdateRule& pushConstantVariable)
    //{
    //    const auto& values = pushConstantVariable.Value;
    //    auto as_int = [&](std::size_t i) { return std::stoi(values.at(i)); };
    //    auto as_float = [&](std::size_t i) { return std::stof(values.at(i)); };

    //    if constexpr (std::is_same_v<T, int>) return as_int(0);
    //    else if constexpr (std::is_same_v<T, uint>) return static_cast<uint>(std::stoul(values.at(0)));
    //    else if constexpr (std::is_same_v<T, float>) return as_float(0);
    //    else if constexpr (std::is_same_v<T, bool>) return as_int(0) != 0;
    //    else if constexpr (std::is_same_v<T, glm::ivec2>)
    //    {
    //        glm::ivec2 v{};
    //        const int n = std::min<int>(static_cast<int>(values.size()), 2);
    //        for (int x = 0; x < n; ++x) v[x] = as_int(x);
    //        return v;
    //    }
    //    else if constexpr (std::is_same_v<T, glm::ivec3>)
    //    {
    //        glm::ivec3 v{};
    //        const int n = std::min<int>(static_cast<int>(values.size()), 3);
    //        for (int x = 0; x < n; ++x) v[x] = as_int(x);
    //        return v;
    //    }
    //    else if constexpr (std::is_same_v<T, glm::ivec4>)
    //    {
    //        glm::ivec4 v{};
    //        const int n = std::min<int>(static_cast<int>(values.size()), 4);
    //        for (int x = 0; x < n; ++x) v[x] = as_int(x);
    //        return v;
    //    }
    //    else if constexpr (std::is_same_v<T, glm::vec2>)
    //    {
    //        glm::vec2 v{};
    //        const int n = std::min<int>(static_cast<int>(values.size()), 2);
    //        for (int x = 0; x < n; ++x) v[x] = as_float(x);
    //        return v;
    //    }
    //    else if constexpr (std::is_same_v<T, glm::vec3>)
    //    {
    //        glm::vec3 v{};
    //        const int n = std::min<int>(static_cast<int>(values.size()), 3);
    //        for (int x = 0; x < n; ++x) v[x] = as_float(x);
    //        return v;
    //    }
    //    else if constexpr (std::is_same_v<T, glm::vec4>)
    //    {
    //        glm::vec4 v{};
    //        const int n = std::min<int>(static_cast<int>(values.size()), 4);
    //        for (int x = 0; x < n; ++x) v[x] = as_float(x);
    //        return v;
    //    }
    //    else if constexpr (std::is_same_v<T, glm::mat2>)
    //    {
    //        glm::mat2 v{ 0.0f };
    //        const int n = std::min<int>(static_cast<int>(values.size()), 4);
    //        for (int x = 0; x < n; ++x) v[x / 2][x % 2] = as_float(x);
    //        return v;
    //    }
    //    else if constexpr (std::is_same_v<T, glm::mat3>)
    //    {
    //        glm::mat3 v{ 0.0f };
    //        const int n = std::min<int>(static_cast<int>(values.size()), 9);
    //        for (int x = 0; x < n; ++x) v[x / 3][x % 3] = as_float(x);
    //        return v;
    //    }
    //    else if constexpr (std::is_same_v<T, glm::mat4>)
    //    {
    //        glm::mat4 v{ 0.0f };
    //        const int n = std::min<int>(static_cast<int>(values.size()), 16);
    //        for (int x = 0; x < n; ++x) v[x / 4][x % 4] = as_float(x);
    //        return v;
    //    }
    //    else static_assert(sizeof(T) == 0, "unsupported push-constant type");
    //}
};
ENGINE_DLL_EXPORT extern ShaderSystem& shaderSystem;
inline ShaderSystem& ShaderSystem::Get()
{
    static ShaderSystem instance;
    return instance;
}