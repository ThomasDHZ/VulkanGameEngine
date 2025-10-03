#pragma once
#include <DLL.h>
#include "InputComponent.h"

enum ComponentTypeEnum
{
    kUndefined,
    kRenderMesh2DComponent,
    kTransform2DComponent,
    kInputComponent,
    kSpriteComponent,
    kTransform3DComponent
};


struct GameObject;
struct GameObjectArchive
{
    UnorderedMap<GameObjectID, GameObject> GameObjectMap;
    UnorderedMap<GameObjectID, InputComponent> InputComponentMap;
    UnorderedMap<GameObjectID, ComponentBehavior> ComponentBehaviorMap;
    UnorderedMap<GameObjectID, Transform2DComponent> Transform2DComponentMap;
};
DLL_EXPORT GameObjectArchive gameObjectArchive;

struct GameObject
{
    GameObjectID GameObjectId;

    GameObject()
    {

    }

    GameObject(GameObjectID gameObjectId)
    {
        GameObjectId = gameObjectId;
    }
};

DLL_EXPORT void GameObject_CreateGameObject(const String& name, ObjectEnum objectEnum, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, VkGuid vramId, vec2 objectPosition);
DLL_EXPORT void GameObject_CreateGameObject(const String& gameObjectPath, const vec2& gameObjectPosition);

DLL_EXPORT void GameObject_LoadTransformComponent(const nlohmann::json& json, GameObjectID id, const vec2& gameObjectPosition);
DLL_EXPORT void GameObject_LoadInputComponent(const nlohmann::json& json, GameObjectID id);
DLL_EXPORT void GameObject_LoadSpriteComponent(const nlohmann::json& json, GameObjectID id);

DLL_EXPORT const GameObject& GameObject_FindGameObject(const GameObjectID& id);
DLL_EXPORT Transform2DComponent& GameObject_FindTransform2DComponent(const GameObjectID& id);
DLL_EXPORT const InputComponent& GameObject_FindInputComponent(const GameObjectID& id);

DLL_EXPORT const Vector<GameObject> GameObject_GameObjectList();
DLL_EXPORT const Vector<Transform2DComponent> GameObject_Transform2DComponentList();
DLL_EXPORT const Vector<InputComponent> GameObject_InputComponentList();

DLL_EXPORT void GameObject_DestroyGameObject(const GameObjectID& gameObjectId);
DLL_EXPORT void GameObject_DestroyGameObjects();