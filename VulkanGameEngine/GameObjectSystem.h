#pragma once
#include "VulkanWindow.h"
#include <VkGuid.h>
#include "Vertex.h"
#include "InputComponent.h"
#include "Transform2DComponent.h"
#include "GameObject.h"
#include <Level2D.h>

class GameObjectSystem
{
private:

public:

    GameObjectSystem();
    ~GameObjectSystem();

    void CreateGameObject(const String& name, uint parentGameObjectId, GameObjectTypeEnum objectEnum, uint64 gameObjectComponentMask, VkGuid vramId, vec2 objectPosition);
    void CreateGameObject(const String& gameObjectPath, const vec2& gameObjectPosition);
    void Update(const float deltaTime);
    void LoadTransformComponent(const nlohmann::json& json, uint gameObjectId, const vec2& gameObjectPosition);
    void LoadInputComponent(const nlohmann::json& json, uint gameObjectId);
    void LoadSpriteComponent(const nlohmann::json& json, GameObject& gameObject);

    const GameObject& FindGameObject(uint gameObjectId);
    Transform2DComponent& FindTransform2DComponent(uint gameObjectId);
    const InputComponent& FindInputComponent(uint gameObjectId);

    const Vector<GameObject> GameObjectList();
    const Vector<Transform2DComponent> Transform2DComponentList();
    const Vector<InputComponent> InputComponentList();

    void DestroyGameObject(uint gameObjectId);
    void DestroyGameObjects();
};
extern GameObjectSystem gameObjectSystem;