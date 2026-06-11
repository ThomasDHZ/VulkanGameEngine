#include "ComponentSystem.h"

void ComponentFactory::RegisterLoader(uint64_t componentType, std::unique_ptr<IComponentLoader> loader)
{
	m_loaders[componentType] = std::move(loader);
}

void ComponentFactory::Load(entt::registry& registry, entt::entity entity, uint64_t componentType, const nlohmann::json& json)
{
    auto it = m_loaders.find(componentType);
    if (it != m_loaders.end()) it->second->Load(registry, entity, json);
    else std::cerr << "[ComponentFactory] No loader registered for component type: " << componentType << std::endl;
}
