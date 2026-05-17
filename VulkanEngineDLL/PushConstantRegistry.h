#pragma once
#include "Platform.h"
#include "JsonStruct.h"
#include "ShaderSystem.h"

struct PushConstantContext
{
    VkGuid                RenderPassGuid;
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

    using UpdateFunc = std::function<void(ShaderPushConstant& pushConstant, const PushConstantContext& context)>;
	UnorderedMap<String, UpdateFunc> registry;

public:
    DLL_EXPORT void RegisterPushConstantValue(const String& sourceName, UpdateFunc func);
    DLL_EXPORT void ApplyPushConstantRules(ShaderPushConstant& pushConstant, const PushConstantContext& pushConstantContext);
    DLL_EXPORT void RegisterDefaultPushConstantRules();
};
extern DLL_EXPORT PushConstantRegistry& pushConstantRegistry;
inline PushConstantRegistry& PushConstantRegistry::Get()
{
    static PushConstantRegistry instance;
    return instance;
}
