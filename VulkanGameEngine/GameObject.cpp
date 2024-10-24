#include "GameObject.h"
#include <Macro.h>
#include <iostream>
#include "MemoryPoolManager.h"
#include "RenderMesh2DComponent.h"

GameObject::GameObject()
{
}

GameObject::GameObject(String name)
{
	Name = name;
}

GameObject::GameObject(String name, List<std::shared_ptr<GameObjectComponent>> gameObjectComponentList)
{
	Name = name;
	GameObjectComponentList = gameObjectComponentList;
}

std::shared_ptr<GameObject> GameObject::CreateGameObject(String name)
{
	std::shared_ptr<GameObject> gameObject = MemoryPoolManager::AllocateNewGameObject();
	new (gameObject.get()) GameObject(name);
	return gameObject;
}

std::shared_ptr<GameObject> GameObject::CreateGameObject(String name, List<ComponentTypeEnum> gameObjectComponentList)
{
	std::shared_ptr<GameObject> gameObject = MemoryPoolManager::AllocateNewGameObject();

	List<std::shared_ptr<GameObjectComponent>> conponentList;
	conponentList.emplace_back(RenderMesh2DComponent::CreateRenderMesh2DComponent("Mesh Renderer"));
	
	new (gameObject.get()) GameObject(name, conponentList);
	return gameObject;
}

void GameObject::Update(float deltaTime)
{
	for (std::shared_ptr<GameObjectComponent> component : GameObjectComponentList)
	{
		component->Update(deltaTime);
	}
}

void GameObject::BufferUpdate(VkCommandBuffer& commandBuffer, float deltaTime)
{
	for (std::shared_ptr<GameObjectComponent> component : GameObjectComponentList)
	{
		component->Update(commandBuffer, deltaTime);
	}
}

void GameObject::Draw(VkCommandBuffer& commandBuffer, VkPipeline& pipeline, VkPipelineLayout& shaderPipelineLayout, VkDescriptorSet& descriptorSet, SceneDataBuffer& sceneProperties)
{
	for (std::shared_ptr<GameObjectComponent> component : GameObjectComponentList)
	{
		component->Draw(commandBuffer, pipeline, shaderPipelineLayout, descriptorSet, sceneProperties);
	}
}

void GameObject::Destroy()
{
	for (std::shared_ptr<GameObjectComponent> component : GameObjectComponentList)
	{
		component->Destroy();
	}
}

void GameObject::AddComponent(std::shared_ptr<GameObjectComponent> newComponent)
{
	GameObjectComponentList.emplace_back(newComponent);
}

void GameObject::RemoveComponent(size_t index)
{
	GameObjectComponentList.erase(GameObjectComponentList.begin() + index);
}

std::shared_ptr<GameObjectComponent> GameObject::GetComponentByName(const std::string& name)
{
	auto component = std::find_if(GameObjectComponentList.begin(), GameObjectComponentList.end(),
		[&name](const std::shared_ptr<GameObjectComponent>& component) {
			return component->Name == name;
		});

	if (component != GameObjectComponentList.end())
	{
		return *component;
	}

	return nullptr;
}

std::shared_ptr<GameObjectComponent> GameObject::GetComponentByComponentType(ComponentTypeEnum type)
{
	auto component = std::find_if(GameObjectComponentList.begin(), GameObjectComponentList.end(),
		[&type](const std::shared_ptr<GameObjectComponent>& component)
		{
			return component->ComponentType == type;
		});

	if (component != GameObjectComponentList.end())
	{
		return *component;
	}

	return nullptr;
}