#include "CustomDrawCommandRegistry.h"
#include "RenderSystem.h"
#include "LightSystem.h"

CustomDrawCommandRegistry& customDrawCommandRegistry = CustomDrawCommandRegistry::Get();

void CustomDrawCommandRegistry::RegisterCustomDrawCommand(const String& sourceName, UpdateFunc func)
{
	_registry[sourceName] = std::move(func);
}

void CustomDrawCommandRegistry::ApplyCustomDrawCommand(const String& sourceName, VkCommandBuffer& commandBuffer, MeshDrawMessage& drawMeshMessage, const CustomDrawInitContext& context, VulkanDrawMessage& message, uint meshDrawIndex, uint mip, uint mipCount)
{
    auto it = _registry.find(sourceName);
    if (it != _registry.end())
    {
        it->second(commandBuffer, drawMeshMessage, context, message, meshDrawIndex, mip, mipCount);
    }
}