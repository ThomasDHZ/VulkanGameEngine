#include "pch.h"
#include "GameObjectSystem.h"
#include "MegaManShot.h"
#include "MegaManObject.h"

void MegaManShot_Behaviors(GameObjectBehavior& componentBehavior)
{
    componentBehavior.CreateObject = MegaManShot_CreateObject;
    componentBehavior.KeyBoardInput = nullptr;
    componentBehavior.ControllerInput = nullptr;
    componentBehavior.Update = MegaManShot_Update;
    componentBehavior.Destroy = MegaManShot_Destroy;
}

void MegaManShot_CreateObject(const String& name, VkGuid vramId, vec2 objectPosition, uint parentGameObjectId)
{
    const GameObject& gameObject = gameObjectSystem.FindGameObject(parentGameObjectId);
    const Transform2DComponent& transform = gameObjectSystem.FindTransform2DComponent(gameObject.GameObjectId);
    MegaManObject* objectData = static_cast<MegaManObject*>(gameObject.GameObjectData);
    if (objectData->CurrentShotTime >= objectData->CoolDownTime &&
        objectData->CurrentShotCount <= objectData->MaxShotCount)
    {
        GameObject& gameObject = gameObjectSystem.GameObjectList.emplace_back(GameObject
            {
                .GameObjectType = GameObjectTypeEnum::kGameObjectMegaManShot,
                .GameObjectComponentMask = kTransform2DComponent | kSpriteComponent,
                .GameObjectId = gameObjectSystem.GetNextGameObjectIndex(),
                .ParentGameObjectId = parentGameObjectId,
                .GameObjectData = gameObjectSystem.LoadObjectData(GameObjectTypeEnum::kGameObjectMegaManShot)
            });
        gameObjectSystem.LoadComponentTable(gameObject, objectPosition, vramId);
        gameObjectSystem.LoadComponentBehavior(gameObject, GameObjectTypeEnum::kGameObjectMegaManShot);
        objectData->CurrentShotCount += 1;
        objectData->CurrentShotTime = 0.0f;
    }
}

void MegaManShot_Update(uint gameObjectId, const float& deltaTime)
{
   GameObject& gameObject = gameObjectSystem.FindGameObject(gameObjectId);
   if (gameObject.GameObjectType == GameObjectTypeEnum::kGameObjectMegaManShot)
   {
       MegaManShot* objectData = static_cast<MegaManShot*>(gameObject.GameObjectData);
       if (objectData->MegaManShotDistanceTraveled > objectData->MegaManShotDistance)
       {
           gameObject.GameObjectAlive = false;
       }
       else
       {
           objectData->MegaManShotDistanceTraveled += 1;

           Transform2DComponent spriteTransform = gameObjectSystem.FindTransform2DComponent(gameObjectId);
           spriteTransform.GameObjectPosition.x += 1;
       }
   }
}

void MegaManShot_Destroy(uint gameObjectId)
{
    GameObject& gameObject = gameObjectSystem.FindGameObject(gameObjectId);
    GameObject& parentGameObject = gameObjectSystem.FindGameObject(gameObject.ParentGameObjectId);
    MegaManObject* objectData = static_cast<MegaManObject*>(parentGameObject.GameObjectData);
    objectData->CurrentShotCount--;
}

