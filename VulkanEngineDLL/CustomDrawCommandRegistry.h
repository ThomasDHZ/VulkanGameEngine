#pragma once
#include "DLL.h"
#include <Platform.h>
#include "JsonStruct.h"
#include "GameObjectSystem.h"
#include <functional>
#include "RenderSystem.h"

class CustomDrawCommandRegistry
{
public:
    static CustomDrawCommandRegistry& Get();
    using UpdateFunc = std::function<void(VkCommandBuffer& commandBuffer, MeshDrawMessage& drawMeshMessage, const CustomDrawInitContext& context, VulkanDrawMessage& message, uint meshDrawIndex, uint mip, uint mipCount)>;

private:
    CustomDrawCommandRegistry() = default;
    CustomDrawCommandRegistry(const CustomDrawCommandRegistry&) = delete;
    CustomDrawCommandRegistry& operator=(const CustomDrawCommandRegistry&) = delete;
    CustomDrawCommandRegistry(CustomDrawCommandRegistry&&) = delete;
    CustomDrawCommandRegistry& operator=(CustomDrawCommandRegistry&&) = delete;

    UnorderedMap<String, UpdateFunc> _registry;

public:
    void RegisterCustomDrawCommand(const String& sourceName, UpdateFunc func);
    void ApplyCustomDrawCommand(const String& sourceName, VkCommandBuffer& commandBuffer, MeshDrawMessage& drawMeshMessage, const CustomDrawInitContext& context, VulkanDrawMessage& message, uint meshDrawIndex, uint mip, uint mipCount);
    void RegisterDefaultCustomDrawCommand();
};
    ENGINE_DLL_EXPORT extern CustomDrawCommandRegistry& customDrawCommandRegistry;
    inline CustomDrawCommandRegistry& CustomDrawCommandRegistry::Get()
    {
        static CustomDrawCommandRegistry instance;
        return instance;
    }

