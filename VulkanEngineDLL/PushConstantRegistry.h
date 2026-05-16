#pragma once
#include "Platform.h"
#include "JsonStruct.h"
#include "ShaderSystem.h"

struct PushConstantContext
{
    uint32                MeshId = UINT32_MAX;
    uint32                DrawIndex = 0;
    uint32                MipLevel = 0;
    uint32                MipCount = 0;
    ivec2                 RenderPassResolution = { 0, 0 };     // base resolution before mip
};

class PushConstantRegistry
{
public: 
    static PushConstantRegistry& Get();

private: 
    PushConstantRegistry() = default;
    ~PushConstantRegistry() = default;
    PushConstantRegistry(const PushConstantRegistry&) = delete;
    PushConstantRegistry& operator=(const PushConstantRegistry&) = delete;
    PushConstantRegistry(PushConstantRegistry&&) = delete;
    PushConstantRegistry& operator=(PushConstantRegistry&&) = delete;

    using UpdateFunc = std::function<void(ShaderPushConstant& pushConstant, const PushConstantContext& context, const String& shaderVariable)>;
	UnorderedMap<PushConstantResolverEnum, UpdateFunc> registry;

    void HandleSimplePushConstant(ShaderPushConstant& pushConstant, const PushConstantUpdateRule& pushConstantRuleList);

    template<typename T>
    void UpdatePushConstantScalar(ShaderPushConstant& pushConstant, const PushConstantUpdateRule& rule)
    {
        T value{};
        std::istringstream iss(rule.Value.c_str());
        if (iss >> value)
        {
            shaderSystem.UpdatePushConstantValue<T>(pushConstant, rule.Variable, value);
        }
        else
        {
          //  std::cerr << "Failed to parse scalar value: " << rule.Value << std::endl;
        }
    }

    template<int N, typename T>
    void UpdatePushConstantVector(ShaderPushConstant& pushConstant, const PushConstantUpdateRule& pushConstantRule)
    {
        glm::vec<N, T> vector{};
        std::istringstream iss(pushConstantRule.Value.c_str());
        for (int x = 0; x < N; ++x)
        {
            iss >> vector[x];
            if (iss.fail())
            {
            //    std::cerr << "Failed to parse " << N << "D vector: " << pushConstantRule.Value << "\n";
                return;
            }
            if (iss.peek() == ',') iss.ignore();
        }
        shaderSystem.UpdatePushConstantValue<glm::vec<N, T>>(pushConstant, pushConstantRule.Variable, vector);
    }

    template<int C, int R, typename T>
    void UpdatePushConstantMatrix(ShaderPushConstant& pushConstant, const PushConstantUpdateRule& pushConstantRule)
    {
        glm::mat<C, R, T> matrix{};
        std::istringstream iss(pushConstantRule.Value.c_str());
        for (int x = 0; x < C * R; ++x)
        {
            iss >> matrix[x / C][x % C];
            if (iss.fail()) 
            {
             //   std::cerr << "Failed to parse " << C << "x" << R << " matrix\n";
                return;
            }
            if (iss.peek() == ',') iss.ignore();
        }
        shaderSystem.UpdatePushConstantValue<glm::mat<C, R, T>>(pushConstant, pushConstantRule.Variable, matrix);
    }

public:
    DLL_EXPORT void RegisterPushConstantValue(const PushConstantResolverEnum& sourceName, UpdateFunc func);
    DLL_EXPORT void ApplyPushConstantRules(ShaderPushConstant& pushConstant, const PushConstantContext& pushConstantContext, const Vector<PushConstantUpdateRule>& pushConstantRuleList);
    DLL_EXPORT void RegisterDefaultPushConstantRules();
};
extern DLL_EXPORT PushConstantRegistry& pushConstantRegistry;
inline PushConstantRegistry& PushConstantRegistry::Get()
{
    static PushConstantRegistry instance;
    return instance;
}
