#pragma once
#include "VulkanWindow.h"
#include <VkGuid.h>
#include <ComponentBehavior.h>
#include "Vertex.h"
#include "InputComponent.h"
#include "Transform2DComponent.h"
#include "GameObject.h"
#include "SceneDataBuffer.h"
#include <Level2D.h>

class GameObjectSystem
{
private:
    UnorderedMap<GameObjectID, GameObject> GameObjectMap;
    UnorderedMap<GameObjectID, Transform2DComponent> Transform2DComponentMap;
    UnorderedMap<GameObjectID, InputComponent> InputComponentMap;

    void LoadComponentBehavior(GameObjectID gameObjectId, ObjectEnum objectEnum);

public:
    UnorderedMap<GameObjectID, ComponentBehavior> ComponentBehaviorMap;
    Vector<Vertex2D> SpriteVertexList =
    {
        Vertex2D(vec2(0.0f, 1.0f), vec2(0.0f, 0.0f)),
        Vertex2D(vec2(1.0f, 1.0f), vec2(1.0f, 0.0f)),
        Vertex2D(vec2(1.0f, 0.0f), vec2(1.0f, 1.0f)),
        Vertex2D(vec2(0.0f, 0.0f), vec2(0.0f, 1.0f))
    };

    Vector<uint32> SpriteIndexList =
    {
        0, 3, 1,
        1, 3, 2
    };

    GameObjectSystem();
    ~GameObjectSystem();

    void CreateGameObject(const String& name, ObjectEnum objectEnum, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, VkGuid vramId, vec2 objectPosition);
    void CreateGameObject(const String& gameObjectPath, const vec2& gameObjectPosition);

    void LoadTransformComponent(const nlohmann::json& json, GameObjectID id, const vec2& gameObjectPosition);
    void LoadInputComponent(const nlohmann::json& json, GameObjectID id);
    void LoadSpriteComponent(const nlohmann::json& json, GameObjectID id);

    const GameObject& FindGameObject(const GameObjectID& id);
    Transform2DComponent& FindTransform2DComponent(const GameObjectID& id);
    const InputComponent& FindInputComponent(const GameObjectID& id);

    const Vector<GameObject> GameObjectList();
    const Vector<Transform2DComponent> Transform2DComponentList();
    const Vector<InputComponent> InputComponentList();

    void DestroyGameObject(const GameObjectID& gameObjectId);
    void DestroyGameObjects();
};
extern GameObjectSystem gameObjectSystem;