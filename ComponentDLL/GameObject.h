#pragma once
#include "pch.h"
#include "Sprite.h"
#include <Transform2DComponent.h>
#include "InputComponent.h"
#include "MegaManShot.h"

enum GameObjectTypeEnum
{
    kGameObjectMegaMan,
    kGameObjectMegaManShot
};

enum ComponentTypeEnum
{
    kUndefined,
    kRenderMesh2DComponent,
    kTransform2DComponent,
    kInputComponent,
    kSpriteComponent,
    kTransform3DComponent,
    kMegaManShotComponent
};

struct GameObject
{
    GameObjectID GameObjectId;
    GameObjectID ParentGameObjectId;
    GameObjectTypeEnum GameObjectType;
};

struct GameObjectArchive
{
    UnorderedMap<GameObjectID, GameObject> GameObjectMap;
    UnorderedMap<GameObjectTypeEnum, GameObjectBehavior> ComponentBehaviorMap;

    UnorderedMap<GameObjectID, InputComponent> InputComponentMap;
    UnorderedMap<GameObjectID, Transform2DComponent> Transform2DComponentMap;
};
DLL_EXPORT GameObjectArchive gameObjectArchive;

DLL_EXPORT void GameObject_CreateGameObject(const String& gameObjectPath, const vec2& gameObjectPosition);
DLL_EXPORT void GameObject_CreateGameObject(const String& name, GameObjectTypeEnum objectEnum, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, VkGuid vramId, vec2 objectPosition);
DLL_EXPORT void GameObject_CreateGameObject(const String& name, GameObjectID parentGameObjectId, GameObjectTypeEnum objectEnum, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, VkGuid vramId, vec2 objectPosition);

DLL_EXPORT void GameObject_Update(const float deltaTime);

DLL_EXPORT void GameObject_LoadComponentBehavior(GameObjectID gameObjectId, GameObjectTypeEnum objectEnum);
DLL_EXPORT void GameObject_LoadTransformComponent(const nlohmann::json& json, GameObjectID id, const vec2& gameObjectPosition);
DLL_EXPORT void GameObject_LoadInputComponent(const nlohmann::json& json, GameObjectID id);
DLL_EXPORT void GameObject_LoadSpriteComponent(const nlohmann::json& json, GameObjectID id);
DLL_EXPORT void GameObject_LoadMegaManShotComponent(GameObjectID id, const vec2& gameObjectPosition);

DLL_EXPORT const GameObject& GameObject_FindGameObject(const GameObjectID& id);
DLL_EXPORT const GameObjectBehavior& GameObject_FindGameObjectBehavior(const GameObjectTypeEnum& id);
DLL_EXPORT Transform2DComponent& GameObject_FindTransform2DComponent(const GameObjectID& id);
DLL_EXPORT const InputComponent& GameObject_FindInputComponent(const GameObjectID& id);

DLL_EXPORT const bool GameObjectExists(const GameObjectID& id);
DLL_EXPORT const bool GameObjectBehaviorExists(const GameObjectTypeEnum objectEnum);

DLL_EXPORT GameObject& GameObject_FindGameObjectById(const GameObjectID& id);
DLL_EXPORT Vector<std::reference_wrapper<GameObject>> GameObject_FindGameObjectByType(const GameObjectTypeEnum& gameObjectType);

DLL_EXPORT const Vector<GameObject> GameObject_GameObjectList();
DLL_EXPORT const Vector<Transform2DComponent> GameObject_Transform2DComponentList();
DLL_EXPORT const Vector<InputComponent> GameObject_InputComponentList();

DLL_EXPORT void GameObject_DestroyGameObject(const GameObjectID& gameObjectId);
DLL_EXPORT void GameObject_DestroyGameObjects();

void GameObject_LoadComponentTable(GameObjectID gameObjectId, const Vector<ComponentTypeEnum>& gameObjectComponentTypeList, vec2& objectPosition, VkGuid& vramId);