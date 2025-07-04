#include "GameObjectSystem.h"
#include "LevelSystem.h"
#include "SpriteSystem.h"

GameObjectSystem gameObjectSystem = GameObjectSystem();

GameObjectSystem::GameObjectSystem()
{
}

GameObjectSystem::~GameObjectSystem()
{
}

void GameObjectSystem::CreateGameObject(const String& name, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, VkGuid vramId, vec2 objectPosition)
{
	GameObjectID id;
	id.id = GameObjectMap.size() + 1;
	GameObjectMap[id] = GameObject(id);

	for (auto component : gameObjectComponentTypeList)
	{
		switch (component)
		{
			case kTransform2DComponent: Transform2DComponentMap[id] = Transform2DComponent(objectPosition); break;
			case kInputComponent: InputComponentMap[id] = InputComponent(id); break;
			case kSpriteComponent: spriteSystem.AddSprite(id, vramId); break;
		}
	}
}

void GameObjectSystem::CreateGameObject(const String& gameObjectPath, const vec2& gameObjectPosition)
{
	GameObjectID id;
	id.id = GameObjectMap.size() + 1;
	GameObjectMap[id] = GameObject(id);

	nlohmann::json json = Json::ReadJson(gameObjectPath);
	for (int x = 0; x < json["GameObjectComponentList"].size(); x++)
	{
		const int componentType = json["GameObjectComponentList"][x]["ComponentType"];
		switch (componentType)
		{
			case kTransform2DComponent:  LoadTransformComponent(json["GameObjectComponentList"][x], id, gameObjectPosition); break;
			case kInputComponent: LoadInputComponent(json["GameObjectComponentList"][x], id); break;
			case kSpriteComponent: LoadSpriteComponent(json["GameObjectComponentList"][x], id); break;
		}
	}
}

void GameObjectSystem::LoadTransformComponent(const nlohmann::json& json, GameObjectID id, const vec2& gameObjectPosition)
{
	Transform2DComponent transform2D;
	transform2D.GameObjectPosition = gameObjectPosition;
	transform2D.GameObjectRotation = vec2{ json["GameObjectRotation"][0], json["GameObjectRotation"][1] };
	transform2D.GameObjectScale = vec2{ json["GameObjectScale"][0], json["GameObjectScale"][1] };
	Transform2DComponentMap[id] = transform2D;
}

void GameObjectSystem::LoadInputComponent(const nlohmann::json& json, GameObjectID id)
{
	InputComponentMap[id] = InputComponent();
}

void GameObjectSystem::LoadSpriteComponent(const nlohmann::json& json, GameObjectID id)
{
	VkGuid vramId = VkGuid(json["VramId"].get<String>().c_str());
	spriteSystem.AddSprite(id, vramId);
}

const GameObject& GameObjectSystem::FindGameObject(const GameObjectID& id)
{
	auto it = GameObjectMap.find(id);
	if (it != GameObjectMap.end())
	{
		return it->second;
	}
	throw std::out_of_range("GameObject not found for given GUID");
}

Transform2DComponent& GameObjectSystem::FindTransform2DComponent(const GameObjectID& id)
{
	auto it = Transform2DComponentMap.find(id);
	if (it != Transform2DComponentMap.end())
	{
		return it->second;
	}
	throw std::out_of_range("Transform2DComponent not found for given GUID");
}

const InputComponent& GameObjectSystem::FindInputComponent(const GameObjectID& id)
{
	auto it = InputComponentMap.find(id);
	if (it != InputComponentMap.end())
	{
		return it->second;
	}
	throw std::out_of_range("InputComponent not found for given GUID");
}

const Vector<GameObject> GameObjectSystem::GameObjectList()
{
	Vector<GameObject> GameObjectList;
	for (const auto& gameObjectMap : GameObjectMap)
	{
		GameObjectList.emplace_back(gameObjectMap.second);
	}
	return GameObjectList;
}

const Vector<Transform2DComponent> GameObjectSystem::Transform2DComponentList()
{
	Vector<Transform2DComponent> transform2DComponentList;
	for (const auto& transform2DComponent : Transform2DComponentMap)
	{
		transform2DComponentList.emplace_back(transform2DComponent.second);
	}
	return transform2DComponentList;
}

const Vector<InputComponent> GameObjectSystem::InputComponentList()
{
	Vector<InputComponent> inputComponentList;
	for (const auto& inputComponentMap : InputComponentMap)
	{
		inputComponentList.emplace_back(inputComponentMap.second);
	}
	return inputComponentList;
}

void GameObjectSystem::DestroyGameObject(const GameObjectID& gameObjectId)
{
//	GameObjectMap.erase(gameObjectId);
	Transform2DComponentMap.erase(gameObjectId);
	InputComponentMap.erase(gameObjectId);
}

void GameObjectSystem::DestroyGameObjects()
{
	for (auto& gameObject : GameObjectMap)
	{
		DestroyGameObject(gameObject.second.GameObjectId);
	}
}